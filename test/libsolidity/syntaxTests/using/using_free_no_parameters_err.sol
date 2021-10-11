function one() pure returns(uint) {
    return 1;
}

contract C {
    using one for uint;
}
// ----
// TypeError 4731: (76-79): The function "one" does not have any parameters, and therefore cannot be bound to the type "uint256".
