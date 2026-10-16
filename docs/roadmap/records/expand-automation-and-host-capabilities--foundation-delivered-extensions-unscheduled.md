[Roadmap index](../README.md) · [Current release and next work](../../../ROADMAP.md)

This is a retained planning record. Dates and checkpoints below describe their
original context; see the [roadmap](../../../ROADMAP.md) for current release status.

## Expand automation and host capabilities — foundation delivered; extensions unscheduled

<a id="rm-84"></a>

**RM#84: Host automation APIs**.

Panackelty should gain the general host capabilities needed by dependable
automation programs. These APIs
must be useful outside the test harness, remain visibly effectful, behave
predictably across supported platforms, and expose structured failures rather
than test-specific shortcuts. Logical standard-library imports are a
prerequisite so programs can use these APIs without knowing repository paths.

- [x] Specify a coherent process API for executable selection, arguments,
      standard input, working directory, environment overrides, exit status,
      and captured standard output and error
- [x] Make process streams byte-oriented with explicit checked UTF-8 decoding,
      define resource and output limits, and prevent deadlocks when both output
      streams are active
- [ ] Specify portable directory enumeration, directory creation, file metadata,
      removal, and recursive operations with deterministic ordering and clear
      symbolic-link and failure behavior; immediate enumeration, one-level
      creation, metadata, and nonrecursive removal are implemented, while
      recursive operations remain deferred
- [x] Add collision-safe temporary-file and temporary-directory creation with
      explicit ownership, cleanup, and failure semantics
- [x] Separate wall-clock time from a monotonic elapsed-time API suitable for
      validation budgets and performance measurements
- [ ] Define the supported-platform and capability policy for behavior such as
      permissions that cannot be represented consistently on every host; do not
      add a universal operation solely for a platform-specific test
- [x] Implement the accepted host boundary in the native VM; retain the fixed
      independent expectations from the retired differential-validation workflow
- [x] Add typed standard-library wrappers that keep all process, filesystem,
      temporary-resource, and clock operations effectful
- [ ] Extend focused, cross-platform, and public-CLI conformance coverage, including
      large simultaneous process streams, invalid UTF-8, missing executables,
      environment and working-directory isolation, cleanup failures, path
      traversal, resource limits, and monotonic timing; the initial suite covers
      streams, isolation, temporary cleanup, bounds, and failure categories;
      exhaustive injected host failures and traversal coverage remain pending
- [x] Build a small Panackelty testing library with assertions, structured test
      results, fixture discovery, temporary isolation, command assertions, and
      deterministic reporting for automated validation; the three
      explicitly imported modules now cover pure assertions and ordered reports,
      sorted immediate fixture directories and explicitly owned workspaces,
      plus bounded byte-exact command assertions and expected host errors.
