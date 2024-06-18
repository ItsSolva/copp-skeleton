#include <stdio.h>  // for getc, printf
#include <stdlib.h> // malloc, free
#include <assert.h>
#include "ijvm_helper.h"
#include "ijvm.h"
#include "stack_struct.h"
#include "util.h" // read this file for debug prints, endianness helper functions

// see ijvm.h for descriptions of the below functions

ijvm *init_ijvm(char *binary_path, FILE *input, FILE *output)
{
  // do not change these first three lines
  ijvm *m = (ijvm *)malloc(sizeof(ijvm));
  // note that malloc gives you memory, but gives no guarantees on the initial
  // values of that memory. It might be all zeroes, or be random data.
  // It is hence important that you initialize all variables in the ijvm
  // struct and do not assume these are set to zero.
  m->in = input;
  m->out = output;

  FILE *fp = fopen(binary_path, "rb");
    if (!fp) return NULL;

    if (!read_magic_number(m, fp) || !read_constant_pool(m, fp) || !read_text_section(m, fp))
    {
        fclose(fp);
        free(m);
        return NULL;
    }

    fclose(fp);

  m->pc = 0;
  m->is_finished = false;

  initialize_stack(m);

  return m;
}

void destroy_ijvm(ijvm *m)
{
  free(m);
}

byte_t *get_text(ijvm *m)
{
  return m->text_data;
}

unsigned int get_text_size(ijvm *m)
{
  return m->text_size;
}

word_t get_constant(ijvm *m, int i)
{
  return m->constant_data[i];
}

unsigned int get_program_counter(ijvm *m)
{
  return m->pc;
}

word_t tos(ijvm *m)
{
  return m->st->data[m->st->index_top - 1];
}

bool finished(ijvm *m)
{
  if (get_program_counter(m) >= get_text_size(m))
    m->is_finished = true;
  return m->is_finished;
}

word_t get_local_variable(ijvm *m, int i)
{
  return m->st->data[i + m->lv];
}

void step(ijvm *m)
{
  byte_t instruction = get_instruction(m);
  switch (instruction)
  {
  case OP_BIPUSH:
        perform_bipush(m);
        break;
    case OP_DUP:
        perform_dup(m);
        break;
    case OP_IADD:
        perform_iadd(m);
        break;
    case OP_IAND:
        perform_iand(m);
        break;
    case OP_IOR:
        perform_ior(m);
        break;
    case OP_ISUB:
        perform_isub(m);
        break;
    case OP_NOP:
        perform_nop(m);
        break;
    case OP_POP:
        perform_pop(m);
        break;
    case OP_SWAP:
        perform_swap(m);
        break;
    case OP_ERR:
        perform_err(m);
        break;
    case OP_HALT:
        perform_halt(m);
        break;
    case OP_IN:
        perform_in(m);
        break;
    case OP_OUT:
        perform_out(m);
        break;
    case OP_GOTO:
        perform_goto(m);
        break;
    case OP_IFEQ:
        perform_ifeq(m);
        break;
    case OP_IFLT:
        perform_iflt(m);
        break;
    case OP_IF_ICMPEQ:
        perform_if_icmpeq(m);
        break;
    case OP_LDC_W:
        perform_ldc_w(m);
        break;
    case OP_ILOAD:
        perform_iload(m);
        break;
    case OP_ISTORE:
        perform_istore(m);
        break;
    case OP_IINC:
        perform_iinc(m);
        break;
    case OP_WIDE:
        perform_wide(m);
        break;
    case OP_INVOKEVIRTUAL:
        perform_invokevirtual(m);
        break;
    case OP_IRETURN:
        perform_ireturn(m);
        break;
  default:
    m->pc++;
    break;
  }
}

byte_t get_instruction(ijvm *m)
{
  return get_text(m)[get_program_counter(m)];
}

ijvm *init_ijvm_std(char *binary_path)
{
  return init_ijvm(binary_path, stdin, stdout);
}

void run(ijvm *m)
{
  while (!finished(m))
  {
    step(m);
  }
}

// Below: methods needed by bonus assignments, see ijvm.h
// You can leave these unimplemented if you are not doing these bonus
// assignments.

int get_call_stack_size(ijvm *m)
{
  // TODO: implement me if doing tail call bonus
  return 0;
}

// Checks if reference is a freed heap array. Note that this assumes that
//
bool is_heap_freed(ijvm *m, word_t reference)
{
  // TODO: implement me if doing garbage collection bonus
  return 0;
}
