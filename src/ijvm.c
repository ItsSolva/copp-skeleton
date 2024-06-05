#include <stdio.h>  // for getc, printf
#include <stdlib.h> // malloc, free
#include "ijvm.h"
#include "stack_struct.h"
#include "util.h" // read this file for debug prints, endianness helper functions

// see ijvm.h for descriptions of the below functions

void push(ijvm* m, int8_t value) 
{ 
  m->st->data[m->st->index_top] = value;
  m->st->index_top++;

  // Check if stack is full
  if(m->st->index_top >= m->st->size) {
    m->st->size *= 2;
    m->st->data = (int8_t *)realloc(m->st->data, m->st->size);
  }
}

int8_t pop(ijvm* m) 
{ 
  int8_t return_val = 0;
  
  if(m->st->index_top > 0) {
    m->st->index_top--;
    return_val = m->st->data[m->st->index_top];
  }

  return return_val;
  
}

ijvm* init_ijvm(char *binary_path, FILE* input , FILE* output) 
{
  // do not change these first three lines
  ijvm* m = (ijvm *) malloc(sizeof(ijvm));
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
  if(m->magic_num != MAGIC_NUMBER) return NULL;

  // Read the constant origin
  fread(&buffer, sizeof(uint8_t), 4, fp);
  m->constant_origin = read_uint32(buffer);
  // Read the constant size
  fread(&buffer, sizeof(uint8_t), 4, fp);
  m->constant_size = read_uint32(buffer);
  // Read the constant data
  m->constant_data = (word_t *)malloc(sizeof(word_t) * m->constant_size/4);
  for (uint32_t i = 0; i < m->constant_size/4; i++)
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

  return m;
}

void destroy_ijvm(ijvm* m) 
{
  free(m); // free memory for struct
}

byte_t *get_text(ijvm* m) 
{
  return m->text_data;
}

unsigned int get_text_size(ijvm* m) 
{
  return m->text_size;
}

word_t get_constant(ijvm* m,int i) 
{
  return m->constant_data[i];
}

unsigned int get_program_counter(ijvm* m) 
{
  return m->pc;
}

word_t tos(ijvm* m) 
{
  
  return m->st->data[m->st->index_top-1];
}

bool finished(ijvm* m) 
{
  return m->is_finished;
}

word_t get_local_variable(ijvm* m, int i) 
{
  // TODO: implement me
  return 0;
}

int16_t get_goto_short(ijvm* m) {
  uint8_t short_bytes[] = {get_text(m)[m->pc+1], get_text(m)[m->pc+2]};
  return read_int16(short_bytes)-1;
}

void step(ijvm* m) 
{
  if (get_program_counter(m) + 1 >= get_text_size(m)) {
    m->is_finished = true;
    return;
  }

  byte_t instruction = get_instruction(m);

  switch(instruction) {
    case OP_BIPUSH: {
      m->pc++;
      push(m, (int8_t)get_text(m)[m->pc]);
      break;
    }
    case OP_DUP: {
      push(m, tos(m));
      break;
    }
    case OP_IADD: {
      push(m, pop(m) + pop(m));
      break;
    }
    case OP_IAND: {
      push(m, pop(m) & pop(m));
      break;
    }
    case OP_IOR: {
      push(m, pop(m) | pop(m));
      break;
    }
    case OP_ISUB: {
      int8_t val1 = pop(m);
      int8_t val2 = pop(m);
      push(m, val2 - val1);
      break;
    }
    case OP_NOP: {
      break;
    }
    case OP_POP: {
      pop(m);
      break;
    }
    case OP_SWAP: {
      int8_t val1 = pop(m);
      int8_t val2 = pop(m);
      push(m, val1);
      push(m, val2);
      break;
    }
    case OP_ERR: {
      fprintf(m->out, "ERROR\n");
      m->is_finished = true;
      break;
    }
    case OP_HALT: {
      m->is_finished = true;
      break;
    }
    case OP_IN: {
      push(m, fgetc(m->in));
      break;
    }
    case OP_OUT: {
      fprintf(m->out, "%c", (char)pop(m));
      break;
    }
    case OP_GOTO: {
      m->pc += get_goto_short(m);
      break;
    }
    case OP_IFEQ: {
      if(pop(m) == 0){
        m->pc += get_goto_short(m);
      } else {
        m->pc += 2;
      }
      break;
    }
    case OP_IFLT: {
      if(pop(m) < 0){
        m->pc += get_goto_short(m);
      } else {
        m->pc += 2;
      }
      break;
    }
    case OP_IF_ICMPEQ: {
      if(pop(m) == pop(m)){
        m->pc += get_goto_short(m);
      } else {
        m->pc += 2;
      }
      break;
    }
  }

  m->pc++;
}



byte_t get_instruction(ijvm* m) 
{ 
  return get_text(m)[get_program_counter(m)]; 
}

ijvm* init_ijvm_std(char *binary_path) 
{
  return init_ijvm(binary_path, stdin, stdout);
}

void run(ijvm* m) 
{
  while (!finished(m)) 
  {
    step(m);
  }
}


// Below: methods needed by bonus assignments, see ijvm.h
// You can leave these unimplemented if you are not doing these bonus 
// assignments.

int get_call_stack_size(ijvm* m) 
{
   // TODO: implement me if doing tail call bonus
   return 0;
}


// Checks if reference is a freed heap array. Note that this assumes that 
// 
bool is_heap_freed(ijvm* m, word_t reference) 
{
   // TODO: implement me if doing garbage collection bonus
   return 0;
}
