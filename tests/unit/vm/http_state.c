#ifdef NDEBUG
#undef NDEBUG
#endif
#include "builtins.h"
#include "decode.h"
#include "verify.h"
#include <stdio.h>
#include <stdlib.h>
#include "http_state.h"
#include "vm.h"

#include <assert.h>
#include <string.h>

static Value *text(const char *value)
{
    return value_data(V_STR, (const uint8_t *)value, strlen(value));
}

void http_state_contracts(void)
{
    VM vm = {0};
    const Builtin *create = builtin("http_state_create");
    const Builtin *register_route = builtin("http_state_register");
    assert(create && register_route);
    Value *args[] = {text("127.0.0.1"), value_size(8000), text("context"), text("default")};
    Value *state = create->call(&vm, create->name, args);
    assert(state && state->kind == V_HTTP_STATE && !state->as.http_state->setup_error);
    assert(args[2]->refs == 2);
    Value *alias = retain(state);
    Value *method = text("GET"), *path = text("/item"), *handler = text("first");
    Value *route[] = {alias, method, path, handler};
    Value *result = register_route->call(&vm, register_route->name, route);
    assert(result && result->kind == V_UNIT);
    release(result);
    assert(state->as.http_state->route_count == 1);
    Value *second = text("second");
    route[3] = second;
    result = register_route->call(&vm, register_route->name, route);
    assert(result && state->as.http_state->setup_error);
    assert(state->as.http_state->routes[0].handler == handler);
    assert(state->as.http_state->route_count == 1 && second->refs == 1);
    release(result);
    state->as.http_state->phase = HTTP_CLOSED;
    result = register_route->call(&vm, register_route->name, route);
    assert(!result && vm.error && vm.fault_kind == VM_FAULT_REQUEST);
    assert(state->as.http_state->route_count == 1);
    release(alias);
    release(state);
    assert(args[2]->refs == 1 && handler->refs == 1);
    release(method); release(path); release(handler); release(second);

    const char *invalid[] = {"", "localhost", "127.1", "127.0.0.01", "256.0.0.1",
                             "127.0.0.1 ", "::1", "1..2.3", "1.2.3.4.5"};
    for (size_t i = 0; i < sizeof(invalid) / sizeof(*invalid); i++) {
        Value *address = text(invalid[i]);
        VMHttpState *bad = http_state_create(address, args[1], args[2]);
        assert(bad && bad->setup_error && bad->phase == HTTP_CREATED);
        http_state_free(bad);
        release(address);
    }
    Value *zero = value_size(0), *large = value_size(65536);
    VMHttpState *bad = http_state_create(args[0], zero, args[2]);
    assert(bad && bad->setup_error); http_state_free(bad);
    bad = http_state_create(args[0], large, args[2]);
    assert(bad && bad->setup_error); http_state_free(bad);
    release(zero); release(large);
    for (size_t i = 0; i < 4; i++) release(args[i]);

    Value *forged = named_value(V_RECORD, "HttpServerState", NULL, NULL, 0);
    Value *bad_args[] = {forged};
    const Builtin *get = builtin("http_state_context");
    vm = (VM){0};
    assert(!get->call(&vm, get->name, bad_args));
    assert(vm.error && vm.fault_kind == VM_FAULT_FATAL);
    release(forged);
    Value *writer = value_new(V_HTTP_WRITER);
    assert(writer);
    writer->as.http_writer = http_writer_create();
    assert(writer->as.http_writer);
    Value *wire = value_data(V_BYTES, (const uint8_t *)"first", 5);
    Value *send[] = {writer, wire};
    const Builtin *commit = builtin("http_writer_commit");
    vm = (VM){0};
    result = commit->call(&vm, commit->name, send);
    assert(result && writer->as.http_writer->completed);
    release(result);
    Value *writer_alias = retain(writer);
    vm = (VM){0};
    assert(!commit->call(&vm, commit->name, send));
    assert(vm.fault_kind == VM_FAULT_REQUEST && writer->as.http_writer->wire == wire);
    writer->as.http_writer->ended = true;
    const Builtin *status = builtin("http_writer_get_status");
    vm = (VM){0};
    assert(!status->call(&vm, status->name, &writer_alias));
    assert(vm.fault_kind == VM_FAULT_REQUEST);
    release(writer_alias);
    release(writer);
    assert(wire->refs == 1);
    release(wire);

    Instruction call_local[] = {{.op = OP_CALL, .name = "wrapper", .arity = 0}, {.op = OP_RETURN}};
    Instruction local_body[] = {{.op = OP_CALL, .name = "http_writer_status", .arity = 2},
                                {.op = OP_RETURN}, {.op = OP_CALL, .name = "print", .arity = 1}};
    Function functions[] = {{.name = "main", .is_async = true, .ins = call_local, .ins_count = 2},
                            {.name = "wrapper", .ins = local_body, .ins_count = 2}};
    Program effects = {.functions = functions, .count = 2};
    const char *effect_error = NULL;
    assert(verify(&effects, &effect_error) && functions[1].local_effect);
    functions[1].ins_count = 3;
    functions[1].local_effect = true; /* Never trust a supplied classification. */
    assert(!verify(&effects, &effect_error) && !functions[1].local_effect);
    functions[1].ins_count = 2;
    functions[0].is_async = false;
    functions[0].pure = true;
    assert(!verify(&effects, &effect_error));

}

static Function *named_test_function(Program *program, const char *suffix)
{
    Function *found = NULL;
    for (size_t i = 0; i < program->count; i++) {
        const char *name = program->functions[i].name;
        size_t length = strlen(name), tail = strlen(suffix);
        if (length >= tail && !strcmp(name + length - tail, suffix)) {
            assert(!found);
            found = &program->functions[i];
        }
    }
    assert(found);
    return found;
}

/* Execute the real package writer methods with a native request-owned writer.
 * The same fixture is compiled by the public CLI, not hand-built instructions. */
void http_writer_artifact(const char *path)
{
    FILE *file = fopen(path, "rb");
    assert(file && !fseek(file, 0, SEEK_END));
    long size = ftell(file);
    assert(size > 0 && !fseek(file, 0, SEEK_SET));
    uint8_t *bytes = malloc((size_t)size);
    assert(bytes && fread(bytes, 1, (size_t)size, file) == (size_t)size && !fclose(file));
    Program program = {0};
    const char *error = NULL;
    assert(decode(bytes, (size_t)size, &program, &error) && verify(&program, &error));
    free(bytes);
    Function *function = named_test_function(&program, "_writer_case");
    assert(function && function->param_count == 2);
    for (size_t scenario = 0; scenario < 9; scenario++) {
        Value *writer = value_new(V_HTTP_WRITER);
        writer->as.http_writer = http_writer_create();
        Value *index = value_size(scenario);
        Value *args[] = {writer, index};
        VM vm = {.program = &program};
        VMExecution *execution = vm_execution_create(&vm, function, args, NULL, NULL, &error);
        assert(execution);
        VMExecutionStatus status = vm_execution_advance(execution, SIZE_MAX);
        bool trapped = scenario == 1 || scenario == 2 || scenario == 3 || scenario == 5 || scenario == 8;
        if (status != (trapped ? VM_TRAPPED : VM_COMPLETED)) {
            fprintf(stderr, "writer scenario %zu: status %d, %s\n", scenario,
                    (int)status, vm.error ? vm.error : "no error");
        }
        assert(status == (trapped ? VM_TRAPPED : VM_COMPLETED));
        if (trapped) assert(vm.fault_kind == VM_FAULT_REQUEST);
        Value *wire = writer->as.http_writer->wire;
        if (scenario == 3 || scenario == 5 || scenario == 8) assert(!wire);
        else {
            assert(wire && wire->kind == V_BYTES);
            char *text = malloc(wire->as.bytes.length + 1);
            assert(text);
            memcpy(text, wire->as.bytes.data, wire->as.bytes.length);
            text[wire->as.bytes.length] = 0;
            if (scenario == 0) {
                assert(strstr(text, "HTTP/1.1 201 Created\r\n"));
                assert(strstr(text, "x-test: after\r\n") && !strstr(text, "before"));
                assert(strstr(text, "Set-Cookie: a=1\r\nSet-Cookie: b=2\r\n"));
                assert(strstr(text, "Content-Type: text/plain; charset=utf-8\r\n"));
                assert(strstr(text, "\r\n\r\nhello"));
            } else if (scenario == 1 || scenario == 2) {
                assert(strstr(text, "HTTP/1.1 200 OK\r\n") && strstr(text, "\r\n\r\nfirst"));
                assert(!strstr(text, "second"));
            } else if (scenario == 4) {
                assert(strstr(text, "HTTP/1.1 204 No Content\r\n") && !strstr(text, "Content-Length"));
                assert(!strcmp(strstr(text, "\r\n\r\n"), "\r\n\r\n"));
            } else if (scenario == 6) {
                assert(strstr(text, "Content-Type: application/octet-stream\r\n"));
            } else if (scenario == 7) {
                assert(strstr(text, "Content-Type: application/custom\r\n") && !strstr(text, "text/plain"));
            }
            free(text);
        }
        assert(vm_execution_destroy(execution));
        release(index);
        release(writer);
    }
    Function *worker = named_test_function(&program, "_connection_case");
    Function *handler = named_test_function(&program, "_context_handler");
    Value *address = text("127.0.0.1"), *port = value_size(8000), *context = value_size(42);
    VMHttpState *server = http_state_create(address, port, context);
    assert(server);
    server->contextual = true;
    Value *method = text("GET"), *route = text("/"), *callable = text(handler->name);
    assert(http_state_register(server, method, route, callable));
    Value *token = value_new(V_HTTP_CONNECTION);
    assert(token);
    token->as.http_connection = calloc(1, sizeof(VMHttpConnection));
    assert(token->as.http_connection);
    VMHttpConnection *connection = token->as.http_connection;
    connection->server = server;
    connection->writer = value_new(V_HTTP_WRITER);
    assert(connection->writer);
    connection->writer->as.http_writer = http_writer_create();
    assert(connection->writer->as.http_writer);
    VM vm = {.program = &program};
    VMExecution *execution = vm_execution_create_async(&vm, worker, &token, NULL, NULL, &error);
    assert(execution);
    connection->execution = execution;
    VMExecutionStatus status = vm_execution_advance(execution, SIZE_MAX);
    if (status != VM_COMPLETED) fprintf(stderr, "connection fixture: %s\n", vm.error ? vm.error : "incomplete");
    assert(status == VM_COMPLETED && server->admitted == 1);
    Value *wire = connection->writer->as.http_writer->wire;
    assert(wire && wire->as.bytes.length >= 4);
    assert(!memcmp(wire->as.bytes.data + wire->as.bytes.length - 4, "\r\n42", 4));
    assert(vm_execution_destroy(execution));
    http_connection_end(connection);
    assert(server->admitted == 0 && connection->ended && !connection->server);
    http_connection_end(connection);
    assert(server->admitted == 0);
    release(token);
    http_state_free(server);
    release(address); release(port); release(context);
    release(method); release(route); release(callable);
    free_program(&program);
    puts("HTTP writer and typed invocation package contracts: ok");
}
