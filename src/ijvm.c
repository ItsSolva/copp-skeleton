#include <stdio.h>  // for getc, printf
#include <stdlib.h> // malloc, free
#include <assert.h>
#include "ijvm.h"
#include "stack_struct.h"
#include "util.h" // read this file for debug prints, endianness helper functions

// see ijvm.h for descriptions of the below functions

void push(ijvm *m, int8_t value)
{
  m->st->data[m->st->index_top] = value;
  m->st->index_top++;

  // Check if stack is full
  if (m->st->index_top >= m->st->size)
  {
    m->st->size *= 2;
    m->st->data = (int8_t *)realloc(m->st->data, m->st->size);
  }
}

int8_t pop(ijvm *m)
{
  m->st->index_top--;
  return m->st->data[m->st->index_top];
}

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

  // TODO: implement me
  FILE *fp = fopen(binary_path, "rb");

  // Read the magic number (first 4 bytes)
  uint8_t buffer[4];
  fread(&buffer, sizeof(uint8_t), 4, fp);
  m->magic_num = read_uint32(buffer);
  // Check the magic number
  if (m->magic_num != MAGIC_NUMBER)
    return NULL;

  // Read the constant origin
  fread(&buffer, sizeof(uint8_t), 4, fp);
  m->constant_origin = read_uint32(buffer);
  // Read the constant size
  fread(&buffer, sizeof(uint8_t), 4, fp);
  m->constant_size = read_uint32(buffer);
  // Read the constant data
  m->constant_data = (word_t *)malloc(sizeof(word_t) * m->constant_size / 4);
  for (uint32_t i = 0; i < m->constant_size / 4; i++)
  {
    fread(&buffer, sizeof(uint8_t), 4, fp);
    m->constant_data[i] = read_uint32(buffer);
  }

  // Read the text pool
  uint8_t buffer_small;
  fread(&buffer, sizeof(uint8_t), 4, fp);
  m->text_origin = read_uint32(buffer);
  // Read the text size
  fread(&buffer, sizeof(uint8_t), 4, fp);
  m->text_size = read_uint32(buffer);
  // Read the text data
  m->text_data = (uint8_t *)malloc(sizeof(uint8_t) * m->text_size);
  for (uint32_t i = 0; i < m->text_size; i++)
  {
    fread(&buffer_small, sizeof(uint8_t), 1, fp);
    m->text_data[i] = buffer_small;
  }

  fclose(fp);

  // Initialize the program counter
  m->pc = 0;
  m->is_finished = false;

  // Initialize the stack
  m->st = (stack *)malloc(sizeof(stack));
  m->st->index_top = 0;
  m->st->size = 1024;
  m->st->data = (int8_t *)malloc(sizeof(int8_t) * m->st->size);

  m->lv = 0;
  for (int i = 0; i < 256; i++)
  {
    push(m, i);
  }

  return m;
}

void destroy_ijvm(ijvm *m)
{
  free(m); // free memory for struct
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
  return m->is_finished;
}

word_t get_local_variable(ijvm *m, int i)
{
  word_t local_var = m->st->data[i + m->lv];
  return local_var;
}

int16_t get_short_arg(ijvm *m)
{
  uint8_t short_bytes[] = {get_text(m)[m->pc + 1], get_text(m)[m->pc + 2]};
  return read_int16(short_bytes);
}

void print_stack(ijvm *m)
{
  printf("STACK: ");
  for (int i = 255; i < m->st->index_top; i++)
  {
    printf("%d, ", m->st->data[i]);
  }
  printf("\n");
}

void step(ijvm *m)
{
  fprintf(stderr, "\nSTEP - PC: %d - TEXT SIZE: %d\n", get_program_counter(m), get_text_size(m));
  if (get_program_counter(m) +1 >= get_text_size(m))
  {
    m->is_finished = true;
    
  }

  byte_t instruction = get_instruction(m);

  switch (instruction)
  {
    case OP_BIPUSH:
    {
      m->pc++;
      push(m, get_instruction(m));
      break;
    }
    case OP_DUP:
    {
      push(m, tos(m));
      break;
    }
    case OP_IADD:
    {
      push(m, pop(m) + pop(m));
      break;
    }
    case OP_IAND:
    {
      push(m, pop(m) & pop(m));
      break;
    }
    case OP_IOR:
    {
      push(m, pop(m) | pop(m));
      break;
    }
    case OP_ISUB:
    {
      int8_t val1 = pop(m);
      int8_t val2 = pop(m);
      push(m, val2 - val1);
      break;
    }
    case OP_NOP:
    {
      break;
    }
    case OP_POP:
    {
      pop(m);
      break;
    }
    case OP_SWAP:
    {
      int8_t val1 = pop(m);
      int8_t val2 = pop(m);
      push(m, val1);
      push(m, val2);
      break;
    }
    case OP_ERR:
    {
      fprintf(m->out, "ERROR\n");
      m->is_finished = true;
      break;
    }
    case OP_HALT:
    {
      m->is_finished = true;
      break;
    }
    case OP_IN:
    {
      push(m, fgetc(m->in));
      break;
    }
    case OP_OUT:
    {
      fprintf(m->out, "%c", (char)pop(m));
      break;
    }
    case OP_GOTO:
    {
      m->pc += get_short_arg(m) - 1;
      break;
    }
    case OP_IFEQ:
    {
      if (pop(m) == 0)
      {
        m->pc += get_short_arg(m) - 1;
      }
      else
      {
        m->pc += 2;
      }
      break;
    }
    case OP_IFLT:
    {
      if (pop(m) < 0)
      {
        m->pc += get_short_arg(m) - 1;
      }
      else
      {
        m->pc += 2;
      }
      break;
    }
    case OP_IF_ICMPEQ:
    {
      if (pop(m) == pop(m))
      {
        m->pc += get_short_arg(m) - 1;
      }
      else
      {
        m->pc += 2;
      }
      break;
    }
    case OP_LDC_W:
    {
      push(m, get_constant(m, get_short_arg(m)));
      m->pc += 2  ;
      break;
    }
    case OP_ILOAD:
    {
      m->pc++;
      push(m, get_local_variable(m, get_instruction(m)));
      break;
    }
    case OP_ISTORE:
    {
      m->pc++;
      m->st->data[get_instruction(m)] = pop(m);
      break;
    }
    case OP_IINC:
    {
      m->pc++;
      byte_t index = get_instruction(m);
      m->pc++;
      m->st->data[index] += get_instruction(m);
      break;
    }
    case OP_WIDE:
    {
      m->pc++;
      byte_t next_instruction = get_instruction(m);
      m->pc++;
      uint16_t index = (uint16_t)((uint16_t)get_instruction(m) << 8) | (uint16_t)get_text(m)[m->pc + 1];
      m->pc++;
      switch (next_instruction)
      {
        case OP_ILOAD:
          push(m, get_local_variable(m, index));
          break;
        case OP_ISTORE:
        {
          m->st->data[index] = pop(m);
          break;
        }
        case OP_IINC:
        {
          m->pc++;
          m->st->data[index] += get_instruction(m);
          break;
        }
      }
      break;
    }
    case OP_INVOKEVIRTUAL:
        {
            // print_stack(m);
            
            int16_t index_constant = get_short_arg(m);
            
            // Save the current frame
            word_t old_pc = m->pc;
            word_t old_lv = m->lv;
            
            // Jump to the method area
            m->pc = get_constant(m, index_constant)-1;
            // fprintf(stderr, "Jumping to method area at %d\n", m->pc);

            // Read number of arguments and local variables from method area
            word_t arg_count = get_short_arg(m);
            m->pc += 2;
            word_t local_var_count = get_short_arg(m);
            m->pc += 2;


            // Set the new frame pointer
            m->lv = m->st->index_top - arg_count + 1;
            m->st->data[m->lv] = m->lv + arg_count + local_var_count;

            // Allocate space for local variables
            for (size_t i = 0; i < local_var_count; i++)
            {
                push(m, -69);
            }

            // Push the return address and old local variable pointer onto the stack
            push(m, old_pc); // Return address (pc after INVOKEVIRTUAL instruction)
            push(m, old_lv);

            fprintf(stderr, "ARG: %d - Local Vars: %d\n", arg_count, local_var_count);

            break;
        }
        case OP_IRETURN:
        {
          fprintf(stderr, "RETURN\n");
            word_t return_value = pop(m);

            // Remove the arguments from the stack
            m->st->index_top = m->lv-1;

            // Restore the old frame
            word_t old_pc = m->st->data[m->lv];
            word_t old_lv = m->st->data[m->lv+1];

            // Restore the program counter and local variable pointer
            m->pc = old_pc+2;
            m->lv = old_lv;


            // Push the return value onto the stack
            push(m, return_value);

            break;
        }
  }
  print_stack(m);
  m->pc++;
  fprintf(stderr, "PC: %d\n", m->pc);
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
