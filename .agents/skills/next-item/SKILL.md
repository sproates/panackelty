---
name: next-item
description: Compare viable next tasks and recommend what to do next in a project. Use when the user asks "what's next", "what's the next item", "what should we work on next", or similar prioritisation questions. Provide reasons for each candidate, a comparative recommendation, rough task size, estimated PR count, and an ELI5 explanation. Do not trigger for simple navigation or the next command in an already agreed procedure.
---

# Next Item

Help the user make an informed choice, rather than presenting one task for approval without alternatives. Apply across projects and respect their existing instructions, authentication, privacy and contribution workflows.

## Establish the current position

Read relevant project instructions and the authoritative backlog, roadmap or issue tracker. Check recent completed work, active PRs and dependencies using available authorised tools. Reuse reliable context, but verify mutable state before claiming an item remains open or a dependency is complete.

Identify the current objective and constraints from the conversation and project records. Distinguish agreed priorities from your own proposed priorities. State material assumptions or unavailable evidence. Do not invent backlog items, completion status or estimates. You may suggest a new candidate, clearly labelled as a proposal.

Ask a focused question only if missing information would materially change the decision and cannot be inferred. Otherwise proceed with stated assumptions.

## Compare actual candidates

Select roughly three to five genuinely viable candidates, or fewer when fewer exist. Do not pad the list. Consider competing needs across the project, rather than automatically continuing the last technical theme. Exclude blocked work from the viable shortlist; mention a significant excluded option and its blocker where useful.

For each candidate, explain:

- The concrete outcome and bounded scope.
- Why it is worth doing now, with evidence such as a user need, known defect, roadmap commitment, or dependency it unlocks.
- Rough size, including implementation, testing, documentation and integration. Use small/medium/large with a brief explanation of what drives the estimate. Give time ranges only when there is a credible basis, with assumptions.
- Estimated number of PRs, preferably a range where uncertain, and what drives the split. Use zero for work that needs no repository change. Do not promise an exact count before understanding the scope.

Estimate the same scope that you describe. If an initiative is too broad, propose a useful first slice and distinguish its estimate from the whole initiative. If discovery is necessary, estimate that bounded discovery task and mark the subsequent implementation as uncertain.

## Recommend and explain the trade-off

Choose one candidate explicitly. Explain why it outranks each alternative against the current objective: user value, urgency, risk reduction, dependencies, effort and readiness. Use qualitative reasoning rather than invented numerical scores. Lower priority does not mean an alternative is unimportant.

Identify the strongest alternative and the circumstance that would change your recommendation. State the recommended task's completion condition and, for multiple PRs, their likely sequence and purpose. Make clear whether the result is a proposal, experiment or working feature.

Provide an ELI5 explanation of the recommended task: what happens today, what we will change, and how that helps. Use a concrete example or simple analogy where helpful. Avoid unexplained jargon and a patronising tone.

## Response shape

Keep the response proportionate to the decision. A useful default is:

1. A compact comparison table: candidate and outcome; why worthwhile now; rough size; estimated PRs. Include concise source links when available.
2. The recommendation and explicit reasons the other candidates rank lower, plus any assumption that could change the ranking.
3. The completion condition and likely PR breakdown for the recommended task.
4. A short ELI5 explanation.

Include all five requested elements even in a short response: viable alternatives and reasons, comparative recommendation, task size, PR count, and ELI5.

A request for prioritisation is not by itself authorisation to implement, create issues, schedule work or merge PRs. Follow existing authorisation and project rules without introducing an additional approval process. If the question arrives during already authorised work, treat it as steering rather than silently abandoning that work.
