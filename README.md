# RATSNAKE Interpreter

 **Ratsnake** is an interpreted language developed by our team that uses a C-based virtual machine that reads and runs a compiled custom binary file **.rtskB**. 
 
## Architecture
Ratsnake's architecture is based on a basic stack based implementation where Objects and primitives are pushed to a stack and popped when an  operator instruction is read in. All **types** in Ratsnake are treated as either **Objects** or **Primitives**, and operator tokens are actually function/ "method" calls of these PseudoObjects.

## keywords and features 
 
| Features|Example  ||
|--|--|--|
| Declare var|``` var x = 5```  ||
| Assign var|``` x = "hello"```  ||
| For loop|``` loop i from(1,5){...}```  ||
| While loop|``` while (condition) {...}```  ||
| Bitwise OR, AND, XOR, LSHIFT, RSHIFT|``` |, &, ^, <<, >>```  ||
| Compare|``` ==, !=, >=, <=, >, <```  ||
| Function definition|``` fn f(args) {}```  ||
| Function call|``` f(args);```  ||
| Comments|``` // This is a comment```  ||
****
| Data types|Description||
|--|--|--|
| Primitive **int**| PrimtiveObject integer data type of ratsnake, holds a long signed integer (8 bytes)  ||
| Primitive **float**| PrimtiveObject float data type of ratsnake, holds a double (8 bytes)  ||
| Primitive **str**| PrimtiveObject string data type of ratsnake, holds a maximum of arounf 4GB of string length *(not reccomended)*  ||


## Syntax

The file explorer is accessible using the button in left corner of the navigation bar. You can create a new file by clicking the **New file** button in the file explorer. You can also create folders by clicking the **New folder** button.

## Compilation
Ratsnake requires GCC to compile, to install gcc run the following commands:
>Linux 
>```Bash
>
>```

>Windows
>```Bash
>
>```
To compile the Ratsnake virtual machine on Linux and Windows, just run the following command on the terminal in the root directory of the project. 
```Bash
make
```
This will produce a ***ratsnake*** executable file if compiled on Linux or a ***ratsnake.exe*** executable if compiled on Windows (Inside the project root directory).

## Running Ratsnake vm

You can rename the current file by clicking the file name in the navigation bar or by clicking the **Rename** button in the file explorer.

