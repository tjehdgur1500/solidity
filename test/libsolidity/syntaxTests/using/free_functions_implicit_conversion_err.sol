function id(uint16 x) pure returns(uint16) {
    return x;
}
contract C {
    using id for uint256;
    using {id} for uint256;
}
// ----
// TypeError 3100: (84-86): The function "id" cannot be bound to the type "uint256" because the type cannot be implicitly converted to the first argument of the function ("uint16").
// TypeError 3100: (111-113): The function "id" cannot be bound to the type "uint256" because the type cannot be implicitly converted to the first argument of the function ("uint16").
