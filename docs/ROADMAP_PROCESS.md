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

## Review and completion

Review priorities after a milestone, significant discovery, or material change
in scope or effort, and whenever the user requests it. This is an event-driven
review rule, not a scheduled automation. Record the date, reason, decision and
important deferrals in a short decision note; do not create administrative work
without a useful decision to preserve.

Before implementation, agree acceptance evidence and relevant failure cases.
Finish against that scope, not broad wording such as "comprehensive". Follow
repository validation requirements; informational documentation uses its
applicable checks, while implementation work retains canonical validation.

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
acceptance. For work without a linked issue, record that fact; do not create an
issue solely for closure. These rules apply to investigations and documentation
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
