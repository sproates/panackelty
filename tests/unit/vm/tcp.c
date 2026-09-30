#include "platform.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include "tcp.h"
#include "vm.h"
#include "verify.h"
#include <arpa/inet.h>
#include <assert.h>
#include <fcntl.h>
#include <poll.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static int listener(size_t *port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    assert(fd >= 0);
    struct sockaddr_in address = {.sin_family = AF_INET, .sin_addr.s_addr = htonl(INADDR_LOOPBACK)};
    assert(!bind(fd, (struct sockaddr *)&address, sizeof(address)) && !listen(fd, 8));
    socklen_t size = sizeof(address);
    assert(!getsockname(fd, (struct sockaddr *)&address, &size));
    *port = ntohs(address.sin_port);
    return fd;
}

static int accept_ready(int fd)
{
    struct pollfd ready = {.fd = fd, .events = POLLIN};
    assert(poll(&ready, 1, 2000) == 1);
    int peer = accept(fd, NULL, NULL);
    assert(peer >= 0);
    return peer;
}

static Value *text(const char *s)
{
    return value_data(V_STR, (const uint8_t *)s, strlen(s));
}

static void result_is(Value *result, const char *tag, const uint8_t *data, size_t size)
{
    assert(result && result->kind == V_VARIANT && !strcmp(result->as.named.name, tag));
    Value *payload = result->as.named.values[0];
    assert(payload->as.bytes.length == size && !memcmp(payload->as.bytes.data, data, size));
}

static char *names[] = {"address", "port", "request", "limit", "timeout"};
static Instruction code[] = {
    {.op = OP_LOAD, .name = "address"}, {.op = OP_LOAD, .name = "port"},
    {.op = OP_LOAD, .name = "request"}, {.op = OP_LOAD, .name = "limit"},
    {.op = OP_LOAD, .name = "timeout"},
    {.op = OP_AWAIT_CALL, .name = "$tcp_exchange", .arity = 5}, {.op = OP_RETURN},
};
static Function function = {.name = "exchange", .is_async = true, .param_count = 5,
    .params = names, .ins = code, .ins_count = 7};
static Program program = {.count = 1, .functions = &function};

static VMExecution *start(VM *vm, size_t port, size_t timeout, bool enabled)
{
    *vm = (VM){.program = &program};
    Value *args[] = {text("127.0.0.1"), value_size(port),
        value_data(V_BYTES, (const uint8_t *)"hi", 2), value_size(32), value_size(timeout)};
    const char *error;
    VMExecution *execution = vm_execution_create(vm, &function, args, NULL, NULL, &error);
    for (size_t i = 0; i < 5; i++) release(args[i]);
    assert(execution && !error);
    if (enabled) assert(vm_execution_enable_tcp(execution));
    return execution;
}

void tcp_contracts(void)
{
    size_t port;
    int server = listener(&port);
    VM slow_vm, fast_vm, disabled_vm;
    VMExecution *slow = start(&slow_vm, port, 100, true);
    VMExecution *fast = start(&fast_vm, port, 2000, true);
    VMExecution *disabled = start(&disabled_vm, port, 2000, false);
    assert(vm_execution_advance(disabled, 100) == VM_TRAPPED);
    assert(strstr(disabled_vm.error, "TCP service is unavailable"));
    assert(vm_execution_destroy(disabled));
    assert(vm_execution_advance(slow, 100) == VM_WAITING);
    int slow_peer = accept_ready(server);
    assert(vm_execution_advance(fast, 100) == VM_WAITING);
    int fast_peer = accept_ready(server);
    assert(!vm_execution_complete_read(fast, NULL));
    assert(!vm_execution_complete_print(fast, NULL));
    assert(vm_execution_poll_tcp(fast, 0));
    uint8_t request[3];
    assert(read(fast_peer, request, sizeof(request)) == 2 && !memcmp(request, "hi", 2));
    assert(read(fast_peer, request, sizeof(request)) == 0); /* client half-close */
    const uint8_t reply[] = {'a', 0, 'b'};
    assert(write(fast_peer, reply, 1) == 1);
    assert(vm_execution_poll_tcp(fast, 0));
    assert(vm_execution_advance(fast, 10) == VM_WAITING);
    assert(write(fast_peer, reply + 1, 2) == 2);
    close(fast_peer);
    VMExecutionStatus status = VM_WAITING;
    for (size_t i = 0; i < 200 && status == VM_WAITING; i++) {
        assert(vm_execution_poll_tcp(fast, 10));
        status = vm_execution_advance(fast, 100);
    }
    assert(status == VM_COMPLETED);
    result_is(vm_execution_result(fast), "Ok", reply, sizeof(reply));
    assert(vm_execution_advance(slow, 1) == VM_WAITING); /* no head-of-line blocking */
    status = VM_WAITING;
    for (size_t i = 0; i < 200 && status == VM_WAITING; i++) {
        assert(vm_execution_poll_tcp(slow, 10));
        status = vm_execution_advance(slow, 100);
    }
    assert(status == VM_COMPLETED);
    result_is(vm_execution_result(slow), "Error", (const uint8_t *)"TCP timed out", 13);
    assert(vm_execution_destroy(fast) && vm_execution_destroy(slow));
    close(slow_peer);

    /* Cancellation closes the kernel socket immediately; no producer can later
     * deliver to the destroyed execution. Repeat to expose descriptor leaks. */
    for (size_t i = 0; i < 32; i++) {
        VM vm;
        VMExecution *cancelled = start(&vm, port, 2000, true);
        assert(vm_execution_advance(cancelled, 100) == VM_WAITING);
        int peer = accept_ready(server);
        assert(vm_execution_destroy(cancelled));
        struct pollfd ready = {.fd = peer, .events = POLLIN};
        assert(poll(&ready, 1, 2000) == 1);
        assert(read(peer, request, sizeof(request)) == 0);
        close(peer);
    }
    close(server);

    /* Dynamic forged inputs are rejected before attempting a connection. */
    Value *args[] = {text("127.0.0.1"), value_size(0), value_bool(true), value_size(0), value_size(0)};
    VMTcpExchange *invalid = tcp_exchange_start(args);
    assert(invalid && tcp_exchange_poll(invalid, 0));
    result_is(tcp_exchange_result(invalid), "Error", (const uint8_t *)"invalid TCP arguments", 21);
    tcp_exchange_destroy(invalid);
    for (size_t i = 0; i < 5; i++) release(args[i]);

    const char *bad_addresses[] = {"localhost", "256.1.1.1", "", "127.0.0.1:80"};
    for (size_t i = 0; i < sizeof(bad_addresses) / sizeof(*bad_addresses); i++) {
        Value *bad[] = {text(bad_addresses[i]), value_size(80),
            value_data(V_BYTES, (const uint8_t *)"", 0), value_size(32), value_size(100)};
        VMTcpExchange *exchange = tcp_exchange_start(bad);
        assert(exchange && tcp_exchange_poll(exchange, 0));
        result_is(tcp_exchange_result(exchange), "Error", (const uint8_t *)"invalid IPv4 address", 20);
        tcp_exchange_destroy(exchange);
        for (size_t j = 0; j < 5; j++) release(bad[j]);
    }

    Instruction await[] = {{.op = OP_AWAIT_CALL, .name = "$tcp_exchange", .arity = 5}, {.op = OP_RETURN}};
    Function main = {.name = "main", .is_async = true, .ins = await, .ins_count = 2};
    Program forged = {.functions = &main, .count = 1};
    const char *error;
    assert(verify(&forged, &error));
    main.is_async = false;
    assert(!verify(&forged, &error));
    main.is_async = true;
    await[0].op = OP_CALL;
    assert(!verify(&forged, &error));
    await[0].op = OP_AWAIT_CALL;
    await[0].arity = 4;
    assert(!verify(&forged, &error));
}
