.PHONY: all check check-phases check-compiler check-compiler-impl check-bytecode check-bytecode-impl check-vm check-vm-impl test unit unit-impl functional functional-impl native native-check bootstrap bootstrap-check bootstrap-check-impl regenerate-seed install package package-archive package-checksum release-smoke quick-start clean

PYTHON ?= python3
CFLAGS ?= -O2
export PYTHONDONTWRITEBYTECODE := 1
export PANACKELTY_STDLIB_PATH := $(abspath src/stdlib)

CHECK_BUDGET_SECONDS ?= 120
INCREMENTAL_BUDGET_SECONDS ?= 15
FUNCTIONAL_BUDGET_SECONDS ?= 75
BOOTSTRAP_BUDGET_SECONDS ?= 60
TIMED := sh tests/run_timed.sh

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
COMPILER_SOURCE := src/compiler/main.panack
STDLIB_CONFORMANCE := tests/functional/cases/stdlib/main.panack
STAGE1_COMPILER := $(BOOTSTRAP_DIR)/stage1/compiler.bc
STAGE2_COMPILER := $(BOOTSTRAP_DIR)/stage2/compiler.bc
STAGE3_COMPILER := $(BOOTSTRAP_DIR)/stage3/compiler.bc
STAGE1_STDLIB := $(BOOTSTRAP_DIR)/stage1/stdlib-conformance.bc
STAGE2_STDLIB := $(BOOTSTRAP_DIR)/stage2/stdlib-conformance.bc
STAGE3_STDLIB := $(BOOTSTRAP_DIR)/stage3/stdlib-conformance.bc

all: native

check:
	@$(TIMED) check $(CHECK_BUDGET_SECONDS) $(MAKE) --no-print-directory check-phases

check-phases: unit functional bootstrap-check quick-start

test: check

check-compiler: native
	@$(TIMED) check-compiler $(INCREMENTAL_BUDGET_SECONDS) $(MAKE) --no-print-directory check-compiler-impl

check-compiler-impl:
	@$(PYTHON) -m unittest discover -s tests/unit/compiler -t . -p 'test_*.py' -q
	@./panack run tests/runner/compiler_lexer_unit.panack
	@./panack run tests/runner/compiler_parser_unit.panack
	@./panack run tests/runner/compiler_resolver_unit.panack
	@./panack run tests/runner/compiler_checker_unit.panack
	@./panack run tests/runner/compiler_purity_unit.panack
	@./panack run tests/runner/compiler_contracts_unit.panack
	@./panack run tests/runner/compiler_integration_unit.panack
	@./panack run tests/runner/main.panack --case cli_commands
	@./panack run tests/runner/main.panack --failures-only

check-bytecode: native native-module-build
	@$(TIMED) check-bytecode $(INCREMENTAL_BUDGET_SECONDS) $(MAKE) --no-print-directory check-bytecode-impl

check-bytecode-impl:
	@"$(PANACK_NATIVE_MODULE_TEST)"
	@$(PYTHON) -m unittest discover -s tests/unit/bytecode -t . -p 'test_*.py' -q
	@./panack run tests/runner/bytecode_unit.panack
	@./panack run tests/runner/bytecode_native_unit.panack
	@./panack run tests/runner/main.panack --case cli_check_disasm
	@./panack run tests/runner/main.panack --case cli_commands

check-vm: native native-module-build native-fault-build
	@$(TIMED) check-vm $(INCREMENTAL_BUDGET_SECONDS) $(MAKE) --no-print-directory check-vm-impl

check-vm-impl:
	@$(MAKE) --no-print-directory native-vm-contracts
	@$(PYTHON) -m unittest discover -s tests/unit/vm -t . -p 'test_*.py' -q
	@./panack run tests/runner/host_runtime_unit.panack
	@./panack run tests/runner/main.panack --case cli_commands
	@./panack run tests/runner/main.panack --case cli_environment_files

unit: native native-module-build native-fault-build
	@$(TIMED) unit $(INCREMENTAL_BUDGET_SECONDS) $(MAKE) --no-print-directory unit-impl

unit-impl:
	@$(MAKE) --no-print-directory native-vm-contracts
	@$(PYTHON) -m unittest discover -s tests/unit -t . -p 'test_*.py' -q
	@./panack run tests/runner/host_runtime_unit.panack
	@./panack run tests/runner/bytecode_unit.panack
	@./panack run tests/runner/bytecode_native_unit.panack
	@./panack run tests/runner/compiler_lexer_unit.panack
	@./panack run tests/runner/compiler_parser_unit.panack
	@./panack run tests/runner/compiler_resolver_unit.panack
	@./panack run tests/runner/compiler_checker_unit.panack
	@./panack run tests/runner/compiler_purity_unit.panack
	@./panack run tests/runner/compiler_contracts_unit.panack
	@./panack run tests/runner/compiler_integration_unit.panack

functional: native
	@$(TIMED) functional $(FUNCTIONAL_BUDGET_SECONDS) $(MAKE) --no-print-directory functional-impl

functional-impl: $(STAGE2_COMPILER)
	@PANACK_TEST_COMPILER="$(abspath $(STAGE2_COMPILER))" ./panack run tests/runner/compiler_driver.panack
	@mkdir -p "$(BUILD_DIR)"
	@report=$$(mktemp); artifact="$(abspath $(BUILD_DIR))/runner-smoke.bc"; \
		trap 'rm -f "$$report" "$$artifact"' 0; \
		PANACK_TEST_COMPILER="$(abspath $(STAGE2_COMPILER))" \
			./panack run tests/runner/main.panack > "$$report" && \
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
	$(CC) $(CFLAGS) $(LDFLAGS) $(VM_OBJECTS) -o $@ $(LDLIBS)

$(BUILD_DIR)/vm/%.o: src/vm/%.c
	@mkdir -p "$(@D)"
	$(CC) $(CPPFLAGS) $(CFLAGS) $(VM_WARNINGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/vm/test_modules: tests/unit/vm/native_modules.c $(VM_LIBRARY_OBJECTS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(VM_WARNINGS) -Isrc/vm $(LDFLAGS) $< $(VM_LIBRARY_OBJECTS) -o $@ $(LDLIBS)

FAULT_OBJECTS := $(patsubst src/vm/%.c,$(BUILD_DIR)/fault/%.o,$(filter-out src/vm/main.c,$(VM_SOURCES)))
export PANACK_NATIVE_FAULT_TEST := $(abspath $(BUILD_DIR)/fault/test_faults)

$(BUILD_DIR)/fault/%.o: src/vm/%.c tests/unit/vm/fault_injection.h
	@mkdir -p "$(@D)"
	$(CC) $(CPPFLAGS) $(CFLAGS) $(VM_WARNINGS) -Isrc/vm -DPANACK_TEST_INJECT -include tests/unit/vm/fault_injection.h -MMD -MP -c $< -o $@

$(BUILD_DIR)/fault/injection.o: tests/unit/vm/fault_injection.c tests/unit/vm/fault_injection.h
	@mkdir -p "$(@D)"
	$(CC) $(CPPFLAGS) $(CFLAGS) $(VM_WARNINGS) -Isrc/vm -c $< -o $@

$(BUILD_DIR)/fault/test_faults: tests/unit/vm/native_faults.c $(BUILD_DIR)/fault/injection.o $(FAULT_OBJECTS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(VM_WARNINGS) -Isrc/vm -DPANACK_TEST_INJECT -include tests/unit/vm/fault_injection.h $(LDFLAGS) $< $(FAULT_OBJECTS) $(BUILD_DIR)/fault/injection.o -o $@ $(LDLIBS)

.PHONY: native-fault-build native-fault
native-fault-build: $(BUILD_DIR)/fault/test_faults
native-fault: native-fault-build
	UBSAN_OPTIONS=halt_on_error=1 "$(abspath $(BUILD_DIR)/fault/test_faults)"

-include $(FAULT_OBJECTS:.o=.d)

.PHONY: native-unit native-module-build native-sanitize native-instrumented-check
native-module-build: $(BUILD_DIR)/vm/test_modules

$(BUILD_DIR)/vm/panack-vm: $(VM_OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(VM_OBJECTS) -o $@ $(LDLIBS)

native-unit: $(BUILD_DIR)/vm/test_modules
	UBSAN_OPTIONS=halt_on_error=1 "$(abspath $(BUILD_DIR)/vm/test_modules)"

export PANACK_NATIVE_BIGINT_TEST := $(abspath $(BUILD_DIR)/vm/test_bigint)
$(BUILD_DIR)/vm/test_bigint: tests/unit/vm/native_bigint.c $(BUILD_DIR)/vm/bigint.o
	$(CC) $(CPPFLAGS) $(CFLAGS) $(VM_WARNINGS) -Isrc/vm $(LDFLAGS) $< $(BUILD_DIR)/vm/bigint.o -o $@ $(LDLIBS)

.PHONY: native-vm-contracts
native-vm-contracts: $(BUILD_DIR)/vm/panack-vm native-module-build native-fault-build $(BUILD_DIR)/vm/test_bigint
	@CC="$(CC)" sh tests/native_headers.sh
	@PANACK_NATIVE_BINARY="$(abspath $(BUILD_DIR)/vm/panack-vm)" "$(abspath $(BUILD_DIR)/vm/panack-vm)" run "$(SEED_COMPILER)" run tests/runner/vm_unit.panack

# Keep instrumentation isolated from ordinary build artifacts and the CLI binary.
native-sanitize:
	$(MAKE) BUILD_DIR=build/sanitize CFLAGS="-O1 -g -fno-omit-frame-pointer -fsanitize=address,undefined" LDFLAGS="-fsanitize=address,undefined" native-instrumented-check

native-instrumented-check: $(BUILD_DIR)/vm/panack-vm
	@PANACK_NATIVE_BINARY="$(abspath $(BUILD_DIR)/vm/panack-vm)" UBSAN_OPTIONS=halt_on_error=1 $(MAKE) --no-print-directory native-vm-contracts
	@PANACK_NATIVE_BINARY="$(abspath $(BUILD_DIR)/vm/panack-vm)" UBSAN_OPTIONS=halt_on_error=1 $(PYTHON) -B -m unittest -q tests.unit.vm.test_native_loader tests.unit.vm.test_native_execution tests.unit.vm.test_native_modules

# LLVM branch coverage uses the same native corpus in a separate build tree.
LLVM_CC ?= clang
LLVM_COV ?= $(shell command -v llvm-cov 2>/dev/null || xcrun --find llvm-cov 2>/dev/null)
LLVM_PROFDATA ?= $(shell command -v llvm-profdata 2>/dev/null || xcrun --find llvm-profdata 2>/dev/null)
.PHONY: native-coverage
native-coverage:
	@test -n "$(LLVM_COV)" -a -n "$(LLVM_PROFDATA)" || { echo "LLVM coverage tools are required" >&2; exit 1; }
	@mkdir -p build/coverage
	@rm -f build/coverage/*.profraw
	LLVM_PROFILE_FILE="$(abspath build/coverage)/%p.profraw" $(MAKE) CC="$(LLVM_CC)" BUILD_DIR=build/coverage CFLAGS="-O1 -g -fprofile-instr-generate -fcoverage-mapping" LDFLAGS="-fprofile-instr-generate" native-instrumented-check
	"$(LLVM_PROFDATA)" merge -sparse build/coverage/*.profraw -o build/coverage/coverage.profdata
	"$(LLVM_COV)" report build/coverage/vm/panack-vm -object build/coverage/vm/test_modules -object build/coverage/fault/test_faults -object build/coverage/vm/test_bigint -instr-profile=build/coverage/coverage.profdata src/vm > build/coverage/summary.txt
	@cat build/coverage/summary.txt
	"$(LLVM_COV)" show build/coverage/vm/panack-vm -object build/coverage/vm/test_modules -object build/coverage/fault/test_faults -object build/coverage/vm/test_bigint -instr-profile=build/coverage/coverage.profdata -show-branches=count -format=html -output-dir=build/coverage/html src/vm

-include $(VM_OBJECTS:.o=.d)

$(STAGE1_COMPILER): $(SEED_COMPILER)
	mkdir -p $(dir $@)
	cp $(SEED_COMPILER) $@
	./panack-vm check $@

$(STAGE2_COMPILER): $(STAGE1_COMPILER) $(COMPILER_SOURCE)
	mkdir -p $(dir $@)
	./panack-vm run $(STAGE1_COMPILER) compile $(COMPILER_SOURCE) -o $@
	./panack-vm check $@

$(STAGE3_COMPILER): $(STAGE2_COMPILER) $(COMPILER_SOURCE)
	mkdir -p $(dir $@)
	./panack-vm run $(STAGE2_COMPILER) compile $(COMPILER_SOURCE) -o $@
	./panack-vm check $@

$(STAGE1_STDLIB): $(STAGE1_COMPILER) $(STDLIB_CONFORMANCE)
	./panack-vm run $(STAGE1_COMPILER) compile $(STDLIB_CONFORMANCE) -o $@

$(STAGE2_STDLIB): $(STAGE2_COMPILER) $(STDLIB_CONFORMANCE)
	./panack-vm run $(STAGE2_COMPILER) compile $(STDLIB_CONFORMANCE) -o $@

$(STAGE3_STDLIB): $(STAGE3_COMPILER) $(STDLIB_CONFORMANCE)
	./panack-vm run $(STAGE3_COMPILER) compile $(STDLIB_CONFORMANCE) -o $@

bootstrap: native $(STAGE3_COMPILER) $(STAGE1_STDLIB) $(STAGE2_STDLIB) $(STAGE3_STDLIB)

bootstrap-check: native
	@$(TIMED) bootstrap $(BOOTSTRAP_BUDGET_SECONDS) $(MAKE) --no-print-directory bootstrap-check-impl

bootstrap-check-impl: $(STAGE3_COMPILER) $(STAGE1_STDLIB) $(STAGE2_STDLIB) $(STAGE3_STDLIB)
	cmp $(STAGE2_COMPILER) $(STAGE3_COMPILER)
	cmp $(STAGE2_STDLIB) $(STAGE3_STDLIB)

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

regenerate-seed:
	$(PYTHON) -B src/bootstrap/panackelty.py compile $(COMPILER_SOURCE) -o $(SEED_COMPILER)

clean:
	rm -f panack-vm
	rm -rf build
	find . -type d -name '__pycache__' -prune -exec rm -rf {} +
	find . -type f \( -name '*.pyc' -o -name '*.pyo' \) -delete
