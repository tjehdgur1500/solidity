// SPDX-License-Identifier: GPL-3.0
#include <test/libsolidity/LSPTest.h>
#include <test/Common.h>

#include <libsolidity/lsp/LanguageServer.h>

#include <libsolutil/JSON.h>
#include <libsolutil/CommonIO.h>

#include <boost/filesystem.hpp>

#include <memory>
#include <vector>

using namespace std;
namespace fs = boost::filesystem;

namespace solidity::frontend::test
{

namespace
{

class MockTransport: public lsp::Transport
{
public:
	std::function<void()> onClose = [](){};

	explicit MockTransport(std::vector<Json::Value> _requests):
		m_requests{std::move(_requests)}
	{}

	std::vector<Json::Value> const& requests() const noexcept { return m_replies; }
	std::vector<Json::Value> const& replies() const noexcept { return m_replies; }

	bool closed() const noexcept override
	{
		return m_readOffset >= m_requests.size();
	}

	std::optional<Json::Value> receive() override
	{
		if (m_readOffset < m_requests.size())
		{
			Json::Value value = m_requests.at(m_readOffset++);
			if (m_readOffset == m_requests.size())
				m_terminate();
			return value;
		}
		return nullopt;
	}

	void notify(std::string const& _method, Json::Value const& _params) override
	{
		Json::Value json;
		json["method"] = _method;
		json["params"] = _params;
		send(json);
	}

	void reply(lsp::MessageID _id, Json::Value const& _message) override
	{
		Json::Value json;
		json["result"] = _message;
		send(json, _id);
	}

	void error(lsp::MessageID _id, lsp::ErrorCode _code, std::string const& _message) override
	{
		Json::Value json;
		json["error"]["code"] = static_cast<int>(_code);
		json["error"]["message"] = _message;
		send(json, _id);
	}

private:
	void send(Json::Value _json, lsp::MessageID _id = Json::nullValue)
	{
		_json["jsonrpc"] = "2.0";
		if (!_id.isNull())
			_json["id"] = _id;

		m_replies.push_back(_json);
	}

private:
	std::vector<Json::Value> m_requests;
	size_t m_readOffset = 0;
	std::vector<Json::Value> m_replies;
	std::function<void()> m_terminate;
};

} // end anonymous namespace

LSPTest::LSPTest(fs::path _path): m_path{std::move(_path)}
{
}

LSPTest::TestResult LSPTest::run(std::ostream& _output)
{
	fmt::print("\nLSPTest.run! {}\n", m_path.string());
	auto const fileContents = util::readFileAsString(m_path);
	std::string errorString;
	Json::Value json;
	if (!util::jsonParseStrict(fileContents, json, &errorString))
	{
		_output << errorString;
		return TestResult::FatalError;
	}
	if (!json.isArray())
	{
		_output << "JSON format error. Top level element must be an array.\n";
		return TestResult::FatalError;
	}

	std::vector<Json::Value> requests;
	std::vector<Json::Value> expectedReplies;

	for (int i = 0; i < static_cast<int>(json.size()); ++i)
	{
		requests.push_back(json[i]["request"]);

		Json::Value output = json[i]["response"];
		if (output.isArray())
		{
			for (size_t k = 0; k < output.size(); ++k)
			{
				expectedReplies.push_back(output[i]);
			}
		}
	}

	lsp::LanguageServer lsp(
		[&](auto message) { _output << message << '\n'; },
		make_unique<MockTransport>(requests)
	);
	static_cast<MockTransport&>(lsp.transport()).onClose = [&]() { lsp.terminate(); };

	return TestResult::Success; // TBD
}

void lspTestCase(fs::path _testCaseFile)
{
	try
	{
		// TODO
		auto testCase = std::make_unique<LSPTest>(_testCaseFile);
		stringstream errorStream;
		switch (testCase->run(errorStream))
		{
		case TestCase::TestResult::Success:
			break;
		case TestCase::TestResult::Failure:
			BOOST_ERROR("Test expectation mismatch.\n" + errorStream.str());
			break;
		case TestCase::TestResult::FatalError:
			BOOST_ERROR("Fatal error during test.\n" + errorStream.str());
			break;
		}
	}
	catch (boost::exception const& _e)
	{
		BOOST_ERROR("Exception during extracted test: " << boost::diagnostic_information(_e));
	}
	catch (std::exception const& _e)
	{
		BOOST_ERROR("Exception during extracted test: " << boost::diagnostic_information(_e));
	}
	catch (...)
	{
		BOOST_ERROR("Unknown exception during extracted test: " << boost::current_exception_diagnostic_information());
	}
}

int LSPTest::registerTestCases(boost::unit_test::test_suite& _suite)
{
	auto const& options = solidity::test::CommonOptions::get();
	return registerTestCases(_suite, options.testPath, fs::path("libsolidity") / "lsp");
}

int LSPTest::registerTestCases(
	boost::unit_test::test_suite& _suite,
	fs::path const& _basePath,
	fs::path const& _path
)
{
	fs::path const fullPath = _basePath / _path;
	int numTestsAdded = 0;

	if (fs::is_directory(fullPath))
	{
		boost::unit_test::test_suite* subTestSuite = BOOST_TEST_SUITE(_path.filename().string());
		for (auto const& entry: boost::iterator_range<fs::directory_iterator>(
			fs::directory_iterator(fullPath),
			fs::directory_iterator()
		))
		{
			auto const pathToChild = entry.path();
			if ((fs::is_regular_file(pathToChild) && pathToChild.extension().string() == ".json") ||
				fs::is_directory(pathToChild)
			)
			{
				numTestsAdded += registerTestCases(
					*subTestSuite,
					_basePath,
					_path / entry.path().filename()
				);
			}
		}
		_suite.add(subTestSuite);
	}
	else
	{
		// This must be a vector of unique_ptrs because Boost.Test keeps the equivalent of a string_view to the filename
		// that is passed in. If the strings were stored directly in the vector, pointers/references to them would be
		// invalidated on reallocation.
		static vector<unique_ptr<string const>> filenames;
		filenames.emplace_back(make_unique<string>(_path.string()));
		auto const testCaseName = (_path.parent_path() / _path.stem()).string(); // strip off file extension
		auto testCase = boost::unit_test::make_test_case(
			[fullPath] { BOOST_REQUIRE_NO_THROW({ lspTestCase(fullPath); }); },
			_path.stem().string(), // test-case name
			*filenames.back(),     // test-case file name
			0                      // test-case line number
		);
		_suite.add(testCase);
	}
	return numTestsAdded;
}

}
