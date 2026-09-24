/* Opaque host-domain values. Included after the VM's value and error helpers. */
static bool host_type_builtin(const char *name) {
    return !strcmp(name, "path_from_text") || !strcmp(name, "path_from_native") ||
        !strcmp(name, "path_to_text") || !strcmp(name, "path_native_bytes") ||
        !strcmp(name, "path_display") || !strcmp(name, "path_absolute") ||
        !strcmp(name, "path_append") || !strcmp(name, "path_directory") ||
        !strcmp(name, "path_filename") || !strcmp(name, "path_current") ||
        !strcmp(name, "duration_nanoseconds") || !strcmp(name, "duration_ticks") ||
        !strcmp(name, "duration_from_seconds") || !strcmp(name, "instant_now") ||
        !strcmp(name, "instant_add") || !strcmp(name, "instant_difference") ||
        !strcmp(name, "instant_before");
}

/* Takes ownership of the payload, including on allocation failure. */
static Value *host_variant(const char *name, Value *payload) {
    if (!payload) return NULL;
    Value *result = named_value(V_VARIANT, name, NULL, &payload, 1);
    release(payload);
    return result;
}
static Value *host_error(const char *name) {
    return host_variant("Error", named_value(V_VARIANT, name, NULL, NULL, 0));
}
static Value *host_type_call(VM *vm, const char *name, Value **a) {
    if (!strcmp(name, "path_current")) return value_data(V_PATH, (const uint8_t *)".", 1);
    if (!strcmp(name, "path_from_text") || !strcmp(name, "path_from_native")) {
        REQUIRE(a[0]->kind == (!strcmp(name, "path_from_text") ? V_STR : V_BYTES),
                "VM trap: invalid path constructor operand");
        if (!a[0]->as.bytes.length) return host_error("EmptyPath");
        if (memchr(a[0]->as.bytes.data, 0, a[0]->as.bytes.length)) return host_error("PathContainsNul");
        return host_variant("Ok", value_data(V_PATH, a[0]->as.bytes.data, a[0]->as.bytes.length));
    }
    if (!strncmp(name, "path_", 5)) {
        REQUIRE(a[0]->kind == V_PATH, "VM trap: operation requires Path");
        const uint8_t *data = a[0]->as.bytes.data;
        size_t length = a[0]->as.bytes.length;
        if (!strcmp(name, "path_to_text")) {
            if (!utf8(data, length)) return host_error("PathNotUtf8");
            return host_variant("Ok", value_data(V_STR, data, length));
        }
        if (!strcmp(name, "path_native_bytes")) return value_data(V_BYTES, data, length);
        if (!strcmp(name, "path_absolute")) return value_bool(data[0] == '/');
        if (!strcmp(name, "path_display")) {
            Buffer out = {0};
            for (size_t i = 0; i < length; i++) {
                char escaped[5];
                bool ok;
                if (data[i] >= 32 && data[i] <= 126 && data[i] != '\\')
                    ok = buffer_add(&out, (const char *)&data[i], 1);
                else {
                    snprintf(escaped, sizeof(escaped), "\\x%02x", data[i]);
                    ok = buffer_text(&out, escaped);
                }
                if (!ok) { free(out.data); return NULL; }
            }
            Value *result = value_data(V_STR, (const uint8_t *)out.data, out.length);
            free(out.data); return result;
        }
        if (!strcmp(name, "path_append")) {
            REQUIRE(a[1]->kind == V_PATH, "VM trap: operation requires Path");
            if (a[1]->as.bytes.data[0] == '/') return host_error("AbsolutePathAppend");
            Buffer out = {0};
            bool ok = buffer_add(&out, (const char *)data, length) &&
                (data[length - 1] == '/' || buffer_text(&out, "/")) &&
                buffer_add(&out, (const char *)a[1]->as.bytes.data, a[1]->as.bytes.length);
            Value *result = ok ? host_variant("Ok", value_data(V_PATH, (const uint8_t *)out.data, out.length)) : NULL;
            free(out.data); return result;
        }
        size_t root = 0, end = length;
        while (root < length && data[root] == '/') root++;
        while (end > root && data[end - 1] == '/') end--;
        size_t start = end;
        while (start > root && data[start - 1] != '/') start--;
        if (!strcmp(name, "path_filename")) {
            if (end == root) return named_value(V_VARIANT, "None", NULL, NULL, 0);
            return host_variant("Some", value_data(V_PATH, data + start, end - start));
        }
        if (start <= root) return root ? value_data(V_PATH, data, root) : value_data(V_PATH, (const uint8_t *)".", 1);
        return value_data(V_PATH, data, start - 1);
    }
    if (!strcmp(name, "duration_nanoseconds")) {
        REQUIRE(a[0]->kind == V_INT || a[0]->kind == V_NAT, "VM trap: duration requires Int");
        return value_big(V_DURATION, &a[0]->as.integer);
    }
    if (!strcmp(name, "duration_ticks")) {
        REQUIRE(a[0]->kind == V_DURATION, "VM trap: operation requires Duration");
        return value_big(V_INT, &a[0]->as.integer);
    }
    if (!strcmp(name, "duration_from_seconds")) {
        REQUIRE(a[0]->kind == V_RAT, "VM trap: duration seconds requires Rat");
        PnBigInt scaled = {0}, q = {0}, rem = {0};
        bool ok = pn_big_copy(&scaled, &a[0]->as.rational.numerator) &&
            pn_big_mul_small(&scaled, 1000000000) &&
            pn_big_divmod(&q, &rem, &scaled, &a[0]->as.rational.denominator);
        Value *result = !ok ? NULL : !pn_big_is_zero(&rem) ? host_error("FractionalNanosecond") :
            host_variant("Ok", value_big(V_DURATION, &q));
        pn_big_free(&scaled); pn_big_free(&q); pn_big_free(&rem); return result;
    }
    if (!strcmp(name, "instant_now")) {
        struct timespec now;
        if (clock_gettime(CLOCK_MONOTONIC, &now)) return host_error("ClockUnavailable");
        PnBigInt ticks = {0};
        bool ok = pn_big_from_u64(&ticks, (uint64_t)now.tv_sec) &&
            pn_big_mul_small(&ticks, 1000000000) && pn_big_add_small(&ticks, (uint32_t)now.tv_nsec);
        Value *result = ok ? host_variant("Ok", value_big(V_INSTANT, &ticks)) : NULL;
        pn_big_free(&ticks); return result;
    }
    REQUIRE(a[0]->kind == V_INSTANT, "VM trap: operation requires Instant");
    REQUIRE(a[1]->kind == (!strcmp(name, "instant_add") ? V_DURATION : V_INSTANT),
            "VM trap: invalid instant operand");
    if (!strcmp(name, "instant_before")) return value_bool(pn_big_compare(&a[0]->as.integer, &a[1]->as.integer) < 0);
    PnBigInt ticks = {0};
    bool add = !strcmp(name, "instant_add");
    bool ok = add ? pn_big_add(&ticks, &a[0]->as.integer, &a[1]->as.integer) :
        pn_big_sub(&ticks, &a[0]->as.integer, &a[1]->as.integer);
    Value *result = ok ? value_big(add ? V_INSTANT : V_DURATION, &ticks) : NULL;
    pn_big_free(&ticks); return result;
}
