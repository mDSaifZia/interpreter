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
    memset(vm->constants, 0, sizeof(vm->constants)); // change this to intialise ints up to 1024

    //initialise bytecode instruction pointer
    vm->bytecode_ip = NULL;

    return vm;
}
/* ///////////////////////// VM FUNCTIONS ///////////////////////// */
// // OPCODE instructions (SYNTAX: OP (NO ARG))
// OP_ADD,        // Add two values                            Binary: 0b 0000 0000 done
// OP_SUB,        // Sub two values (consistency of sub op)    Binary: 0b 0000 0001 done
// OP_MUL,        // Multiply                                  Binary: 0b 0000 0010 done
// OP_DIV,        // Divide                                    Binary: 0b 0000 0011 done
// OP_GET_GLOBAL, // Get a global variable                     Binary: 0b 0000 0100 WIP         
// OP_SET_GLOBAL, // Set a global variable                     Binary: 0b 0000 0101 WIP
// OP_CALL,       // Call function                             Binary: 0b 0000 0110 
// OP_RETURN,     // Return from function                      Binary: 0b 0000 0111 
// OP_HALT,       // Stop execution                            Binary: 0b 0000 1000
// OP_JMP,        // JMP to an offset from current idx         Binary: 0b 0000 1001 done
// OP_JMPIF,      // false ? JMP to and offset from curr idx   Binary: 0b 0000 1010 WIP

// // OPCODE primitives (SYNTAX: TYPE (ARG))
// INT,           // prim obj int representation               Binary: 0b 0000 1011 
// FLOAT,         // prim obj float representation             Binary: 0b 0000 1100
// BOOL,          // prim obj bool representation              Binary: 0b 0000 1101
// STR,           // prim obj str representation               Binary: 0b 0000 1110
// _NULL_,        // prim _NULL_ representation (NO ARGS)      Binary: 0b 0000 1111
// ID,            // ID representation                         Binary: 0b 0001 0000

// // OPCODE flags (SYNTAX: FLAG (NO ARG))
// OP_FUNCDEF,    // Flag for start of function definition     Binary: 0b 0001 0001
// OP_ENDFUNC,    // Flag for end of function definition       Binary: 0b 0001 0010
// OP_CLASSDEF,   // Flag for start of class definition        Binary: 0b 0001 0011
// OP_ENDCLASS,  // Flag for end of class definition           Binary: 0b 0001 0100

// // OPCODE binary operators (SYNTAX: BIN_OP (NO ARGS)) -> INT ONLY
// OP_BLSHIFT,  // Flag for end of class definition            Binary: 0b 0001 0101
// OP_BRSHIFT,  // Flag for end of class definition            Binary: 0b 0001 0110
// OP_BXOR,     // Flag for end of class definition            Binary: 0b 0001 0111
// OP_BOR,      // Flag for end of class definition            Binary: 0b 0001 1000
// OP_BAND,     // Flag for end of class definition            Binary: 0b 0001 1001

/*
INT, FLOAT -> read 8 bytes after opcode
BOOL -> read 1 byte after opcode 
STR -> read 4 bytes after opcode (char length) then read 4 bytes as an int to get num bytes to read
ID -> read 2 bytes after opcode (char length) then read the 2 bytes as an int to get the num bytes to read
JMP -> read 4 bytes after opcode to get JMP offset
JMPIF -> read 4 bytes after opcode to get JMP offset
*/

/* runs the vm */
void run(VM* vm, const char* bytecode_file) {
    FILE *file = fopen(bytecode_file, "rb");
    if (!file) {
        printf("Error: Could not open bytecode file %s\n", bytecode_file);
        return;
    }

    // Read file contents into bytecode memory
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    uint8_t* bytecode = malloc(file_size);
    if (!bytecode) {
        printf("Error: Failed to allocate memory for bytecode.\n");
        fclose(file);
        return;
    }

    fread(bytecode, 1, file_size, file);
    fclose(file);

    vm->bytecode_ip = (uint64_t*) bytecode; // Cast to match existing struct, but careful with byte addressing

    while (1) {
        uint8_t instruction = *(uint8_t*)vm->bytecode_ip;  // cast the read byte to a uint8 to match the Opcodes enums
        vm->bytecode_ip = (uint64_t*)((uint8_t*)vm->bytecode_ip + 1); // move to next bytes

        switch (instruction) {
            case OP_HALT:
                printf("VM halted.\n");
                free(bytecode);
                return;

            case INT: {     //-> read 8 bytes after opcode 
                break;
            }
            case FLOAT: {   //-> read 8 bytes after opcode
                break;
            }
            case BOOL: {    //-> read 1 byte after opcode
                break;
            }
            case STR: {     //-> read 4 bytes after opcode (char length) then read 4 bytes as an int to get num bytes to read
                break;
            }
            case ID: {      //-> read 2 bytes after opcode (char length) then read the 2 bytes as an int to get the num bytes to read (figure out how to push this to global table)
                break;
            }

            case OP_ADD: {
                StackEntry b = pop(vm);
                StackEntry a = pop(vm);
                if (a.entry_type == PRIMITIVE_OBJ && b.entry_type == PRIMITIVE_OBJ) {
                    PrimitiveObject* result =  ((PrimitiveObject *) a.value)->add(((PrimitiveObject *) a.value), ((PrimitiveObject *) b.value)); //cast back to original values
                    push(vm, result, PRIMITIVE_OBJ);
                } else {
                    printf("Error: Invalid types for ADD operation.\n"); // just disallowing other types of additions first but it can be implemented
                }
                break;
            }

            case OP_SUB: {
                StackEntry b = pop(vm);
                StackEntry a = pop(vm);
                if (a.entry_type == PRIMITIVE_OBJ && b.entry_type == PRIMITIVE_OBJ) {

                    if (((PrimitiveObject *) b.value)->type != TYPE_int || ((PrimitiveObject *) b.value)->type != TYPE_float) { // subtraction only supported between ints and floats by default
                        printf("Error: Invalid types for SUB operation.\n"); 
                    } else {
                        
                        // ensure that c applies the negative appropriately 
                        if (((PrimitiveObject *) b.value)->type != TYPE_int) ((int_Object *) b.value)->value *= -1;
                        else ((float_Object *) b.value)->value *= -1;

                        PrimitiveObject* result =  ((PrimitiveObject *) a.value)->add(((PrimitiveObject *) a.value), ((PrimitiveObject *) b.value)); //cast back to primitive 
                        push(vm, result, PRIMITIVE_OBJ);
                    }
                } else {
                    printf("Error: Invalid types for SUB operation.\n"); // just disallowing other types of additions first but it can be implemented
                }
                break;
            }
    
            case OP_MUL: {
                StackEntry b = pop(vm);
                StackEntry a = pop(vm);
                if (a.entry_type == PRIMITIVE_OBJ && b.entry_type == PRIMITIVE_OBJ) {
                    PrimitiveObject* result =  ((PrimitiveObject *) a.value)->mul(((PrimitiveObject *) a.value), ((PrimitiveObject *) b.value)); //cast back to original values
                    push(vm, result, PRIMITIVE_OBJ);
                } else {
                    printf("Error: Invalid types for MUL operation.\n"); // just disallowing other types of multiplication first but it can be implemented
                }
                break;
            }
                

            case OP_DIV: {
                StackEntry b = pop(vm);
                StackEntry a = pop(vm);
                if (a.entry_type == PRIMITIVE_OBJ && b.entry_type == PRIMITIVE_OBJ) {
                    PrimitiveObject* result =  ((PrimitiveObject *) a.value)->div(((PrimitiveObject *) a.value), ((PrimitiveObject *) b.value)); //cast back to original values
                    push(vm, result, PRIMITIVE_OBJ);
                } else {
                    printf("Error: Invalid types for MUL operation.\n"); // just disallowing other types of multiplication first but it can be implemented
                }
                break;
            }
            

            case OP_GET_GLOBAL: {
                StackEntry id = pop(vm); // stackEntry with IDENTIFIER type holds a char * as its value
                if (id.entry_type != IDENTIFIER) {
                    printf("Error: expected IDENTIFIER but got undefined entry type %s.\n");
                }
                char * var_name = (char *) id.value; 
                void* value = hashmap_get(vm->globals, var_name);
                if (value) {
                    push(vm, value, PRIMITIVE_OBJ); // change to cast to appropriate type expected (must handle logic of how to do this)
                } else {
                    printf("Error: Undefined global variable %s.\n", var_name);
                }
                break;
            }

            case OP_SET_GLOBAL: {
                break;
            }

            case OP_JMP: {
                int32_t offset = *(int32_t*)vm->bytecode_ip;  // Read 4-byte offset
                vm->bytecode_ip = (uint64_t*)((uint8_t*)vm->bytecode_ip + 4); // Move past offset
                vm->bytecode_ip = (uint64_t*)((uint8_t*)vm->bytecode_ip + offset); // Apply jump
                break;
            }

            case OP_JMPIF: {
                bool_Object * condition = (bool_Object *) pop(vm).value; // this needs to be refined as this won't work for NULL 
                // implement later
            }

            default:
                printf("Unknown instruction: 0x%02X\n", instruction);
                break;
        }
    }

    free(bytecode);  // Clean up allocated memory
}


/* ///////////////////////// VM FUNCTIONS ///////////////////////// */

/* ///////////////////////// STACK ///////////////////////// */

/* pushes a StackEntry onto stack */
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

/* Pops a StackEntry from stack */
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
