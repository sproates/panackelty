---
name: repository-health
description: Audit repository housekeeping after a major or principal task and when asked whether a repository is healthy. Check dangling pull requests, stale or incorrectly open issues, merged work whose bookkeeping is incomplete, branch clutter, and protected/default-branch state. Summarise findings and distinguish actionable problems from intentional backlog.
---

# Repository Health

Perform a concise evidence-based housekeeping audit. Use current repository state rather than relying on chat history.

## Check work tracking

Inspect open pull requests. Identify abandoned, superseded, duplicate, unexpectedly draft, blocked, or otherwise dangling PRs. Do not call an active PR stale merely because it is old; use its task state, superseding work, discussion and repository conventions.

Inspect open issues against recently merged work and their own acceptance/completion text. Flag issues whose complete stated scope appears delivered but which remain open, or whose bookkeeping contradicts the roadmap. Do not close broad issues when only a slice was delivered. Treat explicit unscheduled proposals and intentional umbrella issues as healthy backlog.

Check recently merged principal work for completion bookkeeping required by the repository: closing linkage, roadmap/ledger updates, post-merge acceptance, and any explicitly promised follow-up. Distinguish a missing record from unfinished implementation.

## Check repository state

Confirm the default branch and relevant protection/required checks when the available tools expose them. Check recent default-branch CI for obvious unresolved failures when this materially affects the health conclusion.

Audit non-default branches. A branch is a deletion candidate only when its work is conclusively merged, superseded, or otherwise preserved on the default branch and it has no open PR or unique work that should be retained. Never delete a branch merely because its name resembles an old task. Preserve release/support branches when repository policy or active workflows need them.

When branch deletion is authorised and supported, remove only verified candidates. If deletion is unavailable, report the exact cleanup candidates rather than pretending they were removed. Prefer enabling automatic deletion of merged PR branches when repository policy permits it, but treat that setting as a separate repository change requiring normal authorisation.

## Report clearly

Give a compact summary covering:
- open/dangling PRs;
- open issues that look incorrectly open, plus intentional umbrella/backlog exceptions;
- incomplete completion bookkeeping or post-merge acceptance;
- default-branch/protection/CI concerns;
- stale merged branches and whether cleanup was performed;
- an overall health statement and concrete actions, if any.

Avoid equating backlog size with poor health. Separate hygiene from defects and unfinished product work.

## When to run

Run this skill when the user asks for repository health/housekeeping and after completion of a major or principal task, once its final PR is merged and post-merge acceptance has been performed or explicitly recorded. For the automatic post-task run, keep the audit lightweight unless it finds an inconsistency.

Do not require it after every small documentation edit, typo fix, intermediate PR, or routine bookkeeping change. It is a completion safeguard, not another implementation gate.

The health audit does not itself authorise issue closure, branch deletion, repository-setting changes, new implementation work, or PR merges. Follow the repository's existing approval rules for mutations.
