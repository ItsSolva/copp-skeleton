#ifndef IJVM_HELPER_H
#define IJVM_HELPER_H

#include <stdbool.h>
#include "ijvm.h"

bool read_magic_number(ijvm *m, FILE *fp);
bool read_constant_pool(ijvm *m, FILE *fp);
bool read_text_section(ijvm *m, FILE *fp);

void initialize_stack(ijvm *m);

word_t pop(ijvm *m);
void push(ijvm *m, word_t value);

int16_t get_short_arg(ijvm *m);

void perform_bipush(ijvm *m);
void perform_dup(ijvm *m);
void perform_iadd(ijvm *m);
void perform_iand(ijvm *m);
void perform_ior(ijvm *m);
void perform_isub(ijvm *m);
void perform_nop(ijvm *m);
void perform_pop(ijvm *m);
void perform_swap(ijvm *m);
void perform_err(ijvm *m);
void perform_halt(ijvm *m);
void perform_in(ijvm *m);
void perform_out(ijvm *m);
void perform_goto(ijvm *m);
void perform_ifeq(ijvm *m);
void perform_iflt(ijvm *m);
void perform_if_icmpeq(ijvm *m);
void perform_ldc_w(ijvm *m);
void perform_iload(ijvm *m);
void perform_istore(ijvm *m);
void perform_iinc(ijvm *m);
void perform_wide(ijvm *m);
void perform_invokevirtual(ijvm *m);
void perform_ireturn(ijvm *m);

#endif // IJVM_HELPERS_H
