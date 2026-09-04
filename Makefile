.PHONY: all check check-phases check-compiler check-compiler-impl check-bytecode check-bytecode-impl check-vm check-vm-impl test unit functional functional-impl native native-check bootstrap bootstrap-check bootstrap-check-impl regenerate-seed install package package-archive package-checksum release-smoke quick-start clean

PYTHON ?= python3
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
TAR_OWNER_FLAGS := --owner=0 --group=0 --numeric-owner
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
SEED_COMPILER ?= bootstrap/compiler-v7.bc
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
	@$(PYTHON) -m unittest -q \
		tests.functional.test_programs.PanackeltyProgramTests.test_bare_source_path_runs_program \
		tests.functional.test_programs.PanackeltyProgramTests.test_invalid_source_programs_fail_check \
		tests.functional.test_programs.PanackeltyProgramTests.test_invalid_source_programs_fail_compile_without_artifacts

check-bytecode: native
	@$(TIMED) check-bytecode $(INCREMENTAL_BUDGET_SECONDS) $(MAKE) --no-print-directory check-bytecode-impl

check-bytecode-impl:
	@$(PYTHON) -m unittest discover -s tests/unit/bytecode -t . -p 'test_*.py' -q
	@$(PYTHON) -m unittest -q \
		tests.functional.test_programs.PanackeltyProgramTests.test_check_accepts_source_and_bytecode \
		tests.functional.test_programs.PanackeltyProgramTests.test_compile_default_output_and_bare_bytecode_path \
		tests.functional.test_programs.PanackeltyProgramTests.test_disasm_matches_for_source_and_bytecode \
		tests.functional.test_programs.PanackeltyProgramTests.test_disasm_rejects_malformed_bytecode

check-vm: native
	@$(TIMED) check-vm $(INCREMENTAL_BUDGET_SECONDS) $(MAKE) --no-print-directory check-vm-impl

check-vm-impl:
	@$(PYTHON) -m unittest discover -s tests/unit/vm -t . -p 'test_*.py' -q
	@$(PYTHON) -m unittest -q \
		tests.functional.test_programs.PanackeltyProgramTests.test_compile_default_output_and_bare_bytecode_path \
		tests.functional.test_programs.PanackeltyProgramTests.test_program_controls_stderr_and_exit_status \
		tests.functional.test_programs.PanackeltyProgramTests.test_public_cli_file_io_round_trips_and_failures \
		tests.functional.test_programs.PanackeltyProgramTests.test_public_cli_reports_denied_file_io \
		tests.functional.test_programs.PanackeltyProgramTests.test_run_passes_program_arguments \
		tests.functional.test_programs.PanackeltyProgramTests.test_standard_library_reads_the_process_environment

unit: native
	@$(TIMED) unit $(INCREMENTAL_BUDGET_SECONDS) $(PYTHON) -m unittest discover -s tests/unit -t . -p 'test_*.py' -q

functional: native
	@$(TIMED) functional $(FUNCTIONAL_BUDGET_SECONDS) $(MAKE) --no-print-directory functional-impl

functional-impl: $(STAGE2_COMPILER)
	@PANACK_TEST_COMPILER=$(abspath $(STAGE2_COMPILER)) $(PYTHON) -m unittest discover -s tests/functional -p 'test_*.py' -q

native: panack-vm

panack-vm: src/vm/native.c src/vm/bigint.c src/vm/bigint.h
	$(CC) -std=c11 -Wall -Wextra -Werror -pedantic src/vm/native.c src/vm/bigint.c -o panack-vm

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
	install -d $(DESTDIR)$(PREFIX)/bin
	install -d $(DESTDIR)$(PREFIX)/libexec/panackelty
	install -d $(DESTDIR)$(PREFIX)/share/panackelty
	install -d $(DESTDIR)$(PREFIX)/share/panackelty/stdlib
	install -d $(DESTDIR)$(PREFIX)/share/doc/panackelty
	install -m 755 panack $(DESTDIR)$(PREFIX)/bin/panack
	install -m 755 panack-vm $(DESTDIR)$(PREFIX)/libexec/panackelty/panack-vm
	install -m 644 VERSION $(DESTDIR)$(PREFIX)/share/panackelty/VERSION
	install -m 644 $(SEED_COMPILER) $(DESTDIR)$(PREFIX)/share/panackelty/compiler-v7.bc
	install -m 644 src/stdlib/*.panack $(DESTDIR)$(PREFIX)/share/panackelty/stdlib/
	install -m 644 LICENSE CHANGELOG.md RELEASE_POLICY.md SECURITY.md SPEC.md $(DESTDIR)$(PREFIX)/share/doc/panackelty/

package: native-check
	$(MAKE) quick-start

package-archive: native
	rm -rf $(PACKAGE_STAGE)
	$(MAKE) install DESTDIR=$(PACKAGE_STAGE) PREFIX=/$(PACKAGE_ROOT_NAME)
	install -d $(PACKAGE_ROOT)/examples
	install -m 644 examples/README.md examples/*.panack $(PACKAGE_ROOT)/examples/
	install -m 644 README.md LICENSE $(PACKAGE_ROOT)/
	COPYFILE_DISABLE=1 tar $(TAR_OWNER_FLAGS) -C $(PACKAGE_STAGE) -czf $(PACKAGE_ARCHIVE) $(PACKAGE_ROOT_NAME)

release-smoke: package-archive
	@$(TIMED) release-smoke $(INCREMENTAL_BUDGET_SECONDS) sh tests/release_archive_smoke.sh $(PACKAGE_ARCHIVE) $(VERSION)

package-checksum: release-smoke
	@cd $(dir $(PACKAGE_ARCHIVE)) && \
	archive=$(notdir $(PACKAGE_ARCHIVE)) && \
	if command -v sha256sum >/dev/null 2>&1; then \
		sha256sum "$$archive"; \
	elif command -v shasum >/dev/null 2>&1; then \
		shasum -a 256 "$$archive"; \
	else \
		echo "package: no SHA-256 utility found" >&2; \
		exit 1; \
	fi >$(notdir $(PACKAGE_CHECKSUM))

quick-start: package-checksum
	@$(TIMED) quick-start $(INCREMENTAL_BUDGET_SECONDS) sh tests/quick_start.sh $(PACKAGE_ARCHIVE) $(VERSION)

regenerate-seed:
	$(PYTHON) -B src/bootstrap/panackelty.py compile $(COMPILER_SOURCE) -o $(SEED_COMPILER)

clean:
	rm -f panack-vm
	rm -rf build
	find . -type d -name '__pycache__' -prune -exec rm -rf {} +
	find . -type f \( -name '*.pyc' -o -name '*.pyo' \) -delete
