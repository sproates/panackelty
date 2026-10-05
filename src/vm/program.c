#include "program.h"

#include <stdlib.h>
#include <string.h>

/* Decoded bytecode owns its names, constants, and instruction arrays. */

void free_program(Program *p)
{
    if (!p) {
        return;
    }
    for (size_t i = 0; p->functions && i < p->count; i++) {
        Function *f = &p->functions[i];
        free(f->name);
        for (size_t j = 0; f->params && j < f->param_count; j++) {
            free(f->params[j]);
        }
        free(f->params);
        for (size_t j = 0; f->ins && j < f->ins_count; j++) {
            Instruction *in = &f->ins[j];
            free(in->name);
            free(in->name2);
            for (size_t k = 0; k < in->count; k++) {
                free(in->items ? in->items[k] : NULL);
            }
            free(in->items);
            pn_big_free(&in->constant.number);
            free(in->constant.text);
        }
        free(f->ins);
    }
    free(p->functions);
    memset(p, 0, sizeof(*p));
}

Function *program_function(Program *p, const char *name)
{
    if (p->verified_function_order) {
        /* Use the order already checked by verify; unverified objects retain
         * linear lookup so malformed input is not assumed to be sorted. */
        size_t low = 0, high = p->count;
        while (low < high) {
            size_t middle = low + (high - low) / 2;
            int order = strcmp(name, p->functions[middle].name);
            if (order == 0) {
                return &p->functions[middle];
            }
            if (order < 0) {
                high = middle;
            } else {
                low = middle + 1;
            }
        }
        return NULL;
    }
    for (size_t i = 0; p->functions && i < p->count; i++) {
        if (!strcmp(name, p->functions[i].name)) {
            return &p->functions[i];
        }
    }
    return NULL;
}
