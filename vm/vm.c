#include "vm.h"
#include "hashmap/hashmap.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ///////////////////////// VM ///////////////////////// */
/* Initialize VM */
VM * initVM() {
    VM * vm = malloc(sizeof(VM));
    if (!vm) {
        printf("Failed to initialise Ratsnake vm.\n");
        return NULL;
    }

    // initialise stack
    vm->stack.base_pointer = 0;
    vm->stack.stack_top = 0;

    //initialise globals
    vm->globals = init_hashmap(MAX_GLOBALS);

    // initialise counters
    vm->constantCount = 0;
    vm->functionCount = 0;
    vm->objectCount = 0;

    // create clean tables 
    memset(vm->functions, 0, sizeof(vm->functions)); // Zero out function table
    memset(vm->objects, 0, sizeof(vm->objects));     // Zero out object table
    memset(vm->constants, 0, sizeof(vm->constants)); // Zero out constant table

    //initialise bytecode instruction pointer
    vm->bytecode_ip = NULL;

    return vm;
}
/* ///////////////////////// VM FUNCTIONS ///////////////////////// */



/* ///////////////////////// VM FUNCTIONS ///////////////////////// */

/* ///////////////////////// VM ///////////////////////// */

/* ///////////////////////// STACK ///////////////////////// */

void push(VM* vm, void* value, StackEntryType type) { 
    Stack *stack = &vm->stack; // Use a pointer to modify the actual stack in VM
    StackEntry entry;
    entry.value = value;
    entry.entry_type = type;

    if (stack->stack_top < STACK_MAX) { // Check stack limit
        stack->stack[stack->stack_top] = entry;
        stack->stack_top++;
    } else { 
        printf("Stack overflow error.\n");
        return;
    }
}

StackEntry pop(VM* vm) {
    Stack *stack = &vm->stack; // Use a pointer to modify the actual stack in VM
    StackEntry entry;

    if (stack->stack_top == 0) { // Stack underflow check
        printf("Attempted to pop from an empty stack. Stack underflow error.\n");
        StackEntry errorEntry = {NULL, PRIMITIVE_OBJ}; // Return an invalid entry
        return errorEntry;
    }
    
    stack->stack_top--; // Move stack top down
    return stack->stack[stack->stack_top]; // Return the popped entry
}

/* ///////////////////////// STACK ///////////////////////// */
