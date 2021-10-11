==== Source: A ====
function id(uint x) pure returns (uint) {
    return x;
}

==== Source: B ====
import "A";

contract C {
    using * for uint;
    function f(uint x) public pure returns (uint) {
        return x.id();
    }
}
// ----
// TypeError 1308: (B:30-47): "using * for T;" cannot be used inside contract definition
