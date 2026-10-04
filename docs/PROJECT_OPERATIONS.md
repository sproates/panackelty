# Reusable project operations

Use these bounded scripts for repeatable repository operations. They keep large
connector responses and file payloads inside one orchestration call, returning
only a compact result. They do not poll, approve or merge a PR. Node is required
only for local operational tools/tests, not the native compiler or `make check`.

| Priority | Operation | Reusable entry point and remaining boundary |
| --- | --- | --- |
| 1 | PR/workflow snapshot | `scripts/project_status.cjs`; one PR read, one head-specific workflow page, one head recheck. |
| 1 | Exact-tree publication | `scripts/project_manifest.cjs` and `scripts/project_publish.cjs`; committed local blobs to a new feature branch. PR creation and existing-branch updates remain separate. |
| Existing | Validation selection and execution | `bash scripts/validate_change.sh --run origin/main`; retain the canonical gates and their timing output. |
| Next | Tracking reconciliation | Reuse stable RM/GI IDs and links; a bounded script could compare PR references, merged state and roadmap records. Priority, completion acceptance and issue updates still need judgment; no automated issue mutation is provided. |

## PR snapshot through connected tools

In this environment GitHub access uses connected tools, not `gh` or shell tokens.
Run this recipe in `functions.exec`, replacing the checkout path and PR number:

```js
const source = await tools.exec_command({
  cmd: 'cat scripts/project_status.cjs',
  workdir: '/workspace/scratch/db4ab5353ceb/panackelty-ops',
  max_output_tokens: 8000
});
if (source.exit_code !== 0) throw new Error('Cannot load status script');
const module = {exports: {}};
new Function('module', source.output)(module);
const api = {
  get_pr_info: tools.mcp__codex_apps__github_get_pr_info,
  fetch_commit_workflow_runs: tools.mcp__codex_apps__github_fetch_commit_workflow_runs
};
text(await module.exports(api, {
  repository: 'sproates/panackelty', number: 264, expected: ['Check', 'Pages']
}));
```

The module also works through `require()` with injected adapters returning either
connector `structuredContent` or the same plain data. Tool failures are reduced
to stage names; bodies, logs and credentials are not returned. It selects the
newest run number per workflow, then run ID and attempt number where supplied.
Conflicting normalized duplicates without attempt identity are unknown. Missing
expected workflows remain unknown; `expected` is an observation list, not a claim
about branch protection. The connector returns only the first page without
completeness metadata, so `coverage` always remains `unknown`. A successful
`observed` result describes only the returned runs; it never means merge ready.
A second PR read marks changed heads stale; failed rereads leave freshness
unknown. Later changes after that read are outside this snapshot.

A live PR #264 smoke test used one orchestration call containing three connector
requests. Its summary was 410 JSON characters versus 5,288 for the sampled raw
PR/run responses (about 92% fewer model-facing characters; token billing was not
measured). This is evidence of reduced response volume, not a latency benchmark.

## Publish a committed delivery

First validate, independently review and commit the local result. Generate the
manifest outside the checkout; the command refuses dirty work, unsupported file
modes, a non-ancestor base and overwriting an existing output file:

```sh
node scripts/project_manifest.cjs sproates/panackelty \
  chore/scripted-project-operations origin/main /tmp/panack-delivery.json \
  'Script routine project operations'
```

The manifest records `repository`, `branch`, `parentSha`, `baseTree`, exact `tree`,
local `commit`, `message` and changed `files` with path, mode, Git blob SHA and
base64 content. Deletions have `sha: null`. Git supplies the diff and blob bytes;
renames become a deletion plus addition. Do not print or paste this payload into
the conversation. In code mode capture both files and publish directly:

```js
const source = await tools.exec_command({
  cmd: 'cat scripts/project_publish.cjs',
  workdir: '/workspace/scratch/db4ab5353ceb/panackelty-ops', max_output_tokens: 8000
});
const payload = await tools.exec_command({
  cmd: 'cat /tmp/panack-delivery.json', max_output_tokens: 20000
});
if (source.exit_code !== 0 || payload.exit_code !== 0) throw new Error('Cannot load delivery');
const module = {exports: {}};
new Function('module', source.output)(module);
text(await module.exports(tools, JSON.parse(payload.output)));
```

Set the payload output limit high enough for the actual manifest size; a truncated
JSON response fails parsing before publication. Connected tool names are explicit
in the publisher. It checks every uploaded blob SHA and the complete resulting
tree before creating a commit/ref. Remote commit metadata may differ from the
local commit, but the reviewed tree must match. It only creates a **new feature
branch**: collisions fail without retry or overwrite. Failures can leave
unreferenced uploaded blobs/trees/commits; no rollback or branch update is
attempted. Creating the PR is separate; approval and merge remain explicit.

## Validation and scope

Run `make project-operations-test` for the Node regression suite. This uses the
same `node:test` conventions as existing website operational tests and is a
separate gate from native `make check`. It covers missing/stale workflow evidence,
reruns, API errors, exact binary/mode/deletion manifests, dirty work rejection and
publication identity/ref safety. No Panackelty language or CLI behavior changes,
so existing functional coverage remains applicable. Website content and release
claims are unaffected: these are contributor operations only.
