VOID_RETURN = [("CONST", ("Void", None)), ("RETURN", None)]


FORGED_DYNAMIC_FAILURES = (
    (
        "rational conversion type",
        [("CONST", ("Nat", 1)), ("CALL", ("nat", 1)), ("RETURN", None)],
        ("rational conversion requires Rat",),
    ),
    (
        "quotient operand type",
        [("CONST", ("Str", "1")), ("CONST", ("Nat", 2)),
         ("CALL", ("quotient", 2)), ("RETURN", None)],
        ("quotient requires Nat operands",),
    ),
    (
        "rational remainder",
        [("CONST", ("Nat", 1)), ("CONST", ("Nat", 3)), ("BINARY", "/"),
         ("CONST", ("Nat", 2)), ("BINARY", "%"), ("RETURN", None)],
        ("Rat does not support remainder",),
    ),
    (
        "Nat underflow",
        [
            ("CONST", ("Nat", 0)),
            ("CONST", ("Nat", 1)),
            ("BINARY", "-"),
            ("RETURN", None),
        ],
        ("Nat underflow",),
    ),
    (
        "invalid byte value",
        [
            ("CALL", ("bytes", 0)),
            ("CONST", ("Nat", 256)),
            ("CALL", ("byte_append", 2)),
            ("RETURN", None),
        ],
        ("outside 0..255",),
    ),
    (
        "invalid UTF-8",
        [
            ("CALL", ("bytes", 0)),
            ("CONST", ("Nat", 255)),
            ("CALL", ("byte_append", 2)),
            ("CALL", ("utf8_decode", 1)),
            ("RETURN", None),
        ],
        ("invalid UTF-8",),
    ),
    (
        "missing map key",
        [
            ("CALL", ("map", 0)),
            ("CONST", ("Str", "missing")),
            ("CALL", ("map_get", 2)),
            ("RETURN", None),
        ],
        ("map key", "was not found"),
    ),
    (
        "division by zero",
        [
            ("CONST", ("Nat", 1)),
            ("CONST", ("Nat", 0)),
            ("BINARY", "/"),
            ("RETURN", None),
        ],
        ("division by zero",),
    ),
    (
        "missing indirect call target",
        [
            ("CONST", ("Str", "missing")),
            ("CALL_VALUE", 0),
            ("RETURN", None),
        ],
        ("indirect call target was not found",),
    ),
    (
        "invalid indirect callable",
        [
            ("CONST", ("Nat", 1)),
            ("CALL_VALUE", 0),
            ("RETURN", None),
        ],
        ("indirect call requires a callable",),
    ),
    (
        "single-value stack underflow",
        [("POP", None), *VOID_RETURN],
        ("operand stack underflow",),
    ),
    (
        "aggregate stack underflow",
        [
            ("CONST", ("Nat", 1)),
            ("MAKE_ARRAY", 2),
            ("RETURN", None),
        ],
        ("operand stack underflow",),
    ),
)
