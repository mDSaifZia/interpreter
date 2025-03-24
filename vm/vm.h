#ifndef VM_H
#define VM_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../CorePrimitives/core_primitives.h"
#include "../AdvancedPrimitives/advanced_primitives.h"
#include "../hashmap/hashmap.h"

#define STACK_MAX 1024
#define MAX_CONSTANTS 1024
#define MAX_GLOBALS 1024
#define MAX_FUNCTIONS 256
#define MAX_OBJECTS 256

/* Bytecode Instructions */
typedef enum {
    // OPCODE instructions (SYNTAX: OP (NO ARG))
    OP_ADD,        // Add two values                            Binary: 0b 0000 0000
    OP_SUB,        // Sub two values (consistency of sub op)    Binary: 0b 0000 0001
    OP_MUL,        // Multiply                                  Binary: 0b 0000 0010
    OP_DIV,        // Divide                                    Binary: 0b 0000 0011
    OP_GET_GLOBAL, // Get a global variable                     Binary: 0b 0000 0100
    OP_SET_GLOBAL, // Set a global variable                     Binary: 0b 0000 0101
    OP_CALL,       // Call function                             Binary: 0b 0000 0110
    OP_RETURN,     // Return from function                      Binary: 0b 0000 0111
    OP_HALT,       // Stop execution                            Binary: 0b 0000 1000
    OP_JMP,        // JMP to an offset from current idx         Binary: 0b 0000 1001
    OP_JMPIF,      // false ? JMP to and offset from curr idx   Binary: 0b 0000 1010

    // OPCODE primitives (SYNTAX: TYPE (ARG))
    INT,           // prim obj int representation               Binary: 0b 0000 1011
    FLOAT,         // prim obj float representation             Binary: 0b 0000 1100
    BOOL,          // prim obj bool representation              Binary: 0b 0000 1101
    STR,           // prim obj str representation               Binary: 0b 0000 1110
    _NULL_,        // prim _NULL_ representation                Binary: 0b 0000 1111
    ID,            // ID representation                         Binary: 0b 0001 0000
    ID_FUNC,

    // OPCODE flags (SYNTAX: FLAG (NO ARG))
    OP_FUNCDEF,    // Flag for start of function definition     Binary: 0b 0001 0001
    OP_ENDFUNC,    // Flag for end of function definition       Binary: 0b 0001 0010
    OP_CLASSDEF,   // Flag for start of class definition        Binary: 0b 0001 0011
    OP_ENDCLASS,  // Flag for end of class definition           Binary: 0b 0001 0100

    // OPCODE binary operators (SYNTAX: BIN_OP (NO ARGS))
    OP_BLSHIFT,  // Flag for end of class definition            Binary: 0b 0001 0101
    OP_BRSHIFT,  // Flag for end of class definition            Binary: 0b 0001 0110
    OP_BXOR,     // Flag for end of class definition            Binary: 0b 0001 0111
    OP_BOR,      // Flag for end of class definition            Binary: 0b 0001 1000
    OP_BAND,     // Flag for end of class definition            Binary: 0b 0001 1001

    // OPCODE local variables (SYNTAX: OP (NO ARG))
    OP_GET_LOCAL,  // Get local variable                        Binary: 0b 0001 1010
    OP_SET_LOCAL,  // Set local variable                        Binary: 0b 0001 1011
    OP_DEFINE_LOCAL, // Define new local variable               Binary: 0b 0001 1100
    OP_ENTER_SCOPE, // Enter a new scope                        Binary: 0b 0001 1101

    // OPCODES standard functions
    OP_PRINT,       // prints to stdout                         Binary: 0b 0001 1110
    OP_INPUT,       // gets values from stdin                   Binary: 0b 0001 1111

    OP_MOD, // (TODO: Implement this in core primitives mengtek u dumb fuck)
    OP_EQ, 
    OP_GEQ,
    OP_QE,
    OP_LEQ,
    OP_LE

} OpCode;


/* /////////////////////////////// STACK TABLE /////////////////////////////// */

typedef enum {
    PRIMITIVE_OBJ,
    ADVANCED_OBJ,
    FUNCTION_FRAME,
    IDENTIFIER
} StackEntryType;

typedef struct StackEntry{
    void * value;
    StackEntryType entry_type;
} StackEntry;

typedef struct Stack{
    size_t base_pointer; // base pointer of the stack
    size_t stack_top;    // stack top always points to free space on stack
    StackEntry stack[STACK_MAX];
} Stack;

/* /////////////////////////////// STACK TABLE /////////////////////////////// */

/* /////////////////////////////// GLOBAL TABLE /////////////////////////////// */
/* This is essentially the same StackEntry but made different so we can mutate it if needed */
typedef struct GlobalEntry{
  void * value;
  StackEntryType entry_type;
} GlobalEntry;

/* /////////////////////////////// GLOBAL TABLE /////////////////////////////// */

/* /////////////////////////////// FUNCTION TABLE /////////////////////////////// */

typedef struct {
    char *name;       // Function name
    size_t bytecode_offset; // Start of function in bytecode
    int paramCount;   // Number of parameters
} FunctionEntry;

/* /////////////////////////////// FUNCTION TABLE /////////////////////////////// */

/* /////////////////////////////// OBJECT TABLE /////////////////////////////// */

typedef struct {
    char *name;       // Object name
    size_t bytecode_offset; // Start of class definition
} ObjectEntry;

/* /////////////////////////////// OBJECT TABLE /////////////////////////////// */

/* VM Structure */
typedef struct {
    Stack stack;  // stack to store entries

    FunctionEntry functions[MAX_FUNCTIONS]; // Function table (subject to change as we just implemented hashmaps)
    int functionCount;

    ObjectEntry objects[MAX_OBJECTS]; // Object table (subject to change as we just implemented hashmaps)
    int objectCount;

    Hashmap * globals;  // Global variable storage

    // implement an instance table for garbage collection

    PrimitiveObject * constants[MAX_CONSTANTS]; // Constant table (stores integer and float constants for quick lookup) We technically do not need to free this as it should never grow beyond the table size
    int constantCount;

    uint64_t* bytecode_ip;  // Pointer to bytecode (bytecode should reasonably not exceed 2^64)
} VM;

/* Function Declarations */
VM * initVM(); // VM "object" like struct
void run(VM* vm, const char* bytecode_file);
void freeVM(VM* vm);

/* stack functions */
void push(VM* vm, void* value, StackEntryType type);
StackEntry pop(VM* vm);

/* constant table functions */

/*
Opcode: Only accepts BOOL, _NULL_ and INT
value: represents the value to find

get_constant(BOOL, 1) -> returns boolObject * True
get_constant(BOOL, 0) -> returns boolObject * False
get_constant(__NULL__, 1) -> returns NULLobject *
*/
PrimitiveObject * get_constant(VM *vm, OpCode opcode, int64_t value);


#endif
