# RATSNAKE Interpreter

 **Ratsnake** is an interpreted dynamically typed language developed by our team. It uses a C-based virtual machine that reads and runs a compiled custom binary file **.rtskB**. 
 
## Architecture
Ratsnake's architecture is based on a basic stack based implementation where Objects and primitives are pushed to a stack and popped when an  operator instruction is read in. All **types** in Ratsnake are treated as either **Objects** or **Primitives**, and operator tokens are actually function/ "method" calls of these PseudoObjects.

Visualisation of Ratsnake's architechture:

Anatomy of PrimitiveObject:

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
****
| Data types|Description|
|--|--|
| Primitive **int**| PrimtiveObject integer data type of ratsnake, holds a long signed integer (8 bytes)  |
| Primitive **float**| PrimtiveObject float data type of ratsnake, holds a double (8 bytes)  |
| Primitive **str**| PrimtiveObject string data type of ratsnake, holds a maximum of arounf 4GB of string length *(not reccomended)*  |
| Primitive **NULL**| PrimtiveObject NULL data type of ratsnake. Represents NULL value *Despite it's name Primitive NULL is most similar to Python's None* |
| Primitive **Bool**| PrimtiveObject bool data type of ratsnake, holds a int8_t (1 bytes) either 1 or 0 |
| Object **Object**| Unimplemented but is mean't to represent all advanced object types and classes. |


## Syntax

The file explorer is accessible using the button in left corner of the navigation bar. You can create a new file by clicking the **New file** button in the file explorer. You can also create folders by clicking the **New folder** button.

## Compilation
Ratsnake requires GCC to compile, to install gcc run the following commands:
>**Linux** (Ubuntu/Debian/WSL based distributions) Bash
>```Bash
>sudo apt update
>sudo apt install build-essential
>gcc --version
>sudo dnf install gcc gcc-c++ make
>gcc --version // check your gcc version
>```

>**Windows** 
>```Powershell
>Set-ExecutionPolicy Bypass -Scope Process -Force; `
>[System.Net.ServicePointManager]::SecurityProtocol = `
>[System.Net.ServicePointManager]::SecurityProtocol -bor >3072; `
>iex ((New-Object System.Net.WebClient).DownloadString
>('https://community.chocolatey.org/install.ps1'))
>choco install mingw -y
>gcc --version //check your gcc version
>```
To compile the Ratsnake virtual machine on Linux and Windows, just run the following command on the terminal in the root directory of the project. 
```Bash
make
```
This will produce a ***ratsnake*** executable file if compiled on Linux or a ***ratsnake.exe*** executable if compiled on Windows (Inside the project root directory).

## Running Ratsnake vm

You can rename the current file by clicking the file name in the navigation bar or by clicking the **Rename** button in the file explorer.

