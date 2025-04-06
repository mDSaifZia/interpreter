#include "vm.h"
#include "../hashmap/hashmap.h"
#include "stackframe.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ///////////////////////// VM ///////////////////////// */
/* Initialize VM */
VM *initVM() {
  VM *vm = malloc(sizeof(VM));
  if (!vm) {
    printf("Failed to initialise Ratsnake vm.\n");
    return NULL;
  }

  // initialise stack
  vm->stack.base_pointer = 0;
  vm->stack.stack_top = 0;

  // initialise globals
  vm->globals = init_hashmap(MAX_GLOBALS);
  vm->functions = init_hashmap(MAX_FUNCTIONS);

  // initialise counters
  vm->constantCount = 0;
  /*vm->functionCount = 0;*/
  vm->objectCount = 0;

  // create clean tables
  memset(vm->objects, 0, sizeof(vm->objects)); // Zero out object table

  /* Populate the constant table with a few predefined constants: __NULL__,
   * TRUE, FALSE */
  memset(
      vm->constants, 0,
      sizeof(vm->constants)); // Zero out constant table (as it is a array of
                              // ptrs, this means it is initialise to NULL ptrs)
  // preload constants
  vm->constants[vm->constantCount++] = (PrimitiveObject *)get_null(vm); // index 0
  vm->constants[vm->constantCount++] =
      (PrimitiveObject *)new_bool(vm, 0); // index 1 (false)
  vm->constants[vm->constantCount++] =
      (PrimitiveObject *)new_bool(vm, 1); // index 2 (true)
  for (int i = -510; i <= 510; i++) {
    vm->constants[vm->constantCount++] = (PrimitiveObject *)new_int(vm, i);
  }

  // initialise bytecode instruction pointer
  vm->bytecode_ip = NULL;

  return vm;
}
/* ///////////////////////// VM FUNCTIONS ///////////////////////// */
// // OPCODE instructions (SYNTAX: OP (NO ARG))
// OP_ADD,        // Add two values                            done
// OP_SUB,        // Sub two values (consistency of sub op)    done
// OP_MUL,        // Multiply                                  done
// OP_DIV,        // Divide                                    done
// OP_GET_GLOBAL, // Get a global variable                     done
// OP_SET_GLOBAL, // Set a global variable                     done
// OP_CALL,       // Call function                             done
// OP_RETURN,     // Return from function                      done
// OP_HALT,       // Stop execution                            done
// OP_JMP,        // JMP to an offset from current idx         done
// OP_JMPIF,      // false ? JMP to and offset from curr idx   done

// // OPCODE primitives (SYNTAX: TYPE (ARG))
// INT,           // prim obj int representation               done
// FLOAT,         // prim obj float representation             done
// BOOL,          // prim obj bool representation              done
// STR,           // prim obj str representation               done
// _NULL_,        // prim _NULL_ representation (NO ARGS)      done
// ID,            // ID representation                         done

// // OPCODE flags (SYNTAX: FLAG (NO ARG))
// OP_FUNCDEF,    // Flag for start of function definition     done
// OP_ENDFUNC,    // Flag for end of function definition       done
// OP_CLASSDEF,   // Flag for start of class definition        -
// OP_ENDCLASS,  // Flag for end of class definition           -

// // OPCODE binary operators (SYNTAX: BIN_OP (NO ARGS))
// OP_BLSHIFT,  // Binary Left bitshift 
// OP_BRSHIFT,  // Binary Right bitshift
// OP_BXOR,     // Binary XOR
// OP_BOR,      // Binary OR
// OP_BAND,     // Binary AND

// // OPCODE local variables (SYNTAX: OP (NO ARG))
// OP_GET_LOCAL,  // Get local variable                        done
// OP_SET_LOCAL,  // Set local variable                        done
// LOCAL,         // LOCAL ID                                  done

// // OPCODES standard functions
// OP_PRINT,       // prints to stdout
// OP_INPUT,       // gets values from stdin

// OP_MOD,
// OP_NEQ
// OP_EQ,
// OP_GEQ,
// OP_GT,
// OP_LEQ,
// OP_LT

/*
INT, FLOAT -> read 8 bytes after opcode
BOOL -> read 1 byte after opcode
STR -> read 4 bytes after opcode (char length) then read 4 bytes as an int to
get num bytes to read ID -> read 2 bytes after opcode (char length) then read
the 2 bytes as an int to get the num bytes to read JMP -> read 4 bytes after
opcode to get JMP offset JMPIF -> read 4 bytes after opcode to get JMP offset
*/

/* Truthy value function helper function for vm not meant to be used outside of
 * vm scope */
int is_truthy(PrimitiveObject *obj) {
  if (!obj)
    return 0; // Null is false

  switch (obj->type) {
  case TYPE_bool: // either one or 0
    return ((bool_Object *)obj)->value ? 1 : 0;

  case TYPE_int: // truthy as long as it is not 0
    return ((int_Object *)obj)->value != 0 ? 1 : 0;

  case TYPE_float: // truthy as long as it is not 0
    return ((float_Object *)obj)->value != 0.0 ? 1 : 0;

  case TYPE_str: // truthy so long as it is not an empty string
    return ((str_Object *)obj)->value[0] != '\0' ? 1 : 0;

  case TYPE_Null: // always false
    return 0;

  default:
    printf("Warning: Unexpected type in truthy check.\n");
    return 0;
  }
}

/* get constant function definition */
PrimitiveObject *get_constant(VM *vm, OpCode opcode, int64_t value) {
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

/* Function to load function definitions from bytecode */
void load_functions(VM *vm, uint8_t *bytecode, size_t func_section_start,
                    size_t func_section_end) {

  uint8_t *function_section = bytecode + func_section_start;
  uint8_t *section_end = bytecode + func_section_end;

  // No functions declared
  if (section_end - function_section == 0) {
    return;
  }

//   printf("Reading from function section offset: %ld\n", function_section - bytecode);
//   printf("First byte in section: 0x%02X\n", *function_section);
  while (function_section < section_end) {
    if (*function_section != OP_FUNCDEF) {
        printf("Error: Expected function definition flag.\n");
        printf("Reading from function section offset: %ld\n", function_section - bytecode);
        printf("First byte in section: 0x%02X\n", *function_section);
        exit(EXIT_FAILURE);
    }
    function_section++; // Move past OP_FUNCDEF byte
    
    // Read num_args (2 bytes)
    uint16_t num_args;
    memcpy(&num_args, function_section, sizeof(uint16_t));
    function_section += sizeof(uint16_t);

    // Read num_var (aka local_count) (2 bytes)
    uint16_t local_count;
    memcpy(&local_count, function_section, sizeof(uint16_t));
    function_section += sizeof(uint16_t);

    // Read function ID opcode
    if (*function_section != ID) {
      printf("Error: Expected ID opcode for function name.\n");
      return;
    }
    function_section++; // Move past ID opcode

    // Read function name length (2 bytes)
    uint16_t name_length;
    memcpy(&name_length, function_section, sizeof(uint16_t));
    function_section += sizeof(uint16_t);

    // Read function name
    char *func_name = malloc(name_length + 1);
    memcpy(func_name, function_section, name_length);
    func_name[name_length] = '\0'; // Null terminate
    printf("loading function: %s\n",func_name);
    function_section += name_length;

    // Create function entry
    FunctionEntry *func_entry = malloc(sizeof(FunctionEntry));
    if (!func_entry) {
      printf("Error: Failed to allocate memory for function entry.\n");
      free(func_name);
      return;
    }

    func_entry->name = strdup(func_name);
    func_entry->num_args = num_args;
    func_entry->local_count = local_count;

    // Store the current ip (start of function body)
    func_entry->func_body_address = (size_t)function_section;

    // Add to function table
    hashmap_set(vm->functions, func_name, func_entry, free);

    // Free the temporary function name
    free(func_name);

    uint8_t *function_end = function_section;

    while (function_end < section_end) {
      uint8_t opcode = *function_end;

      if (opcode == OP_ENDFUNC) {
        function_end++; // include the OP_ENDFUNC in the body
        break;
      }

      function_end++; // move past opcode

      // this switch statement is here to ensure we don't accidentally read the argument bytes as opcodes
      switch (opcode) {
        case INT:
        case FLOAT:
          function_end += 8; break;

        case BOOL:
          function_end += 1; break;

        case STR: {
          uint32_t len;
          memcpy(&len, function_end, 4);
          function_end += 4 + len;
          break;
        }

        case LOCAL:
            function_end += 2;
            break;

        case ID: {
          uint16_t len;
          memcpy(&len, function_end, 2);
          function_end += 2 + len;
          break;
        }

        case OP_JMP:
        case OP_JMPIF:
          function_end += 4; break;

        // any other opcodes are single byte
        default:
          break;
      }
    }
    function_section = function_end;
  }
}

/* runs the vm */
void run(VM *vm, const char *bytecode_file) {
  FILE *file = fopen(bytecode_file, "rb");
  if (!file) {
    printf("Error: Could not open bytecode file %s\n", bytecode_file);
    return;
  }

  // Read file contents into bytecode memory
  fseek(file, 0, SEEK_END);
  long file_size = ftell(file);
  rewind(file);

  uint8_t *bytecode = malloc(file_size);
  if (!bytecode) {
    printf("Error: Failed to allocate memory for bytecode.\n");
    fclose(file);
    return;
  }

  fread(bytecode, 1, file_size, file);
  fclose(file);

  vm->bytecode_ip = (uint64_t *)bytecode; // Cast to match existing struct, but
                                          // must careful with byte addressing

  // Read header (64 bytes)
  BytecodeHeader header;
  memcpy(&header, bytecode, sizeof(BytecodeHeader));

  /*printf("just checking if header has been read\n");*/

  load_functions(vm, bytecode, header.func_section_start,
                 header.func_section_end);

  // Set instruction pointer to start of executable code section
  vm->bytecode_ip = (uint64_t *)(bytecode + header.execution_section_start);

  while (1) {
    uint8_t instruction =
        *(uint8_t *)vm->bytecode_ip; // cast the read byte to a uint8 to match
                                     // the Opcodes enums
    vm->bytecode_ip =
        (uint64_t *)((uint8_t *)vm->bytecode_ip + 1); // move to next bytes

    switch (instruction) {
    case OP_HALT:
      printf("VM halted.\n");
      free(bytecode);
      return;

    case INT: { // [1 byte opcode][8 byte int64]
      int64_t value;
    //   PrimitiveObject *int_to_push;

      memcpy(&value, vm->bytecode_ip,
             sizeof(int64_t)); // Copy raw bytes into value
      vm->bytecode_ip = (uint64_t *)((uint8_t *)vm->bytecode_ip +
                                     sizeof(int64_t)); // Move past 8 bytes
    //   int_to_push = get_constant(vm, INT, value);
    //   if (!int_to_push)
    //     int_to_push = (PrimitiveObject *)new_int(vm, value);
      push(vm, new_int(vm, value), PRIMITIVE_OBJ);
      break;
    }

    case FLOAT: { // [1 byte opcode][8 byte double]
      double value;
      memcpy(&value, vm->bytecode_ip,
             sizeof(double)); // Copy raw bytes into value
      vm->bytecode_ip = (uint64_t *)((uint8_t *)vm->bytecode_ip +
                                     sizeof(double)); // Move past 8 bytes
      push(vm, new_float(value), PRIMITIVE_OBJ);
      break;
    }

    case BOOL: { // [1 byte opcode][1 byte int8]
      uint8_t bool_value;
      PrimitiveObject *bool_to_push;
      memcpy(&bool_value, vm->bytecode_ip, sizeof(uint8_t)); // Read single byte
      vm->bytecode_ip = (uint64_t *)((uint8_t *)vm->bytecode_ip +
                                     sizeof(uint8_t)); // Move past 1 byte
      push(vm, get_constant(vm, BOOL, bool_value), PRIMITIVE_OBJ);
      break;
    }

    case STR: { // [1 byte opcode][4 byte length][ length number of bytes]
      uint32_t length;
      memcpy(&length, vm->bytecode_ip,
             sizeof(uint32_t)); // Read 4 bytes as string length
      vm->bytecode_ip =
          (uint64_t *)((uint8_t *)vm->bytecode_ip +
                       sizeof(uint32_t)); // Move past length field

      char *str_value = malloc(length + 1);
      memcpy(str_value, vm->bytecode_ip, length); // Copy string data
      str_value[length] = '\0';                   // Null-terminate

      vm->bytecode_ip = (uint64_t *)((uint8_t *)vm->bytecode_ip +
                                     length); // Move past string bytes

      push(vm, new_str(str_value), PRIMITIVE_OBJ);
      free(str_value); // Free our temp variable
      break;
    }

    case ID: { // [1 byte opcode][2 byte ID length][ ID length number of bytes]
        uint16_t length;
        memcpy(&length, vm->bytecode_ip,
                sizeof(uint16_t)); // Read 2 bytes for ID length
        vm->bytecode_ip =
            (uint64_t *)((uint8_t *)vm->bytecode_ip +
                        sizeof(uint16_t)); // Move past length field

        char *identifier =
            malloc(length + 1); // we malloced here so we must free when we hit
                                // OP_GET_GLOBAL and OP_SET_GLOBAL
        memcpy(identifier, vm->bytecode_ip, length); // Copy ID string
        identifier[length] = '\0';                   // Null-terminate

        vm->bytecode_ip = (uint64_t *)((uint8_t *)vm->bytecode_ip +
                                        length); // Move past ID bytes

        push(vm, identifier, IDENTIFIER); // Push identifier as raw string
        break;
    }

    case _NULL_: {
        push(vm, get_constant(vm, _NULL_, 0), PRIMITIVE_OBJ);
        break;
    }

    case OP_ADD: { // modify this to first check the constant table before
                    // attempting to create a new int
        StackEntry b = pop(vm);
        StackEntry a = pop(vm);
        if (a.entry_type == PRIMITIVE_OBJ && b.entry_type == PRIMITIVE_OBJ) {
        PrimitiveObject *result =
            ((PrimitiveObject *)a.value)
                ->add(((PrimitiveObject *)a.value),
                        ((PrimitiveObject *)
                            b.value)); // cast back to original values
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
        PrimitiveObject *a_obj = (PrimitiveObject *)a.value;
        PrimitiveObject *b_obj = (PrimitiveObject *)b.value;

        // Check if b is of type INT or FLOAT
        if ((b_obj->type == TYPE_int || b_obj->type == TYPE_float)) {

            // Create a copy of b to negate
            PrimitiveObject *negated_b;

            if (b_obj->type == TYPE_int) {
            // Create a negated copy of the int
            int64_t negated_value = -(((int_Object *)b_obj)->value);
            negated_b = (PrimitiveObject *)new_int(vm, negated_value);
            } else { // TYPE_float
            // Create a negated copy of the float
            double negated_value = -(((float_Object *)b_obj)->value);
            negated_b = (PrimitiveObject *)new_float(negated_value);
            }

            /*printf("%ld\n", ((int_Object *)a_obj)->value);*/
            /*printf("%ld\n", ((int_Object *)negated_b)->value);*/
            // Now add a and negated_b
            PrimitiveObject *result = a_obj->add(a_obj, negated_b);
            /*printf("%ld\n", ((int_Object *)result)->value);*/
            /*printf("%d\n", result->type);*/
            push(vm, result, PRIMITIVE_OBJ);

            // If negated_b isn't in the constant pool, we should free it
            // This would require tracking if it came from the constant pool
        } else {
            printf("Error: Subtraction only supported between numeric types.\n");
        }
        } else {
        printf("Error: Invalid types for SUB operation.\n");
        }
        break;
    }
    case OP_MUL: { // modify this to first check constant table
        StackEntry b = pop(vm);
        StackEntry a = pop(vm);
        if (a.entry_type == PRIMITIVE_OBJ && b.entry_type == PRIMITIVE_OBJ) {
        PrimitiveObject *result =
            ((PrimitiveObject *)a.value)
                ->mul(((PrimitiveObject *)a.value),
                        ((PrimitiveObject *)
                            b.value)); // cast back to original values
        push(vm, result, PRIMITIVE_OBJ);
        } else {
        printf("Error: Invalid types for MUL operation.\n"); // just disallowing
                                                                // other types of
                                                                // multiplication
                                                                // first but it can
                                                                // be implemented
        }
        break;
    }

    case OP_DIV: { // modify this to first check the constant table
        StackEntry b = pop(vm);
        StackEntry a = pop(vm);
        if (a.entry_type == PRIMITIVE_OBJ && b.entry_type == PRIMITIVE_OBJ) {
        PrimitiveObject *result =
            ((PrimitiveObject *)a.value)
                ->div(((PrimitiveObject *)a.value),
                        ((PrimitiveObject *)
                            b.value)); // cast back to original values
        push(vm, result, PRIMITIVE_OBJ);
        } else {
        printf("Error: Invalid types for MUL operation.\n"); // just disallowing
                                                                // other types of
                                                                // multiplication
                                                                // first but it can
                                                                // be implemented
        }
        break;
    }

    case OP_POP:
        // printf("executing OP_POP\n");
        pop(vm);
        break;

    /* Handle all operations at once */
    case OP_EQ:
    case OP_NEQ:
    case OP_GT:
    case OP_GEQ:
    case OP_LT:
    case OP_LEQ: {
        StackEntry b = pop(vm);
        StackEntry a = pop(vm);
        
        if (a.entry_type == PRIMITIVE_OBJ && b.entry_type == PRIMITIVE_OBJ) {
            PrimitiveObject *a_obj = (PrimitiveObject *)a.value;
            PrimitiveObject *b_obj = (PrimitiveObject *)b.value;
            int result = 0;
            
            switch (instruction) {
                case OP_EQ:  
                    // printf("comparing eq\n");
                    result = a_obj->eq(a_obj, b_obj);  
                    break;
                case OP_NEQ: 
                    // printf("comparing neq\n");
                    result = a_obj->neq(a_obj, b_obj); 
                    break;
                case OP_GT:  
                    // printf("comparing gt\n");
                    result = a_obj->gt(a_obj, b_obj);  
                    break;
                case OP_GEQ: 
                    // printf("comparing geq\n");
                    result = a_obj->geq(a_obj, b_obj); 
                    break;
                case OP_LT: 
                    // printf("comparing lt\n");
                    result = a_obj->lt(a_obj, b_obj);  
                    break;
                case OP_LEQ:
                    // printf("comparing leq\n");
                    result = a_obj->leq(a_obj, b_obj); 
                    break;
            }
            // printf("result: %d \n", result);
            push(vm, get_constant(vm, BOOL, result), PRIMITIVE_OBJ);
        // We can add inother else ifs for advanced primitive object types
        } else {
            printf("Error: Comparison not implemented for non PRIMITIVE_OBJ types.\n");
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
    OP_SET_GLOBAL -> (pops y, pops 4, sets y to 4 in GT) -> stack_bottom []
    stack_top ID y -> (pushed onto stack) -> stack_bottom [y] stack_top
    OP_GET_GLOBAL -> (pops y, gets y from GT and pushed onto stack) -> stack_bottom
    [4] stack_top ID x (pushed onto stack) -> stack_bottom [4, x] stack_top
    OP_SET_GLOBAL -> (pops x, pops 4, sets x to 4 in GT) -> stack_bottom []
    stack_top
    */
    case OP_GET_GLOBAL: {
        // printf("popping from global\n");
      StackEntry id = pop(vm);

      if (id.entry_type != IDENTIFIER) {
        printf("Error: Expected IDENTIFIER for global name.\n");
        break;
      }

      char *var_name = (char *)id.value;
      GlobalEntry *entry = (GlobalEntry *)hashmap_get(vm->globals, var_name);

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
    OP_SET_GLOBAL -> (pops x, pops 4, sets x to 4 in GT) -> stack_bottom []
    stack_top
    */
    case OP_SET_GLOBAL: {
    //   printf("popping id\n");
      StackEntry id = pop(vm);
    //   printf("popping value\n");
      StackEntry value = pop(vm);

      if (id.entry_type != IDENTIFIER) {
        printf("Error: Expected IDENTIFIER for global name.\n");
        break;
      }

      char *var_name = (char *)id.value;

      GlobalEntry *entry = malloc(sizeof(GlobalEntry));
      entry->value = value.value;
      entry->entry_type = value.entry_type;

      /*
      Keep in mind we still have a potential memory leak here as we do not
      garbage collect the values of the globalEntry (We are intentionally not
      freeing the values here as there may be multiple references to them in
      stackframes and vars so this a job for the GC) (However we can safely free
      GlobalEntry as it is simply a wrapper)
      */
      hashmap_set(vm->globals, var_name, entry,
                  free); // free is added here as we have to free the previous
                         // globalEntry when we reassign a variable

      free(var_name); // was malloc'ed during ID opcode
      break;
    }

    case OP_JMP: { //[1 byte opcode][4 byte signed offset]
      int32_t offset;
      memcpy(&offset, vm->bytecode_ip,
             sizeof(int32_t)); // Read 4 bytes as a signed offset
      vm->bytecode_ip =
          (uint64_t *)((uint8_t *)vm->bytecode_ip +
                       sizeof(int32_t)); // Move past the offset bytes

      // Apply jump
      vm->bytecode_ip = (uint64_t *)((uint8_t *)vm->bytecode_ip + offset);
      break;
    }

    case OP_JMPIF: {
      int32_t offset;
      memcpy(&offset, vm->bytecode_ip, sizeof(int32_t)); // Read 4-byte offset
      vm->bytecode_ip = (uint64_t *)((uint8_t *)vm->bytecode_ip +
                                     sizeof(int32_t)); // Move past offset

      StackEntry condition = pop(vm);
      if (condition.entry_type != PRIMITIVE_OBJ) {
        printf("Error: Expected PRIMITIVE_OBJ for conditional jump.\n");
        break;
      }

      if (!is_truthy((PrimitiveObject *)condition.value)) {
        vm->bytecode_ip =
            (uint64_t *)((uint8_t *)vm->bytecode_ip + offset); // Apply jump
      }
      break;
    }

    case OP_GET_LOCAL: { // [1 byte opcode]
      StackEntry local_id = pop(vm);

      if (local_id.entry_type != IDENTIFIER) {
        printf("Error: Expected IDENTIFIER for local variable access.\n");
        break;
      }

      uint16_t index = (uint16_t)(uintptr_t)local_id.value;
      localEntry local = get_local(vm, index);
      if (local.value != NULL) {
        push(vm, local.value, local.entry_type);
      } else {
        printf("Error: Failed to get local variable at index %d.\n", index);
        free(bytecode);
        return;
      }
      break;
    }

    case OP_SET_LOCAL: { // [1 byte opcode]
      StackEntry local_id = pop(vm);

      if (local_id.entry_type != IDENTIFIER) {
        printf("Error: Expected IDENTIFIER for local variable assignment.\n");
        free(bytecode);
        return;
      }
      uint16_t index = (uint16_t)(uintptr_t)local_id.value;

      StackEntry value = pop(vm);
      set_local(vm, index, value);
      break;
    }

    case LOCAL: { // [1 byte opcode][2 byte local index]
      uint16_t index;
      memcpy(&index, vm->bytecode_ip,
             sizeof(uint16_t)); // Read 2 bytes for local index
      vm->bytecode_ip =
          (uint64_t *)((uint8_t *)vm->bytecode_ip +
                       sizeof(uint16_t)); // Move past the index bytes

      // Push the local index onto the stack (similar to how ID works)
      push(vm, (void *)(uintptr_t)index,
           IDENTIFIER); // Store the index directly
      break;
    }

    case OP_CALL: {
      // Pop the function identifier from the stack
      StackEntry func_id = pop(vm);

      if (func_id.entry_type != IDENTIFIER) {
        printf("Error: Expected function identifier for CALL operation.\n");
        break;
      }

      char *func_name = (char *)func_id.value;
    //   printf("Calling func: %s\n", func_name);
      FunctionEntry *func =
          (FunctionEntry *)hashmap_get(vm->functions, func_name);

      if (!func) {
        printf("Error: Undefined function '%s'.\n", func_name);
        free(func_name);
        free(bytecode);
        return;
      }

      // Pop and assign arguments from the stack (in reverse order)
      StackEntry args[func->num_args];
      for (int i = func->num_args - 1; i >= 0; i--) {
        // printf("popping arg: %d\n", i);
        args[i] = pop(vm);
      }

      // Save current instruction pointer for return
      uint64_t *return_address = vm->bytecode_ip;

      // Create a new stack frame
      StackFrame *frame =
          init_stack_frame(vm, return_address, func->local_count);
      if (!frame) {
        printf("Error: Failed to create stack frame for function call.\n");
        free(func_name);
        break;
      }

      // Save current stack position as the new base pointer
      size_t new_base_pointer = vm->stack.stack_top;

      // Push the frame onto the stack
      push(vm, frame, FUNCTION_FRAME);

      // Update the base pointer to the new stack frame
      vm->stack.base_pointer = new_base_pointer;

      // Set the arguments as local variables in the stack frame.
      for (int i = 0; i < func->num_args; i++) {
        set_local(vm, i, args[i]);
      }

      // Jump to function body
      vm->bytecode_ip = (uint64_t *)func->func_body_address;
      free(func_name);
      break;
    }

    case OP_RETURN: {
      return_from_frame(vm);
      break;
    }

    default:
      printf("Unknown instruction: 0x%02X\n", instruction);
      exit(EXIT_FAILURE);
      break;
    }
  }

  free(bytecode); // Clean up allocated memory
}

/* ///////////////////////// VM FUNCTIONS ///////////////////////// */

/* ///////////////////////// STACK ///////////////////////// */

/* pushes a StackEntry onto stack */
void push(VM *vm, void *value, StackEntryType type) {
  Stack *stack = &vm->stack; // Use a pointer to modify the actual stack in VM
  StackEntry entry;
//   printf("pushing: %d\n", ((PrimitiveObject *) entry.value)->type);
  entry.value = value;
  entry.entry_type = type;

  if (stack->stack_top < STACK_MAX) { // Check stack limit
    stack->stack[stack->stack_top] = entry;
    /*printf("Stack top: %ld\n", stack->stack_top);*/
        stack->stack_top++;
  } else {
    printf("Stack overflow error.\n");
    exit(EXIT_FAILURE);
  }
}

/* Pops a StackEntry from stack */
StackEntry pop(VM *vm) {
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
