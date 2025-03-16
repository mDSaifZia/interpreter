#ifndef VM_H
#define VM_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "CorePrimitives/core_primitives.h"
#include "AdvancedPrimitives/advanced_primitives.h"

#define STACK_MAX 1024
#define MAX_CONSTANTS 1024
#define MAX_GLOBALS 1024
#define MAX_FUNCTIONS 256
#define MAX_OBJECTS 256

/* Bytecode Instructions */
typedef enum {
    // OPCODE instructions
    OP_CONSTANT,   // Load constant from constant table         Binary: 0b 0000 0000
    OP_ADD,        // Add two values                            Binary: 0b 0000 0001
    OP_SUB,        // Subtract                                  Binary: 0b 0000 0010
    OP_MUL,        // Multiply                                  Binary: 0b 0000 0011
    OP_DIV,        // Divide                                    Binary: 0b 0000 0100
    OP_ASSIGN,     // Assign variable                           Binary: 0b 0000 0101
    OP_GET_GLOBAL, // Get a global variable                     Binary: 0b 0000 0110              
    OP_SET_GLOBAL, // Set a global variable                     Binary: 0b 0000 0111
    OP_CALL,       // Call function                             Binary: 0b 0000 1000
    OP_RETURN,     // Return from function                      Binary: 0b 0000 1001
    OP_HALT,       // Stop execution                            Binary: 0b 0000 1010

    // OPCODE flags
    OP_FUNCDEF,    // Flag for start of function definition     Binary: 0b 0000 1011
    OP_ENDFUNC,    // Flag for end of function definition       Binary: 0b 0000 1100
    OP_CLASSDEF,   // Flag for start of class definition        Binary: 0b 0000 1101
    OP_ENDCLASS   // Flag for end of class definition           Binary: 0b 0000 1110

} OpCode;

/* Constant Table Entry */
typedef struct {
    PrimitiveObject* value;
} ConstantEntry;

/* Global Table Entry */
typedef struct {
    char *name;
    void* value;  // Can hold PrimitiveObject* OR Object*
    int isObject; // 0 = Primitive, 1 = Object
} GlobalEntry;

/* Function Table Entry */
typedef struct {
    char *name;       // Function name
    size_t bytecode_offset; // Start of function in bytecode
    int paramCount;   // Number of parameters
} FunctionEntry;

/* Object Table Entry */
typedef struct {
    char *name;       // Object name
    size_t bytecode_offset; // Start of class definition
} ObjectEntry;

/* VM Structure */
typedef struct {
    void* stack[STACK_MAX];  // Operand stack (stores both PrimitiveObjects and Objects)
    int stackTop;

    ConstantEntry constants[MAX_CONSTANTS]; // Constant table
    int constantCount;

    GlobalEntry globals[MAX_GLOBALS];  // Global variable storage
    int globalCount;

    FunctionEntry functions[MAX_FUNCTIONS]; // Function table
    int functionCount;

    ObjectEntry objects[MAX_OBJECTS]; // Object table
    int objectCount;

    uint64_t* bytecode;  // Pointer to bytecode (bytecode should reasonably not exceed 2^64)
    size_t ip;          // Instruction pointer
} VM;

/* Function Declarations */
void initVM(VM* vm, uint8_t* bytecode);
void push(VM* vm, void* value, int isObject);
void* pop(VM* vm, int* isObject);
void run(VM* vm);
void freeVM(VM* vm);

#endif /* VM_H */
