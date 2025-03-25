#ifndef STACKFRAME_H
#define STACKFRAME_H

#include "../hashmap/hashmap.h"
#include "vm.h"
#include <stdint.h>
#include <stdlib.h>

#define MAX_LOCALS 1024

typedef struct { // Exactly the same as stackentry. Doing this to abstract from stackentry.
  void *value;
  StackEntryType entry_type;
} localEntry;

typedef struct StackFrame {
  uint64_t *return_address; // stores the position of ip after op_call
  /*
    1.) Initialize locals array with null pointers to not get random values
    for uninitialized local variables and to throw an error.
   */
  localEntry *locals[MAX_LOCALS]; // Local variable array
  /* Keep track of num of initialized indices
   using local count.
   * */
  size_t local_count; // Number of local variables
} StackFrame;

// Initialize a new stack frame
StackFrame *init_stack_frame(uint64_t *return_address, size_t local_count);

// Get a local variable from the current stack frame
localEntry get_local(VM *vm, uint16_t index);

// Set a local variable in the current stack frame
void set_local(VM *vm, uint16_t index, StackEntry value);

// Exit from the current stack frame (for function returns)
void return_from_frame(VM *vm);

// Free resources associated with a stack frame
void free_stack_frame(StackFrame *frame);

#endif
