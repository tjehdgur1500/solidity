function id(uint x) pure returns (uint) {
    return x;
}

contract C {
    using id for uint;

    function g(uint z) pure external {
        z.id();
    }
}
