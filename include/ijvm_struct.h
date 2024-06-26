
#ifndef IJVM_STRUCT_H
#define IJVM_STRUCT_H

#include <stdio.h>  /* contains type FILE * */

#include "ijvm_types.h"
#include "stack_struct.h"
/**
 * All the state of your IJVM machine goes in this struct!
 **/

typedef struct IJVM {
    // do not changes these two variables
    FILE *in;   // use fgetc(ijvm->in) to get a character from in.
                // This will return EOF if no char is available.
    FILE *out;  // use for example fprintf(ijvm->out, "%c", value); to print value to out

  // your variables go here
  uint32_t magic_num;
  // Constant Pool
  uint32_t constant_origin;
  uint32_t constant_size;
  word_t *constant_data;

  // Text Pool
  uint32_t text_origin;
  uint32_t text_size;
  uint8_t *text_data;

  // Program Counter
  word_t pc;
  word_t lv;
  bool is_finished;

  // Stack
  stack *st;



} ijvm;

#endif 
