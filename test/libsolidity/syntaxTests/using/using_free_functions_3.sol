function id(uint x) pure returns (uint) {
    return x;
}

function zero(uint) pure returns (uint) {
    return 0;
}

contract C {
    using {id, zero} for uint;

    function g(uint z) pure external {
        z.zero();
    }
}
