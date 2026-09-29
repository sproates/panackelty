# Portable bytecode vectors

Each `.hex` file contains the complete bytes of one Panackelty bytecode
artifact as lowercase hexadecimal text. Whitespace is insignificant. This
representation keeps the fixtures portable across source-control systems and
lets every loader implementation consume the same bytes with only a hex
decoder.

Version 9 implementations must produce these results:

| Vector | Expected result |
| --- | --- |
| `minimal-v9.hex` | Loads, reserializes byte-identically, and runs `main` to `Void`. |
| `minimal-v4.hex` | Rejected as an unsupported legacy version. |
| `minimal-v5.hex` | Rejected as an unsupported legacy version. |
| `minimal-v6.hex` | Rejected as an unsupported legacy version. |
| `minimal-v8.hex` | Rejected as an unsupported legacy version. |
| `minimal-v7.hex` | Rejected as an unsupported legacy version. |
| `bad-magic.hex` | Rejected as not being a Panackelty bytecode file. |
| `unknown-opcode-v9.hex` | Rejected because `ff` is not an opcode. |
| `invalid-jump-v9.hex` | Rejected because jump target 99 is outside the function. |
| `nonminimal-integer-v9.hex` | Rejected because zero has a one-byte magnitude instead of the canonical empty magnitude. |
| `truncated-v9.hex` | Rejected because the declared function name is incomplete. |
| `trailing-v9.hex` | Rejected because a byte follows the complete payload. |

Named old-version vectors are retained unchanged for compatibility testing.
The current unversioned contract corpora migrate their header to v9; payloads
remain fixed except reserved-flag and unsupported-version rejection cases.

The direct bytecode migration adds [contract cases](contract_cases/README.md),
source/golden codec artifacts under `codec_contracts/`, and decoded instruction
listings under `valid_contracts/`. Both native probes use these reviewed inputs;
they do not generate expected bytes while testing.
