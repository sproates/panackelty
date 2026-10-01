# Local website previews

Work record: [#160](https://github.com/sproates/panackelty/issues/160).

## One command

From the repository root, using Node 24, Git, POSIX shell and tar:

```sh
node scripts/preview.cjs
```

This builds the current working tree into a fresh temporary directory and starts
a server at http://127.0.0.1:4173/. Open that address in a browser on the same
machine. The visible notice identifies the commit and local modifications.

Stop with Ctrl-C. After editing, rerun the command and refresh the browser to see
a new build. There is no file watcher or automatic browser opening. SIGINT and
SIGTERM stop the server and remove its temporary files; failed startup also
cleans up. A forced kill or machine crash can leave a temporary directory for
the operating system to clear. Saved builds are never overwritten or removed.

The equivalent explicit command is `node scripts/preview.cjs start`. To use
another port, for example when 4173 is occupied:

```sh
node scripts/preview.cjs start 4180
```

No hosting account, deployment credentials, assistant or native compiler build
is needed. Building downloads and verifies the browser release pinned in
`site/playground.json`; internet access is needed for that download.

## Saved builds and CI artifacts

For a build that persists after stopping the server:

```sh
node scripts/preview.cjs build build/review-2
node scripts/preview.cjs serve build/review-2 4180
```

Existing output is rejected to prevent stale files surviving. Choose a fresh
directory when rebuilding a saved artifact. The separate `serve` command never
deletes that directory. Defaults for these commands remain `build/preview`
and port 4173.

Production and previews share `scripts/assemble_site.sh` for website/playground
assembly. The preview adds identity on the home and playground pages,
`preview.json`, and no-index hints; browser runtime bytes stay unchanged.
Coverage is explicitly unavailable for the local build, with a link to separately
identified production coverage.

The Pages PR job builds the synthetic merge revision, records its head/base,
runs the existing 21 browser scenarios and archives
`website-preview-RUN_ID-RUN_ATTEMPT` for seven days. Download and extract it,
then serve the directory with the command above. CI artifacts are optional:
authors can preview uncommitted changes without opening a PR or waiting for CI.

## Author workflow and remote machines

The scope clarified on 2026-10-01 is a temporary preview for the author, not
a public review URL or permanent staging site. The earlier hosted-publication
proposal is superseded. There is no preview deployment workflow, provider setup,
fork publication approval or close-PR hosting cleanup to implement. Production
publication remains unchanged.

The server binds only to loopback. In a terminal or IDE on the development
machine, open the printed URL. In a remote workspace, use that environment's
existing private port-forwarding or preview UI if available. The URL must map to
the server's origin root so absolute site links and playground assets work.

An iPhone's localhost refers to the iPhone, not a remote development machine.
A successful HTTP check in the workspace does not prove phone accessibility.
When the local route is unavailable, an author can explicitly request a private
snapshot using their environment's existing review tools. In this ChatGPT cloud
session the user requested the Sites skill, which refreshed the existing
owner-only [review site](https://panackelty-staging.sproates846529.chatgpt.site)
from the clean PR #171 build at `c843571d07a7fb78fcfc34338372af29f4c8383b`.
The user confirmed it worked on their iPhone on 2026-10-01.

That snapshot is hosted separately; it is not a tunnel to localhost and does not
automatically update when source files change. Refresh it only when requested,
preserving its private audience. It remains available until replaced or removed.
Sites is optional for this author's workflow, not a requirement for contributors
or the local preview command. No automatic hosted PR publication is introduced.

## Follow-up

[#162](https://github.com/sproates/panackelty/issues/162) records the unscheduled
idea of replacing the Node serving component with a Panackelty-written local
HTTP/static-file server. It does not block this workflow or require rewriting
the build tooling.
