#include "vm.h"
#include "../hashmap/hashmap.h"
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

    /* Populate the constant table with a few predefined constants: __NULL__, TRUE, FALSE */
    memset(vm->constants, 0, sizeof(vm->constants)); // Zero out constant table (as it is a array of ptrs, this means it is initialise to NULL ptrs)
    // preload constants
    vm->constants[vm->constantCount++] = (PrimitiveObject *)get_null();        // index 0
    vm->constants[vm->constantCount++] = (PrimitiveObject *)new_bool(0);       // index 1 (false)
    vm->constants[vm->constantCount++] = (PrimitiveObject *)new_bool(1);       // index 2 (true)
    for (int i = -510; i <= 510; i++) {
        vm->constants[vm->constantCount++] = (PrimitiveObject *)new_int(i);
    }

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
// OP_GET_GLOBAL, // Get a global variable                     Binary: 0b 0000 0100 done         
// OP_SET_GLOBAL, // Set a global variable                     Binary: 0b 0000 0101 done
// OP_CALL,       // Call function                             Binary: 0b 0000 0110 
// OP_RETURN,     // Return from function                      Binary: 0b 0000 0111 
// OP_HALT,       // Stop execution                            Binary: 0b 0000 1000 done
// OP_JMP,        // JMP to an offset from current idx         Binary: 0b 0000 1001 done
// OP_JMPIF,      // false ? JMP to and offset from curr idx   Binary: 0b 0000 1010 done 

// // OPCODE primitives (SYNTAX: TYPE (ARG))
// INT,           // prim obj int representation               Binary: 0b 0000 1011 done 
// FLOAT,         // prim obj float representation             Binary: 0b 0000 1100 done
// BOOL,          // prim obj bool representation              Binary: 0b 0000 1101 done 
// STR,           // prim obj str representation               Binary: 0b 0000 1110 done 
// _NULL_,        // prim _NULL_ representation (NO ARGS)      Binary: 0b 0000 1111 done
// ID,            // ID representation                         Binary: 0b 0001 0000 done

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

// OP_mod, // (TODO: Implement this in core primitives mengtek u dumb fuck)
// OP_EQ, 
// OP_GEQ,
// OP_QE,
// OP_LEQ,
// OP_LE

/*
INT, FLOAT -> read 8 bytes after opcode
BOOL -> read 1 byte after opcode
STR -> read 4 bytes after opcode (char length) then read 4 bytes as an int to get num bytes to read
ID -> read 2 bytes after opcode (char length) then read the 2 bytes as an int to get the num bytes to read
JMP -> read 4 bytes after opcode to get JMP offset
JMPIF -> read 4 bytes after opcode to get JMP offset
*/

/* Truthy value function helper function for vm not meant to be used outside of vm scope */
int is_truthy(PrimitiveObject* obj) {
    if (!obj) return 0; // Null is false

    switch (obj->type) {
        case TYPE_bool: // either one or 0
            return ((bool_Object*)obj)->value ? 1 : 0;

        case TYPE_int: // truthy as long as it is not 0
            return ((int_Object*)obj)->value != 0 ? 1 : 0;

        case TYPE_float: // truthy as long as it is not 0
            return ((float_Object*)obj)->value != 0.0 ? 1 : 0;

        case TYPE_str: // truthy so long as it is not an empty string
            return ((str_Object*)obj)->value[0] != '\0' ? 1 : 0;

        case TYPE_Null: // always false
            return 0;

        default:
            printf("Warning: Unexpected type in truthy check.\n");
            return 0;
    }
}

/* get constant function definition */
PrimitiveObject * get_constant(VM *vm, OpCode opcode, int64_t value) {
    switch (opcode) {
        case _NULL_:
            return vm->constants[0]; // __NULL__

        case BOOL:
            return vm->constants[value ? 2 : 1];

        case INT:
            if (value >= -510 && value <= 510) {
                return vm->constants[3 + value + 510];
            } else {
                return NULL; // Not in constant pool
            }

        default:
            printf("Error: get_constant only supports BOOL, INT, _NULL_\n");
            return NULL;
    }
}




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

    vm->bytecode_ip = (uint64_t*) bytecode; // Cast to match existing struct, but must careful with byte addressing

    // add in the header reading here

    while (1) {
        uint8_t instruction = *(uint8_t*)vm->bytecode_ip;  // cast the read byte to a uint8 to match the Opcodes enums
        vm->bytecode_ip = (uint64_t*)((uint8_t*)vm->bytecode_ip + 1); // move to next bytes

        switch (instruction) {
            case OP_HALT:
                printf("VM halted.\n");
                free(bytecode);
                return;

            case INT: { // [1 byte opcode][8 byte int64]
                int64_t value; 
                PrimitiveObject * int_to_push;

                memcpy(&value, vm->bytecode_ip, sizeof(int64_t));  // Copy raw bytes into value
                vm->bytecode_ip = (uint64_t*)((uint8_t*)vm->bytecode_ip + sizeof(int64_t)); // Move past 8 bytes
                int_to_push = get_constant(vm, INT, value);
                if (!int_to_push) int_to_push = (PrimitiveObject *) new_int(value);
                push(vm, int_to_push, PRIMITIVE_OBJ);
                break;
            }
            
            case FLOAT: { // [1 byte opcode][8 byte double]
                double value;
                memcpy(&value, vm->bytecode_ip, sizeof(double)); // Copy raw bytes into value
                vm->bytecode_ip = (uint64_t*)((uint8_t*)vm->bytecode_ip + sizeof(double)); // Move past 8 bytes
                push(vm, new_float(value), PRIMITIVE_OBJ);
                break;
            }
            
            case BOOL: { // [1 byte opcode][1 byte int8]
                uint8_t bool_value; 
                PrimitiveObject * bool_to_push;
                memcpy(&bool_value, vm->bytecode_ip, sizeof(uint8_t)); // Read single byte
                vm->bytecode_ip = (uint64_t*)((uint8_t*)vm->bytecode_ip + sizeof(uint8_t)); // Move past 1 byte
                push(vm, get_constant(vm, BOOL, bool_value), PRIMITIVE_OBJ);
                break;
            }
            
            case STR: { // [1 byte opcode][4 byte length][ length number of bytes]
                uint32_t length;
                memcpy(&length, vm->bytecode_ip, sizeof(uint32_t)); // Read 4 bytes as string length
                vm->bytecode_ip = (uint64_t*)((uint8_t*)vm->bytecode_ip + sizeof(uint32_t)); // Move past length field
            
                char* str_value = malloc(length + 1);
                memcpy(str_value, vm->bytecode_ip, length); // Copy string data
                str_value[length] = '\0'; // Null-terminate
            
                vm->bytecode_ip = (uint64_t*)((uint8_t*)vm->bytecode_ip + length); // Move past string bytes
            
                push(vm, new_str(str_value), PRIMITIVE_OBJ);
                free(str_value); // Free our temp variable
                break;
            }
            
            
            case ID: {   // [1 byte opcode][2 byte ID length][ ID length number of bytes]
                uint16_t length;
                memcpy(&length, vm->bytecode_ip, sizeof(uint16_t)); // Read 2 bytes for ID length
                vm->bytecode_ip = (uint64_t*)((uint8_t*)vm->bytecode_ip + sizeof(uint16_t)); // Move past length field
            
                char* identifier = malloc(length + 1); // we malloced here so we must free when we hit OP_GET_GLOBAL and OP_SET_GLOBAL
                memcpy(identifier, vm->bytecode_ip, length); // Copy ID string
                identifier[length] = '\0'; // Null-terminate
            
                vm->bytecode_ip = (uint64_t*)((uint8_t*)vm->bytecode_ip + length); // Move past ID bytes
            
                push(vm, identifier, IDENTIFIER);  // Push identifier as raw string
                break;
            }

            case _NULL_: {
                push(vm, get_constant(vm, _NULL_, 0), PRIMITIVE_OBJ);
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

            
                        /*
            EXAMPLE:
            y = 4
            x = y 
            should translate to:
            INT 4 -> (pushed onto stack) -> stack_bottom [4] stack_top
            ID y -> (pushed onto stack) -> stack_bottom [4, y] stack_top
            OP_SET_GLOBAL -> (pops y, pops 4, sets y to 4 in GT) -> stack_bottom [] stack_top
            ID y -> (pushed onto stack) -> stack_bottom [y] stack_top
            OP_GET_GLOBAL -> (pops y, gets y from GT and pushed onto stack) -> stack_bottom [4] stack_top
            ID x (pushed onto stack) -> stack_bottom [4, x] stack_top
            OP_SET_GLOBAL -> (pops x, pops 4, sets x to 4 in GT) -> stack_bottom [] stack_top
            */
            case OP_GET_GLOBAL: {
                StackEntry id = pop(vm);
            
                if (id.entry_type != IDENTIFIER) {
                    printf("Error: Expected IDENTIFIER for global name.\n");
                    break;
                }
            
                char* var_name = (char*)id.value;
                GlobalEntry* entry = (GlobalEntry*)hashmap_get(vm->globals, var_name);
            
                if (!entry) {
                    printf("Error: Undefined global variable \"%s\".\n", var_name);
                    free(var_name);
                    break;
                }
                
                push(vm, entry->value, entry->entry_type);
            
                free(var_name); // malloced during ID opcode
                break;
            }
            
            /*
            EXAMPLE:
            x = 4 
            should translate to:
            INT 4 -> (pushed onto stack) -> stack_bottom [4] stack_top
            ID x -> (pushed onto stack) -> stack_bottom [4, x] stack_top
            OP_SET_GLOBAL -> (pops x, pops 4, sets x to 4 in GT) -> stack_bottom [] stack_top
            */
            case OP_SET_GLOBAL: {
                StackEntry id = pop(vm);
                StackEntry value = pop(vm);
            
                if (id.entry_type != IDENTIFIER) {
                    printf("Error: Expected IDENTIFIER for global name.\n");
                    break;
                }
            
                char* var_name = (char*)id.value;
            
                GlobalEntry* entry = malloc(sizeof(GlobalEntry));
                entry->value = value.value;
                entry->entry_type = value.entry_type;
                
                /*
                Keep in mind we still have a potential memory leak here as we do not garbage collect the values of the globalEntry
                (We are intentionally not freeing the values here as there may be multiple references to them in stackframes and vars so this a job for the GC)
                (However we can safely free GlobalEntry as it is simply a wrapper)
                */
                hashmap_set(vm->globals, var_name, entry, free); // free is added here as we have to free the previous globalEntry when we reassign a variable
            
                free(var_name); // was malloc'ed during ID opcode
                break;
            }
            

            case OP_JMP: { //[1 byte opcode][4 byte signed offset]
                int32_t offset;
                memcpy(&offset, vm->bytecode_ip, sizeof(int32_t)); // Read 4 bytes as a signed offset
                vm->bytecode_ip = (uint64_t*)((uint8_t*)vm->bytecode_ip + sizeof(int32_t)); // Move past the offset bytes
            
                // Apply jump
                vm->bytecode_ip = (uint64_t*)((uint8_t*)vm->bytecode_ip + offset);
                break;
            }

            case OP_JMPIF: {
                int32_t offset;
                memcpy(&offset, vm->bytecode_ip, sizeof(int32_t)); // Read 4-byte offset
                vm->bytecode_ip = (uint64_t*)((uint8_t*)vm->bytecode_ip + sizeof(int32_t)); // Move past offset
            
                StackEntry condition = pop(vm);
                if (condition.entry_type != PRIMITIVE_OBJ) {
                    printf("Error: Expected PRIMITIVE_OBJ for conditional jump.\n");
                    break;
                }
            
                if (!is_truthy((PrimitiveObject*)condition.value)) {
                    vm->bytecode_ip = (uint64_t*)((uint8_t*)vm->bytecode_ip + offset); // Apply jump
                }
                break;
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
