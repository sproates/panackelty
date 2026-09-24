VOID_RETURN = [("CONST", ("Void", None)), ("RETURN", None)]


FORGED_DYNAMIC_FAILURES = (
    ('invalid fs_read operand', [('CONST', ('Nat', 0)), ('CONST', ('Nat', 0)), ('CALL', ('fs_read', 2)), ('RETURN', None)], ("VM trap",)),
    ('invalid fs_write operand', [('CONST', ('Nat', 0)), ('CONST', ('Nat', 0)), ('CALL', ('fs_write', 2)), ('RETURN', None)], ("VM trap",)),
    ('invalid fs_metadata operand', [('CONST', ('Nat', 0)), ('CALL', ('fs_metadata', 1)), ('RETURN', None)], ("VM trap",)),
    ('invalid fs_list operand', [('CONST', ('Nat', 0)), ('CALL', ('fs_list', 1)), ('RETURN', None)], ("VM trap",)),
    ('invalid host_sleep operand', [('CONST', ('Nat', 0)), ('CALL', ('host_sleep', 1)), ('RETURN', None)], ("VM trap",)),
    ('invalid host_decode_utf8 operand', [('CONST', ('Nat', 0)), ('CALL', ('host_decode_utf8', 1)), ('RETURN', None)], ("VM trap",)),
    ('invalid process_run operand', [('CONST', ('Nat', 0)), ('CONST', ('Nat', 0)), ('CONST', ('Nat', 0)), ('CONST', ('Nat', 0)), ('CONST', ('Nat', 0)), ('CONST', ('Nat', 0)), ('CONST', ('Nat', 0)), ('CALL', ('process_run', 7)), ('RETURN', None)], ("VM trap",)),
    (
        'forged Path record',
        [
            ('CONST', ('Nat', 0)),
            ('MAKE_RECORD', ('Path', ['raw'])),
            ('CALL', ('path_native_bytes', 1)),
            ('RETURN', None),
        ],
        ('requires Path',),
    ),
    (
        'forged Duration record',
        [
            ('CONST', ('Nat', 0)),
            ('MAKE_RECORD', ('Duration', ['raw'])),
            ('CALL', ('duration_ticks', 1)),
            ('RETURN', None),
        ],
        ('requires Duration',),
    ),
    (
        'forged Instant record',
        [
            ('CONST', ('Nat', 0)),
            ('MAKE_RECORD', ('Instant', ['raw'])),
            ('CONST', ('Nat', 0)),
            ('CALL', ('instant_before', 2)),
            ('RETURN', None),
        ],
        ('requires Instant',),
    ),
    (
        'invalid path_from_text operand',
        [
            ('CONST', ('Bool', True)),
            ('CALL', ('path_from_text', 1)),
            ('RETURN', None),
        ],
        ('invalid path constructor',),
    ),
    (
        'invalid path_from_native operand',
        [
            ('CONST', ('Bool', True)),
            ('CALL', ('path_from_native', 1)),
            ('RETURN', None),
        ],
        ('invalid path constructor',),
    ),
    (
        'invalid duration_from_seconds operand',
        [
            ('CONST', ('Bool', True)),
            ('CALL', ('duration_from_seconds', 1)),
            ('RETURN', None),
        ],
        ('requires Rat',),
    ),
    (
        'invalid duration_nanoseconds operand',
        [
            ('CONST', ('Bool', True)),
            ('CALL', ('duration_nanoseconds', 1)),
            ('RETURN', None),
        ],
        ('requires Int',),
    ),
    (
        'invalid instant_add operand',
        [
            ('CONST', ('Bool', True)),
            ('CONST', ('Bool', True)),
            ('CALL', ('instant_add', 2)),
            ('RETURN', None),
        ],
        ('requires Instant',),
    ),
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
