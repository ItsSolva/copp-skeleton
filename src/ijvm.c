#include <stdio.h>  // for getc, printf
#include <stdlib.h> // malloc, free
#include "ijvm.h"
#include "util.h" // read this file for debug prints, endianness helper functions

// see ijvm.h for descriptions of the below functions

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
  if(m->magic_num != MAGIC_NUMBER) return 0;

  // Read the constant origin
  fread(&buffer, sizeof(uint8_t), 4, fp);
  m->constant_origin = read_uint32(buffer);
  // Read the constant size
  fread(&buffer, sizeof(uint8_t), 4, fp);
  m->constant_size = read_uint32(buffer);
  // Read the constant data
  m->constant_data = (int32_t *)malloc(sizeof(int32_t) * m->constant_size/4);
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
  
  return m;
}

void destroy_ijvm(ijvm* m) 
{
  // TODO: implement me

  free(m); // free memory for struct
}

byte_t *get_text(ijvm* m) 
{
  // TODO: implement me
  return m->text_data;
}

unsigned int get_text_size(ijvm* m) 
{
  // TODO: implement me
  return m->text_size;
}

word_t get_constant(ijvm* m,int i) 
{
  // TODO: implement me
  return m->constant_data[i];
}

unsigned int get_program_counter(ijvm* m) 
{
  // TODO: implement me
  return 0;
}

word_t tos(ijvm* m) 
{
  // this operation should NOT pop (remove top element from stack)
  // TODO: implement me
  return -1;
}

bool finished(ijvm* m) 
{
  // TODO: implement me
  return false;
}

word_t get_local_variable(ijvm* m, int i) 
{
  // TODO: implement me
  return 0;
}

void step(ijvm* m) 
{
  // TODO: implement me

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
