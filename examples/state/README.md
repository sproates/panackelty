# A counter without HTTP

Requires the alpha.16 development toolchain. From the checkout or extracted
development package:

```sh
./bin/panack run examples/state/counter.panack
```

The output is `1`, `2`, then `2`, each on its own line. `increment` is a pure
anonymous function. `next` is an ordinary effectful closure: it retains the
immutable `count` handle and calls `state.update` each time it is invoked.
`state.read` observes the last committed value.

There is no captured mutable variable. Both anonymous functions could be replaced
with named functions; State does not require closures or an HTTP server.
