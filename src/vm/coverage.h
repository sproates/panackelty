#ifndef PANACKELTY_COVERAGE_H
#define PANACKELTY_COVERAGE_H

#include "program.h"

#include <stdio.h>

#define VM_COVERAGE_MAX_CELLS 262144u
#define VM_COVERAGE_MAX_RAW 16777216u

typedef struct VMCoverage VMCoverage;
typedef struct {
    uint64_t attempted, completed, fallthrough, branched;
} VMCoverageCell;

/* Opt-in, single-threaded observation. Borrows a verified program; the caller
 * owns the collector and must detach it from every VM before destroying it.
 * Limits are checked before execution. A low counter ceiling supports overflow
 * tests; production uses UINT64_MAX. Overflow marks data invalid, never wraps or
 * changes program semantics. Reusing a collector for another execution marks a
 * gap: multi-execution manifests/aggregation are a separate layer. */
VMCoverage *vm_coverage_create(Program *program, size_t cell_limit, uint64_t ceiling);
void vm_coverage_destroy(VMCoverage *coverage);
size_t vm_coverage_bytes(const VMCoverage *coverage);
void vm_coverage_start(VMCoverage *coverage, Program *program);
size_t vm_coverage_function(VMCoverage *coverage, Function *function);
void vm_coverage_enter(VMCoverage *coverage, size_t function);
void vm_coverage_attempt(VMCoverage *coverage, size_t function, size_t pc);
void vm_coverage_finish(VMCoverage *coverage, size_t function, size_t pc, bool branched);
void vm_coverage_gap(VMCoverage *coverage);
/* 0 partial, 1 returned, 2 trapped, 3 explicit exit. */
void vm_coverage_terminal(VMCoverage *coverage, unsigned terminal);
unsigned vm_coverage_flags(const VMCoverage *coverage);
uint64_t vm_coverage_entries(const VMCoverage *coverage, size_t function);
const VMCoverageCell *vm_coverage_cell(const VMCoverage *coverage, size_t function, size_t pc);
/* Canonical big-endian raw record. Identity bytes are opaque and never parsed;
 * source attribution must reproduce them locally. Size is bounded before run. */
bool vm_coverage_raw_size(const VMCoverage *coverage, size_t artifact_size,
                          size_t inventory_size, size_t *size);
bool vm_coverage_write(const VMCoverage *coverage, FILE *output,
                       const uint8_t *artifact, size_t artifact_size,
                       const uint8_t *inventory, size_t inventory_size);

#endif
