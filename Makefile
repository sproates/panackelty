.PHONY: all check check-phases check-compiler check-compiler-impl check-bytecode check-bytecode-impl check-vm check-vm-impl test unit unit-impl functional functional-impl native native-check bootstrap bootstrap-check bootstrap-check-impl regenerate-seed install package package-archive package-checksum release-smoke quick-start clean

CFLAGS ?= -O2
export PANACKELTY_STDLIB_PATH := $(abspath src/stdlib)

CHECK_BUDGET_SECONDS ?= 120
INCREMENTAL_BUDGET_SECONDS ?= 15
FUNCTIONAL_BUDGET_SECONDS ?= 75
BOOTSTRAP_BUDGET_SECONDS ?= 60
TIMED := sh tests/run_timed.sh
PROFILE := sh tests/profile_command.sh
PROBE := sh tests/run_probe.sh
PROBES := sh tests/run_probes.sh
export VALIDATION_JOBS ?= 2

.NOTPARALLEL: check-phases

BUILD_DIR ?= build
PREFIX ?= /usr/local
DESTDIR ?=
VERSION := $(strip $(shell sed -n '1p' VERSION))
HOST_SYSTEM := $(shell uname -s)
HOST_ARCH ?= $(shell uname -m)
ifeq ($(HOST_SYSTEM),Darwin)
HOST_OS ?= macos
TAR_OWNER_FLAGS := --uid 0 --gid 0 --uname root --gname root
else ifeq ($(HOST_SYSTEM),Linux)
HOST_OS ?= linux
TAR_OWNER_FLAGS := --owner=root --group=root
else
HOST_OS ?= $(HOST_SYSTEM)
TAR_OWNER_FLAGS :=
endif
PACKAGE_NAME ?= panackelty-$(VERSION)-$(HOST_OS)-$(HOST_ARCH)
PACKAGE_ROOT_NAME ?= panackelty
PACKAGE_STAGE := $(abspath $(BUILD_DIR))/package
PACKAGE_ROOT := $(PACKAGE_STAGE)/$(PACKAGE_ROOT_NAME)
PACKAGE_ARCHIVE := $(abspath $(BUILD_DIR))/$(PACKAGE_NAME).tar.gz
PACKAGE_CHECKSUM := $(PACKAGE_ARCHIVE).sha256
BOOTSTRAP_DIR := $(BUILD_DIR)/bootstrap
SEED_COMPILER ?= bootstrap/compiler-v8.bc
SEED_DIGEST ?= $(SEED_COMPILER).sha256
COMPILER_SOURCE := src/compiler/main.panack
STDLIB_CONFORMANCE := tests/functional/cases/stdlib/main.panack
STAGE1_COMPILER := $(BOOTSTRAP_DIR)/stage1/compiler.bc
STAGE2_COMPILER := $(BOOTSTRAP_DIR)/stage2/compiler.bc
STAGE3_COMPILER := $(BOOTSTRAP_DIR)/stage3/compiler.bc
STAGE1_STDLIB := $(BOOTSTRAP_DIR)/stage1/stdlib-conformance.bc
STAGE2_STDLIB := $(BOOTSTRAP_DIR)/stage2/stdlib-conformance.bc
STAGE3_STDLIB := $(BOOTSTRAP_DIR)/stage3/stdlib-conformance.bc
export PANACK_PROBE_CACHE := $(abspath $(BUILD_DIR))/probes
export PANACK_PROBE_SEED := $(abspath $(SEED_COMPILER))

all: native

check:
	@$(TIMED) check $(CHECK_BUDGET_SECONDS) sh tests/check.sh $(MAKE) --no-print-directory check-phases

check-phases: policy unit functional bootstrap-check quick-start

test: check

check-compiler: native
	@$(TIMED) check-compiler $(INCREMENTAL_BUDGET_SECONDS) $(MAKE) --no-print-directory check-compiler-impl

check-compiler-impl:
	@$(PROFILE) harness/compiler sh tests/harness.sh compiler
	@$(PROFILE) seed-refresh/failure-contracts sh tests/seed_refresh.sh
	@$(MAKE) --no-print-directory native-oracle-artifacts
	@$(PROBES) tests/runner/compiler_lexer_unit.panack \
		tests/runner/compiler_parser_unit.panack \
		tests/runner/compiler_resolver_unit.panack \
		tests/runner/compiler_checker_unit.panack \
		tests/runner/compiler_purity_unit.panack \
		tests/runner/compiler_contracts_unit.panack \
		tests/runner/compiler_integration_unit.panack
	@$(PROFILE) functional/case/cli_commands $(PROBE) tests/runner/main.panack --case cli_commands
	@$(PROFILE) functional/failures $(PROBE) tests/runner/main.panack --failures-only

check-bytecode: native native-module-build
	@$(TIMED) check-bytecode $(INCREMENTAL_BUDGET_SECONDS) $(MAKE) --no-print-directory check-bytecode-impl

check-bytecode-impl:
	@"$(PANACK_NATIVE_MODULE_TEST)"
	@$(PROFILE) probe/bytecode_unit $(PROBE) tests/runner/bytecode_unit.panack
	@$(PROFILE) probe/bytecode_native_unit $(PROBE) tests/runner/bytecode_native_unit.panack
	@$(PROFILE) functional/case/cli_check_disasm $(PROBE) tests/runner/main.panack --case cli_check_disasm
	@$(PROFILE) functional/case/cli_commands $(PROBE) tests/runner/main.panack --case cli_commands

check-vm: native native-module-build native-fault-build
	@$(TIMED) check-vm $(INCREMENTAL_BUDGET_SECONDS) $(MAKE) --no-print-directory check-vm-impl

check-vm-impl:
	@$(PROFILE) native-contracts $(MAKE) --no-print-directory native-vm-contracts native-oracle-contracts
	@$(PROFILE) probe/host_runtime_unit $(PROBE) tests/runner/host_runtime_unit.panack
	@$(PROFILE) functional/case/cli_commands $(PROBE) tests/runner/main.panack --case cli_commands
	@$(PROFILE) functional/case/cli_environment_files $(PROBE) tests/runner/main.panack --case cli_environment_files

unit: native native-module-build native-fault-build
	@$(TIMED) unit $(INCREMENTAL_BUDGET_SECONDS) $(MAKE) --no-print-directory unit-impl

unit-impl:
	@$(PROFILE) harness sh tests/harness.sh
	@$(PROFILE) probe/report_capture_unit $(PROBE) tests/runner/report_capture_unit.panack
	@$(PROFILE) seed-refresh/failure-contracts sh tests/seed_refresh.sh
	@$(PROFILE) native-contracts $(MAKE) --no-print-directory native-vm-contracts native-oracle-contracts
	@$(PROBES) tests/runner/host_runtime_unit.panack \
		tests/runner/bytecode_unit.panack \
		tests/runner/bytecode_native_unit.panack \
		tests/runner/compiler_lexer_unit.panack \
		tests/runner/compiler_parser_unit.panack \
		tests/runner/compiler_resolver_unit.panack \
		tests/runner/compiler_checker_unit.panack \
		tests/runner/compiler_purity_unit.panack \
		tests/runner/compiler_contracts_unit.panack \
		tests/runner/compiler_integration_unit.panack

functional: native
	@$(TIMED) functional $(FUNCTIONAL_BUDGET_SECONDS) $(MAKE) --no-print-directory functional-impl

functional-impl: $(STAGE2_COMPILER)
	@PANACK_TEST_COMPILER="$(abspath $(STAGE2_COMPILER))" $(PROFILE) functional/compiler-driver $(PROBE) tests/runner/compiler_driver.panack
	@mkdir -p "$(BUILD_DIR)"
	@report=$$(mktemp); artifact="$(abspath $(BUILD_DIR))/runner-smoke.bc"; \
		trap 'rm -f "$$report" "$$artifact"' 0; \
		if [ -n "$${PANACK_CHECK_RUNNER_REPORT:-}" ]; then \
			cp "$$PANACK_CHECK_RUNNER_REPORT" "$$report"; \
		else \
			PANACK_TEST_COMPILER="$(abspath $(STAGE2_COMPILER))" \
				$(PROFILE) functional/runner $(PROBE) tests/runner/main.panack > "$$report"; \
		fi && \
		cat "$$report" && \
		PANACK_TEST_COMPILER="$(abspath $(STAGE2_COMPILER))" \
			PANACK_TEST_RUNNER_REPORT="$$report" \
			./panack run tests/functional/cases/runner_smoke/main.panack && \
		./panack compile tests/functional/cases/runner_smoke/main.panack -o "$$artifact" && \
		PANACK_TEST_COMPILER="$(abspath $(STAGE2_COMPILER))" \
			PANACK_TEST_RUNNER_REPORT="$$report" ./panack run "$$artifact"

native: panack-vm

# Compile each VM component separately. Dependency files track header edits.
VM_SOURCES := $(sort $(wildcard src/vm/*.c))
VM_OBJECTS := $(patsubst src/vm/%.c,$(BUILD_DIR)/vm/%.o,$(VM_SOURCES))
VM_LIBRARY_OBJECTS := $(filter-out $(BUILD_DIR)/vm/main.o,$(VM_OBJECTS))
export PANACK_NATIVE_MODULE_TEST := $(abspath $(BUILD_DIR)/vm/test_modules)
VM_WARNINGS := -std=c11 -Wall -Wextra -Werror -pedantic

panack-vm: $(VM_OBJECTS)
	$(PROFILE) "native-build/$@" $(CC) $(CFLAGS) $(LDFLAGS) $(VM_OBJECTS) -o $@ $(LDLIBS)

$(BUILD_DIR)/vm/%.o: src/vm/%.c
	@mkdir -p "$(@D)"
	$(PROFILE) "native-build/$@" $(CC) $(CPPFLAGS) $(CFLAGS) $(VM_WARNINGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/vm/test_modules: tests/unit/vm/native_modules.c $(VM_LIBRARY_OBJECTS)
	$(PROFILE) "native-build/$@" $(CC) $(CPPFLAGS) $(CFLAGS) $(VM_WARNINGS) -Isrc/vm $(LDFLAGS) $< $(VM_LIBRARY_OBJECTS) -o $@ $(LDLIBS)

FAULT_OBJECTS := $(patsubst src/vm/%.c,$(BUILD_DIR)/fault/%.o,$(filter-out src/vm/main.c,$(VM_SOURCES)))
export PANACK_NATIVE_FAULT_TEST := $(abspath $(BUILD_DIR)/fault/test_faults)

$(BUILD_DIR)/fault/%.o: src/vm/%.c tests/unit/vm/fault_injection.h
	@mkdir -p "$(@D)"
	$(PROFILE) "native-build/$@" $(CC) $(CPPFLAGS) $(CFLAGS) $(VM_WARNINGS) -Isrc/vm -DPANACK_TEST_INJECT -include tests/unit/vm/fault_injection.h -MMD -MP -c $< -o $@

$(BUILD_DIR)/fault/injection.o: tests/unit/vm/fault_injection.c tests/unit/vm/fault_injection.h
	@mkdir -p "$(@D)"
	$(PROFILE) "native-build/$@" $(CC) $(CPPFLAGS) $(CFLAGS) $(VM_WARNINGS) -Isrc/vm -c $< -o $@

$(BUILD_DIR)/fault/test_faults: tests/unit/vm/native_faults.c $(BUILD_DIR)/fault/injection.o $(FAULT_OBJECTS)
	$(PROFILE) "native-build/$@" $(CC) $(CPPFLAGS) $(CFLAGS) $(VM_WARNINGS) -Isrc/vm -DPANACK_TEST_INJECT -include tests/unit/vm/fault_injection.h $(LDFLAGS) $< $(FAULT_OBJECTS) $(BUILD_DIR)/fault/injection.o -o $@ $(LDLIBS)

.PHONY: native-fault-build native-fault
native-fault-build: $(BUILD_DIR)/fault/test_faults
native-fault: native-fault-build
	UBSAN_OPTIONS=halt_on_error=1 "$(abspath $(BUILD_DIR)/fault/test_faults)"

-include $(FAULT_OBJECTS:.o=.d)

.PHONY: native-unit native-module-build native-sanitize native-instrumented-check
native-module-build: $(BUILD_DIR)/vm/test_modules

$(BUILD_DIR)/vm/panack-vm: $(VM_OBJECTS)
	$(PROFILE) "native-build/$@" $(CC) $(CFLAGS) $(LDFLAGS) $(VM_OBJECTS) -o $@ $(LDLIBS)

native-unit: $(BUILD_DIR)/vm/test_modules
	UBSAN_OPTIONS=halt_on_error=1 "$(abspath $(BUILD_DIR)/vm/test_modules)"

export PANACK_NATIVE_BIGINT_TEST := $(abspath $(BUILD_DIR)/vm/test_bigint)
$(BUILD_DIR)/vm/test_bigint: tests/unit/vm/native_bigint.c $(BUILD_DIR)/vm/bigint.o
	$(PROFILE) "native-build/$@" $(CC) $(CPPFLAGS) $(CFLAGS) $(VM_WARNINGS) -Isrc/vm $(LDFLAGS) $< $(BUILD_DIR)/vm/bigint.o -o $@ $(LDLIBS)

.PHONY: native-vm-contracts
native-vm-contracts: $(BUILD_DIR)/vm/panack-vm native-module-build native-fault-build $(BUILD_DIR)/vm/test_bigint
	@CC="$(CC)" $(PROFILE) native-headers sh tests/native_headers.sh
	@PANACK_NATIVE_BINARY="$(abspath $(BUILD_DIR)/vm/panack-vm)" $(PROFILE) probe/vm_unit env PANACK_PROBE_VM="$(abspath $(BUILD_DIR)/vm/panack-vm)" $(PROBE) tests/runner/vm_unit.panack

.PHONY: native-oracle-contracts native-oracle-contracts-impl native-oracle-artifacts
native-oracle-artifacts: $(BUILD_DIR)/vm/panack-vm
	@PANACK_NATIVE_BINARY="$(abspath $(BUILD_DIR)/vm/panack-vm)" SEED_COMPILER="$(SEED_COMPILER)" $(PROFILE) native-oracle sh tests/native_oracle_contracts.sh artifacts

native-oracle-contracts: native
	@$(MAKE) --no-print-directory native-oracle-contracts-impl

native-oracle-contracts-impl: $(BUILD_DIR)/vm/panack-vm native-module-build $(STAGE2_COMPILER)
	@PANACK_NATIVE_BINARY="$(abspath $(BUILD_DIR)/vm/panack-vm)" SEED_COMPILER="$(SEED_COMPILER)" PANACK_TEST_COMPILER="$(abspath $(STAGE2_COMPILER))" $(PROFILE) native-oracle sh tests/native_oracle_contracts.sh

# Keep instrumentation isolated from ordinary build artifacts and the CLI binary.
# Corpus programs call the public CLI too. Build that ordinary binary before
# recursing with instrumentation, without replacing it with an instrumented VM.
native-sanitize: native
	$(MAKE) BUILD_DIR=build/sanitize CFLAGS="-O1 -g -fno-omit-frame-pointer -fsanitize=address,undefined" LDFLAGS="-fsanitize=address,undefined" native-instrumented-check

native-instrumented-check: $(BUILD_DIR)/vm/panack-vm
	@PANACK_NATIVE_BINARY="$(abspath $(BUILD_DIR)/vm/panack-vm)" UBSAN_OPTIONS=halt_on_error=1 $(MAKE) --no-print-directory native-vm-contracts
	@UBSAN_OPTIONS=halt_on_error=1 $(MAKE) --no-print-directory native-oracle-contracts-impl

# LLVM branch coverage uses the same native corpus in a separate build tree.
LLVM_CC ?= clang
LLVM_COV ?= $(shell command -v llvm-cov 2>/dev/null || xcrun --find llvm-cov 2>/dev/null)
LLVM_PROFDATA ?= $(shell command -v llvm-profdata 2>/dev/null || xcrun --find llvm-profdata 2>/dev/null)
.PHONY: native-coverage
native-coverage: native
	@test -n "$(LLVM_COV)" -a -n "$(LLVM_PROFDATA)" || { echo "LLVM coverage tools are required" >&2; exit 1; }
	@mkdir -p build/coverage
	@rm -f build/coverage/*.profraw
	LLVM_PROFILE_FILE="$(abspath build/coverage)/%p.profraw" $(MAKE) CC="$(LLVM_CC)" BUILD_DIR=build/coverage CFLAGS="-O1 -g -fprofile-instr-generate -fcoverage-mapping" LDFLAGS="-fprofile-instr-generate" native-instrumented-check
	"$(LLVM_PROFDATA)" merge -sparse build/coverage/*.profraw -o build/coverage/coverage.profdata
	"$(LLVM_COV)" report build/coverage/vm/panack-vm -object build/coverage/vm/test_modules -object build/coverage/fault/test_faults -object build/coverage/vm/test_bigint -instr-profile=build/coverage/coverage.profdata src/vm > build/coverage/summary.txt
	@cat build/coverage/summary.txt
	"$(LLVM_COV)" show build/coverage/vm/panack-vm -object build/coverage/vm/test_modules -object build/coverage/fault/test_faults -object build/coverage/vm/test_bigint -instr-profile=build/coverage/coverage.profdata -show-branches=count -format=html -output-dir=build/coverage/html src/vm

-include $(VM_OBJECTS:.o=.d)

$(STAGE1_COMPILER): $(SEED_COMPILER) probe-inputs
	@mkdir -p "$(dir $@)"
	@cmp -s "$(SEED_COMPILER)" "$@" || cp "$(SEED_COMPILER)" "$@"
	@./panack-vm check "$@"

.PHONY: probe-inputs
probe-inputs:

# The helper checks content, including imports, even when timestamps are restored.
$(STAGE2_COMPILER): $(STAGE1_COMPILER) probe-inputs
	@mkdir -p "$(dir $@)"
	@PANACK_PROBE_SEED="$(abspath $(STAGE1_COMPILER))" $(PROFILE) "bootstrap-build/$@" $(PROBE) --compile $(COMPILER_SOURCE) "$@"
	@./panack-vm check "$@"

$(STAGE3_COMPILER): $(STAGE2_COMPILER) $(COMPILER_SOURCE)
	mkdir -p $(dir $@)
	$(PROFILE) "bootstrap-build/$@" ./panack-vm run $(STAGE2_COMPILER) compile $(COMPILER_SOURCE) -o $@
	./panack-vm check $@

$(STAGE1_STDLIB): $(STAGE1_COMPILER) $(STDLIB_CONFORMANCE)
	$(PROFILE) "bootstrap-build/$@" ./panack-vm run $(STAGE1_COMPILER) compile $(STDLIB_CONFORMANCE) -o $@

$(STAGE2_STDLIB): $(STAGE2_COMPILER) $(STDLIB_CONFORMANCE)
	$(PROFILE) "bootstrap-build/$@" ./panack-vm run $(STAGE2_COMPILER) compile $(STDLIB_CONFORMANCE) -o $@

$(STAGE3_STDLIB): $(STAGE3_COMPILER) $(STDLIB_CONFORMANCE)
	$(PROFILE) "bootstrap-build/$@" ./panack-vm run $(STAGE3_COMPILER) compile $(STDLIB_CONFORMANCE) -o $@

bootstrap: native $(STAGE3_COMPILER) $(STAGE1_STDLIB) $(STAGE2_STDLIB) $(STAGE3_STDLIB)

bootstrap-check: native
	@$(TIMED) bootstrap $(BOOTSTRAP_BUDGET_SECONDS) $(MAKE) --no-print-directory bootstrap-check-impl

bootstrap-check-impl: $(STAGE3_COMPILER) $(STAGE1_STDLIB) $(STAGE2_STDLIB) $(STAGE3_STDLIB)
	cmp $(STAGE2_COMPILER) $(STAGE3_COMPILER)
	cmp $(STAGE1_STDLIB) $(STAGE2_STDLIB)
	cmp $(STAGE2_STDLIB) $(STAGE3_STDLIB)
	$(PROFILE) seed-refresh/native-staging sh tests/seed_refresh.sh --native

native-check: bootstrap-check
	sh tests/native_conformance.sh

install: native
	install -d "$(DESTDIR)$(PREFIX)/bin"
	install -d "$(DESTDIR)$(PREFIX)/libexec/panackelty"
	install -d "$(DESTDIR)$(PREFIX)/share/panackelty"
	install -d "$(DESTDIR)$(PREFIX)/share/panackelty/stdlib"
	install -d "$(DESTDIR)$(PREFIX)/share/doc/panackelty"
	install -m 755 panack "$(DESTDIR)$(PREFIX)/bin/panack"
	install -m 755 panack-vm "$(DESTDIR)$(PREFIX)/libexec/panackelty/panack-vm"
	install -m 644 VERSION "$(DESTDIR)$(PREFIX)/share/panackelty/VERSION"
	install -m 644 $(SEED_COMPILER) "$(DESTDIR)$(PREFIX)/share/panackelty/compiler-v8.bc"
	install -m 644 src/stdlib/*.panack "$(DESTDIR)$(PREFIX)/share/panackelty/stdlib/"
	install -m 644 LICENSE CHANGELOG.md RELEASE_POLICY.md SECURITY.md SPEC.md "$(DESTDIR)$(PREFIX)/share/doc/panackelty/"

package: native-check
	$(MAKE) quick-start

package-archive: native
	rm -rf "$(PACKAGE_STAGE)"
	$(MAKE) install DESTDIR="$(PACKAGE_STAGE)" PREFIX="/$(PACKAGE_ROOT_NAME)"
	install -d "$(PACKAGE_ROOT)/examples"
	install -m 644 examples/README.md examples/*.panack "$(PACKAGE_ROOT)/examples/"
	install -m 644 README.md LICENSE "$(PACKAGE_ROOT)/"
	COPYFILE_DISABLE=1 tar $(TAR_OWNER_FLAGS) -C "$(PACKAGE_STAGE)" -czf "$(PACKAGE_ARCHIVE)" "$(PACKAGE_ROOT_NAME)"

release-smoke: package-archive
	@$(TIMED) release-smoke $(INCREMENTAL_BUDGET_SECONDS) sh tests/release_archive_smoke.sh "$(PACKAGE_ARCHIVE)" "$(VERSION)"

package-checksum: release-smoke
	@archive="$(PACKAGE_ARCHIVE)" && \
	cd "$${archive%/*}" && \
	archive=$${archive##*/} && \
	if command -v sha256sum >/dev/null 2>&1; then \
		sha256sum "$$archive"; \
	elif command -v shasum >/dev/null 2>&1; then \
		shasum -a 256 "$$archive"; \
	else \
		echo "package: no SHA-256 utility found" >&2; \
		exit 1; \
	fi >"$(PACKAGE_CHECKSUM)"

quick-start: package-checksum
	@$(TIMED) quick-start $(INCREMENTAL_BUDGET_SECONDS) sh tests/quick_start.sh "$(PACKAGE_ARCHIVE)" "$(VERSION)"

regenerate-seed: native
	sh bootstrap/regenerate-seed.sh ./panack-vm "$(SEED_COMPILER)" "$(SEED_DIGEST)" "$(COMPILER_SOURCE)" "$(STDLIB_CONFORMANCE)"

clean:
	rm -f panack-vm
	rm -rf build

.PHONY: harness
harness: native
	@$(TIMED) harness $(INCREMENTAL_BUDGET_SECONDS) sh tests/harness.sh

.PHONY: docs ci-check policy check-no-interpreter
docs:
	@git diff --check HEAD
	@bash scripts/check_docs.sh

ci-check:
	@bash tests/ci_scope.sh

policy:
	@bash tests/ci_scope.sh
	@sh tests/no_python.sh
	@sh tests/no_python_test.sh

check-no-interpreter:
	@sh tests/without_interpreter.sh
