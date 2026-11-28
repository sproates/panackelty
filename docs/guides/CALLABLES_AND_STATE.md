# Anonymous functions, closures and shared State

These features require alpha.16. Use the native toolchain for HTTP/TCP examples;
the standalone counter also works in the matching alpha.16 browser package.

## Pass a small function where it is used

An anonymous function lets you write the callback alongside the operation that
uses it. With `values: [Nat]`, `values.map((value) => value + 1)` adds one to each
element. The known element type supplies the parameter type. Without that context,
write an annotation, such as `(value: Nat) => value + 1`.

The body determines whether an unmarked function is pure or has ordinary effects.
Use `async (...) => { ... }` for an async callback. Creating a callback does not
run it. Call a stored function using `callback.call(...)`, or pass it to an API
that expects a callback. Named functions still use `@name` when passed as values.

## Retain the values a callback needs

A closure retains the immutable bindings it refers to. Those values remain
available after the function that created the closure returns. Capturing a value
does not capture a mutable variable that someone can silently change later.
Implicit capture of `mut` bindings is rejected: pass the value explicitly, take
an immutable snapshot, or use State for changes that calls should share.

Request writers, connections and other native resources cannot be captured.
Receive them as callback parameters. The HTTP closure example captures its start
instant and State handle; the request and response writer are parameters.

## Make shared mutation explicit

Import `stdlib/state`, or `stdlib/state/async` from async code. Both expose
`create`, `read` and `update` and use the same `State[T]` handle. All three
operations are effectful, including creation. Await them with the async import.

`create(initial)` creates a cell. Copying its handle shares that cell. `read(cell)`
returns an immutable snapshot. `update(cell, callback)` passes the current value
to one pure callback, commits its result and returns the committed value.

The callback is called once. Competing reads and updates on that cell wait while
the update is running. Other cells and runnable tasks can progress. Keep a
read-modify-write operation inside the callback: reading first and replacing the
value later can overwrite someone else's update.

The payload can contain immutable data, including records and collections. It
cannot contain cells, callbacks, tasks or native resources. Closures can retain
cells, so the shared mutation is visible at the `state.update` call. Handle
equality compares cell identity, not the current contents.

The [standalone counter](../../examples/state/counter.panack) demonstrates a pure
transform and a retained cell without HTTP. It prints `1`, `2`, then `2`.

## Combine state and an async HTTP handler

The [closure example](../../examples/network/http_state_closure/src/main.panack) keeps
its start instant and request counter in an anonymous handler. The
[contextual example](../../examples/network/http_state_contextual/src/main.panack)
uses a named handler and passes the same values through an explicit context.
Neither style is required by State; choose the one that makes your code clearest.

Both examples count committed increments for `/status` and report monotonic
uptime. An unrecognised route does not increment the count. A request rejected
before entering the handler is not counted either.

The handler increments before reading the clock. If that clock read fails, the
response is HTTP 503 and includes the committed count. Failure to read the clock
at startup prints an explanation and exits instead of starting an unusable service.

Cancellation or a trap before commit preserves the old cell value. Cancellation
or response failure after commit does not undo it. A cancelled waiter never runs
the callback. These rules define what the counter measures; it is not a count of
successfully delivered responses. Restarting the server resets the count.

## Keep external work outside an update

An update callback cannot read the clock, perform I/O, await work or read/update
another cell. Do that work before or after the update. If the update needs a value
obtained asynchronously, await it first and pass the resulting immutable data
into the callback. There are no callback retries, public locks or multi-cell
transactions in this API.
