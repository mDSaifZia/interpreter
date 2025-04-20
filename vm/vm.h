#ifndef VM_H
#define VM_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../CorePrimitives/core_primitives.h"
#include "../AdvancedPrimitives/advanced_primitives.h"
#include "../hashmap/hashmap.h"

#define STACK_MAX 4096
#define MAX_CONSTANTS 1024
#define MAX_GLOBALS 1024
#define MAX_FUNCTIONS 1024
#define MAX_OBJECTS 1024

/* Forward declaration */
typedef struct PrimitiveObject PrimitiveObject;

/* Bytecode Instructions */
typedef enum {
    // OPCODE instructions (SYNTAX: OP (NO ARG))
    OP_ADD,        // Add two values [1 byte]
    OP_MUL,        // Multiply [1 byte]
    OP_SUB,        // Sub two values (consistency of sub op) [1 byte]
    OP_DIV,        // Divide [1 byte]
    OP_GET_GLOBAL, // Get a global variable [1 byte]
    OP_SET_GLOBAL, // Set a global variable [1 byte]
    OP_CALL,       // Call function [1 byte]
    OP_RETURN,     // Return from function [1 byte]
    OP_HALT,       // Stop execution [1 byte]
    OP_JMP,        // JMP to an offset from current idx [1 byte]
    OP_JMPIF,      // false ? JMP to and offset from curr idx [1 byte]

    // OPCODE primitives (SYNTAX: TYPE (ARG))
    INT,           // prim obj int representation [1 byte][8 bytes]
    FLOAT,         // prim obj float representation [1 byte][8 bytes]
    BOOL,          // prim obj bool representation [1 byte][1 byte]
    STR,           // prim obj str representation [1 byte][]
    _NULL_,        // prim _NULL_ representation [1 byte]
    ID,            // ID representation [1 byte opcode][2 byte ID length][ ID length number of bytes]

    // OPCODE flags (SYNTAX: FLAG (NO ARG))
    OP_FUNCDEF,    // Flag for start of function definition [1 byte]
    OP_ENDFUNC,    // Flag for end of function definition [1 byte]
    OP_CLASSDEF,   // Flag for start of class definition [1 byte]
    OP_ENDCLASS,  // Flag for end of class definition [1 byte]

    // OPCODE binary operators (SYNTAX: BIN_OP (NO ARGS))
    OP_BLSHIFT,  // Bitwise left shift [1 byte]
    OP_BRSHIFT,  // Bitwise right shift [1 byte]
    OP_BXOR,     // Bitwise XOR [1 byte]
    OP_BOR,      // Bitwise OR [1 byte]
    OP_BAND,     // Bitwise AND [1 byte]

    // OPDCODE logical operators
    OP_LOGICAL_AND,
    OP_LOGICAL_OR,
    OP_LOGICAL_NOT,

    // OPCODE local variables (SYNTAX: OP (NO ARG)) (I may remove these)
    OP_GET_LOCAL,  // Get local variable [1 byte]
    OP_SET_LOCAL,  // Set local variable [1 byte]
    LOCAL,        // Analogous to ID for local variables arguments [1 byte][2 bytes]

    // OPCODES standard functions
    OP_PRINT,       // prints to stdout [1 byte]
    OP_INPUT,       // gets values from stdin [1 byte]

    OP_POP, //[1 byte]

    OP_MOD, // [1 byte]
    OP_NEQ, // [1 byte]
    OP_EQ, // [1 byte]
    OP_GEQ, // [1 byte]
    OP_GT, // [1 byte]
    OP_LEQ,// [1 byte]
    OP_LT, // [1 byte]

    OP_PARSEINT,
    OP_PARSESTR,
    OP_PARSEFLOAT,
    OP_PARSEBOOL
} OpCode;


/* /////////////////////////////// STACK TABLE /////////////////////////////// */

typedef enum {
    PRIMITIVE_OBJ,
    ADVANCED_OBJ,
    FUNCTION_FRAME,
    IDENTIFIER,
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
    size_t func_body_address; // Location of first instruction in body
    int num_args;      //Need to know number of arguments to pop out during OP_CALL
    int local_count;    //Number of local variables (including arguments)
} FunctionEntry;

/* /////////////////////////////// FUNCTION TABLE /////////////////////////////// */

/* /////////////////////////////// OBJECT TABLE /////////////////////////////// */

typedef struct {
    char *name;       // Object name
    size_t bytecode_offset; // Start of class definition
} ObjectEntry;

/* /////////////////////////////// OBJECT TABLE /////////////////////////////// */

/* /////////////////////////////// HEADER /////////////////////////////// */

typedef struct {
  size_t func_section_start; // Start location of function section
  size_t func_section_end;   // End location of function section
  size_t class_section_start; // Start location of class section
  size_t class_section_end;   // End location of class section
  size_t execution_section_start;   // Start location of bytecode that is executed
  uint8_t padding[24];         // 24 bytes of padding
} BytecodeHeader;

/* /////////////////////////////// HEADER /////////////////////////////// */


/* VM Structure */
typedef struct VM {
    Stack stack;  // stack to store entries

    ObjectEntry objects[MAX_OBJECTS]; // Object table (subject to change as we just implemented hashmaps)
    int objectCount;

    Hashmap * globals;  // Global variable storage

    Hashmap * functions;  // Function storage

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
we might deprecate this
*/
PrimitiveObject * get_constant(VM *vm, OpCode opcode, int64_t value);


#endif
