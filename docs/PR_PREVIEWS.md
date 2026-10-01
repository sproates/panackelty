# Website PR previews

Work record: [#160](https://github.com/sproates/panackelty/issues/160).
This first slice provides the portable build, local server and tested CI
artifact. Automatic hosted URLs, fork publication approval and cleanup are
the next slice, not live capabilities of this change.

## Local use

From a clone, using Node 24, Git, POSIX shell and tar:

```sh
node scripts/preview.cjs build
node scripts/preview.cjs serve
```

Open http://127.0.0.1:4173/ in your browser. Stop with Ctrl-C. The server binds
only to loopback; it is not a public sharing service. No hosting account,
GitHub token, assistant or native compiler build is needed. Building downloads
the checksummed browser release pinned in `site/playground.json`. It uses the
working tree, labels local modifications, and records the Git commit.

To rebuild, remove the generated `build/preview` directory or choose a fresh
output path. Existing output is rejected to prevent stale assets surviving.
Both commands accept an output directory; serve accepts an optional port:

```sh
node scripts/preview.cjs build build/review-2
node scripts/preview.cjs serve build/review-2 4180
```

## Artifact contract

Production and previews share `scripts/assemble_site.sh` for website and
playground assembly. The preview adds visible identity on the home and
playground pages, `preview.json`, and no-index hints. Browser binaries and
versioned runtime assets remain unchanged. Coverage is explicitly unavailable
for this review build, with a link to separately identified production coverage.

The Pages PR job builds the synthetic merge revision, records the PR head and
base as well as that revision, and runs the existing 21 browser scenarios.
After tests pass it archives `website-preview-RUN_ID-RUN_ATTEMPT` for seven days.
Download it from the Actions run and serve the extracted directory with the
same local command. An artifact download is not a hosted review URL.

The static artifact expects its own origin root, not a PR subdirectory under
the production website. Publication must identify repository, PR head, merge
revision, base revision, run ID and attempt independently of artifact claims.
An artifact is untrusted static content; never run scripts from it in a job
with deployment credentials. Dirty local builds must not become hosted CI previews.

## Deployment follow-up contract

Production stays on GitHub Pages. Recommend a dedicated Cloudflare Pages
Direct Upload project for previews: it supports branch preview URLs and
immutable deployment URLs without requiring contributor hosting accounts.
This is a proposed provider choice; account/project access and limits must be
verified before configuration. No new paid service or credentials are created
by this first slice. A separate GitHub Pages repository is an alternative but
requires managing a combined preview tree and cross-repository publication.

The trusted publisher must live on the default branch and consume only a
successful, identified Pages PR build from this repository. Check the live PR
is still open and its head/base still match; bind selection to workflow ID,
run ID, attempt and artifact ID. Never use a PR-provided URL or repository name
to select a download. Validate artifact bounds, regular files and expected
layout; reject symlinks, traversal and provider-executable inputs such as
Pages Functions or `_worker.js`. Run neither PR code nor package installation
with deployment secrets. Use a separately scoped preview project credential.

Same-repository PRs publish automatically after relevant tests. Fork builds
remain unprivileged; publishing requires approval tied to the exact revision
through a protected environment or trusted maintainer dispatch. An approval
must not carry over to a new head. Fork owners need no service account.

Manual dispatch takes a PR number and rebuilds/reselects its current revision;
it must follow the same tests and trust boundary. Write one bot-owned comment
with the current immutable URL and source commit, and a GitHub deployment link.
Failures remain visible. Serialize publisher/cleanup operations per PR and
recheck current revision immediately before and after deployment, deleting
stale deployments instead of promoting them. Closing a PR removes its preview
deployments and marks its GitHub deployment inactive; reopened PRs can rebuild.

Live acceptance requires same-repository creation/update, deliberately reversed
build completion order, fork approval/new-head rejection, manual dispatch,
failure reporting, closure cleanup and phone review, with production unchanged.
The task is complete only after those checks, not after the build artifact PR.

Provider references:
- https://developers.cloudflare.com/pages/how-to/use-direct-upload-with-continuous-integration/
- https://developers.cloudflare.com/pages/configuration/preview-deployments/
- https://docs.github.com/en/actions/reference/workflows-and-actions/deployments-and-environments
