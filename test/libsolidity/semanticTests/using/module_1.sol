==== Source: A ====
function id(uint x) pure returns (uint) {
    return x;
}

==== Source: B ====
import "A" as M;
contract C {
    using M for uint;
	function f(uint x) public returns (uint) {
        return x.id();
    }
}
// ====
// compileViaYul: also
// ----
// f(uint256): 5 -> 5
// f(uint256): 10 -> 10
