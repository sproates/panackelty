# Namespace examples

These examples ship with native alpha.13. Each directory is a complete standalone
source tree; run its `main.panack` with the matching installed toolchain.

| Example | Demonstrates |
| --- | --- |
| [Trip planner](trip_planner/main.panack) | Combining route, weather and budget modules with aliases and selected imports |
| [Twin sensors](twin_sensors/main.panack) | Distinct same-named record types in separate modules |
| [Public API](public_api/main.panack) | Public re-exports backed by a private implementation |

From a built checkout or extracted release root:

```sh
./bin/panack run examples/namespaces/trip_planner/main.panack
./bin/panack compile examples/namespaces/trip_planner/main.panack -o trip.bc
./bin/panack run trip.bc
```

Only the entry module's `main` runs. Imports are confined to the source tree and
active standard library. See the [module specification](../../docs/reference/SPEC.md#modules)
for visibility and import rules. Local package dependencies additionally use a
manifest; they are distinct from these standalone namespace examples.

## Returning a type from another module

Suppose the travel API offers `weekend()`, which returns a `Trip`. The type
is defined in [details.panack](public_api/details.panack). A private import
lets the implementation construct a trip, but callers of the travel API also
need to be able to name its public return type.

With `import project/details::{Trip}`, Panack points at `Trip` in
`pub pure weekend(): Trip` and reports that the public signature exposes an
inaccessible type. You can [check this rejected program](../../tests/functional/failures/private_signature_import/main.panack)
with `./bin/panack check tests/functional/failures/private_signature_import/main.panack`.

The [working API](public_api/api.panack) exposes the original type by adding
`pub` to that import:

```panack
pub import project/details::{Trip}

pub pure weekend(): Trip {
  Trip("Granada", 3)
}
```

The [caller](public_api/main.panack) imports the API as `travel_api` and writes
`trip: travel_api.Trip = travel_api.weekend()`. It can then read `trip.city`.
Only types that appear in the public contract need to be reachable. Helpers
used exclusively inside a function body can stay private.

Run `./bin/panack run examples/namespaces/public_api/main.panack` to try it.
