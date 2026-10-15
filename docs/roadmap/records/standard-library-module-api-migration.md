[Roadmap index](../README.md) · [Current release and next work](../../../ROADMAP.md)

This is a retained planning record. Dates and checkpoints below describe their
original context; see the [roadmap](../../../ROADMAP.md) for current release status.

## Standard-library module API migration

<a id="rm-42"></a>

**RM#42: Standard library namespaces** — Done on merge of
[PR#404: Measured standard-library module API migration](https://github.com/sproates/panackelty/pull/404).

The migration groups APIs into short, mostly flat domain modules; exposes a
curated `prelude` while keeping `numeric` separate; removes the superseded public
`path_string_*` and Str-based trapping filesystem wrappers after caller
migration; and preserves the `bytes()` constructor by exposing the bytes module
as `prelude.byte_buffer`. The user's decisions and measured replay are recorded
in the [P4 replay evidence](../../../tests/VALIDATION_PROFILE.md#owner-approved-p4-replay-evidence).
P4 earns its existing 5% programme weight in the 50% total on merge of PR#404.
The P8 clean-installed language/local-package release gate and alpha.13 website
adoption remain separate.

Review how existing library APIs should be grouped, exported and referenced so
users can discover them and avoid collisions. This is a separate deliverable
from general namespace support, with an explicit dependency assessment.

- [x] Inventory current modules and exported names; identify collisions,
      inconsistent naming and unnecessarily exposed implementation details
- [x] Propose coherent namespace boundaries and ergonomic qualified/unqualified
      usage, evaluated on real programs and compiler-assisted discoverability
- [x] Decide which improvements are possible with today's imports and which
      require the language namespace proposal; avoid assuming they must ship
      together or making incompatible naming decisions independently
- [x] Plan compatibility or a deliberate preview migration for compiler sources,
      libraries, examples, tests, documentation and installed packages
- [x] Define acceptance evidence that public APIs remain accessible, names resolve
      predictably and existing behavior is preserved through the migration

PR#404 migrated compiler, standard-library, test and example callers, added
positive and legacy-API rejection functional coverage, and passed its exact-head
hosted Check plus the post-merge `next` Check. Its local matched clean checks
were 602s before and 619s after; this single pair does not establish a cause or
general performance change. [PR#405](https://github.com/sproates/panackelty/pull/405)
merged the completion record to `next`; GI#235 was explicitly closed on
2026-10-12. GI#106 remains open and owns the broader workflow investigation;
P8 still owns release acceptance.
