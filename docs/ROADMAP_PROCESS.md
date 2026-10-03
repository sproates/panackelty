# Roadmap decision process

Use this lightweight process to choose Panackelty work deliberately. Strong
correctness and test evidence, useful capabilities, and an enjoyable developer
experience are strategic goals. Compiler assistance deserves explicit attention
beyond the presentation of error messages. Evaluate adoption by human developers
and AI coding agents, while retaining human readability and maintainability.
Use the ambition in [ROADMAP.md](../ROADMAP.md) to guide assessment, not to
claim current platform support or superiority. This process does not predetermine
which implementation should come next.

## Item states

| State | Meaning |
| --- | --- |
| Idea | Worth recording; no implementation commitment |
| Assessing | Investigating value, feasibility, effort or dependencies |
| Ready for prioritisation | Enough evidence to compare with other work |
| Planned | Priority, scope and completion criteria agreed with the user |
| In progress | Actively being implemented |
| Verification pending | Implementation merged; specified post-merge acceptance remains |
| Done | Acceptance criteria and required verification complete |
| Deferred | Deliberately postponed, with a reason and revisit trigger |

State and priority are separate: an assessed idea need not become planned work.
Record uncertainty rather than upgrading a hypothesis to a verified gap. An item
can return to assessment when new evidence changes its scope or feasibility.

## Work identifiers and references

Use **RM#n: short name** for roadmap work, **GI#n: short name** for GitHub
issues, and **PR#n: short name** for pull requests in conversation, handovers and
new or substantially rewritten records. Include a concise, descriptive name so
a reader can understand the reference without opening it. For example,
[RM#56: Performance baselines](../ROADMAP.md#rm-56) maps to
[GI#141: Performance baselines](https://github.com/sproates/panackelty/issues/141).
For another repository, include its owner/repository as well as the typed reference.
Use actual numbers and verified links; do not invent an issue for an unissued item.
GitHub closing syntax remains `Closes #n` or `Refs #n` where required for automation;
accompany it with the descriptive GI reference. Preserve literal commands, URLs,
API fields and historical quotations where changing their syntax would be wrong.
Existing historical prose need not be mechanically rewritten.

Assign each independently tracked roadmap initiative, proposal, workstream,
delivery milestone or follow-up one stable RM number and short name, whether or
not it has a GitHub issue. Completed work retains its identity. A checklist of
acceptance criteria belongs to its parent item; give a child its own ID when it
has an independently tracked outcome. Policy, review history, navigation and
repeated summaries do not receive separate IDs. A repeated entry links to its
canonical `ROADMAP.md#rm-N` anchor. Programme stage labels such as U2 remain useful
aliases; they do not replace RM identities or imply additional GitHub issues.

Allocate the next unused integer above the maximum ever allocated, using the
roadmap's next-available pointer and checking the branch against current main
before publication. Resolve concurrent allocation collisions before merging.
Never renumber IDs because items move, complete or change priority, and never
reuse a retired number. Keep a tombstone and successor link if an item is merged,
split or removed; new independently tracked outcomes receive new IDs. Rename a
short name only deliberately, preserving its ID and stable anchor.

Keep GitHub mappings explicit and distinct: one umbrella issue may cover several
roadmap outcomes, and a roadmap item may have no issue or several related issues.
An issue or PR number is never inferred from an RM number. Preserve existing
section headings and old anchors when adding IDs so external links keep working.
Before delivery, check unique RM allocations and anchors, coverage of independent
work entries, consistent repeated references and the next-available pointer,
alongside the applicable documentation and link validation. Numbering alone does
not select implementation, alter priority or grant merge approval.

## Short candidate assessment

Use enough detail to make a decision, proportional to the size and uncertainty
of the work. A small, well-understood improvement does not need a design essay.

- **Problem and evidence:** affected users or maintainers, observed friction,
  current repository evidence, and whether this is an observed defect, verified
  omission, deliberate limitation, stale documentation or hypothesis.
- **Value:** practical usefulness, developer enjoyment/productivity, correctness,
  reliability or maintainability. Describe the intended observable improvement.
- **Effort:** small, medium, large or unknown; include implementation, meaningful
  tests, documentation, release/integration work and ongoing maintenance. Explain
  the estimate's assumptions; these sizes are not delivery-date commitments.
- **Risk and urgency:** consequences of implementing the change and of delaying
  it, including reversibility, compatibility and the severity of known defects.
- **Dependencies and confidence:** prerequisites, work enabled, uncertain claims
  and the evidence needed to resolve them.
- **Smallest useful outcome:** a bounded improvement or feasibility experiment,
  explicit non-goals and acceptance evidence. A useful design decision can be
  the outcome of an assessment PR without claiming an implemented feature.

Unknown effort calls for a bounded investigation where justified; it does not
make an item automatically high or low priority. Execution coverage and assertion
quality are distinct. Neither coverage percentages nor test counts alone define
correctness or completion.

## Comparing and scheduling work

Compare candidates using the assessment above and record a short explanation of
why one comes before another. Do not substitute a single numerical score for
judgment about user value, effort, risk, dependencies and confidence.

Give useful compiler assistance and enjoyable development explicit weight
alongside strong testing. Evaluate proposed assistance on representative tasks,
including discoverability, precision, false positives and feedback latency.
Preserve room for inexpensive, valuable improvements; a large infrastructure
initiative must not indefinitely block all other useful work.

Use **Now / Next / Later** for scheduled priorities, keeping unscheduled ideas
and deferred items separate. Keep one principal implementation initiative active
unless there is a clear, recorded reason to overlap work. Consider known defects
promptly according to actual risk rather than blindly following queue order.

The assistant maintains evidence and proposes changes. The user and assistant
agree priorities in conversation; provide a concise review there so opening a
PR is not required to understand the decision. Discussion or process approval
is not approval for every listed feature. PR approval and permission to merge
remain explicit; agreement on priorities alone does not authorize a merge.

## Document roles and ownership

| Document | Role |
| --- | --- |
| `ROADMAP.md` | Authoritative current item state, agreed priority, concise rationale and links |
| This process | Rules for assessment, prioritisation and review |
| GitHub Issues | Detailed work records, investigation evidence and discussion linked from the roadmap |
| Supporting proposals | Detailed design, assumptions and options linked from an item |
| `tests/COVERAGE.md` | Behavioral test evidence and identified test gaps; links to roadmap priority |
| `tests/VALIDATION_PROFILE.md` | Reproducible performance observations and historical experiments |
| `SELF_HOSTING.md` | Bootstrap evidence and milestone history |
| `CHANGELOG.md` and release records | Delivered release changes |

The assistant handling a change is responsible for reconciling affected status
and evidence as part of that change's handover, with priority decisions owned
by the user. Keep one authoritative status per active item in `ROADMAP.md`;
supporting documents link to it rather than maintaining competing priority lists.
Historical snapshots may retain their original status when clearly labelled as
historical. Existing duplicated or stale entries should be reconciled during
the roadmap refresh, not silently treated as current commitments.

## Programme tracking

Use the repository [Programme Status skill](../.agents/skills/programme-status/SKILL.md)
when planning a programme, reporting progress, and at every programme task delivery
or completion. `ROADMAP.md` owns the programme scope, baseline and concise current
evidence; linked issues hold delivery detail. Report every scoped task, including
accepted, blocked, deferred and paused work, with its stable RM identity, state,
programme weight, evidenced task completion and earned contribution. Keep stage
aliases and GI/PR mappings explicit. Accepted means Done under the item states;
In review describes delivery awaiting review or merge, not accepted work.

Establish coarse provisional weights from relative implementation, testing,
integration, migration, documentation and acceptance effort when planning. Explain
uncertainty and major differences; do not divide equally by task count. Weights
sum to 100%. Retain the baseline between reports; explain re-estimation or scope
changes and their effect on the previous total. Paused/deferred scope stays in the
denominator unless explicitly removed. Count parent outcomes or allocated child
weights, never both. Programme tracking does not change priority or start work.

Task completion follows delivered criteria, not time, commits, PR counts or
activity. Explain partial credit; remaining acceptance prevents 100% completion.
Planning alone earns no implementation credit, though an explicitly scoped design
task can be accepted against its own criteria. Earned contribution in percentage
points is weight (%) times task completion (%) divided by 100; sum contributions
for estimated overall completion. Distinguish this estimate from formal acceptance.
If evidence is unavailable or uncertain, show unknown completion and a defensible
range or known subtotal, not an invented exact overall percentage.

Each report leads with the total and latest meaningful change, includes the full
task table and baseline assumptions, and states blockers and the next action.
Update the canonical register as part of authorised delivery and preserve evidence
links. Reporting alone does not authorise tracker edits, implementation, scope
reduction, merges, automation or messages to others. Follow existing approval rules.

## GitHub Issues workflow

Start from this repository, not remembered conversations. Read this process and
[ROADMAP.md](../ROADMAP.md), then follow the roadmap's item links to
[GitHub Issues](https://github.com/sproates/panackelty/issues). The repository is
authoritative for ambition, agreed priorities, item state, accepted decisions
and completion summaries. Issues hold detailed tasks and discussion; essential
decisions must not exist only in a chat, issue comment or GitHub Project.

1. Search open and closed issues and the roadmap before creating an item.
   Use **Roadmap proposal** for features, investigations and maintenance; use
   **Bug report** for defects and SECURITY.md for sensitive reports. Link the
   matching roadmap section and related issues/PRs. Split independently
   deliverable work; do not bulk-migrate the historical backlog.
2. In the issue body, record the candidate assessment above: problem/evidence,
   users and agents affected, value, effort, risk, dependencies/confidence,
   smallest outcome/non-goals and acceptance evidence. Unknowns are acceptable.
   Identify the scope as a feature, investigation, maintenance or defect.
3. Treat new proposals as **Idea** unless another state is explicitly agreed.
   Open/closed is issue lifecycle, not roadmap priority. Labels and Projects
   may aid discovery but are optional and cannot override the repository.
4. Before scheduling implementation, bring a concise recommendation to the user
   in chat. Record accepted priority, rationale, state and issue link in a
   roadmap PR. A linked issue need not duplicate the authoritative priority.
   If records conflict, resolve against the repository and document any newly
   agreed change; do not silently promote a discussion into a commitment.
5. Link implementation PRs to their issue and roadmap item. Apply the
   [completion handover](#completion-handover) before requesting merge approval:
   the final delivery PR includes its completion summary and closing keywords
   unless specified post-merge acceptance remains.
6. Verify issue closure after the acceptance evidence is complete and the
   repository completion summary merges. For duplicates or rejected proposals, record
   the reason and canonical link; preserve significant decisions in the repo.
   Deferred work keeps a reason and revisit trigger, not an implied deadline.

Adopt incrementally: after this workflow PR is merged, create/link issues for
the expanded assessment and bounded experiment first. Move other active items
when they are assessed; preserve older roadmap detail until it has a useful
replacement. Each migrated item retains a concise repository summary, state,
dependencies, agreed rationale and outcome, plus its issue URL. This preserves
planning continuity if GitHub is unavailable; detailed discussion remains on
GitHub, so a future full migration would require an explicit export/backup plan.
No GitHub Project, bulk migration or new labels are required by this change.

If issue access is unavailable, record the limitation and proposed work in the
repository PR; never invent issue numbers or claim updates succeeded. Do not
change repository settings or bypass access controls to complete planning.
Use feature branches, GitHub noreply commit identity and explicit user permission
for each PR merge. Approval of this process does not authorise feature delivery,
settings changes or subsequent merges.

## Website impact and follow-ups

For every core change and release, the author or agent must assess whether the
website needs an update. Consider syntax, APIs, semantics, examples, supported
platforms, installation commands, download links and version/feature claims.
Record the outcome in the PR: no impact with a reason, or a link to a concrete
entry in the [website follow-up register](../ROADMAP.md#website-follow-up-register).
An existing entry can be updated rather than duplicated. Record it in the same
PR as the core change, before completion handover; do not leave it only in chat.
The author or agent owns recording and handover until a named maintainer accepts
ownership. An unknown release version must be recorded as a prerequisite, not
invented as a download target.

Distinguish two cases:

- **Promotion pending:** the published site remains accurate for its explicitly
  identified, pinned release. Record what should change when a newer release is
  deliberately adopted. This is unscheduled work and does not block core delivery.
- **Correctness defect:** an existing link is broken, an example fails for its
  advertised version, or a published claim is false or misleading. Record a defect
  promptly and bring the correction to the user for prioritisation. Do not treat
  it as harmless version lag or wait for the repository split. If a proposed
  release or asset change would break the current site, preserve the existing
  contract or coordinate a verified correction before that change is published.

Recording an entry does not authorise implementation, deployment, a scheduled
reminder or automatic version promotion. Review pending entries during release
preparation, website changes and roadmap grooming. Corrections still follow the
normal PR and explicit merge-approval rules.

The website must describe the versions it actually offers. A newer core release
may leave it unchanged, provided old artifacts remain available and claims stay
true. Keep release pins and download links explicit; never publish a link to a
planned or missing release. Check public artifact availability and checksums,
run examples with the declared native/browser release, and review syntax,
capability and "latest" claims before publishing. Machine checks do not establish
that explanatory prose is truthful. Retain referenced release artifacts; if a
withdrawal is necessary, coordinate replacement or removal of affected links
before withdrawal. Keep correction work open until live verification confirms
links, examples, version labels and relevant claims are accurate.

This is a contribution and review requirement now. Automated website gates and
independent publishing remain part of the unscheduled website proposal.

## Review and completion

Review priorities when the user requests it or when significant new evidence
changes assumptions, dependencies, scope, effort or the value of the next task.
Keep reviews proportionate: a focused decision can be enough; a full backlog
rewrite is not required. Record the date, reason, decision and important deferrals
in a short roadmap note. Do not treat silence as agreement to a new priority.

There is no task-count threshold, completion counter, reset baseline or mandatory
review after a fixed number of deliverables. Continue the agreed work while its
priority and assumptions remain valid. Completion bookkeeping remains part of
every delivery. A review does not grant permission to merge, schedule automation,
or implement unselected backlog items.

Before implementation, agree acceptance evidence and relevant failure cases.
Finish against that scope, not broad wording such as "comprehensive". Follow
repository validation requirements; informational documentation uses its
applicable checks, while implementation work retains canonical validation.

### Independent review

Before requesting merge approval for a substantive delivery PR, obtain an
independent review from someone other than its implementer, proportionate to the
changed scope. This includes code, tests, CI/configuration, releases, design, public
contracts and contributor instructions. Routine status-only or typo changes may
use self-review.
Review correctness, acceptance evidence, regression risks and affected records;
for documentation, review substantive claims and process consistency as well as
links. Record the exact reviewed commit SHA or tree, findings, their resolution
and any remaining limitations in the delivery evidence. If independent review is
unavailable, report that gap rather than describing self-review as independent.

Any later change invalidates coverage of the affected content until the reviewer
checks the new revision. Before merge approval, verify that the PR head matches
the reviewed revision or record evidence that its tree is identical. After merge,
verify that the delivered change matches the reviewed content, inspecting any
merge resolutions or changes affecting that content before claiming acceptance.
Identical trees suffice when the base is unchanged; unrelated changes on main do
not alone invalidate the review. Review, green checks and scope approval never replace
explicit permission to merge each PR.

### Completion handover

Completion bookkeeping belongs to the delivery work. Before requesting merge
approval, identify the PR's outcome and reconcile its issue, roadmap entry and
acceptance evidence. The PR template prompts this review; it cannot determine
whether semantic acceptance is actually complete.

| PR outcome | Required roadmap update | Issue linkage |
| --- | --- | --- |
| Final delivery; acceptance can be verified before merge | Include Done status, delivered scope and verified acceptance in this PR, effective when merged | Use `Closes #...` in the PR body for each completed issue |
| Implementation complete; acceptance requires deployment or another post-merge check | Include Verification pending, the exact remaining checks and the person or agent responsible | Use `Refs #...`; keep the issue open |
| Intermediate slice | Record delivered scope and concrete remaining work; retain In progress only while implementation is active | Use `Refs #...`; do not close the parent task |

For a final delivery PR, ensure all required checks have passed on the reviewed
head before merge approval. Write the completion summary as the resulting state
of the merged repository; the PR remains in review until it merges. Do not invent
a merge SHA or date in advance, and do not make a routine second bookkeeping PR
a prerequisite when acceptance is already complete. Optional future extensions
do not keep an otherwise completed, bounded task open.

After an explicitly authorised merge, the agent handling delivery must verify
the merged roadmap state and linked issue closure before its final handover.
If automatic closure did not occur, close the issue once acceptance and the
merged completion summary are confirmed. Keep released, implemented-unreleased
and proposed capability distinct.

Where post-merge acceptance is required, perform the authorised checks in the
same delivery session where possible. Once they pass, promptly prepare the
completion-summary PR with the evidence and closing keyword. That PR still needs
separate merge approval. If verification is blocked, retain Verification pending
with the concrete blocker, next action and responsible person or agent; do not
claim completion or defer an unexplained status to future grooming.

Every final handover states the task's completion status and any remaining
acceptance. For programme deliveries, include the complete weighted programme
report required by [programme tracking](#programme-tracking), even for an
intermediate slice; do not present an unmerged change as accepted progress. For
work without a linked issue, record that fact; do not create an issue solely for
closure. These rules apply to investigations and documentation
tasks as well as implementation. An investigation closes against its agreed
decision or evidence, not delivery of the feature it investigated.

## Adoption sequence

1. Agree this process in conversation and document it in a small PR.
2. After adopting the process, perform the holistic gap assessment with its
   criteria, including a review of existing roadmap documents and practice.
3. Compare findings and agree priorities in conversation.
4. Update the roadmap with the agreed work and deliberate deferrals.

The process was adopted in PR #83 on 2026-09-29. In the subsequent discussion,
the user agreed the broad general-purpose ambition and explicit adoption focus
on developers and AI coding agents, and approved repository-led use of Issues.
This refresh records that direction and defines the workflow. An initial review
has not settled implementation priorities: the expanded architectural assessment
and bounded comparative experiment precede that decision. Compiler assistance,
source coverage and other candidates must still be compared on evidence, value,
effort and dependencies. REPL and further validation speed work remain lower
priority; no platform backend, toolkit or interoperability strategy is selected.
