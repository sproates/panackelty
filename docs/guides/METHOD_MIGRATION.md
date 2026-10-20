# Migrating to type-owned methods

This guide describes the unreleased alpha.14 toolchain on `next`. Published
alpha.13 downloads and the website's alpha.13 playground retain their own
contract. Upgrade the compiler, VM and standard library together; do not copy new
library sources into an older installation. Rebuild saved artifacts with the
matching toolchain before distributing them.

Receiver calls select a method on the checked receiver type. An unrelated global
function with the same name cannot supply a missing method. Namespace functions
and record fields remain distinct. Callable fields need explicit invocation:
`(value.callback).call(argument)`.

| Earlier source | Owned method |
| --- | --- |
| `value->method(argument)` during migration | `value.method(argument)` |
| `quotient(n, divisor)` | `n.divide(divisor)` |
| `text.encode_utf8(message)` | `message.encode_utf8()` |
| `text.reversed(message)` | `message.reverse()` |
| `bytes.length(data)` | `data.length()` |
| `bytes.push(data, value)` | `data.push(value)` |
| `bytes.join(left, right)` | `left.join(right)` |
| `bytes.at(data, index)` | `data.at(index)` |
| `path.join(parent, child)` | `parent.join(child)` |
| `option.value_or(value, fallback)` | `value.value_or(fallback)` |

`Str` and arrays use `.len()`; `Bytes` uses `.length()`. Natural truncated division
is `.divide()`. Keep exact rational `/` where exact division is intended.

Remove imports of the deleted wrapper-only `stdlib/text`, `stdlib/numeric`,
`stdlib/option` and `stdlib/result` modules. Constructors and services remain
namespace functions, for example `bytes.empty()`, `path.from_text(...)`,
`time.from_nanoseconds(...)`, `filesystem.read(...)` and `tcp.exchange(...)`.
The [wrapper register](../design/type-owned-dispatch/p10-stdlib-callers.md#exact-temporary-source-wrapper-register)
lists the complete source-wrapper migration. `host.decode_utf8(data)` is a
separate checked host service; it is not equivalent to the pure, trapping
`data.decode_utf8()` method.

```panack
record Fruit { value: Nat }
impl Fruit {
  pure apple(): Nat { self.value }
  pure banana(): Nat { self.value + 1 }
}
main(): Void {
  first = Fruit(10)
  second = Fruit(20)
  print(first.apple())
  print(first.banana())
  print(second.apple())
  print(second.banana())
}
```

The output is `10`, `11`, `20`, `21`, one value per line. Owned methods retain
visibility, generic inference, purity and asynchronous contracts. Server methods
and Map methods can share names without sharing signatures or dispatch. The
language acceptance fixtures do not establish HTTP framework networking support;
that work remains GI#518.
