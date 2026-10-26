#ifndef PANACK_COMPILER_WATCHDOG_BUDGET_H
#define PANACK_COMPILER_WATCHDOG_BUDGET_H

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define WATCHDOG_TEXT 4096
#define WATCHDOG_DEPTH 32
typedef struct {
    unsigned phase;
    char module[WATCHDOG_TEXT];
    double used;
} WatchdogUnit;
typedef struct {
    WatchdogUnit units[WATCHDOG_DEPTH];
    size_t depth, completed;
    double seconds, last_event;
    char function[WATCHDOG_TEXT], completed_text[WATCHDOG_TEXT];
    char error[WATCHDOG_TEXT * 4];
} WatchdogBudget;

static const char *const watchdog_phases[] = {
    "startup or between stages", "load_project", "prepare_loaded_project",
    "compile_program", "serialize_bytecode", "check_module_effects",
    "lex_file", "parse_module_complete", "check_module_signatures",
    "check_module_guards", "check_module_bodies_explained"
};

static bool watchdog_fail(WatchdogBudget *budget, const char *message)
{
    snprintf(budget->error, sizeof(budget->error), "%s", message);
    return false;
}

static void watchdog_init(WatchdogBudget *budget, double seconds, double now)
{
    memset(budget, 0, sizeof(*budget));
    budget->depth = 1;
    budget->seconds = seconds;
    budget->last_event = now;
}

/* Checking the current clock does not charge it permanently. A later read of
 * buffered progress can therefore account for exact child event boundaries. */
static bool watchdog_check(WatchdogBudget *budget, double now)
{
    if (now < budget->last_event) return watchdog_fail(budget, "Progress clock moved backwards.");
    WatchdogUnit *unit = &budget->units[budget->depth - 1];
    double elapsed = unit->used + now - budget->last_event;
    if (elapsed < budget->seconds) return true;
    snprintf(budget->error, sizeof(budget->error),
        "Compiler progress watchdog stopped %s after %.3f seconds of active work (limit %.3fs).\n"
        "Module: %s\nLast function observed: %s\nLast completed work: %s\n",
        watchdog_phases[unit->phase], elapsed, budget->seconds,
        unit->module[0] ? unit->module : "whole compiler stage",
        budget->function[0] ? budget->function : "none in this stage",
        budget->completed_text[0] ? budget->completed_text : "none");
    return false;
}

static const char *watchdog_plain(const char *name)
{
    if (!strncmp(name, "$module", 7)) {
        const char *separator = strchr(name, '_');
        if (separator) return separator + 1;
    }
    return name;
}

static bool watchdog_event(WatchdogBudget *budget, bool entering, const char *name,
                           const char *module, const char *function, double now)
{
    if (!watchdog_check(budget, now)) return false;
    budget->units[budget->depth - 1].used += now - budget->last_event;
    budget->last_event = now;
    name = watchdog_plain(name);
    if (!strcmp(name, "bind_body_block")) {
        if (entering) {
            int length = snprintf(budget->function, sizeof(budget->function), "%s::%s", module, function);
            if (length < 0 || (size_t)length >= sizeof(budget->function))
                return watchdog_fail(budget, "Function identity exceeds the observer limit.");
        }
        return true; /* Function activity never renews a module allowance. */
    }
    unsigned phase = 0;
    for (unsigned i = 1; i < sizeof(watchdog_phases) / sizeof(watchdog_phases[0]); i++) {
        if (!strcmp(name, watchdog_phases[i])) { phase = i; break; }
    }
    if (!phase) return true;
    if (phase >= 6 && !*module) return watchdog_fail(budget, "Module progress has no source path.");
    if (entering) {
        if (budget->depth == WATCHDOG_DEPTH || strlen(module) >= WATCHDOG_TEXT)
            return watchdog_fail(budget, "Progress nesting or module identity exceeds the observer limit.");
        WatchdogUnit *unit = &budget->units[budget->depth++];
        *unit = (WatchdogUnit){.phase = phase};
        memcpy(unit->module, module, strlen(module) + 1);
        budget->function[0] = 0;
    } else {
        WatchdogUnit *unit = &budget->units[budget->depth - 1];
        if (budget->depth == 1 || unit->phase != phase || strcmp(unit->module, module))
            return watchdog_fail(budget, "Progress return does not match its active module/stage.");
        snprintf(budget->completed_text, sizeof(budget->completed_text),
                 "%s %.3fs: %.3800s", name, unit->used, module);
        budget->depth--;
        budget->completed++;
    }
    return true;
}

static bool watchdog_finish(WatchdogBudget *budget, double now)
{
    if (!watchdog_check(budget, now)) return false;
    if (!budget->completed || budget->depth != 1)
        return watchdog_fail(budget, "Compiler exited without complete progress evidence.");
    return true;
}
#endif
