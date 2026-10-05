#include "coverage.h"

#include <stdlib.h>

struct VMCoverage {
    Program *program;
    VMCoverageCell *cells;
    size_t *offsets;
    uint64_t *entries;
    size_t count, bytes;
    uint64_t ceiling;
    unsigned flags, terminal;
    bool started;
};

VMCoverage *vm_coverage_create(Program *program, size_t cell_limit, uint64_t ceiling)
{
    if (!program || !ceiling || !cell_limit || cell_limit > VM_COVERAGE_MAX_CELLS ||
        program->count > VM_COVERAGE_MAX_CELLS) {
        return NULL;
    }
    size_t count = 0;
    for (size_t i = 0; i < program->count; i++) {
        if (program->functions[i].ins_count > cell_limit - count) {
            return NULL;
        }
        count += program->functions[i].ins_count;
    }
    VMCoverage *coverage = calloc(1, sizeof(*coverage));
    if (!coverage) {
        return NULL;
    }
    coverage->program = program;
    coverage->ceiling = ceiling;
    coverage->count = count;
    coverage->cells = calloc(count ? count : 1, sizeof(*coverage->cells));
    coverage->offsets = calloc(program->count ? program->count : 1, sizeof(*coverage->offsets));
    coverage->entries = calloc(program->count ? program->count : 1, sizeof(*coverage->entries));
    if (!coverage->cells || !coverage->offsets || !coverage->entries) {
        vm_coverage_destroy(coverage);
        return NULL;
    }
    size_t offset = 0;
    for (size_t i = 0; i < program->count; i++) {
        coverage->offsets[i] = offset;
        offset += program->functions[i].ins_count;
    }
    coverage->bytes = sizeof(*coverage) + (count ? count : 1) * sizeof(*coverage->cells) +
        (program->count ? program->count : 1) * (sizeof(*coverage->offsets) + sizeof(*coverage->entries));
    return coverage;
}

void vm_coverage_destroy(VMCoverage *coverage)
{
    if (coverage) {
        free(coverage->cells);
        free(coverage->offsets);
        free(coverage->entries);
        free(coverage);
    }
}

size_t vm_coverage_bytes(const VMCoverage *coverage) { return coverage->bytes; }
unsigned vm_coverage_flags(const VMCoverage *coverage) { return coverage->flags; }

void vm_coverage_gap(VMCoverage *coverage)
{
    if (coverage) {
        coverage->flags |= 1;
    }
}

void vm_coverage_start(VMCoverage *coverage, Program *program)
{
    if (coverage) {
        if (coverage->started || coverage->program != program) {
            vm_coverage_gap(coverage);
        }
        coverage->started = true;
    }
}

size_t vm_coverage_function(VMCoverage *coverage, Function *function)
{
    for (size_t i = 0; i < coverage->program->count; i++) {
        if (&coverage->program->functions[i] == function) {
            return i;
        }
    }
    vm_coverage_gap(coverage);
    return SIZE_MAX;
}

static void increment(VMCoverage *coverage, uint64_t *value)
{
    if (*value == coverage->ceiling) {
        coverage->flags |= 2;
    } else {
        (*value)++;
    }
}

void vm_coverage_enter(VMCoverage *coverage, size_t function)
{
    if (function < coverage->program->count) {
        increment(coverage, &coverage->entries[function]);
    }
}

uint64_t vm_coverage_entries(const VMCoverage *coverage, size_t function)
{
    return function < coverage->program->count ? coverage->entries[function] : 0;
}

const VMCoverageCell *vm_coverage_cell(const VMCoverage *coverage, size_t function, size_t pc)
{
    if (function >= coverage->program->count || pc >= coverage->program->functions[function].ins_count) {
        return NULL;
    }
    return &coverage->cells[coverage->offsets[function] + pc];
}

void vm_coverage_attempt(VMCoverage *coverage, size_t function, size_t pc)
{
    VMCoverageCell *cell = (VMCoverageCell *)vm_coverage_cell(coverage, function, pc);
    if (cell) {
        increment(coverage, &cell->attempted);
    } else {
        vm_coverage_gap(coverage);
    }
}

void vm_coverage_finish(VMCoverage *coverage, size_t function, size_t pc, bool branched)
{
    VMCoverageCell *cell = (VMCoverageCell *)vm_coverage_cell(coverage, function, pc);
    if (!cell) {
        vm_coverage_gap(coverage);
        return;
    }
    increment(coverage, &cell->completed);
    uint8_t op = coverage->program->functions[function].ins[pc].op;
    if (op == OP_JUMP_FALSE || op == OP_ITER_NEXT || op == OP_MATCH_VARIANT) {
        increment(coverage, branched ? &cell->branched : &cell->fallthrough);
    }
}

void vm_coverage_terminal(VMCoverage *coverage, unsigned terminal)
{
    if (coverage && terminal <= 3) {
        coverage->terminal = terminal;
    }
}

bool vm_coverage_raw_size(const VMCoverage *coverage, size_t artifact_size,
                          size_t inventory_size, size_t *size)
{
    /* marker(11), blobs(8), function count(4), footer marker(4), status/flags(2). */
    if (!coverage || artifact_size > VM_COVERAGE_MAX_RAW || inventory_size > VM_COVERAGE_MAX_RAW) {
        return false;
    }
    size_t total = 29 + artifact_size + inventory_size + coverage->program->count * 12 + coverage->count * 32;
    if (total > VM_COVERAGE_MAX_RAW) {
        return false;
    }
    *size = total;
    return true;
}

static bool write_number(FILE *output, uint64_t value, size_t width)
{
    uint8_t bytes[8];
    for (size_t i = 0; i < width; i++) {
        bytes[width - i - 1] = (uint8_t)(value & 255);
        value >>= 8;
    }
    return fwrite(bytes, 1, width, output) == width;
}

bool vm_coverage_write(const VMCoverage *coverage, FILE *output,
                       const uint8_t *artifact, size_t artifact_size,
                       const uint8_t *inventory, size_t inventory_size)
{
    size_t size;
    if (!vm_coverage_raw_size(coverage, artifact_size, inventory_size, &size) ||
        fwrite("PANACKCOV1\n", 1, 11, output) != 11 ||
        !write_number(output, artifact_size, 4) ||
        fwrite(artifact, 1, artifact_size, output) != artifact_size ||
        !write_number(output, inventory_size, 4) ||
        fwrite(inventory, 1, inventory_size, output) != inventory_size ||
        !write_number(output, coverage->program->count, 4)) {
        return false;
    }
    for (size_t i = 0; i < coverage->program->count; i++) {
        const Function *function = &coverage->program->functions[i];
        if (!write_number(output, coverage->entries[i], 8) ||
            !write_number(output, function->ins_count, 4)) {
            return false;
        }
        for (size_t pc = 0; pc < function->ins_count; pc++) {
            const VMCoverageCell *cell = vm_coverage_cell(coverage, i, pc);
            if (!write_number(output, cell->attempted, 8) ||
                !write_number(output, cell->completed, 8) ||
                !write_number(output, cell->fallthrough, 8) ||
                !write_number(output, cell->branched, 8)) {
                return false;
            }
        }
    }
    return fwrite("END\n", 1, 4, output) == 4 &&
        write_number(output, coverage->terminal, 1) && write_number(output, coverage->flags, 1) &&
        !ferror(output);
}
