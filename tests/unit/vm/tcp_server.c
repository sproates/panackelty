#include "platform.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include "tcp_server.h"
#include "verify.h"

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

static char *parameters[] = {"request"};
static Instruction echo_code[] = {
    {.op = OP_LOAD, .name = "request"},
    {.op = OP_MAKE_VARIANT, .name = "Result", .name2 = "Ok", .count = 1},
    {.op = OP_RETURN}};
static Instruction main_code[] = {{.op = OP_CALL, .name = "$unit", .arity = 0}, {.op = OP_RETURN}};
static Function functions[] = {{.name = "main", .pure = true, .ins = main_code, .ins_count = 2},
                               {.name = "reply",
                                .is_async = true,
                                .param_count = 1,
                                .params = parameters,
                                .ins = echo_code,
                                .ins_count = 3}};
#define echo_function functions[1]
static Program program = {.functions = functions, .count = 2};
static VMTcpServerLimits limits = {4, 2, 32768, 32768, 2000, 2000, 1000};

static size_t unused_port(void)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    assert(fd >= 0);
    struct sockaddr_in address = {.sin_family = AF_INET, .sin_addr.s_addr = htonl(INADDR_LOOPBACK)};
    assert(!bind(fd, (struct sockaddr *)&address, sizeof(address)));
    socklen_t size = sizeof(address);
    assert(!getsockname(fd, (struct sockaddr *)&address, &size));
    close(fd);
    return ntohs(address.sin_port);
}

static VMTcpServer *start(size_t port, VMTcpServerLimits options)
{
    VM vm = {.program = &program};
    Value *address = value_data(V_STR, (const uint8_t *)"127.0.0.1", 9);
    VMTcpServer *server = tcp_server_start(&vm, &echo_function, address, port, options, true);
    release(address);
    assert(server && !tcp_server_error(server) && !tcp_server_done(server));
    return server;
}

static int connect_client(size_t port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    assert(fd >= 0);
    struct sockaddr_in address = {.sin_family = AF_INET,
                                  .sin_addr.s_addr = htonl(INADDR_LOOPBACK),
                                  .sin_port = htons((uint16_t)port)};
    assert(!connect(fd, (struct sockaddr *)&address, sizeof(address)));
    assert(!fcntl(fd, F_SETFL, O_NONBLOCK));
    return fd;
}

static void pump(VMTcpServer *server)
{
    tcp_server_poll(server, 0);
    tcp_server_advance(server, 64);
}

static void drain(VMTcpServer *server)
{
    for (size_t i = 0; i < 4000 && !tcp_server_done(server); i++) {
        tcp_server_poll(server, 1);
        tcp_server_advance(server, 64);
    }
    assert(tcp_server_done(server));
}

static void reply_is(VMTcpServer *server, int fd, const uint8_t *expected, size_t size)
{
    uint8_t buffer[32768];
    size_t received = 0;
    bool eof = false;
    for (size_t i = 0; i < 4000 && !eof; i++) {
        tcp_server_poll(server, 1);
        tcp_server_advance(server, 64);
        ssize_t n = recv(fd, buffer + received, sizeof(buffer) - received, 0);
        if (n > 0) {
            received += (size_t)n;
        } else if (!n) {
            eof = true;
        } else {
            assert(errno == EAGAIN || errno == EWOULDBLOCK);
        }
    }
    assert(eof && received == size && !memcmp(buffer, expected, size));
}

static Value *outcomes(VMTcpServer *server, size_t count)
{
    assert(!tcp_server_error(server));
    Value *result = tcp_server_result(server);
    assert(result && !strcmp(result->as.named.name, "Ok"));
    Value *array = result->as.named.values[0];
    assert(array->kind == V_ARRAY && array->as.sequence.count == count);
    return array;
}

static void outbound_contracts(void)
{
    int backend = socket(AF_INET, SOCK_STREAM, 0);
    assert(backend >= 0);
    struct sockaddr_in address = {.sin_family = AF_INET, .sin_addr.s_addr = htonl(INADDR_LOOPBACK)};
    assert(!bind(backend, (struct sockaddr *)&address, sizeof(address)) && !listen(backend, 2));
    socklen_t length = sizeof(address);
    assert(!getsockname(backend, (struct sockaddr *)&address, &length));
    assert(!fcntl(backend, F_SETFL, O_NONBLOCK));
    Instruction code[] = {
        {.op = OP_CONST, .constant = {.tag = 3, .text = "127.0.0.1", .text_length = 9}},
        {.op = OP_CONST},
        {.op = OP_LOAD, .name = "request"},
        {.op = OP_CONST},
        {.op = OP_CONST},
        {.op = OP_AWAIT_CALL, .name = "$tcp_exchange", .arity = 5},
        {.op = OP_RETURN}};
    assert(pn_big_from_u64(&code[1].constant.number, ntohs(address.sin_port)));
    assert(pn_big_from_u64(&code[3].constant.number, 32));
    assert(pn_big_from_u64(&code[4].constant.number, 2000));
    echo_function.ins = code;
    echo_function.ins_count = 7;
    size_t port = unused_port();
    VMTcpServerLimits options = limits;
    options.drain_ms = 0;
    VMTcpServer *server = start(port, options);
    int fronts[2], peers[2];
    for (size_t i = 0; i < 2; i++) {
        fronts[i] = connect_client(port);
        assert(send(fronts[i], "x", 1, 0) == 1 && !shutdown(fronts[i], SHUT_WR));
        peers[i] = -1;
        for (size_t n = 0; n < 4000 && peers[i] < 0; n++) {
            tcp_server_poll(server, 1);
            tcp_server_advance(server, 64);
            peers[i] = accept(backend, NULL, NULL);
            if (peers[i] < 0) {
                assert(errno == EAGAIN || errno == EWOULDBLOCK);
            }
        }
        assert(peers[i] >= 0);
    }
    assert(!fcntl(peers[1], F_SETFL, O_NONBLOCK));
    bool request_eof = false;
    for (size_t i = 0; i < 4000 && !request_eof; i++) {
        tcp_server_poll(server, 1);
        tcp_server_advance(server, 64);
        char received[8];
        ssize_t n = read(peers[1], received, sizeof(received));
        if (!n) {
            request_eof = true;
        } else if (n < 0) {
            assert(errno == EAGAIN || errno == EWOULDBLOCK);
        } else {
            assert(n == 1 && received[0] == 'x');
        }
    }
    assert(request_eof);
    assert(write(peers[1], "x", 1) == 1);
    close(peers[1]);
    reply_is(server, fronts[1], (const uint8_t *)"x", 1);
    /* Stop cancels the other handler's real outbound wait immediately. */
    tcp_server_stop(server);
    assert(tcp_server_done(server));
    Value *reports = outcomes(server, 2);
    assert(!strcmp(reports->as.sequence.items[0]->as.named.name, "Error"));
    assert(!strcmp(reports->as.sequence.items[1]->as.named.name, "Ok"));
    tcp_server_destroy(server);
    char buffer[8];
    struct pollfd ready = {.fd = peers[0], .events = POLLIN};
    assert(poll(&ready, 1, 2000) == 1);
    /* The request may have been sent before cancellation. Its write half has
     * closed either way, and destroying the owner cannot resume its child. */
    ssize_t n = read(peers[0], buffer, sizeof(buffer));
    assert(n == 0 || n == 1);
    if (n == 1) {
        assert(read(peers[0], buffer, sizeof(buffer)) == 0);
    }
    close(peers[0]);
    close(fronts[0]);
    close(fronts[1]);
    close(backend);
    pn_big_free(&code[1].constant.number);
    pn_big_free(&code[3].constant.number);
    pn_big_free(&code[4].constant.number);
    echo_function.ins = echo_code;
    echo_function.ins_count = 3;
}

static void zero_grace_contract(void)
{
    VMTcpServerLimits options = limits;
    options.clients = options.concurrency = 2;
    options.drain_ms = 0;
    size_t port = unused_port();
    VMTcpServer *server = start(port, options);
    int first = connect_client(port);
    pump(server); /* accept */
    assert(!shutdown(first, SHUT_WR));
    tcp_server_poll(server, 0);     /* EOF */
    tcp_server_advance(server, 64); /* response waiting for write readiness */
    int last = connect_client(port);
    tcp_server_poll(server, 0); /* accept and POLLOUT in the same snapshot */
    assert(tcp_server_done(server));
    Value *reports = outcomes(server, 2);
    for (size_t i = 0; i < 2; i++) {
        assert(!strcmp(reports->as.sequence.items[i]->as.named.name, "Error"));
    }
    tcp_server_destroy(server);
    close(first);
    close(last);
}

static void embedded_contracts(void)
{
    char *params[] = {"address", "port", "handler", "limits"};
    Instruction code[] = {{.op = OP_LOAD, .name = "address"},
                          {.op = OP_LOAD, .name = "port"},
                          {.op = OP_LOAD, .name = "handler"},
                          {.op = OP_LOAD, .name = "limits"},
                          {.op = OP_AWAIT_CALL, .name = "$tcp_serve", .arity = 4},
                          {.op = OP_RETURN}};
    Function all[] = {functions[0],
                      functions[1],
                      {.name = "serve",
                       .is_async = true,
                       .param_count = 4,
                       .params = params,
                       .ins = code,
                       .ins_count = 6}};
    Program complete = {.functions = all, .count = 3};
    const char *error;
    assert(verify(&complete, &error));
    char *fields[] = {"clients",        "concurrency",       "request_limit",
                      "response_limit", "client_timeout_ms", "admission_timeout_ms",
                      "drain_ms"};
    Value *values[] = {value_size(2),   value_size(2),   value_size(8), value_size(8),
                       value_size(100), value_size(100), value_size(0)};
    Value *args[] = {value_data(V_STR, (const uint8_t *)"127.0.0.1", 9), value_size(unused_port()),
                     value_data(V_STR, (const uint8_t *)"reply", 5),
                     named_value(V_RECORD, "TcpServerLimits", fields, values, 7)};
    for (size_t i = 0; i < 7; i++) {
        release(values[i]);
    }
    for (size_t mode = 0; mode < 7; mode++) {
        VM vm = {.program = &complete};
        VMExecution *execution = vm_execution_create(&vm, &all[2], args, NULL, NULL, &error);
        assert(execution && !error);
        if (mode == 1 || mode == 3) {
            assert(vm_execution_enable_tcp(execution));
        }
        if (mode >= 2) {
            assert(vm_execution_enable_server(execution));
        }
        if (mode == 6) {
            args[3]->as.named.names[0][0] = 'X';
        }
        if (mode == 3) {
            args[3]->as.named.values[0]->kind = V_BOOL;
        }
        if (mode == 4) {
            args[2]->as.bytes.data[0] = 0;
        }
        if (mode == 5) {
            all[1].is_async = false; /* forged callable target */
        }
        VMExecutionStatus status = vm_execution_advance(execution, 100);
        if (mode < 2 || mode == 3 || mode == 6) {
            assert(status == VM_TRAPPED);
        } else {
            assert(status == VM_WAITING);
            assert(!vm_execution_complete_read(execution, NULL));
            assert(!vm_execution_complete_print(execution, NULL));
            assert(vm_execution_stop_server(execution));
            assert(vm_execution_pump_server(execution, 64, 0));
            status = vm_execution_advance(execution, 100);
            assert(status == (mode == 2 ? VM_COMPLETED : VM_TRAPPED));
        }
        args[3]->as.named.values[0]->kind = V_NAT;
        args[2]->as.bytes.data[0] = 'r';
        args[3]->as.named.names[0][0] = 'c';
        all[1].is_async = true;
        assert(vm_execution_destroy(execution));
    }
    for (size_t i = 0; i < 4; i++) {
        release(args[i]);
    }
    code[4].op = OP_CALL;
    assert(!verify(&complete, &error));
    code[4].op = OP_AWAIT_CALL;
    code[4].arity = 3;
    assert(!verify(&complete, &error));
}

void tcp_server_contracts(void)
{
    const char *error;
    assert(verify(&program, &error));
    size_t port = unused_port();
    VMTcpServer *server = start(port, limits);
    int slow = connect_client(port);
    pump(server);
    int fast = connect_client(port);
    const uint8_t request[] = {'h', 0, 'i'};
    assert(send(fast, request, sizeof(request), 0) == (ssize_t)sizeof(request));
    assert(!shutdown(fast, SHUT_WR));
    reply_is(server, fast, request, sizeof(request));
    assert(!tcp_server_done(server)); /* stalled reader did not block fast */
    close(fast);
    /* Reuse the completed slot while the first connection still waits. */
    int next = connect_client(port);
    assert(!shutdown(next, SHUT_WR));
    reply_is(server, next, request, 0);
    close(next);
    tcp_server_stop(server);
    assert(!shutdown(slow, SHUT_WR));
    reply_is(server, slow, request, 0);
    close(slow);
    drain(server);
    Value *reports = outcomes(server, 3);
    for (size_t i = 0; i < 3; i++) {
        assert(!strcmp(reports->as.sequence.items[i]->as.named.name, "Ok"));
    }
    tcp_server_destroy(server);

    /* Request overflow, response overflow, zero-length limits and client expiry. */
    for (size_t mode = 0; mode < 4; mode++) {
        VMTcpServerLimits options = limits;
        options.clients = options.concurrency = 1;
        if (mode == 0 || mode == 2) {
            options.request_limit = 0;
        }
        if (mode == 1 || mode == 2) {
            options.response_limit = 0;
        }
        if (mode == 3) {
            options.client_timeout_ms = 1;
        }
        port = unused_port();
        server = start(port, options);
        int client = connect_client(port);
        if (mode < 2) {
            assert(send(client, request, sizeof(request), 0) == (ssize_t)sizeof(request));
        }
        if (mode < 3) {
            assert(!shutdown(client, SHUT_WR));
        }
        drain(server);
        reports = outcomes(server, 1);
        assert(!strcmp(reports->as.sequence.items[0]->as.named.name, mode == 2 ? "Ok" : "Error"));
        close(client);
        tcp_server_destroy(server);
    }

    /* Grace expiry and immediate destruction release pending descriptors. */
    for (size_t mode = 0; mode < 4; mode++) {
        VMTcpServerLimits options = limits;
        options.drain_ms = 0;
        port = unused_port();
        server = start(port, options);
        int client = connect_client(port);
        pump(server);
        if (mode == 1) {
            assert(!shutdown(client, SHUT_WR));
            tcp_server_poll(server, 0);
        }
        if (mode == 3) {
            tcp_server_advance(server, 64); /* response awaiting write */
        }
        if (mode == 2 || mode == 3) {
            tcp_server_stop(server);
            assert(tcp_server_done(server));
            reports = outcomes(server, 1);
            assert(!strcmp(reports->as.sequence.items[0]->as.named.name, "Error"));
        }
        tcp_server_destroy(server);
        struct pollfd ready = {.fd = client, .events = POLLIN};
        assert(poll(&ready, 1, 2000) == 1);
        uint8_t byte;
        assert(recv(client, &byte, 1, 0) == 0);
        close(client);
    }

    /* No admission, and conflicting listener errors. */
    VMTcpServerLimits options = limits;
    options.admission_timeout_ms = 1;
    server = start(unused_port(), options);
    drain(server);
    outcomes(server, 0);
    tcp_server_destroy(server);
    port = unused_port();
    server = start(port, limits);
    VM vm = {.program = &program};
    Value *address = value_data(V_STR, (const uint8_t *)"127.0.0.1", 9);
    VMTcpServer *conflict = tcp_server_start(&vm, &echo_function, address, port, limits, true);
    assert(tcp_server_done(conflict) && !tcp_server_error(conflict));
    assert(!strcmp(tcp_server_result(conflict)->as.named.name, "Error"));
    tcp_server_destroy(conflict);
    tcp_server_destroy(server);
    Function foreign = echo_function;
    server = tcp_server_start(&vm, &foreign, address, port, limits, true);
    assert(tcp_server_done(server) && tcp_server_error(server));
    tcp_server_destroy(server);
    release(address);

    /* Handler traps and malformed results fail the owner; budgeted endless
     * computation remains cancellable at its real client deadline. */
    Instruction invalid[] = {{.op = OP_CONST, .constant = {.tag = 4}}, {.op = OP_RETURN}};
    Instruction trapped[] = {{.op = OP_MATCH_FAIL}, {.op = OP_RETURN}};
    Instruction endless[] = {{.op = OP_JUMP, .target = 0}, {.op = OP_RETURN}};
    Instruction *saved = echo_function.ins;
    for (size_t mode = 0; mode < 3; mode++) {
        echo_function.ins = mode == 0 ? invalid : mode == 1 ? trapped : endless;
        echo_function.ins_count = 2;
        assert(verify(&program, &error));
        options = limits;
        options.clients = options.concurrency = 1;
        options.client_timeout_ms = 5;
        port = unused_port();
        server = start(port, options);
        int client = connect_client(port);
        assert(!shutdown(client, SHUT_WR));
        drain(server);
        if (mode < 2) {
            assert(tcp_server_error(server));
        } else {
            outcomes(server, 1);
        }
        tcp_server_destroy(server);
        close(client);
    }
    echo_function.ins = saved;
    echo_function.ins_count = 3;
    outbound_contracts();
    embedded_contracts();
    zero_grace_contract();
}
