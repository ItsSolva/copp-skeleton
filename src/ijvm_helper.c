#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "ijvm_helper.h"
#include "util.h"

bool read_magic_number(ijvm *m, FILE *fp)
{
    uint8_t buffer[4];
    fread(&buffer, sizeof(uint8_t), 4, fp);
    m->magic_num = read_uint32(buffer);
    return (m->magic_num == MAGIC_NUMBER);
}

bool read_constant_pool(ijvm *m, FILE *fp)
{
    uint8_t buffer[4];
    fread(&buffer, sizeof(uint8_t), 4, fp);
    m->constant_origin = read_uint32(buffer);

    fread(&buffer, sizeof(uint8_t), 4, fp);
    m->constant_size = read_uint32(buffer);

    m->constant_data = (word_t *)malloc(sizeof(word_t) * m->constant_size / 4);
    for (uint32_t i = 0; i < m->constant_size / 4; i++)
    {
        fread(&buffer, sizeof(uint8_t), 4, fp);
        m->constant_data[i] = read_uint32(buffer);
    }
    return true;
}

bool read_text_section(ijvm *m, FILE *fp)
{
    uint8_t buffer[4];
    fread(&buffer, sizeof(uint8_t), 4, fp);
    m->text_origin = read_uint32(buffer);

    fread(&buffer, sizeof(uint8_t), 4, fp);
    m->text_size = read_uint32(buffer);

    m->text_data = (uint8_t *)malloc(sizeof(uint8_t) * m->text_size);
    for (uint32_t i = 0; i < m->text_size; i++)
    {
        fread(&buffer[0], sizeof(uint8_t), 1, fp);
        m->text_data[i] = buffer[0];
    }
    return true;
}

void initialize_stack(ijvm *m)
{
    m->st = (stack *)malloc(sizeof(stack));
    m->st->index_top = 256;
    m->st->size = 1024;
    m->st->data = (word_t *)malloc(sizeof(word_t) * m->st->size);
    m->lv = 0;
}

void push(ijvm *m, word_t value)
{
  if (m->st->index_top >= m->st->size)
  {
    m->st->size *= 2;
    m->st->data = (word_t *)realloc(m->st->data, sizeof(word_t)*m->st->size);
  }

  m->st->data[m->st->index_top] = value;
  m->st->index_top++;
}

word_t pop(ijvm *m)
{
  word_t value = tos(m);
  m->st->index_top--;
  return value;
}

int16_t get_short_arg(ijvm *m)
{
  uint8_t short_bytes[] = {get_text(m)[m->pc + 1], get_text(m)[m->pc + 2]};
  return read_int16(short_bytes);
}

void perform_bipush(ijvm *m)
{
    m->pc++;
    push(m, (int8_t)get_instruction(m));
    m->pc++;
}

void perform_dup(ijvm *m)
{
    push(m, tos(m));
    m->pc++;
}

void perform_iadd(ijvm *m)
{
    push(m, pop(m) + pop(m));
    m->pc++;
}

void perform_iand(ijvm *m)
{
    push(m, pop(m) & pop(m));
    m->pc++;
}

void perform_ior(ijvm *m)
{
    push(m, pop(m) | pop(m));
    m->pc++;
}

void perform_isub(ijvm *m)
{
    word_t val1 = pop(m);
    word_t val2 = pop(m);
    push(m, val2 - val1);
    m->pc++;
}

void perform_nop(ijvm *m)
{
    m->pc++;
}

void perform_pop(ijvm *m)
{
    pop(m);
    m->pc++;
}

void perform_swap(ijvm *m)
{
    word_t val1 = pop(m);
    word_t val2 = pop(m);
    push(m, val1);
    push(m, val2);
    m->pc++;
}

void perform_err(ijvm *m)
{
    fprintf(m->out, "ERROR\n");
    m->is_finished = true;
}

void perform_halt(ijvm *m)
{
    m->is_finished = true;
    m->pc++;
}

void perform_in(ijvm *m)
{
    int c = fgetc(m->in);
    if (c == EOF)
        push(m, 0);
    else
        push(m, c);
    m->pc++;
}

void perform_out(ijvm *m)
{
    fprintf(m->out, "%c", pop(m));
    m->pc++;
}

void perform_goto(ijvm *m)
{
    m->pc += get_short_arg(m);
}

void perform_ifeq(ijvm *m)
{
    if (pop(m) == 0)
        m->pc += get_short_arg(m);
    else
        m->pc += 3;
}

void perform_iflt(ijvm *m)
{
    if (pop(m) < 0)
        m->pc += get_short_arg(m);
    else
        m->pc += 3;
}

void perform_if_icmpeq(ijvm *m)
{
    if (pop(m) == pop(m))
        m->pc += get_short_arg(m);
    else
        m->pc += 3;
}

void perform_ldc_w(ijvm *m)
{
    push(m, get_constant(m, get_short_arg(m)));
    m->pc += 3;
}

void perform_iload(ijvm *m)
{
    m->pc++;
    push(m, get_local_variable(m, get_instruction(m)));
    m->pc++;
}

void perform_istore(ijvm *m)
{
    m->pc++;
    m->st->data[m->lv + get_instruction(m)] = pop(m);
    m->pc++;
}

void perform_iinc(ijvm *m)
{
    m->pc++;
    byte_t index = get_instruction(m);
    m->pc++;
    m->st->data[m->lv + index] += (int8_t)get_instruction(m);
    m->pc++;
}

void perform_wide(ijvm *m)
{
    m->pc++;
    uint16_t index = get_short_arg(m);
    switch (get_instruction(m))
    {
    case OP_ILOAD:
        push(m, get_local_variable(m, index));
        m->pc += 3;
        break;
    case OP_ISTORE:
        m->st->data[m->lv + index] = pop(m);
        m->pc += 3;
        break;
    case OP_IINC:
        m->pc += 3;
        m->st->data[m->lv + index] += (int8_t)get_instruction(m);
        m->pc++;
        break;
    }
}

void perform_invokevirtual(ijvm *m)
{
    word_t old_pc = m->pc;
    word_t old_lv = m->lv;

    m->pc = get_constant(m, get_short_arg(m));

    word_t arg_count = read_uint16(get_text(m) + m->pc);
    m->pc += 2;
    word_t local_var_count = read_uint16(get_text(m) + m->pc);
    m->pc += 2;

    for (size_t i = 0; i < local_var_count; i++)
    {
        push(m, -69);
    }

    m->lv = m->st->index_top - arg_count - local_var_count;
    m->st->data[m->lv] = m->st->index_top;

    push(m, old_pc);
    push(m, old_lv);
}

void perform_ireturn(ijvm *m)
{
    word_t return_value = pop(m);

    m->st->index_top = m->lv;

    m->pc = m->st->data[m->st->data[m->lv]] + 3;
    m->lv = m->st->data[m->st->data[m->lv] + 1];

    push(m, return_value);
}
