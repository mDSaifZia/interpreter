# RATSNAKE Interpreter

 **Ratsnake** is an interpreted dynamically typed language developed by our team. It uses a C-based virtual machine that reads and runs a compiled custom binary file produced by the vm. 

## Architecture
Ratsnake's architecture is based on a basic stack based language implementation where Objects and primitives are pushed to a stack and popped when an  operator instruction is read in. All **types** in Ratsnake are treated as either **Objects** or **Primitives**, and operator tokens are actually function/ "method" calls of these PseudoObjects. Ratsnake, also uses a scope structure similar to Local-Enclosing-Global-Builtin (LGEB) scoping, where we have dropped the "enclosing" scope as advanced objects have yet to be implemented.

Visualisation of Ratsnake's architechture:

The pipeline follows:
>`source.rtsk` → (Frontend Manager (python)) →  `AST` → (IR Compiler (C)) → `source.bytecode` → (Binary Compiler (C)) → `source.rtskbin` → (VM (C)) → code execution
Anatomy of PrimitiveObject:
```C
typedef enum {
    TYPE_int,
    TYPE_float,
    TYPE_bool,
    TYPE_str,
    TYPE_Null,
} PrimitiveType;

/* maps the enum types to a string representation */
extern const char *PrimitiveTypeNames[];

/* Forward declaration */
typedef struct PrimitiveObject PrimitiveObject;

/* Function pointer type for binary operations */
typedef PrimitiveObject* (*BinaryOp)(PrimitiveObject*, PrimitiveObject*);
typedef int (*CompareOp)(PrimitiveObject*, PrimitiveObject*);
typedef char* (*DunderString)(PrimitiveObject*);

/* Base primitive object */
struct PrimitiveObject {
    PrimitiveType type;
    VM* vm;
    BinaryOp add;
    BinaryOp mul;
    BinaryOp div;
    BinaryOp mod;
    CompareOp eq;
    CompareOp neq;
    CompareOp geq;
    CompareOp gt;
    CompareOp leq;
    CompareOp lt;
    DunderString __str__;
};
```
## keywords and features 

| Features|Example  |
|--|--|
| Declare var|``` var x = 5```  |
| Assign var|``` x = "hello"```  |
| For loop|``` loop i from(1,5){...}```  |
| While loop|``` while (condition) {...}```  |
| Bitwise OR, AND, XOR, LSHIFT, RSHIFT|``` \|, &, ^, <<, >>```  |
| Compare|``` ==, !=, >=, <=, >, <```  |
| Logical|``` \|\|, &&, !```  |
| Function definition|``` fn f(args) {}```  |
| Function call|``` f(args);```  |
| Comments|``` // This is a comment```  |
| Dynamic typing|``` var x =  "Hello" ; x = 5; ```  |
| Declare block| ```{}```|
| Delimit next instruction|```;```|
| Print|```print(value)```|
| Input|```var x = input(message)```|
****
| Data types|Description|
|--|--|
| Primitive **int**| PrimitiveObject integer data type of ratsnake, holds a long signed integer (8 bytes)  |
| Primitive **float**| PrimitiveObject float data type of ratsnake, holds a double (8 bytes)  |
| Primitive **str**| PrimitiveObject string data type of ratsnake, holds a maximum of around 4GB of string length *(not reccomended)*  |
| Primitive **NULL**| PrimitiveObject NULL data type of ratsnake. Represents NULL value *Despite it's name Primitive NULL is most similar to Python's None* |
| Primitive **Bool**| PrimitiveObject bool data type of ratsnake, holds a int8_t (1 bytes) either 1 or 0 |
| Object **Object**| Unimplemented but is meant to represent all advanced object types and classes. |
****
Below is the list of OPCODES of Ratsnake. Source code is first parsed and then transpiled into this intermediate representation *(before finally being compiled into the custom binary format)*.
#### Arithmetic
| OPCODE |Description|
|--|--|
|OP_ADD| Calls add function pointer of PrimitiveObjects and Objects|
|OP_MUL| Calls mul function pointer of PrimitiveObjects and Objects|
|OP_SUB| Calls sub function pointer of PrimitiveObjects and Objects|
|OP_DIV| Calls div function pointer of PrimitiveObjects and Objects|
|OP_MOD|Pops 2 Objects from stack and calls mod "method"|
#### Identifier opcodes
| OPCODE |Description|
|--|--|
|OP_GET_GLOBAL| Pops an id from stack and pushes the object assigned to that id onto stack| 
|OP_SET_GLOBAL| Pops an object and an id from stack and assigns id to the object in global table|
|OP_GET_LOCAL|Pops a local id from stack and gets the assigned value|
|OP_SET_LOCAL|Pops an object and id from the stack and sets the assigned value|
|LOCAL|Local identifier with identifier index|
|ID| Global identifier with identifier value|
#### Function opcodes
| OPCODE |Description|
|--|--|
|OP_CALL| Pops an id from stack and attempts to call the function id|  
|OP_RETURN| Pushes the final return of a function onto the stack and destroys stackframe.|
|OP_HALT| Halts the VM|
#### Control flow
| OPCODE |Description|
|--|--|
|OP_JMP| Jumps from the current position by an offset|
|OP_JMPIF| Pops a value from stack and jumps from the current offset if the value is truthy|
#### Primitive Types
| OPCODE |Description|
|--|--|
|INT| Creates a primitive int with specified value|
|FLOAT| Creates a primitive float with specified value|
|BOOL| Creates a primitive bool **true/false**|  
|STR| Creates a primitive string with string value|
|\_NULL\_| Creates a primitive NULL|
#### Function and Class flags
| OPCODE |Description|
|--|--|
|OP_FUNCDEF|Flag for the start of function definition|
|OP_ENDFUNC|Flag for the end of function definition|
|OP_CLASSDEF|Flag for start of class definition *Not implemented*|
|OP_ENDCLASS|Flag for end of class definition *Not implemented*|
#### Bitwise Operators
| OPCODE |Description|
|--|--|
|OP_BLSHIFT|Pops 2 primitive ints from stack and performs LSHIFT|
|OP_BRSHIFT|Pops 2 primitive ints from stack and performs RSHIFT|
|OP_BXOR|Pops 2 primitive ints from stack and performs XOR|
|OP_BOR|Pops 2 primitive ints from stack and performs OR|
|OP_BAND|Pops 2 primitive ints from stack and performs AND|
#### Uninary Operators
| OPCODE |Description|
|--|--|
|OP_LOGICAL_AND|Pops 2 Objects from stack and performs logical AND on their truthiness|
|OP_LOGICAL_OR|Pops 2 Objects from stack and performs logical OR on their truthiness|
|OP_LOGICAL_NOT|Pops 1 Object from stack and performs logical NOT on their truthiness|
#### I/O
| OPCODE |Description|
|--|--|
|OP_PRINT|Pops an object from stack and calls its str "method" and prints it to stdout|
|OP_INPUT|Waits for an input from stdin pushes a str onto stack|
#### Stack OPCODES
| OPCODE |Description|
|--|--|
|OP_POP|Pops a value from stack|
#### Comparison Operators
| OPCODE |Description|
|--|--|
|OP_NEQ|Pops 2 Objects from stack and checks for inequality|
|OP_EQ|Pops 2 Objects from stack and checks for equality|
|OP_GEQ|Pops 2 Objects from stack and checks for greater than equal|
|OP_GT|Pops 2 Objects from stack and checks for greater than|
|OP_LEQ|Pops 2 Objects from stack and checks for less than equal|
|OP_LT|Pops 2 Objects from stack and checks for less than|

> Memonics:
> `NUMARGS` and `NUMVARS` are special header keywords used during function compilation and are directly translated into 2-byte integers in the final bytecode.
> `FUNCID` is a memonic for the actual opcode `ID` it is not a unique opcode. This is done for readbility when inspecting the .bytecode file.

## Syntax
Ratsnake's source code syntax is based on a hybrid between C, Python and JavaScript. 

**Examples:**
>Source Code
>```
>fn fib(n) {
>    if (n <= 1) {
>       return 1;
>    }
>    return fib(n-1) + fib(n-2);
>}
>var result = fib(10);
>print(result);
>```
****
The bytecode generated by the parser and lexer for the function above can be found below.
>Intermediate Representation Bytecode
>```
>[64 byte header]
>INT 10
>FUNCID 3 fib
>OP_CALL
>OP_PRINT
>OP_FUNCDEF
>NUMARGS 1
>NUMVARS 1
>IDFUNC 3 fib
>LOCAL 0
>OP_GET_LOCAL
>INT 1
>OP_LEQ
>OP_JMPIF 10
>LOCAL 0
>OP_GET_LOCAL
>OP_RETURN
>OP_JMP 49
>LOCAL 0
>OP_GET_LOCAL
>INT 2
>OP_SUB
>IDFUNC 3 fib
>OP_CALL
>LOCAL 0
>OP_GET_LOCAL
>INT 1
>OP_SUB
>IDFUNC 3 fib
>OP_CALL
>OP_ADD
>OP_RETURN
>OP_JMP 0
>__NULL__
>OP_RETURN
>OP_ENDFUNC
>OP_FUNCDEF
>```

## Compilation
Ratsnake requires GCC and a compiler version of C99 to compile as it makes use of dynamic stack allocation, to install gcc run the following commands:
>**Linux** (Ubuntu/Debian/WSL based distributions) Bash
>```Bash
>sudo apt update
>sudo apt install build-essential
>gcc --version
>sudo dnf install gcc gcc-c++ make
>gcc --version // check your gcc version
>```

>**Windows** (10/ 11) PowerShell (Run PowerShell in elevated position: Administrator Mode)
>```Powershell
>Set-ExecutionPolicy Bypass -Scope Process -Force; `
>[System.Net.ServicePointManager]::SecurityProtocol = `
>[System.Net.ServicePointManager]::SecurityProtocol -bor >3072; `
>iex ((New-Object System.Net.WebClient).DownloadString
>('https://community.chocolatey.org/install.ps1'))
>choco install mingw -y
>choco install make -y
>gcc --version //check your gcc version
>```

To compile the Ratsnake virtual machine on Linux and Windows, just run the following command on the terminal in the root directory of the project.
> **Disclaimer:** Project is not compiled with flags `-ANSI -pdentic -Wall -Werror` as it makes use of some syntax and data structure practices which do not align with them.
```Bash
make
```
This will produce a ***ratsnake*** executable file if compiled on Linux or a ***ratsnake.exe*** executable if compiled on Windows (Inside the project root directory).

## Running Ratsnake vm
Below is the general help command to run ratsnake. It requires the path/name of the source code file (.rtsk) and has 2 optional flags that can be inserted in any order.
```
./ratsnake source_code.rtsk [-keep_ir] [-keep_bin]
```
-keep_ir: keeps the .bytecode file after vm finishes

-keep_bin: keeps the .rtskbin file after vm finishes

**Examples**
**Powershell**
```Bash
./ratsnake source_code.rtsk -keep_ir
// runs the source_code file and retains ONLY the intermediate representation bytecode file after execution.
```
**Linux**
```Bash
./ratsnake source_code.rtsk
// runs the source_code file and does not retain the rtsk binary or the intermediate representation files after execution.
```

## License
MIT License
