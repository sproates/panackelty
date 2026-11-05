/* Direct contracts across the separately compiled VM modules.
 * Assertions stay enabled even when a release build supplies -DNDEBUG.
 */
#include "platform.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include "builtins.h"
#include "decode.h"
#include "numeric.h"
#include "render.h"
#include "utf8.h"
#include "verify.h"
#include "value.h"
#include "vm.h"

#include <assert.h>
#include <fcntl.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

void http_transport_contracts(void);
void http_state_contracts(void);
void http_writer_artifact(const char *path);
void root_read_contracts(void);
void console_write_contracts(void);
void resumable_contracts(void);
void coverage_contracts(void);
void coverage_session_contracts(void);
void async_contracts(void);
void tcp_contracts(void);
void tcp_server_contracts(void);
void task_contracts(void);
void task_artifact(const char *path);
void resumable_artifact(const char *path, size_t budget);

/* Persist an opt-in checkpoint so timeout reporting survives child termination. */
static void record_module_progress(const char *stage)
{
    const char *path = getenv("PANACK_NATIVE_MODULE_PROGRESS_FILE");
    if (!path || !*path) return;
    FILE *file = fopen(path, "w");
    if (!file) return;
    fprintf(file, "%s\n", stage);
    fclose(file);
}

static void values_own_copies_and_retain_children(void)
{
    uint8_t bytes[] = {'a', 0, 'b'};
    Value *child = value_data(V_BYTES, bytes, sizeof(bytes));
    assert(child && child->refs == 1);
    bytes[0] = 'z';
    assert(child->as.bytes.data[0] == 'a');

    Value *items[] = {child, child};
    Value *array = value_sequence(V_ARRAY, items, 2);
    assert(array && child->refs == 3);
    release(array);
    assert(child->refs == 1);

    char label[] = "payload";
    char *names[] = {label};
    Value *record = named_value(V_RECORD, "Box", names, items, 1);
    assert(record && child->refs == 2);
    label[0] = 'X';
    assert(strcmp(record->as.named.names[0], "payload") == 0);
    release(record);
    assert(child->refs == 1);
    release(child);
}

static void persistent_versions_keep_shared_children_alive(void)
{
    VM vm = {0};
    Value *child = value_data(V_STR, (const uint8_t *)"shared", 6);
    Value *versions[33] = {value_sequence(V_ARRAY, NULL, 0)};
    assert(child && versions[0]);
    for (size_t i = 1; i < 33; i++) {
        Value *arguments[] = {versions[i - 1], child};
        versions[i] = builtin_call(&vm, "append", arguments);
        assert(versions[i] && !vm.error);
        assert(versions[i]->as.sequence.count == i);
        assert(versions[i - 1]->as.sequence.count == i - 1);
        assert(versions[i]->as.sequence.items[i - 1] == child);
    }
    size_t owned = 1;
    for (size_t i = 1; i < 33; i++) {
        bool later_view = false;
        for (size_t j = i + 1; j < 33; j++) {
            if (versions[i]->as.sequence.items == versions[j]->as.sequence.items) {
                later_view = true;
            }
        }
        if (!later_view) {
            owned += versions[i]->as.sequence.count;
        }
        for (size_t j = 0; j < i; j++) {
            assert(versions[i]->as.sequence.items[j] == child);
        }
    }
    /* Each backing store owns its children once, including shared prefix views. */
    assert(child->refs == owned);
    /* Release in a different order from construction to expose aliasing mistakes. */
    for (size_t i = 0; i < 33; i += 2) {
        release(versions[i]);
    }
    for (size_t i = 1; i < 33; i += 2) {
        release(versions[i]);
    }
    assert(child->refs == 1 && !memcmp(child->as.bytes.data, "shared", 6));
    release(child);
}

static void append_storage_preserves_branches_and_drops_hidden_children(void)
{
    Value *item = value_size(42), *other = value_size(7);
    Value *empty = value_sequence(V_ARRAY, NULL, 0);
    Value *base = value_array_append(empty, item);
    release(empty);
    assert(base && base->as.sequence.count == 1);
    Value *extension = value_array_append(base, other);
    assert(extension && extension->as.sequence.items == base->as.sequence.items);
    assert(base->as.sequence.count == 1 && extension->as.sequence.count == 2);
    Value *branch = value_array_append(base, item);
    assert(branch && branch->as.sequence.items != base->as.sequence.items);
    assert(branch->as.sequence.items[1] == item && extension->as.sequence.items[1] == other);
    release(extension);
    assert(other->refs == 1); /* A dropped extension must not pin an invisible child. */
    extension = value_array_append(base, other);
    assert(extension && extension->as.sequence.items == base->as.sequence.items);
    release(base); /* The longer view must survive destruction of its prefix. */
    assert(extension->as.sequence.count == 2 && extension->as.sequence.items[0] == item);
    release(extension);
    release(branch);
    assert(item->refs == 1 && other->refs == 1);

    base = value_sequence(V_ARRAY, NULL, 0);
    for (size_t i = 0; i < 128; i++) {
        Value *next = value_array_append(base, item);
        assert(next && next->as.sequence.count == i + 1);
        release(base);
        base = next;
    }
    for (size_t i = 0; i < 128; i++) {
        assert(base->as.sequence.items[i] == item);
    }
    assert(item->refs == 129);
    release(base);
    assert(item->refs == 1);
    empty = value_sequence(V_ARRAY, NULL, 0);
    base = value_array_append(empty, item);
    release(empty);
    assert(base);
    size_t count = base->as.sequence.count;
    base->as.sequence.count = SIZE_MAX / sizeof(Value *);
    assert(!value_array_append(base, item));
    base->as.sequence.count = count;
    release(base);
    release(item);
    release(other);
}

static void append_container_children_do_not_form_storage_cycles(void)
{
    Value *number = value_size(1), *empty = value_sequence(V_ARRAY, NULL, 0);
    Value *base = value_array_append(empty, number);
    release(empty);
    char *names[] = {"child"};
    Value *record = named_value(V_RECORD, "Box", names, &base, 1);
    Value *variant = named_value(V_VARIANT, "Some", NULL, &record, 1);
    Value *map = value_sequence(V_MAP, (Value *[]){number, base}, 2);
    Value *set = value_sequence(V_SET, &base, 1);
    Value *iterator = value_new(V_ITER);
    assert(base && record && variant && map && set && iterator);
    iterator->as.iterator.iterable = retain(base);
    Value *children[] = {base, record, variant, map, set, iterator};
    for (size_t i = 0; i < sizeof(children) / sizeof(*children); i++) {
        Value *result = value_array_append(base, children[i]);
        assert(result && result->as.sequence.items != base->as.sequence.items);
        assert(base->as.sequence.count == 1 && result->as.sequence.items[1] == children[i]);
        release(result);
    }
    Value *deep = retain(number);
    for (size_t i = 0; i < 80; i++) {
        Value *next = named_value(V_VARIANT, "Some", NULL, &deep, 1);
        assert(next);
        release(deep);
        deep = next;
    }
    Value *deep_result = value_array_append(base, deep);
    assert(deep_result && deep_result->as.sequence.items != base->as.sequence.items);
    release(deep_result);
    release(deep);
    Value *scalar_record = named_value(V_RECORD, "Token", names, &number, 1);
    Value *scalar_result = value_array_append(base, scalar_record);
    assert(scalar_result && scalar_result->as.sequence.items == base->as.sequence.items);
    release(scalar_result);
    assert(scalar_record->refs == 1);
    release(scalar_record);
    /* A prefix also owns the invisible suffix of its shared backing store. */
    empty = value_sequence(V_ARRAY, NULL, 0);
    Value *separate = value_array_append(empty, number);
    release(empty);
    Value *hidden_alias = value_array_append(separate, base);
    assert(hidden_alias && hidden_alias->as.sequence.items == separate->as.sequence.items);
    Value *indirect = value_array_append(base, separate);
    assert(indirect && indirect->as.sequence.items != base->as.sequence.items);
    release(indirect);
    release(hidden_alias);
    /* After the suffix is dropped this small independent collection is safe. */
    Value *independent = value_array_append(base, separate);
    assert(independent && independent->as.sequence.items == base->as.sequence.items);
    release(independent);
    release(separate);
    release(iterator);
    release(set);
    release(map);
    release(variant);
    release(record);
    release(base);
    assert(number->refs == 1);
    release(number);
}

static void append_branches_match_independent_copies(void)
{
    Value *values[16], *copies[16], *items[4];
    for (size_t i = 0; i < 4; i++) {
        items[i] = value_size(i);
        assert(items[i]);
    }
    for (size_t i = 0; i < 16; i++) {
        values[i] = value_sequence(V_ARRAY, NULL, 0);
        copies[i] = value_sequence(V_ARRAY, NULL, 0);
        assert(values[i] && copies[i]);
    }
    uint32_t state = 7;
    for (size_t step = 0; step < 256; step++) {
        state = state * UINT32_C(1664525) + UINT32_C(1013904223);
        size_t source = (state >> 8) % 16, destination = (state >> 16) % 16;
        Value *item = items[state % 4];
        Value *result = value_array_append(values[source], item);
        size_t count = copies[source]->as.sequence.count;
        Value **children = malloc((count + 1) * sizeof(*children));
        assert(result && children);
        for (size_t i = 0; i < count; i++) {
            children[i] = copies[source]->as.sequence.items[i];
        }
        children[count] = item;
        Value *copy = value_sequence(V_ARRAY, children, count + 1);
        free(children);
        assert(copy);
        release(values[destination]);
        release(copies[destination]);
        values[destination] = result;
        copies[destination] = copy;
        for (size_t i = 0; i < 16; i++) {
            assert(value_equal(values[i], copies[i]));
        }
    }
    for (size_t i = 0; i < 16; i++) {
        release(values[i]);
        release(copies[i]);
    }
    for (size_t i = 0; i < 4; i++) {
        assert(items[i]->refs == 1);
        release(items[i]);
    }
}

static void exact_arithmetic_borrows_operands(void)
{
    Value *left = value_size(7), *right = value_size(3);
    const char *error = NULL;
    Value *quotient = binary_value(3, left, right, &error);
    assert(quotient && quotient->kind == V_RAT && !error);
    assert(left->refs == 1 && right->refs == 1);
    size_t integer;
    assert(value_index(left, &integer) && integer == 7);
    assert(value_index(right, &integer) && integer == 3);

    Buffer rendered = {0};
    assert(render(&rendered, quotient, false));
    assert(strcmp(rendered.data, "7/3") == 0);
    free(rendered.data);
    release(quotient);

    Value *zero = value_size(0);
    assert(!binary_value(3, left, zero, &error));
    assert(strcmp(error, "VM trap: division by zero") == 0);
    assert(left->refs == 1 && zero->refs == 1);
    release(zero);
    release(left);
    release(right);
}

static void utf8_offsets_preserve_code_points(void)
{
    const uint8_t text[] = {'a', 0xc3, 0xa9, 0xf0, 0x9f, 0x98, 0x80};
    assert(utf8(text, sizeof(text)));
    assert(!utf8(text, sizeof(text) - 1));
    const uint8_t overlong[] = {0xc0, 0x80};
    const uint8_t surrogate[] = {0xed, 0xa0, 0x80};
    assert(!utf8(overlong, sizeof(overlong)));
    assert(!utf8(surrogate, sizeof(surrogate)));
    Value *value = value_data(V_STR, text, sizeof(text));
    assert(value && value->as.bytes.characters == 3 && !value->as.bytes.ascii);
    size_t start, end;
    assert(utf8_offset(value, 1, &start, &end) && start == 1 && end == 3);
    assert(utf8_offset(value, 2, &start, &end) && start == 3 && end == 7);
    assert(!utf8_offset(value, 3, &start, &end));
    release(value);
}

static void opaque_host_values_keep_distinct_kinds(void)
{
    Value *duration = value_size(0), *another = value_size(0), *instant = value_size(0);
    assert(duration && another && instant);
    duration->kind = V_DURATION;
    another->kind = V_DURATION;
    instant->kind = V_INSTANT;
    assert(value_equal(duration, another) && !value_equal(duration, instant));
    const char *error = NULL;
    Value *same = binary_value(5, duration, instant, &error);
    assert(same && same->kind == V_BOOL && !same->as.boolean && !error);
    release(same);
    release(duration);
    release(another);
    release(instant);
}

static void host_builtins_reject_wrong_operand_types(void)
{
    struct HostCall {
        const char *name;
        size_t arity;
    } calls[] = {{"fs_read_beneath", 3},
                 {"fs_directories_overlap", 2},
                 {"fs_read", 2},
                 {"fs_write", 2},
                 {"fs_metadata", 1},
                 {"fs_list", 1},
                 {"fs_create_directory", 1},
                 {"fs_remove_file", 1},
                 {"fs_remove_directory", 1},
                 {"fs_temp_file", 1},
                 {"fs_temp_directory", 1},
                 {"host_sleep", 1},
                 {"process_run", 7},
                 {"process_run_isolated", 7}};
    Value *wrong = value_size(0);
    assert(wrong);
    Value *args[] = {wrong, wrong, wrong, wrong, wrong, wrong, wrong};
    for (size_t i = 0; i < sizeof(calls) / sizeof(*calls); i++) {
        const Builtin *entry = builtin(calls[i].name);
        assert(entry && entry->arity == calls[i].arity && !entry->pure);
        VM vm = {0};
        Value *result = builtin_call(&vm, calls[i].name, args);
        assert(!result && vm.error && strstr(vm.error, "VM trap:"));
        assert(wrong->refs == 1);
    }
    release(wrong);
}

static void process_environment_isolation_is_exact(void)
{
    char *inherited[] = {"GIT_CONFIG_COUNT=1", "GIT_CONFIG_KEY_0=panack.probe",
                         "GIT_CONFIG_VALUE_0=ambient", "PANACK_COVERAGE_TICKET=ambient"};
    VM vm = {0};
    vm.environment = inherited;
    vm.env_count = 4;
    Value *setting = value_data(V_STR, (const uint8_t *)"ONLY=explicit", 13);
    Value *environment[] = {setting};
    Value *duration = value_size(1000000000);
    duration->kind = V_DURATION;
    Value *args[] = {value_data(V_PATH, (const uint8_t *)"/usr/bin/env", 12),
                     value_sequence(V_ARRAY, NULL, 0),
                     value_data(V_BYTES, NULL, 0),
                     value_data(V_PATH, (const uint8_t *)".", 1),
                     value_sequence(V_ARRAY, environment, 1),
                     duration,
                     value_size(4096)};
    for (size_t mode = 0; mode < 2; mode++) {
        Value *result = builtin_call(&vm, mode ? "process_run" : "process_run_isolated", args);
        assert(!vm.error && result && !strcmp(result->as.named.name, "Ok"));
        Value *output = result->as.named.values[0]->as.named.values[2];
        if (!mode) {
            assert(output->as.bytes.length == 14);
            assert(!memcmp(output->as.bytes.data, "ONLY=explicit\n", 14));
        } else {
            assert(strstr((const char *)output->as.bytes.data, "GIT_CONFIG_COUNT=1\n"));
            assert(strstr((const char *)output->as.bytes.data, "ONLY=explicit\n"));
        }
        release(result);
    }
    release(args[4]);
    args[4] = value_sequence(V_ARRAY, NULL, 0);
    Value *result = builtin_call(&vm, "process_run_isolated", args);
    assert(result && !strcmp(result->as.named.name, "Ok"));
    assert(result->as.named.values[0]->as.named.values[2]->as.bytes.length == 0);
    release(result);
    for (size_t i = 0; i < 7; i++) {
        release(args[i]);
    }
    release(setting);
}

static Value *rooted_read(const char *root, const char *relative)
{
    Value *arguments[] = {
        value_data(V_PATH, (const uint8_t *)root, strlen(root)),
        value_data(V_PATH, (const uint8_t *)relative, strlen(relative)),
        value_size(1024),
    };
    assert(arguments[0] && arguments[1] && arguments[2]);
    VM vm = {0};
    Value *result = builtin_call(&vm, "fs_read_beneath", arguments);
    assert(!vm.error);
    for (size_t i = 0; i < 3; i++) {
        release(arguments[i]);
    }
    return result;
}

static void write_test_file(const char *path, const char *contents)
{
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    assert(fd >= 0);
    size_t length = strlen(contents), written = 0;
    while (written < length) {
        ssize_t count = write(fd, contents + written, length - written);
        assert(count > 0);
        written += (size_t)count;
    }
    assert(close(fd) == 0);
}

static void filesystem_data_limits_are_independent_of_process_limits(void)
{
    char path[] = "/tmp/panack-file-bound-XXXXXX";
    int fd = mkstemp(path);
    assert(fd >= 0 && !close(fd));
    VM vm = {0};
    Value *arguments[] = {value_data(V_PATH, (const uint8_t *)path, strlen(path)),
                          value_size(128u * 1024u * 1024u)};
    Value *result = builtin_call(&vm, "fs_read", arguments);
    assert(result && !strcmp(result->as.named.name, "Ok"));
    release(result);
    release(arguments[1]);
    arguments[1] = value_size(128u * 1024u * 1024u + 1);
    result = builtin_call(&vm, "fs_read", arguments);
    assert(result && !strcmp(result->as.named.name, "Error"));
    release(result);
    release(arguments[1]);
    arguments[1] = value_data(V_BYTES, NULL, 0);
    /* Rejection must precede access to the deliberately absent oversized data. */
    arguments[1]->as.bytes.length = 128u * 1024u * 1024u + 1;
    result = builtin_call(&vm, "fs_write", arguments);
    assert(result && !strcmp(result->as.named.name, "Error"));
    arguments[1]->as.bytes.length = 0;
    release(result);
    release(arguments[0]);
    release(arguments[1]);
    assert(!unlink(path));
}

static void filesystem_root_reads_stay_beneath_the_open_root(void)
{
    char base[] = "/tmp/panack-root-read-XXXXXX";
    assert(mkdtemp(base));
    char root[512], subdirectory[512], source[512], in_root_link[512];
    char escape_link[512], nested_link[512], nested_escape[512], outside[512], root_alias[512];
    char race_link[512];
    assert(snprintf(root, sizeof(root), "%s/root", base) > 0);
    assert(snprintf(subdirectory, sizeof(subdirectory), "%s/root/sub", base) > 0);
    assert(snprintf(source, sizeof(source), "%s/root/sub/data", base) > 0);
    assert(snprintf(in_root_link, sizeof(in_root_link), "%s/root/inside", base) > 0);
    assert(snprintf(escape_link, sizeof(escape_link), "%s/root/escape", base) > 0);
    assert(snprintf(nested_link, sizeof(nested_link), "%s/root/sub/relative", base) > 0);
    assert(snprintf(nested_escape, sizeof(nested_escape), "%s/root/sub/escape", base) > 0);
    assert(snprintf(outside, sizeof(outside), "%s/outside", base) > 0);
    assert(snprintf(root_alias, sizeof(root_alias), "%s/alias", base) > 0);
    assert(snprintf(race_link, sizeof(race_link), "%s/root/race", base) > 0);
    assert(mkdir(root, 0700) == 0 && mkdir(subdirectory, 0700) == 0);
    write_test_file(source, "inside");
    write_test_file(outside, "secret");
    assert(symlink("sub/data", in_root_link) == 0);
    assert(symlink(outside, escape_link) == 0);
    assert(symlink("../sub/data", nested_link) == 0);
    assert(symlink("../../outside", nested_escape) == 0);
    assert(symlink(root, root_alias) == 0);

    Value *inside = rooted_read(root, "inside");
    assert(inside && inside->kind == V_VARIANT && !strcmp(inside->as.named.name, "Ok"));
    Value *initial_read = inside->as.named.values[0];
    assert(initial_read && initial_read->kind == V_RECORD &&
           !strcmp(initial_read->as.named.name, "FilesystemRootRead"));
    Value *identity = initial_read->as.named.values[0];
    Value *contents = initial_read->as.named.values[1];
    assert(identity && identity->kind == V_RECORD &&
           !strcmp(identity->as.named.name, "FilesystemRootIdentity"));
    assert(contents && contents->kind == V_BYTES && contents->as.bytes.length == 6 &&
           !memcmp(contents->as.bytes.data, "inside", 6));
    retain(identity);
    release(inside);

    Value *alias_read = rooted_read(root_alias, "sub/data");
    assert(alias_read && alias_read->kind == V_VARIANT &&
           !strcmp(alias_read->as.named.name, "Ok"));
    Value *alias_identity = alias_read->as.named.values[0]->as.named.values[0];
    assert(value_equal(identity, alias_identity));
    release(alias_read);

    Value *escape = rooted_read(root, "escape");
    assert(escape && escape->kind == V_VARIANT && !strcmp(escape->as.named.name, "Error"));
    assert(!strcmp((char *)escape->as.named.values[0]->as.named.values[1]->as.bytes.data,
                   "invalid_argument"));
    release(escape);

    Value *nested = rooted_read(root, "sub/relative");
    assert(nested && nested->kind == V_VARIANT && !strcmp(nested->as.named.name, "Ok"));
    assert(!memcmp(nested->as.named.values[0]->as.named.values[1]->as.bytes.data, "inside", 6));
    release(nested);

    Value *nested_escape_result = rooted_read(root, "sub/escape");
    assert(nested_escape_result && nested_escape_result->kind == V_VARIANT &&
           !strcmp(nested_escape_result->as.named.name, "Error"));
    assert(!strcmp((char *)nested_escape_result->as.named.values[0]->as.named.values[1]->as.bytes.data,
                   "invalid_argument"));
    release(nested_escape_result);

    Value *parent = rooted_read(root, "../outside");
    assert(parent && parent->kind == V_VARIANT && !strcmp(parent->as.named.name, "Error"));
    assert(!strcmp((char *)parent->as.named.values[0]->as.named.values[1]->as.bytes.data,
                   "invalid_argument"));
    release(parent);

    release(identity);
    record_module_progress("filesystem_root_static");
    assert(symlink("sub/data", race_link) == 0);
    int race_started[2];
    assert(pipe(race_started) == 0);
    pid_t racer = fork();
    assert(racer >= 0);
    if (racer == 0) {
        close(race_started[0]);
        assert(write(race_started[1], "x", 1) == 1);
        close(race_started[1]);
        for (size_t i = 0; i < 20000; i++) {
            if (unlink(race_link) || symlink("../outside", race_link)) {
                _exit(2);
            }
            if (unlink(race_link) || symlink("sub/data", race_link)) {
                _exit(2);
            }
            (void)sched_yield();
        }
        _exit(0);
    }
    close(race_started[1]);
    char started;
    assert(read(race_started[0], &started, 1) == 1 && started == 'x');
    close(race_started[0]);
    for (size_t i = 0; i < 500; i++) {
        Value *raced = rooted_read(root, "race");
        assert(raced && raced->kind == V_VARIANT);
        if (!strcmp(raced->as.named.name, "Ok")) {
            Value *raced_read = raced->as.named.values[0];
            Value *raced_contents = raced_read->as.named.values[1];
            assert(raced_contents && raced_contents->kind == V_BYTES &&
                   raced_contents->as.bytes.length == 6 &&
                   !memcmp(raced_contents->as.bytes.data, "inside", 6));
        } else {
            assert(!strcmp(raced->as.named.name, "Error"));
        }
        release(raced);
    }
    record_module_progress("filesystem_root_race_reads");
    int racer_status = 0;
    assert(waitpid(racer, &racer_status, 0) == racer);
    assert(WIFEXITED(racer_status) && WEXITSTATUS(racer_status) == 0);
    record_module_progress("filesystem_root_racer_joined");
    assert(unlink(race_link) == 0);
    assert(unlink(root_alias) == 0 && unlink(nested_escape) == 0 && unlink(nested_link) == 0);
    assert(unlink(escape_link) == 0 && unlink(in_root_link) == 0);
    assert(unlink(source) == 0 && unlink(outside) == 0);
    assert(rmdir(subdirectory) == 0 && rmdir(root) == 0 && rmdir(base) == 0);
}

static Value *directories_overlap(const char *left, const char *right)
{
    Value *arguments[] = {
        value_data(V_PATH, (const uint8_t *)left, strlen(left)),
        value_data(V_PATH, (const uint8_t *)right, strlen(right)),
    };
    assert(arguments[0] && arguments[1]);
    VM vm = {0};
    Value *result = builtin_call(&vm, "fs_directories_overlap", arguments);
    assert(!vm.error);
    release(arguments[0]);
    release(arguments[1]);
    return result;
}

static void filesystem_directory_overlap_uses_physical_ancestry(void)
{
    char base[] = "/tmp/panack-dir-overlap-XXXXXX";
    assert(mkdtemp(base));
    char root[512], child[512], sibling[512], alias[512], missing[512];
    assert(snprintf(root, sizeof(root), "%s/root", base) > 0);
    assert(snprintf(child, sizeof(child), "%s/root/source", base) > 0);
    assert(snprintf(sibling, sizeof(sibling), "%s/sibling", base) > 0);
    assert(snprintf(alias, sizeof(alias), "%s/root-alias", base) > 0);
    assert(snprintf(missing, sizeof(missing), "%s/missing", base) > 0);
    assert(mkdir(root, 0700) == 0 && mkdir(child, 0700) == 0 && mkdir(sibling, 0700) == 0);
    assert(symlink(root, alias) == 0);

    Value *overlap = directories_overlap(root, child);
    assert(overlap && overlap->kind == V_VARIANT && !strcmp(overlap->as.named.name, "Ok"));
    assert(overlap->as.named.values[0]->kind == V_BOOL && overlap->as.named.values[0]->as.boolean);
    release(overlap);

    overlap = directories_overlap(child, root);
    assert(overlap && overlap->kind == V_VARIANT && !strcmp(overlap->as.named.name, "Ok"));
    assert(overlap->as.named.values[0]->kind == V_BOOL && overlap->as.named.values[0]->as.boolean);
    release(overlap);

    overlap = directories_overlap(root, alias);
    assert(overlap && overlap->kind == V_VARIANT && !strcmp(overlap->as.named.name, "Ok"));
    assert(overlap->as.named.values[0]->kind == V_BOOL && overlap->as.named.values[0]->as.boolean);
    release(overlap);

    overlap = directories_overlap(child, sibling);
    assert(overlap && overlap->kind == V_VARIANT && !strcmp(overlap->as.named.name, "Ok"));
    assert(overlap->as.named.values[0]->kind == V_BOOL && !overlap->as.named.values[0]->as.boolean);
    release(overlap);

    overlap = directories_overlap(root, missing);
    assert(overlap && overlap->kind == V_VARIANT && !strcmp(overlap->as.named.name, "Error"));
    release(overlap);

    assert(unlink(alias) == 0);
    assert(rmdir(sibling) == 0 && rmdir(child) == 0 && rmdir(root) == 0 && rmdir(base) == 0);
}

static void void_return_preserves_value_kind(void)
{
    Instruction instructions[] = {{.op = OP_CONST, .constant = {.tag = 5}},
                                  {.op = OP_RETURN}};
    Function function = {.name = "main", .ins_count = 2, .ins = instructions};
    Program program = {.count = 1, .functions = &function};
    VM vm = {.program = &program};
    Value *result = execute(&vm, &function, NULL);
    assert(result && result->kind == V_VOID && !vm.error);
    release(result);
}

static void frames_release_arguments_on_return_and_trap(void)
{
    char *parameters[] = {"argument"};
    Instruction instructions[] = {
        {.op = OP_LOAD, .name = "argument"},
        {.op = OP_RETURN},
    };
    Function function = {.name = "main",
                         .param_count = 1,
                         .params = parameters,
                         .ins_count = 2,
                         .ins = instructions};
    Program program = {.count = 1, .functions = &function};
    VM vm = {.program = &program};
    Value *argument = value_size(42);
    Value *result = execute(&vm, &function, &argument);
    assert(result == argument && !vm.error && argument->refs == 2);
    release(result);
    assert(argument->refs == 1);

    /* A missing second operand must release the operand already popped.
     * Refcounts detect leaks even on hosts without LeakSanitizer support.
     */
    const uint8_t operations[] = {OP_BINARY, OP_MAKE_RANGE, OP_INDEX_GET};
    for (size_t i = 0; i < sizeof(operations); i++) {
        vm.error = NULL;
        instructions[1].op = operations[i];
        assert(!execute(&vm, &function, &argument));
        assert(strcmp(vm.error, "VM trap: operand stack underflow") == 0);
        assert(argument->refs == 1);
    }

    /* Multi-value constructors and calls must discard every partially popped argument. */
    char *fields[] = {"first", "second"};
    const uint8_t constructors[] = {OP_MAKE_ARRAY, OP_MAKE_RECORD, OP_MAKE_VARIANT, OP_CALL};
    for (size_t i = 0; i < sizeof(constructors); i++) {
        vm.error = NULL;
        instructions[1] = (Instruction){.op = constructors[i],
                                        .count = 2,
                                        .arity = 2,
                                        .name = "missing",
                                        .name2 = "Missing",
                                        .items = fields};
        assert(!execute(&vm, &function, &argument));
        assert(strcmp(vm.error, "VM trap: operand stack underflow") == 0);
        assert(argument->refs == 1);
    }
    release(argument);
}

static void nested_calls_preserve_caller_ownership(void)
{
    char *parameters[] = {"value"};
    Instruction inner[] = {{.op = OP_LOAD, .name = "value"}, {.op = OP_RETURN}};
    Instruction outer[] = {{.op = OP_LOAD, .name = "value"},
                           {.op = OP_CALL, .name = "identity", .arity = 1},
                           {.op = OP_RETURN}};
    Function functions[] = {
        {.name = "identity", .param_count = 1, .params = parameters, .ins_count = 2, .ins = inner},
        {.name = "main", .param_count = 1, .params = parameters, .ins_count = 3, .ins = outer},
    };
    Program program = {.count = 2, .functions = functions};
    VM vm = {.program = &program};
    Value *argument = value_size(123);
    Value *result = execute(&vm, &functions[1], &argument);
    assert(result == argument && argument->refs == 2 && !vm.error);
    release(result);

    inner[1].op = OP_MATCH_FAIL;
    assert(!execute(&vm, &functions[1], &argument));
    assert(vm.error && argument->refs == 1);
    release(argument);
}

/* Direct semantic verification of decoded objects; wire-level vectors also run
 * through the public native decoder and the Panackelty codec. */
static void verifier_rejects_forged_structures(void)
{
    Instruction instructions[] = {{.op = OP_CONST, .constant = {.tag = 5}},
                                  {.op = OP_RETURN}};
    Function functions[] = {{.name = "main", .ins_count = 2, .ins = instructions},
                            {.name = "helper", .ins_count = 2, .ins = instructions}};
    Program program = {.count = 1, .functions = functions};
    const char *error = NULL;
    assert(verify(&program, &error));
    program.count = 0;
    assert(!verify(&program, &error) && strstr(error, "no main"));
    program.count = 1;
    functions[0].param_count = 1;
    assert(!verify(&program, &error) && strstr(error, "parameters"));
    functions[0].param_count = 0;
    functions[0].name = "";
    assert(!verify(&program, &error) && strstr(error, "no main"));
    functions[0].name = "main";
    functions[0].ins_count = 0;
    assert(!verify(&program, &error) && strstr(error, "empty"));
    functions[0].ins_count = 1;
    assert(!verify(&program, &error) && strstr(error, "no RETURN"));
    functions[0].ins_count = 2;
    instructions[0].op = OP_JUMP;
    instructions[0].target = 2;
    assert(!verify(&program, &error) && strstr(error, "jump target"));
    instructions[0].op = OP_INTERPOLATE;
    instructions[0].count = 0;
    assert(!verify(&program, &error) && strstr(error, "INTERPOLATE"));
    instructions[0].op = OP_CALL;
    instructions[0].name = "missing";
    assert(!verify(&program, &error) && strstr(error, "unknown function"));
    instructions[0].name = "print";
    instructions[0].arity = 0;
    assert(!verify(&program, &error) && strstr(error, "arity"));
    instructions[0].arity = 1;
    functions[0].pure = true;
    assert(!verify(&program, &error) && strstr(error, "pure function"));
    functions[0].pure = false;
    instructions[0].op = OP_CONST;
    program.count = 2;
    functions[1].name = "main";
    assert(!verify(&program, &error) && strstr(error, "canonical"));
    functions[1].name = "helper";
    assert(!verify(&program, &error) && strstr(error, "canonical"));
    functions[1].name = "zebra";
    assert(verify(&program, &error));
    char *duplicate_params[] = {"value", "value"};
    functions[1].params = duplicate_params;
    functions[1].param_count = 2;
    assert(!verify(&program, &error) && strstr(error, "signature"));
    functions[1].param_count = 0;
    const uint8_t tiny[] = {0};
    Program oversized;
    assert(!decode(tiny, MAX_ARTIFACT + 1u, &oversized, &error) &&
           strstr(error, "size limit"));
    free_program(&oversized);

    Instruction returned = {.op = OP_RETURN};
    Function ordered[] = {
        {.name = "alpha", .ins_count = 1, .ins = &returned},
        {.name = "helper", .ins_count = 1, .ins = &returned},
        {.name = "main", .ins_count = 1, .ins = &returned},
        {.name = "omega", .ins_count = 1, .ins = &returned},
        {.name = "zebra", .ins_count = 1, .ins = &returned}
    };
    Program lookup = {.count = 5, .functions = ordered};
    assert(verify(&lookup, &error) && lookup.verified_function_order);
    for (size_t i = 0; i < lookup.count; i++) {
        assert(program_function(&lookup, ordered[i].name) == &ordered[i]);
    }
    const char *missing[] = {"", "aardvark", "hel", "mainx", "zzzz"};
    for (size_t i = 0; i < sizeof(missing) / sizeof(*missing); i++) {
        assert(program_function(&lookup, missing[i]) == NULL);
    }
    ordered[0].name = "zzebra";
    assert(!verify(&lookup, &error) && !lookup.verified_function_order);
    assert(program_function(&lookup, "zzebra") == &ordered[0]);
    lookup.count = 0;
    assert(program_function(&lookup, "main") == NULL);
}

static void decode_mutations_release_partial_programs(void)
{
    /* Version 8: main() { CONST Void; RETURN }. Mutations never execute. */
    const uint8_t valid[] = {0x50, 0x41, 0x4e, 0x41, 0x43, 0x4b,     0x42, 0x43,     0,   0,
                             9,    0,    1,    0,    4,    'm',      'a',  'i',      'n', 0,
                             0,    0,    0,    0,    2,    OP_CONST, 5,    OP_RETURN};
    for (size_t length = 0; length < sizeof(valid); length++) {
        Program program;
        const char *error = NULL;
        assert(!decode(valid, length, &program, &error));
        assert(error);
        free_program(&program);
        assert(!program.functions && !program.count);
    }
    for (size_t position = 0; position < sizeof(valid); position++) {
        for (unsigned byte = 0; byte <= 255; byte++) {
            uint8_t mutated[sizeof(valid)];
            memcpy(mutated, valid, sizeof(valid));
            mutated[position] = (uint8_t)byte;
            Program program;
            const char *error = NULL;
            if (decode(mutated, sizeof(mutated), &program, &error)) {
                (void)verify(&program, &error);
            } else {
                assert(error);
            }
            free_program(&program);
            assert(!program.functions && !program.count);
        }
    }
}

static void nested_bytecode_releases_rejected_programs(void)
{
    /* Structurally valid, but the function is named fail rather than main. */
    const uint8_t invalid[] = {0x50, 0x41, 0x4e, 0x41, 0x43, 0x4b,     0x42, 0x43,     0,   0,
                               9,    0,    1,    0,    4,    'f',      'a',  'i',      'l', 0,
                               0,    0,    0,    0,    2,    OP_CONST, 5,    OP_RETURN};
    Value *bytes = value_data(V_BYTES, invalid, sizeof(invalid));
    Value *arguments = value_sequence(V_ARRAY, NULL, 0);
    Value *inputs[] = {bytes, arguments};
    const char *builtins[] = {"run_bytecode", "run_bytecode_args"};
    for (size_t i = 0; i < 2; i++) {
        VM vm = {0};
        assert(!builtin_call(&vm, builtins[i], inputs));
        assert(strcmp(vm.error, "bytecode has no main function") == 0);
        assert(bytes->refs == 1 && arguments->refs == 1);
    }
    release(bytes);
    release(arguments);
}

static void builtin_domains_preserve_errors_and_results(void)
{
    VM vm = {0};
    Value *unit = builtin_call(&vm, "$unit", NULL);
    assert(unit && unit->kind == V_UNIT && !vm.error);
    release(unit);
    Value *empty = builtin_call(&vm, "bytes", NULL);
    assert(empty && empty->kind == V_BYTES && !empty->as.bytes.length);
    release(empty);

    Value *wrong = value_size(7);
    Value *arguments[] = {wrong, wrong, wrong};
    const char *invalid[] = {"utf8_decode", "$method_has", "path_from_text",
                             "host_sleep",  "fs_metadata", "run_bytecode",
                             "nat"};
    for (size_t i = 0; i < sizeof(invalid) / sizeof(*invalid); i++) {
        vm.error = NULL;
        assert(!builtin_call(&vm, invalid[i], arguments));
        assert(vm.error && strstr(vm.error, "unknown builtin") == NULL);
        assert(wrong->refs == 1);
    }
    vm.error = NULL;
    assert(!builtin_call(&vm, "not_a_builtin", NULL));
    assert(strcmp(vm.error, "VM trap: unknown builtin") == 0);
    release(wrong);
}

static void bigint_boundaries(void)
{
    const uint64_t values[] = {
        0, 1, 999999999, 1000000000, 999999999999999999ULL, 1000000000000000000ULL, UINT64_MAX};
    for (size_t i = 0; i < sizeof(values) / sizeof(*values); i++) {
        PnBigInt number = {0};
        assert(pn_big_from_u64(&number, values[i]));
        char expected[32];
        snprintf(expected, sizeof(expected), "%llu", (unsigned long long)values[i]);
        char *actual = pn_big_string(&number);
        assert(actual && !strcmp(actual, expected));
        free(actual);
        pn_big_free(&number);
    }
}

/* Batched probe checked against fixed, independently calculated expectations. */
static void arithmetic_probe(void)
{
    char mode[16], a_text[512], b_text[512];
    int operation, a_exponent, b_exponent;
    while (scanf("%15s %d %511s %d %511s %d", mode, &operation, a_text, &a_exponent, b_text,
                 &b_exponent) == 6) {
        PnBigInt a = {0}, b = {0}, result = {0}, remainder = {0};
        bool negative_a = a_text[0] == '-', negative_b = b_text[0] == '-';
        assert(pn_big_from_digits(&a, a_text + negative_a, strlen(a_text) - negative_a,
                                  negative_a ? -1 : 1));
        assert(pn_big_from_digits(&b, b_text + negative_b, strlen(b_text) - negative_b,
                                  negative_b ? -1 : 1));
        if (!strcmp(mode, "integer")) {
            bool ok = operation == 0   ? pn_big_add(&result, &a, &b)
                      : operation == 1 ? pn_big_sub(&result, &a, &b)
                      : operation == 2 ? pn_big_mul(&result, &a, &b)
                                       : pn_big_divmod(&result, &remainder, &a, &b);
            assert(ok);
            char *text = pn_big_string(&result), *rest = pn_big_string(&remainder);
            assert(text && rest);
            printf("%s %s\n", text, rest);
            free(text);
            free(rest);
        } else {
            Decimal left = {.coefficient = a, .exponent = a_exponent};
            Decimal right = {.coefficient = b, .exponent = b_exponent};
            if (operation == 5) {
                printf("%d\n", decimal_compare(&left, &right));
            } else {
                const char *error = NULL;
                Value *value = decimal_binary((uint8_t)operation, &left, &right, &error);
                assert(value && !error);
                Buffer text = {0};
                assert(render(&text, value, false));
                puts(text.data);
                free(text.data);
                release(value);
            }
        }
        pn_big_free(&a);
        pn_big_free(&b);
        pn_big_free(&result);
        pn_big_free(&remainder);
    }
    assert(feof(stdin));
}

static void mutate_artifact(const char *path)
{
    FILE *file = fopen(path, "rb");
    assert(file && !fseek(file, 0, SEEK_END));
    long size = ftell(file);
    assert(size > 0 && size < 65536 && !fseek(file, 0, SEEK_SET));
    size_t length = (size_t)size;
    uint8_t *bytes = malloc(length);
    assert(bytes && fread(bytes, 1, length, file) == length && !fclose(file));
    Program program;
    const char *error = NULL;
    assert(decode(bytes, length, &program, &error) && verify(&program, &error));
    free_program(&program);
    size_t mutations = 0;
    for (size_t truncated = 0; truncated < length; truncated++) {
        error = NULL;
        assert(!decode(bytes, truncated, &program, &error) && error);
        free_program(&program);
    }
    /* Exercise bit flips plus length/sign boundary values at every byte. */
    const uint8_t replacements[] = {0, 1, 0x7f, 0x80, 0xfe, 0xff};
    for (size_t position = 0; position < length; position++) {
        uint8_t original = bytes[position];
        for (size_t change = 0; change < 14; change++) {
            bytes[position] =
                change < 8 ? original ^ (uint8_t)(1u << change) : replacements[change - 8];
            error = NULL;
            if (decode(bytes, length, &program, &error)) {
                (void)verify(&program, &error);
            } else {
                assert(error);
            }
            free_program(&program);
            mutations++;
        }
        bytes[position] = original;
    }
    free(bytes);
    printf("decoder corpus: %zu mutations, %zu truncations\n", mutations, length);
}

/* The CLI intentionally discards main's result. Keep the former oracle's
 * return-kind assertion on every successful shared VM fixture as a C contract. */
static void artifact_returns_void(const char *path)
{
    FILE *file = fopen(path, "rb");
    assert(file && !fseek(file, 0, SEEK_END));
    long size = ftell(file);
    assert(size > 0 && size < 1048576 && !fseek(file, 0, SEEK_SET));
    uint8_t *bytes = malloc((size_t)size);
    assert(bytes && fread(bytes, 1, (size_t)size, file) == (size_t)size && !fclose(file));
    Program program = {0};
    const char *error = NULL;
    assert(decode(bytes, (size_t)size, &program, &error) && verify(&program, &error));
    free(bytes);
    VM vm = {.program = &program};
    Value *result = execute(&vm, program_function(&program, "main"), NULL);
    assert(result && result->kind == V_VOID && !vm.error);
    release(result);
    free_program(&program);
}

int main(int argc, char **argv)
{
    if (argc == 3 && !strcmp(argv[1], "http-writer")) {
        http_writer_artifact(argv[2]);
        return 0;
    }
    if (argc == 2 && !strcmp(argv[1], "process-isolation")) {
        process_environment_isolation_is_exact();
        puts("native process isolation: ok");
        return 0;
    }
    if (argc == 2 && !strcmp(argv[1], "filesystem-root")) {
        filesystem_data_limits_are_independent_of_process_limits();
        filesystem_root_reads_stay_beneath_the_open_root();
        puts("native filesystem contracts: ok");
        return 0;
    }
    if (argc == 3 && !strcmp(argv[1], "tasks")) {
        task_artifact(argv[2]);
        return 0;
    }
    if (argc == 4 && !strcmp(argv[1], "resume")) {
        char *end;
        unsigned long budget = strtoul(argv[3], &end, 10);
        assert(*argv[3] && !*end && budget > 0);
        resumable_artifact(argv[2], (size_t)budget);
        return 0;
    }
    if (argc == 3 && strcmp(argv[1], "return-void") == 0) {
        artifact_returns_void(argv[2]);
        return 0;
    }
    if (argc == 3 && strcmp(argv[1], "mutate") == 0) {
        mutate_artifact(argv[2]);
        return 0;
    }
    if (argc > 1 && strcmp(argv[1], "arithmetic") == 0) {
        arithmetic_probe();
        return 0;
    }
    if (argc > 1 && strcmp(argv[1], "registry") == 0) {
        for (int i = 2; i < argc; i++) {
            const Builtin *entry = builtin(argv[i]);
            assert(entry && entry->call);
            printf("%s %u %u\n", entry->name, entry->arity, entry->pure ? 1 : 0);
        }
        return 0;
    }
    http_state_contracts();
    resumable_contracts();
    record_module_progress("resumable");
    coverage_contracts();
    record_module_progress("coverage");
    coverage_session_contracts();
    record_module_progress("coverage_session");
    async_contracts();
    record_module_progress("async");
    tcp_contracts();
    record_module_progress("tcp");
    http_transport_contracts();
    root_read_contracts();
    console_write_contracts();
    record_module_progress("http_transport");
    tcp_server_contracts();
    record_module_progress("tcp_server");
    task_contracts();
    record_module_progress("tasks");
    bigint_boundaries();
    record_module_progress("bigint_boundaries");
    values_own_copies_and_retain_children();
    record_module_progress("value_ownership");
    persistent_versions_keep_shared_children_alive();
    record_module_progress("persistent_versions");
    append_storage_preserves_branches_and_drops_hidden_children();
    record_module_progress("append_storage");
    append_container_children_do_not_form_storage_cycles();
    record_module_progress("append_child_cycles");
    append_branches_match_independent_copies();
    record_module_progress("append_branches");
    exact_arithmetic_borrows_operands();
    record_module_progress("exact_arithmetic");
    utf8_offsets_preserve_code_points();
    record_module_progress("utf8_offsets");
    opaque_host_values_keep_distinct_kinds();
    record_module_progress("opaque_host_values");
    host_builtins_reject_wrong_operand_types();
    process_environment_isolation_is_exact();
    record_module_progress("host_builtin_types");
    filesystem_data_limits_are_independent_of_process_limits();
    record_module_progress("filesystem_data_limits");
    filesystem_root_reads_stay_beneath_the_open_root();
    record_module_progress("filesystem_root");
    filesystem_directory_overlap_uses_physical_ancestry();
    record_module_progress("filesystem_ancestry");
    void_return_preserves_value_kind();
    record_module_progress("void_return");
    frames_release_arguments_on_return_and_trap();
    record_module_progress("frame_ownership");
    nested_calls_preserve_caller_ownership();
    record_module_progress("nested_calls");
    verifier_rejects_forged_structures();
    record_module_progress("verifier");
    decode_mutations_release_partial_programs();
    record_module_progress("decode_mutations");
    nested_bytecode_releases_rejected_programs();
    record_module_progress("nested_bytecode");
    builtin_domains_preserve_errors_and_results();
    record_module_progress("builtin_domains");
    puts("native module contracts: ok");
    return 0;
}
