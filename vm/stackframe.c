#include "stackframe.h"
#include "vm.h"
#include <stdio.h>
#include <string.h>

// Initialize a new stack frame
StackFrame *init_stack_frame(VM *vm, uint64_t *return_address, size_t local_count) {
  StackFrame *frame = malloc(sizeof(StackFrame));
  if (!frame) {
    printf("Failed to allocate memory for stack frame.\n");
    return NULL;
  }

  frame->return_address = return_address;
  frame->local_count = local_count;
  frame->parent_base_pointer = vm->stack.base_pointer;
  memset(frame->locals, 0, sizeof(frame->locals));

  return frame;
}

// Get a local variable from the current stack frame
localEntry get_local(VM *vm, uint16_t index) {
  // Get the current stack frame
  StackEntry frame_entry = vm->stack.stack[vm->stack.base_pointer];

  if (frame_entry.entry_type != FUNCTION_FRAME) {
    printf("Error: Expected stack frame at base pointer.\n");
    localEntry error_entry = {NULL, PRIMITIVE_OBJ};
    return error_entry;
  }

  StackFrame *frame = (StackFrame *)frame_entry.value;

  if (index >= frame->local_count) {
    printf("Error: Local variable index out of bounds (%d >= %zu).\n", index,
           frame->local_count);
    localEntry error_entry = {NULL, PRIMITIVE_OBJ};
    return error_entry;
  }
  if (frame->locals[index] == NULL) {
    printf("Error: Accessing uninitialized local variable.\n");
    localEntry error_entry = {NULL, PRIMITIVE_OBJ};
    return error_entry;
  }

  return *(frame->locals[index]);
}

// Set a local variable in the current stack frame
void set_local(VM *vm, uint16_t index, StackEntry stack_entry) {
  StackEntry frame_entry = vm->stack.stack[vm->stack.base_pointer];

  if (frame_entry.entry_type !=
      FUNCTION_FRAME) { // TODO: Fix later. Possibly dead code
    printf("Error: Expected stack frame at base pointer.\n");
    return;
  }

  StackFrame *frame = (StackFrame *)frame_entry.value;

  if (index >= MAX_LOCALS) {
    printf("Error: Local Variable does not exist.\n");
    return;
  }

  if (frame->locals[index]) {
    free(frame->locals[index]);
  }

  localEntry *entry = malloc(sizeof(localEntry));
  entry->value = stack_entry.value;
  entry->entry_type = stack_entry.entry_type;

  frame->locals[index] = entry;
}

// Exit from the current stack frame (for function returns)
void return_from_frame(VM *vm) {
  StackEntry frame_entry = vm->stack.stack[vm->stack.base_pointer];

  if (frame_entry.entry_type != FUNCTION_FRAME) {
    printf("Error: Expected stack frame at base pointer.\n");
    return;
  }

  StackFrame *frame = (StackFrame *)frame_entry.value;
  uint64_t *return_address = frame->return_address;
  // Pop the function return value
  StackEntry returnVal = pop(vm);
  // Reset stack top to base pointer
  vm->stack.stack_top = vm->stack.base_pointer;
  // Overwrite the frame entry to return value
  push(vm, returnVal.value, returnVal.entry_type);
  vm->bytecode_ip = return_address;
  vm->stack.base_pointer = frame->parent_base_pointer;
  free_stack_frame(frame);
}

// Free resources associated with a stack frame
void free_stack_frame(StackFrame *frame) {
  if (frame) {
    // Free all allocated local entries
    for (int i = 0; i < frame->local_count; i++) {
      if (frame->locals[i]) {
        free(frame->locals[i]); // Only freeing the local entries not the values
      }
    }
    free(frame);
  }
}
