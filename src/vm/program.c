#include "program.h"

#include <stdlib.h>
#include <string.h>

/* Decoded bytecode owns its names, constants, and instruction arrays. */

void free_program(Program *p)
{
    if (!p) {
        return;
    }
    for (size_t i = 0; i < p->count; i++) {
        Function *f = &p->functions[i];
        free(f->name);
        for (size_t j = 0; j < f->param_count; j++) {
            free(f->params[j]);
        }
        free(f->params);
        for (size_t j = 0; j < f->ins_count; j++) {
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
    for (size_t i = 0; i < p->count; i++) {
        if (!strcmp(name, p->functions[i].name)) {
            return &p->functions[i];
        }
    }
    return NULL;
}
