#include "builtins.h"

#include "builtins_internal.h"
#include "host.h"
#include "host_capabilities.h"
#include "filesystem_stage.h"
#include "gzip_stream.h"
#include "digest.h"
#include "host_types.h"

#include <stdlib.h>
#include <string.h>

/* One registry supplies arity, purity, and implementation selection. */

static Value *async_service_only(VM *vm, const char *name, Value **arguments)
{
    (void)name;
    (void)arguments;
    vm->error = "VM trap: async service requires await";
    return NULL;
}

bool builtin_is_async(const Builtin *entry)
{
    return entry && entry->call == async_service_only;
}

bool builtin_is_local_state(const Builtin *entry)
{
    return entry && (entry->call == builtins_http_state_call ||
                     entry->call == builtins_http_writer_call ||
                     entry->call == builtins_http_connection_call);
}

static const Builtin BUILTINS[] = {
    {"http_bytes_slice", 3, true, builtins_http_connection_call},
    {"http_body", 1, false, builtins_http_connection_call},
    {"http_body_size", 1, false, builtins_http_connection_call},
    {"http_body_append", 4, false, builtins_http_connection_call},
    {"http_connection_continue", 1, false, async_service_only},
    {"http_listen_ordinary", 2, false, async_service_only},
    {"http_listen_contextual", 2, false, async_service_only},
    {"http_close", 1, false, async_service_only},
    {"http_destroy", 1, false, async_service_only},
    {"http_connection_invoke", 4, false, async_service_only},
    {"http_connection_writer", 1, false, builtins_http_connection_call},
    {"http_connection_routes", 1, false, builtins_http_connection_call},
    {"http_connection_fallback", 1, false, builtins_http_connection_call},
    {"http_connection_admit", 1, false, builtins_http_connection_call},

    {"http_writer_get_status", 1, false, builtins_http_writer_call},
    {"http_writer_get_headers", 1, false, builtins_http_writer_call},
    {"http_writer_reject", 1, false, builtins_http_writer_call},
    {"http_writer_status", 2, false, builtins_http_writer_call},
    {"http_writer_headers", 2, false, builtins_http_writer_call},
    {"http_writer_commit", 2, false, builtins_http_writer_call},

    {"http_state_create", 3, false, builtins_http_state_call},
    {"http_state_context", 1, true, builtins_http_state_call},
    {"http_state_setup_error", 1, false, builtins_http_state_call},
    {"http_state_fail_setup", 1, false, builtins_http_state_call},
    {"http_state_register", 4, false, builtins_http_state_call},
    {"tcp_service", 6, false, async_service_only},
    {"host_arguments_async", 0, false, async_service_only},
    {"host_stdout_async", 1, false, async_service_only},
    {"host_stderr_async", 1, false, async_service_only},
    {"host_exit_async", 1, false, async_service_only},
    {"async_fake_read", 1, false, async_service_only},
    {"$tcp_exchange", 5, false, async_service_only},
    {"$http_transport_exchange", 6, false, async_service_only},
    {"$tcp_serve", 4, false, async_service_only},
    {"$tcp_serve_stream", 5, false, async_service_only},
    {"$tcp_server_read_chunk", 1, false, async_service_only},
    {"fs_root_identity_async", 1, false, async_service_only},
    {"fs_read_beneath_async", 3, false, async_service_only},
    {"fs_read_beneath", 3, false, host_capability_call},
    {"fs_directories_overlap", 2, false, host_capability_call},
    {"fs_read", 2, false, host_capability_call},
    {"fs_write", 2, false, host_capability_call},
    {"fs_metadata", 1, false, host_capability_call},
    {"fs_list", 1, false, host_capability_call},
    {"fs_create_directory", 1, false, host_capability_call},
    {"fs_remove_file", 1, false, host_capability_call},
    {"fs_remove_directory", 1, false, host_capability_call},
    {"fs_stage_begin", 1, false, filesystem_stage_call},
    {"fs_stage_path", 1, false, filesystem_stage_call},
    {"fs_stage_directory", 2, false, filesystem_stage_call},
    {"fs_stage_write", 4, false, filesystem_stage_call},
    {"fs_stage_publish", 3, false, filesystem_stage_call},
    {"fs_stage_close", 1, false, filesystem_stage_call},
    {"digest_sha256_open", 0, false, digest_call},
    {"digest_update", 2, false, digest_call},
    {"digest_finish", 1, false, digest_call},
    {"digest_close", 1, false, digest_call},
    {"gzip_open", 1, false, gzip_stream_call},
    {"gzip_step", 3, false, gzip_stream_call},
    {"gzip_close", 1, false, gzip_stream_call},
    {"fs_read_chunk", 3, false, filesystem_stage_call},
    {"fs_atomic_write", 2, false, filesystem_stage_call},
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
