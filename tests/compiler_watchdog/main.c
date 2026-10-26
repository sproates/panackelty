#include "platform.h"
#include "budget.h"

#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

/* Native test supervisor. It needs no interpreter and changes neither the seed
 * nor the public launcher. All evidence survives failure under BUILD_DIR. */
#define OUTPUT_LIMIT 1048576
#define TRACE_LIMIT (16 * 1048576)
static volatile sig_atomic_t interrupted;
static void interrupted_signal(int number) { interrupted = number; }
static double monotonic(void)
{
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now)) { perror("clock_gettime"); exit(125); }
    return (double)now.tv_sec + (double)now.tv_nsec / 1000000000.0;
}
static void pause_briefly(void)
{
    struct timespec delay = {.tv_nsec = 20000000};
    nanosleep(&delay, NULL);
}
static bool number(const char *text, double *value)
{
    char *end;
    errno = 0;
    *value = strtod(text, &end);
    return *text && !*end && !errno && isfinite(*value);
}
static bool record(WatchdogBudget *budget, char *line)
{
    char *fields[9], *cursor = line;
    for (size_t i = 0; i < 9; i++) {
        fields[i] = cursor;
        char *tab = strchr(cursor, '\t');
        if (i == 8) {
            if (tab) return watchdog_fail(budget, "Malformed progress record.");
        } else {
            if (!tab) return watchdog_fail(budget, "Incomplete progress record.");
            *tab = 0;
            cursor = tab + 1;
        }
    }
    bool entering = !strcmp(fields[1], "enter");
    if (!entering && strcmp(fields[1], "return")) return watchdog_fail(budget, "Unknown progress event.");
    double clocks[4];
    for (size_t i = 0; i < 4; i++) {
        if (!number(fields[i + 3], &clocks[i])) return watchdog_fail(budget, "Invalid progress timestamp.");
    }
    return watchdog_event(budget, entering, fields[2], fields[7], fields[8], clocks[0]);
}
typedef struct {
    char line[WATCHDOG_TEXT * 3];
    size_t used, total;
} Reader;
static bool read_progress(FILE *input, Reader *reader, WatchdogBudget *budget, bool final)
{
    clearerr(input);
    int c;
    while ((c = fgetc(input)) != EOF) {
        if (++reader->total > TRACE_LIMIT || reader->used + 1 >= sizeof(reader->line))
            return watchdog_fail(budget, "Progress output exceeds the observer limit.");
        if (c == '\n') {
            reader->line[reader->used] = 0;
            if (!record(budget, reader->line)) return false;
            reader->used = 0;
        } else {
            reader->line[reader->used++] = (char)c;
        }
    }
    if (ferror(input)) return watchdog_fail(budget, "Cannot read compiler progress.");
    if (final && reader->used) return watchdog_fail(budget, "Compiler left a partial progress record.");
    return true;
}
static bool within_output_limit(const char *path)
{
    struct stat info;
    return !stat(path, &info) && info.st_size <= OUTPUT_LIMIT;
}
static bool relay(const char *path, FILE *destination)
{
    FILE *input = fopen(path, "rb");
    if (!input) return false;
    char buffer[4096];
    size_t count, total = 0;
    bool ok = true;
    while ((count = fread(buffer, 1, sizeof(buffer), input))) {
        if (total + count > OUTPUT_LIMIT) { ok = false; break; }
        if (fwrite(buffer, 1, count, destination) != count) { ok = false; break; }
        total += count;
    }
    if (ferror(input)) ok = false;
    fclose(input);
    return ok;
}
static void stop_group(pid_t child)
{
    kill(-child, SIGTERM);
    double until = monotonic() + 0.25;
    while (monotonic() < until) pause_briefly();
    kill(-child, SIGKILL);
}
static int self_test(void)
{
    WatchdogBudget b;
#define EXPECT(condition) do { if (!(condition)) { fprintf(stderr, "watchdog contract failed: %s\n", #condition); return 1; } } while (0)
    watchdog_init(&b, 90, 0);
    EXPECT(watchdog_event(&b, true, "load_project", "", "", 0));
    for (unsigned i = 0; i < 5; i++) {
        EXPECT(watchdog_event(&b, true, "check_module_bodies_explained", "healthy.panack", "", i * 40));
        EXPECT(watchdog_event(&b, false, "check_module_bodies_explained", "healthy.panack", "", (i + 1) * 40));
    }
    EXPECT(watchdog_event(&b, false, "load_project", "", "", 200));
    EXPECT(watchdog_finish(&b, 201));
    watchdog_init(&b, 2, 0);
    EXPECT(watchdog_event(&b, true, "check_module_bodies_explained", "stuck.panack", "", 0));
    EXPECT(watchdog_event(&b, true, "bind_body_block", "stuck.panack", "loop", 1));
    EXPECT(watchdog_event(&b, false, "bind_body_block", "stuck.panack", "loop", 1.5));
    EXPECT(!watchdog_check(&b, 2.1));
    EXPECT(strstr(b.error, "stuck.panack") && strstr(b.error, "loop"));
    watchdog_init(&b, 2, 0);
    EXPECT(watchdog_event(&b, true, "compile_program", "", "", 0));
    EXPECT(!watchdog_event(&b, false, "compile_program", "", "", 3));
    watchdog_init(&b, 2, 0);
    EXPECT(watchdog_event(&b, true, "load_project", "", "", 0));
    EXPECT(watchdog_event(&b, true, "lex_file", "a.panack", "", 1));
    EXPECT(watchdog_event(&b, false, "lex_file", "a.panack", "", 2));
    EXPECT(!watchdog_check(&b, 3.1));
    watchdog_init(&b, 2, 0);
    EXPECT(!watchdog_finish(&b, 1));
    EXPECT(!watchdog_check(&b, 3));
    watchdog_init(&b, 2, 0);
    EXPECT(!watchdog_event(&b, true, "check_module_signatures", "", "", 0));
    EXPECT(watchdog_event(&b, true, "check_module_signatures", "a", "", 0));
    EXPECT(!watchdog_event(&b, false, "check_module_signatures", "b", "", 1));
    EXPECT(!watchdog_finish(&b, 1));
    /* Polling must not advance the event clock and double-charge buffered work. */
    watchdog_init(&b, 2, 0);
    EXPECT(watchdog_check(&b, 1));
    EXPECT(watchdog_event(&b, true, "compile_program", "", "", 0.5));
    EXPECT(watchdog_event(&b, false, "compile_program", "", "", 1.5));
    EXPECT(watchdog_finish(&b, 2));
    puts("PASS compiler progress budget contracts");
    return 0;
#undef EXPECT
}
static void test_event(FILE *trace, const char *event, const char *phase,
                       const char *module, const char *function)
{
    fprintf(trace, "%ld\t%s\t%s\t%.6f\t0\t0\t0\t%s\t%s\n",
            (long)getpid(), event, phase, monotonic(), module, function);
    fflush(trace);
}
static int test_child(const char *mode)
{
    const char *path = getenv("PANACK_COMPILER_TRACE");
    if (!path) return 125;
    FILE *trace = fopen(path, "a");
    if (!trace) return 125;
    if (!strcmp(mode, "healthy")) {
        test_event(trace, "enter", "load_project", "", "");
        for (unsigned i = 0; i < 4; i++) {
            test_event(trace, "enter", "check_module_bodies_explained", "healthy.panack", "");
            double until = monotonic() + 0.2;
            while (monotonic() < until) pause_briefly();
            test_event(trace, "return", "check_module_bodies_explained", "healthy.panack", "");
        }
        test_event(trace, "return", "load_project", "", "");
    } else if (!strcmp(mode, "heartbeats")) {
        test_event(trace, "enter", "check_module_bodies_explained", "stuck.panack", "");
        for (;;) {
            test_event(trace, "enter", "bind_body_block", "stuck.panack", "loop");
            pause_briefly();
            test_event(trace, "return", "bind_body_block", "stuck.panack", "loop");
        }
    } else if (!strcmp(mode, "silent")) {
        signal(SIGTERM, SIG_IGN);
        for (;;) pause();
    } else if (!strcmp(mode, "partial")) {
        fputs("partial progress", trace);
    } else if (strcmp(mode, "missing")) {
        fclose(trace);
        return 125;
    }
    fclose(trace);
    return 0;
}
int main(int argc, char **argv)
{
    if (argc == 2 && !strcmp(argv[1], "--self-test")) return self_test();
    if (argc == 3 && !strcmp(argv[1], "--test-child")) return test_child(argv[2]);
    double seconds;
    if (argc < 4 || !number(argv[1], &seconds) || seconds <= 0 || seconds > 600) {
        fputs("usage: compiler-watchdog SECONDS EVIDENCE_DIRECTORY EXECUTABLE [ARGUMENTS...]\n", stderr);
        return 125;
    }
    char directory[WATCHDOG_TEXT], trace[WATCHDOG_TEXT], out[WATCHDOG_TEXT], err[WATCHDOG_TEXT], report[WATCHDOG_TEXT];
    if (strlen(argv[2]) > WATCHDOG_TEXT - 80) return 125;
    snprintf(directory, sizeof(directory), "%s/run.XXXXXX", argv[2]);
    if (!mkdtemp(directory)) { perror("create compiler evidence directory"); return 125; }
#define EVIDENCE_PATH(target, suffix) do { strcpy(target, directory); strcat(target, suffix); } while (0)
    EVIDENCE_PATH(trace, "/progress.tsv");
    EVIDENCE_PATH(out, "/stdout.txt");
    EVIDENCE_PATH(err, "/stderr.txt");
    EVIDENCE_PATH(report, "/result.txt");
#undef EVIDENCE_PATH
    FILE *initial = fopen(trace, "w");
    int output = open(out, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    int errors = open(err, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (!initial || output < 0 || errors < 0) { perror("create compiler evidence files"); return 125; }
    fclose(initial);
    FILE *input = fopen(trace, "r");
    if (!input) return 125;
    WatchdogBudget budget;
    double started = monotonic();
    watchdog_init(&budget, seconds, started);
    struct sigaction action = {.sa_handler = interrupted_signal};
    sigemptyset(&action.sa_mask);
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGTERM, &action, NULL);
    pid_t child = fork();
    if (child < 0) { perror("fork compiler"); return 125; }
    if (!child) {
        setpgid(0, 0);
        signal(SIGINT, SIG_DFL); signal(SIGTERM, SIG_DFL);
        fclose(input);
        if (dup2(output, STDOUT_FILENO) < 0 || dup2(errors, STDERR_FILENO) < 0 ||
            setenv("PANACK_COMPILER_TRACE", trace, 1)) _Exit(125);
        close(output); close(errors);
        execv(argv[3], &argv[3]);
        perror("execute observed compiler"); _Exit(125);
    }
    setpgid(child, child);
    close(output); close(errors);
    Reader reader = {0};
    bool failed = false, reaped = false;
    int status = 0;
    while (!reaped) {
        pid_t result = waitpid(child, &status, WNOHANG);
        if (result == child) reaped = true;
        else if (result < 0 && errno != EINTR) { watchdog_fail(&budget, "Cannot wait for compiler."); failed = true; }
        if (interrupted) { watchdog_fail(&budget, "Compiler watchdog was interrupted."); failed = true; }
        if (!failed && !read_progress(input, &reader, &budget, reaped)) failed = true;
        if (!failed && !watchdog_check(&budget, monotonic())) failed = true;
        if (!failed && (!within_output_limit(out) || !within_output_limit(err))) {
            watchdog_fail(&budget, "Compiler output exceeds the one-megabyte test limit."); failed = true;
        }
        if (failed) {
            stop_group(child);
            if (!reaped) { while (waitpid(child, &status, 0) < 0 && errno == EINTR) {} }
            break;
        }
        if (!reaped) pause_briefly();
    }
    fclose(input);
    int code = WIFEXITED(status) ? WEXITSTATUS(status) : 128 + WTERMSIG(status);
    if (!failed && code == 0 && !watchdog_finish(&budget, monotonic())) failed = true;
    bool relayed_out = relay(out, stdout), relayed_err = relay(err, stderr);
    if (!relayed_out || !relayed_err) {
        if (!failed) watchdog_fail(&budget, "Cannot relay compiler output.");
        failed = true;
    }
    FILE *summary = fopen(report, "w");
    if (!summary) { perror("save compiler watchdog result"); return 125; }
    fprintf(summary, "elapsed_seconds=%.6f\ncompleted_units=%zu\nchild_status=%d\nwatchdog_failed=%d\n",
            monotonic() - started, budget.completed, code, failed);
    fputs("command:", summary);
    for (int i = 3; i < argc; i++) fprintf(summary, " %s", argv[i]);
    fputc('\n', summary);
    if (failed) {
        fprintf(summary, "%s\n", budget.error);
        fprintf(stderr, "%s\nCompiler evidence: %s\n", budget.error, directory);
    } else if (code != 0) {
        fprintf(stderr, "Compiler command failed (status %d). Evidence: %s\n", code, directory);
    }
    fclose(summary);
    return failed ? 124 : code;
}
