contract C {
    using {f, h} for uint;

    function f(uint) internal {
    }
    function g(uint) external {
    }
    function h(uint) internal {
    }
}
// ----
// TypeError 4167: (24-25): Only free functions can be bound to a type in a "using" statement
// TypeError 4167: (27-28): Only free functions can be bound to a type in a "using" statement
