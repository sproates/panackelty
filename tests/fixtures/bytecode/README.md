# Portable bytecode vectors

Each `.hex` file contains the complete bytes of one Panackelty bytecode
artifact as lowercase hexadecimal text. Whitespace is insignificant. This
representation keeps the fixtures portable across source-control systems and
lets every loader implementation consume the same bytes with only a hex
decoder.

Version 8 implementations must produce these results:

| Vector | Expected result |
| --- | --- |
| `minimal-v8.hex` | Loads, reserializes byte-identically, and runs `main` to `Void`. |
| `minimal-v4.hex` | Rejected as an unsupported legacy version. |
| `minimal-v5.hex` | Rejected as an unsupported legacy version. |
| `minimal-v6.hex` | Rejected as an unsupported legacy version. |
| `minimal-v7.hex` | Rejected as an unsupported legacy version. |
| `bad-magic.hex` | Rejected as not being a Panackelty bytecode file. |
| `unknown-opcode-v8.hex` | Rejected because `ff` is not an opcode. |
| `invalid-jump-v8.hex` | Rejected because jump target 99 is outside the function. |
| `nonminimal-integer-v8.hex` | Rejected because zero has a one-byte magnitude instead of the canonical empty magnitude. |
| `truncated-v8.hex` | Rejected because the declared function name is incomplete. |
| `trailing-v8.hex` | Rejected because a byte follows the complete payload. |

When the bytecode version or encoding changes, retain old-version vectors for
compatibility testing and add new files rather than rewriting their bytes.

The direct bytecode migration adds [contract cases](contract_cases/README.md),
source/golden codec artifacts under `codec_contracts/`, and decoded instruction
listings under `valid_contracts/`. Both native probes use these reviewed inputs;
they do not generate expected bytes while testing.
