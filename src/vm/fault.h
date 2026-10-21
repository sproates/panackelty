#ifndef PANACKELTY_FAULT_H
#define PANACKELTY_FAULT_H

/* Unknown/integrity/allocation faults are fatal by default. Only explicit
 * language-domain failures may cross a request-local recovery boundary. */
typedef enum { VM_FAULT_FATAL = 0, VM_FAULT_REQUEST } VMFaultKind;

#endif
