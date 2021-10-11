function id(uint x) pure returns (uint) {
    return x;
}

function zero(address) pure returns (address) {
    return address(0);
}

contract C {
    using * for *;
    function f(uint x) pure external returns (uint) {
        return x.id();
    }
    function g(address a) pure external returns (address) {
        return a.zero();
    }
}
// ----
// TypeError 1308: (150-164): "using * for T;" cannot be used inside contract definition
