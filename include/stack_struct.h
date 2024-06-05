
#ifndef STACK_STRUCT_H
#define STACK_STRUCT_H

#include "ijvm_types.h"

typedef struct STACK {
  int8_t *data;
  uint32_t size;
  uint32_t index_top;

} stack;

#endif 
