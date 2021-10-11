contract C {
    struct S { uint t; }
    function r() public {
        S memory x;
        x.d;
    }
    using S for S;
}
// ----
// TypeError 8187: (113-114): Expected library name, free-function(s) name(s), module name or *.
