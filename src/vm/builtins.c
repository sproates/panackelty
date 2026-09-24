#include "builtins.h"

#include "builtins_internal.h"
#include "host.h"
#include "host_capabilities.h"
#include "host_types.h"

#include <stdlib.h>
#include <string.h>

/* One registry supplies arity, purity, and implementation selection. */

static const Builtin BUILTINS[] = {
    {"fs_read", 2, false, host_capability_call},
    {"fs_write", 2, false, host_capability_call},
    {"fs_metadata", 1, false, host_capability_call},
    {"fs_list", 1, false, host_capability_call},
    {"fs_create_directory", 1, false, host_capability_call},
    {"fs_remove_file", 1, false, host_capability_call},
    {"fs_remove_directory", 1, false, host_capability_call},
    {"fs_temp_file", 1, false, host_capability_call},
    {"fs_temp_directory", 1, false, host_capability_call},
    {"host_decode_utf8", 1, true, host_capability_call},
    {"host_sleep", 1, false, host_capability_call},
    {"process_run", 7, false, host_capability_call},
    {"path_from_text", 1, true, host_type_call},
    {"path_from_native", 1, true, host_type_call},
    {"path_to_text", 1, true, host_type_call},
    {"path_native_bytes", 1, true, host_type_call},
    {"path_display", 1, true, host_type_call},
    {"path_absolute", 1, true, host_type_call},
    {"path_append", 2, true, host_type_call},
    {"path_directory", 1, true, host_type_call},
    {"path_filename", 1, true, host_type_call},
    {"path_current", 0, true, host_type_call},
    {"duration_nanoseconds", 1, true, host_type_call},
    {"duration_ticks", 1, true, host_type_call},
    {"duration_from_seconds", 1, true, host_type_call},
    {"instant_now", 0, false, host_type_call},
    {"instant_add", 2, true, host_type_call},
    {"instant_difference", 2, true, host_type_call},
    {"instant_before", 2, true, host_type_call},

    {"$unit", 0, true, builtins_numeric_call},
    {"nat", 1, true, builtins_numeric_call},
    {"dec", 1, true, builtins_numeric_call},
    {"quotient", 2, true, builtins_numeric_call},
    {"print", 1, false, host_call},
    {"read_line", 0, false, host_call},
    {"read_file", 1, false, host_call},
    {"write_file", 2, false, host_call},
    {"len", 1, true, builtins_text_call},
    {"append", 2, true, builtins_collections_call},
    {"concat", 2, true, builtins_text_call},
    {"slice", 3, true, builtins_text_call},
    {"starts_with", 2, true, builtins_text_call},
    {"starts_with_at", 3, true, builtins_text_call},
    {"reverse", 1, true, builtins_text_call},
    {"is_digit", 1, true, builtins_text_call},
    {"is_letter", 1, true, builtins_text_call},
    {"is_whitespace", 1, true, builtins_text_call},
    {"map", 0, true, builtins_collections_call},
    {"map_put", 3, true, builtins_collections_call},
    {"map_has", 2, true, builtins_collections_call},
    {"map_get", 2, true, builtins_collections_call},
    {"$method_put", 3, true, builtins_collections_call},
    {"$method_has", 2, true, builtins_collections_call},
    {"$method_get", 2, true, builtins_collections_call},
    {"set", 0, true, builtins_collections_call},
    {"set_add", 2, true, builtins_collections_call},
    {"set_has", 2, true, builtins_collections_call},
    {"$method_add", 2, true, builtins_collections_call},
    {"bytes", 0, true, builtins_collections_call},
    {"byte_append", 2, true, builtins_text_call},
    {"bytes_concat", 2, true, builtins_text_call},
    {"byte_len", 1, true, builtins_text_call},
    {"byte_get", 2, true, builtins_text_call},
    {"utf8_encode", 1, true, builtins_text_call},
    {"utf8_decode", 1, true, builtins_text_call},
    {"read_bytes", 1, false, host_call},
    {"write_bytes", 2, false, host_call},
    {"nat_from_str", 1, true, builtins_text_call},
    {"command_args", 0, false, host_call},
    {"environment_has", 1, false, host_call},
    {"environment_get", 1, false, host_call},
    {"eprint", 1, false, host_call},
    {"process_exit", 1, false, host_call},
    {"path_resolve", 1, false, host_call},
    {"path_parent", 1, true, host_call},
    {"path_join", 2, true, host_call},
    {"path_suffix", 1, true, host_call},
    {"path_with_suffix", 2, true, host_call},
    {"path_is_absolute", 1, true, host_call},
    {"file_exists", 1, false, host_call},
    {"run_bytecode", 1, false, builtins_vm_call},
    {"run_bytecode_args", 2, false, builtins_vm_call},
};

const Builtin *builtin(const char *name)
{
    for (size_t i = 0; i < sizeof(BUILTINS) / sizeof(*BUILTINS); i++) {
        if (!strcmp(name, BUILTINS[i].name)) {
            return &BUILTINS[i];
        }
    }
    return NULL;
}

Value *builtin_call(VM *vm, const char *name, Value **arguments)
{
    const Builtin *entry = builtin(name);
    if (!entry) {
        vm->error = "VM trap: unknown builtin";
        return NULL;
    }
    return entry->call(vm, name, arguments);
}
