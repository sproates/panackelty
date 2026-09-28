# Delivery-pilot result data

See [the report](../../AGENT_DELIVERY_PILOT.md) for method, interpretation and
limitations. [results.json](results.json) holds per-trial outcomes, case counts,
command counts and elapsed observations. [provenance.json](provenance.json)
records the source baseline, toolchain versions and runtime hashes.

## Raw evidence access and reproducibility

The maintainer has a private archive named
`panackelty-agent-pilot-evidence-2026-09-29.tar.gz`, SHA-256:
`f143f6a470d9edcd56bf3158bdffbe8726f4be130ac073e8938669cbab39c8cb`.
It contains task/maintenance prompts, command logs, participant notes,
initial/final application and self-test sources, evaluator, failure control and
restoration helper. Binaries/caches and large generated fixtures are excluded;
the fixture-generation commands are retained. Python sources exist only in that
frozen comparison evidence, not as a repository build or CI dependency.

Automatic approval review rejected public upload of the raw archive because it
contains prompts, command logs and environment details. It is deliberately not
included in this public repository. Ask the maintainer for authorised access;
do not upload raw logs or reconstruct a public copy without approval. Essential
findings, limitations and proposed priorities remain in the repository.

Restoration and replay were checked locally for one Panackelty inventory and
one Python HTTP submission. The public summary alone is insufficient to rerun
the acceptance suite; the private archive includes `control/restore.cjs`, which
materialises frozen sources into a new directory and links a built baseline
checkout, followed by `control/evaluate.cjs TRIAL_ID initial|final`.

Baseline commit: `f1b083f0be3a70ae0677feb268f56d3b556b7807`.
The original environment used Node 24.19.0, Python 3.12.14 and a prebuilt native
Panackelty toolchain. Replaying frozen submissions can reproduce acceptance
observations. Fresh model output is not reproducible: serving revision,
sampling seed and token telemetry were unavailable. Raw command traces contain
original absolute scratch paths and must not be blindly run as shell scripts.
