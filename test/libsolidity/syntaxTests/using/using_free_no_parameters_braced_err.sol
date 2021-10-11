function one() pure returns(uint) {
    return 1;
}
function id(uint) pure returns(uint) {
    return 1;
}

contract C {
    using {id, one} for uint;
}
// ----
// TypeError 4731: (136-139): The function "one" does not have any parameters, and therefore cannot be bound to the type "uint256".
