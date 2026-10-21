# Alpha.14 release acceptance

Status: candidate preparation in progress. Publication and public-archive
verification are still pending. The website must publish after the native release.

## Selected scope

- Type-owned method dispatch and dot notation: GI#525, P01–P15 accepted on `next`.
- Routed HTTP server framework: GI#518, P01–P15 accepted on `next`.
- HTTP documentation slice of GI#552: Server and Client references, runnable
  examples, request/response handling, configuration, lifecycle and static serving.
  The wider standard-library reference remains separate work.
- Example inventory preflight and the validation repairs already integrated on `next`.

Coloured diagnostics are not a release gate. Remaining compiler-understanding,
package-management design, provenance and validation-speed work follows the
agreed release-scope rebalance; it is not silently included in alpha.14.

## Release sequence

1. Complete the HTTP documentation and version preparation on `next`.
2. Pass required validation and promote the reviewed source to `main` through a PR.
3. Run the Release workflow for the exact main commit and version `0.1.0-alpha.14`.
4. Verify both public native archives, checksums and provenance against the
   retained CI artifacts and supported-platform smoke results.
5. Promote installer/download guidance and the website's reviewed release pins.
   Publish the HTTP documentation only after the native release is verified.
6. Verify deployed website bytes, release links and examples, then record final
   acceptance here.

## Evidence so far

Website CI PR #52 merged independently of the alpha.14 content. Main run
39520213084 reused its successful PR candidate, skipped duplicate assembly,
release-integrity and browser jobs, and passed publication. The website still
serves alpha.13. PR #51 remains the separate alpha.14 documentation/publication PR.

The HTTP client documentation example was executed locally with both `get` and
`get_with_limits`, against the ordinary HTTP server example. Both returned
status 200 and the expected body. These checks used the current native compiler;
public alpha.14 package verification remains pending.

Full local `make check` passed in 519 seconds, including bootstrap fixed-point
verification and release smoke tests. The historical timing budget produced a
warning, not a test failure. Documentation checks passed. Website examples passed
in source and saved-bytecode form against the alpha.14 compiler.

Promotion, release IDs and public asset checksums will be recorded once each
step completes. No publication is claimed here.
