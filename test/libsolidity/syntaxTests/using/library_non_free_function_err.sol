library L {
    function id(uint x) internal returns(uint) {
        return x;
    }
}

contract C {
    using L.id for uint;
    function f(uint x) external {
        x.id();
    }
}
// ----
// TypeError 4167: (111-115): Only free functions can be bound to a type in a "using" statement
