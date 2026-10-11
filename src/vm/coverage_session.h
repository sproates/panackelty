#ifndef PANACKELTY_COVERAGE_SESSION_H
#define PANACKELTY_COVERAGE_SESSION_H
#include "coverage.h"
typedef struct VMCoverageRun VMCoverageRun;
/* Optional native execution-tree collection. Artifact identities and the parent
 * run are borrowed by child admission. Keep a parent alive until every task or
 * server context that borrows it is destroyed; close owned runs exactly once.
 * Failed admission is observable as an incomplete manifest, never zero hits.
 * The collector returned below is borrowed and must not be destroyed separately.
 * Process tickets are heap strings owned by the caller. These local records are
 * bounded evidence, not cryptographically authenticated records. */
VMCoverageRun *vm_coverage_session_create(const char *, size_t, const char **, const char **, Program *);
VMCoverageRun *vm_coverage_session_inherit(const char *, const uint8_t *, size_t, Program *);
VMCoverageRun *vm_coverage_child(VMCoverageRun *, Program *, const uint8_t *, size_t, const char *);
bool vm_coverage_run_matches(VMCoverageRun *, const uint8_t *, size_t);
VMCoverage *vm_coverage_run_collector(VMCoverageRun *);
char *vm_coverage_process_ticket(VMCoverageRun *);
bool vm_coverage_run_close(VMCoverageRun *);
#endif
