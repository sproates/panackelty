[Roadmap index](../README.md) · [Current release and next work](../../../ROADMAP.md)

This is a retained planning record. Dates and checkpoints below describe their
original context; see the [roadmap](../../../ROADMAP.md) for current release status.

## Next integration workflow — active; promotion acceptance remains

<a id="rm-135"></a>

**RM#135: Next integration workflow** · [GI#287: Next integration workflow](https://github.com/sproates/panackelty/issues/287).

**In progress — active core integration since 2026-10-05.** The user's decision
on 2026-10-05 supersedes the earlier 2026-10-04 release-before-activation gate:
`next` is the core integration branch, and core feature PRs branch from and target
`next`. Independent website publishing was accepted on 2026-10-05; [GI#178:
Independent website publishing](https://github.com/sproates/panackelty/issues/178)
is closed, so website separation is complete and is no longer an activation
prerequisite.

Preserve PR-only integration, required checks, branch currency and the prohibition
on force-pushes and branch deletion. Do not enable auto-merge. Agreed-scope core
PRs may merge to `next` once applicable review and required checks pass. Every
promotion to `main` still requires explicit user approval; production releases
remain tied to approved `main` ancestry. The current `next` protections and checks
were verified on 2026-10-07; PR#321 and PR#323 demonstrate the active integration
path.

Remaining acceptance is to document and verify promotion history, release
ancestry, branch realignment after releases, hotfix synchronization and rollback.
Keep website, browser and coverage publication independent of core integration
and preserve their own release boundaries. RM#135 remains In progress until those
promotion and synchronization checks are accepted; its scope changes no other
programme weights or priorities.
