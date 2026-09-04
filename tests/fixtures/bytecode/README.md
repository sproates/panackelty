# Portable bytecode vectors

Each `.hex` file contains the complete bytes of one Panackelty bytecode
artifact as lowercase hexadecimal text. Whitespace is insignificant. This
representation keeps the fixtures portable across source-control systems and
lets every loader implementation consume the same bytes with only a hex
decoder.

Version 7 implementations must produce these results:

| Vector | Expected result |
| --- | --- |
| `minimal-v7.hex` | Loads, reserializes byte-identically, and runs `main` to `Void`. |
| `minimal-v4.hex` | Rejected as an unsupported legacy version. |
| `minimal-v5.hex` | Rejected as an unsupported legacy version. |
| `minimal-v6.hex` | Rejected as an unsupported legacy version. |
| `bad-magic.hex` | Rejected as not being a Panackelty bytecode file. |
| `unknown-opcode-v7.hex` | Rejected because `ff` is not an opcode. |
| `invalid-jump-v7.hex` | Rejected because jump target 99 is outside the function. |
| `nonminimal-integer-v7.hex` | Rejected because zero has a one-byte magnitude instead of the canonical empty magnitude. |
| `truncated-v7.hex` | Rejected because the declared function name is incomplete. |
| `trailing-v7.hex` | Rejected because a byte follows the complete payload. |

When the bytecode version or encoding changes, retain old-version vectors for
compatibility testing and add new files rather than rewriting their bytes.
