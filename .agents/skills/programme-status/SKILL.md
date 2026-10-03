---
name: programme-status
description: Report programme progress using all tasks, verified completion states, scope-based weights and earned contributions. Use for programme status, programme progress, overall completion, and automatically after each programme task delivery or completion. Also establish a provisional weighted baseline when planning a programme. Do not apply to unrelated single tasks or equate a team roster with programme progress.
---

# Programme status

Report the complete programme after each task delivery or completion, and when
asked. Follow the repository's planning, completion and approval rules.
Use the request and repository records to identify the programme; read its canonical
plan/tracker and current delivery evidence. For Panackelty, read
[ROADMAP.md](../../../ROADMAP.md) and the
[roadmap process](../../../docs/ROADMAP_PROCESS.md). Reuse reliable evidence
already retrieved. If several programmes could be meant, resolve from context or
ask one focused question.

## Establish evidence

List every scoped task, including completed, pending, blocked, deferred and paused
work. Preserve stable task IDs and concise names; link to authoritative records.
For Panackelty, use PR#n, GI#n and RM#n with short names. Distinguish planned,
implementation, review, merge and final acceptance. Never equate an open PR,
finished draft or completed experiment with an accepted production task.
State stale or unavailable evidence and do not invent ownership or agent activity.

## Weight the programme

When planning a programme or reporting one without a baseline, establish a
provisional baseline. Use an existing agreed baseline when present. Otherwise
estimate provisional weights from relative scope and effort: implementation,
testing, integration, migration,
documentation and acceptance. Explain major differences and uncertainty. Do not
simply divide 100% by task count. Weights must sum to 100%; choose coarse estimates
rather than spurious precision. Unknown scope should be marked uncertain, with a
provisional assumption or range where defensible, never disguised as equal weights.

Keep weights stable between reports. Explain re-estimation or scope changes and
their effect on the previous total; do not silently move the denominator. Do not
remove deferred/paused work from the denominator unless scope was explicitly
changed. Do not count both parent outcomes and their child deliveries; allocate
children within the parent's weight or show them as evidence only.

Estimate task completion from delivered sub-outcomes or acceptance criteria,
not time spent, activity, commits or PR counts. Give a brief basis for partial
credit and distinguish estimated progress from formal acceptance. Mark uncertain
completion as unknown rather than manufacture precision; report a range or known
contribution subtotal when needed. A task with remaining acceptance is not 100%
complete. Planning records alone do not complete an implementation task.

Compute earned contribution in percentage points as:

    task weight (%) × task completion (%) / 100

Sum earned contributions for overall estimated programme completion. Example:
a task weighted 20%, with evidenced completion of 50%, contributes 10 percentage
points. Accepted-task counts may accompany but never replace weighted progress.

## Present the report

Lead with overall estimated completion and the latest meaningful change. Include
a table containing every task, its status, programme weight, task completion and
earned contribution. Include completed tasks; do not show only remaining work.
Use concise status/evidence text so the full list remains readable on mobile.
Show totals, weight-baseline assumptions, material blockers and a concrete next
action. State what changed since the last report, including baseline revisions.
Keep unrelated operational activity out of project progress.

Store programme-specific baselines and evidence in the project's normal canonical
tracker when authorised; do not bake transient programme data into this skill.
Reporting does not itself authorise edits, implementation, merges, scope reduction,
new automations or messages to others. Follow the project's existing approval rules.
