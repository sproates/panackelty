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
