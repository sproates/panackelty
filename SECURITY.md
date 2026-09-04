# Security policy

Panackelty is an experimental compiler and native bytecode runtime. Only the
latest published developer preview is supported with security fixes.

## Reporting a vulnerability

Do not open a public issue for a suspected vulnerability. Use the repository's
private **Report a vulnerability** form under its Security tab. Include:

- the output of `panack --version`;
- the operating system and processor architecture;
- the smallest source or bytecode input that demonstrates the issue;
- the observed impact and reproduction steps; and
- whether the report concerns untrusted bytecode, file access, memory safety,
  resource exhaustion, or another boundary.

The maintainers will acknowledge the report, assess whether it affects the
latest preview, and coordinate disclosure and a fixed release when appropriate.
Until a public repository with private vulnerability reporting is available,
security reports should be kept private rather than filed in another project's
tracker.

## Scope

Compiler crashes, native VM memory-safety problems, verifier bypasses, unintended
file or environment access, and practical denial-of-service inputs are in scope.
Ordinary diagnostics for invalid source, documented preview limitations, and
incompatibility between separately versioned preview releases are not security
vulnerabilities by themselves.
