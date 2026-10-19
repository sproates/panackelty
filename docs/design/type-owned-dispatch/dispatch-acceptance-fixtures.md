# Dispatch acceptance fixtures

[P05 built-in ownership registry](p05-builtin-owners.md) records canonical built-in source-method ownership and spoof rejection. P06 owned intrinsic dispatch is next.

Acceptance tests for [the implementation handoff](dispatch-implementation-handoff.md).
[P03 delivery evidence](p03-server-map.md) records executable F01, its import and
owner-identity variants, and F02-F08/F10. The source blocks below retain the
planned P06 Map-arrow spelling; P03 uses existing Map dots as specified below.
P04 adds F11 insufficient-context rejection and the nominal/core-enum generic matrix; see [P04 evidence](p04-generic-owners.md). F09, F12 and F13 remain later-packet tests. These tests establish language
dispatch, not an HTTP implementation.

## F01: Server and Map coexistence

Create `tests/functional/cases/owned_server_map/` using the `owned_imports` project layout. Copy its manifest structure with package name `experiment.owned_server_map`; preserve the baseline's toolchain requirement until a deliberate version change. Set `source.path` to `tests/functional/cases/owned_server_map/src/main.panack`.

For P03 only, use current dot syntax for Map calls inside these source blocks, while retaining arrow syntax for Server calls. In P06 replace those Map calls with the arrow spelling shown here. In P14 change all owned arrows to dot. The expected stdout must remain identical in all three phases.

`src/server.panack`:

```panack
pub record Server {
  id: Nat,
  routes: Map[Str, Nat]
}

impl Server {
  pub pure get(route: Str, fallback: Nat): Nat {
    if self.routes->has(route) {
      self.routes->get(route)
    } else {
      self.id + fallback
    }
  }

  pub pure put(route: Str, amount: Nat): Server {
    Server(self.id, self.routes->put(route, self.id + amount))
  }
}
```

`src/main.panack`:

```panack
import "server.panack"::{Server}

pure get(server: Server): Nat { 901 }
pure put(server: Server): Nat { 902 }

make_server(): Server {
  print("receiver evaluated once")
  Server(30, map())
}

main(): Void {
  first = Server(10, map())
  second = Server(20, map())
  first_updated = first->put("/fruit", 1)
  second_updated = second->put("/fruit", 1)
  print(first_updated->get("/fruit", 99))
  print(second_updated->get("/fruit", 99))
  print(first->get("/fruit", 5))
  print(second->get("/fruit", 5))

  initial: Map[Str, Nat] = map()
  routes = initial->put("/fruit", 7)
  changed = routes->put("/fruit", 9)
  print(routes->get("/fruit"))
  print(routes->get("/fruit"))
  print(changed->get("/fruit"))
  print(make_server()->put("/fruit", 1)->get("/fruit", 99))
  print(get(first))
  print(put(second))
}
```

`expected.stdout`:

```text
11
21
15
25
7
7
9
receiver evaluated once
31
901
902
```

Require exit status zero and empty stderr. The unchanged first/second records and original `routes` establish persistence. The repeated 7 also provides an explicit check that repeated lookup does not mutate the Map. Server.get takes two explicit arguments while Map.get takes one; Server.put returns Server while Map.put returns Map. Global get/put remain ordinary functions and cannot redirect receiver calls.

Also test namespace import, selective alias and facade re-export variants. Reuse the same server implementation and expected output rather than creating subtly different owners. A second module defining its own `Server` must remain distinct; give its get/put deliberately different output and assert that it never wins lookup for the first module's Server.

## Negative fixtures

Use the existing failure-fixture conventions and public project-aware frontend. Each fixture has one intended failure to avoid cascading diagnostics. Assert nonzero compile exit status, the source span of the failing call/declaration, and semantic diagnostic content. Match established diagnostic formatting; do not make line-number decoration part of this new contract.

| ID | Input change | Required diagnostic content | Forbidden result |
| --- | --- | --- | --- |
| F02 | `first->get("/fruit")` | Server.get expects two explicit arguments, received one | Applying Map.get signature or accepting global get |
| F03 | `routes->get("/fruit", 99)` | Map.get expects one explicit argument, received two | Selecting Server.get |
| F04 | `first->put("/fruit", "wrong")` | Server.put argument amount expects Nat, received Str | A Map value-type diagnostic |
| F05 | Remove Server.get, retain global get | Unknown owned method Server.get | Global fallback |
| F06 | Make Server.put private, call from main | Server.put is private | Successful call via alias/re-export |
| F07 | Duplicate Server.get in a second local impl | Duplicate method on same owner | Signature-based overload selection |
| F08 | Implement imported Server in main | Owner must be defined in implementing module | Foreign extension accepted |
| F09 | Implement Map in a user module named core.panack | Built-in owner implementation not permitted | Filename grants toolchain privileges |
| F10 | Pure method calls an ordinary effectful method | Existing purity-boundary diagnostic naming offending call | Effect inferred from method spelling |
| F11 | Unresolved empty-container receiver with insufficient type context | Insufficient receiver/type evidence under current inference rules | Guessing type from get/put name |
| F12 | After P12, `first.get(...)` before owned-dot support | Legacy receiver-dot syntax unsupported | Old sugar still executes |
| F13 | After P14, `first->get(...)` | Arrow receiver syntax unsupported | Permanent compatibility alias |

F12 is an intermediate-state test. Replace it at P13 with owned-dot success and keep negative tests proving global fallback remains unavailable. Do not keep a test demanding rejection of final valid syntax.

## Generic and identity matrix

Add focused signature/binding tests for these cases in P04 through P08:

- Box[Nat] and Box[Str] instances return their own correctly substituted element type.
- Option[Nat].value_or and Result[Nat, Str].value_or select different owned declarations with the same method name.
- An array map method changes element type; reduce retains accumulator and callback constraints.
- Explicit method type parameters do not overwrite implementation parameters.
- Duplicate method binders or a method binder shadowing an implementation binder are rejected.
- A nominal guarded type does not acquire its base type's methods accidentally; explicit owned methods retain guard checks.
- Public signatures exposing private owner or result types are rejected.
- A checked method call retains original declaration identity through a namespace alias and re-export.
- Core error types re-exported from path/time modules have one identity, including pattern matching and generic signatures.
- A callable field uses `(record.field).call(...)`; `record.field(...)` cannot silently invoke it.
- A local value shadowing a namespace alias selects value methods. Failed value/namespace lookups do not cross over.

## Callback and async HTTP readiness

F01 deliberately avoids pretending to specify the HTTP route-handler API. P07 must inspect the HTTP work's actual declared callback type and add a second server-shaped fixture that stores or invokes that exact kind of callback, preserving its argument, result and effect contracts.

GI#518 specifies `AsyncFn[Request,ResponseWriter,Result[Unit,Str]]` for ordinary Server handlers and `AsyncFn[T,Request,ResponseWriter,Result[Unit,Str]]` for ContextualServer[T] handlers. Registration methods return Unit. Use these contracts for the P07 acceptance fixture, checking the current issue for any later explicitly agreed changes. Mark actual handler integration unverified; do not choose a production signature or claim it is covered by F01. An async handler must have an actual await/execution test, with rejection of an invalid pure call boundary. No sockets are needed for this language test.

## Removal evidence

Searches identify review candidates; they are not proof of correctness on their own. At P12 inspect remaining matches for `lowered_method_name`, `$method_`, `$owned_call$`, `$owned$`, `$core_`, `map_get`, `map_put`, `set_add` and `set_has`. Classify every retained match as intentional VM implementation detail, rejection fixture or historical documentation. No retained source/global lookup path may provide legacy receiver semantics.

Use actual bound-call evidence for ambiguous dot calls. Do not declare migration complete because a regex search finds no `.get(` strings. Namespace functions such as `environment.get` must continue to work, and final owned-dot calls intentionally contain that spelling.
