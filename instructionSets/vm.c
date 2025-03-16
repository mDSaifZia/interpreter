#include "vm.h"
#include <stdio.h>
#include <string.h>

/* Initialize VM */
void initVM(VM* vm, uint8_t* bytecode) {
    vm->stackTop = 0;
    vm->constantCount = 0;
    vm->globalCount = 0;
    vm->functionCount = 0;
    vm->objectCount = 0;
    vm->bytecode = bytecode;
    vm->ip = 0;
}

/* Push onto stack */
void push(VM* vm, void* value, int isObject) {
    if (vm->stackTop < STACK_MAX) {
        vm->stack[vm->stackTop++] = value;
    } else {
        printf("Stack Overflow!\n");
    }
}

/* Pop from stack */
void* pop(VM* vm, int* isObject) {
    if (vm->stackTop > 0) {
        void* value = vm->stack[--vm->stackTop];
        *isObject = (value != NULL && ((Object*)value)->base.type != NULL);
        return value;
    } else {
        printf("Stack Underflow!\n");
        return NULL;
    }
}

/* Get a global variable (primitive or object) */
void* getGlobal(VM* vm, const char* name, int* isObject) {
    for (int i = 0; i < vm->globalCount; i++) {
        if (strcmp(vm->globals[i].name, name) == 0) {
            *isObject = vm->globals[i].isObject;
            return vm->globals[i].value;
        }
    }
    printf("Error: Undefined global variable '%s'\n", name);
    return NULL;
}

/* Set a global variable (primitive or object) */
void setGlobal(VM* vm, const char* name, void* value, int isObject) {
    for (int i = 0; i < vm->globalCount; i++) {
        if (strcmp(vm->globals[i].name, name) == 0) {
            vm->globals[i].value = value;
            vm->globals[i].isObject = isObject;
            return;
        }
    }
    if (vm->globalCount < MAX_GLOBALS) {
        vm->globals[vm->globalCount++] = (GlobalEntry){strdup(name), value, isObject};
    } else {
        printf("Error: Global table full!\n");
    }
}

/* Find a function's bytecode offset */
size_t getFunctionOffset(VM* vm, const char* name) {
    for (int i = 0; i < vm->functionCount; i++) {
        if (strcmp(vm->functions[i].name, name) == 0) {
            return vm->functions[i].bytecode_offset;
        }
    }
    printf("Error: Undefined function '%s'\n", name);
    return 0;
}

/* Execute Bytecode */
void run(VM* vm) {
    while (1) {
        uint8_t instruction = vm->bytecode[vm->ip++];

        switch (instruction) {
            case OP_CONSTANT: {
                uint8_t constantIndex = vm->bytecode[vm->ip++];
                push(vm, vm->constants[constantIndex].value, 0);  // 0 for primitive
                break;
            }
            case OP_ADD: {
                int isObjA, isObjB;
                PrimitiveObject* b = (PrimitiveObject*)pop(vm, &isObjB);
                PrimitiveObject* a = (PrimitiveObject*)pop(vm, &isObjA);
                if (!isObjA && !isObjB) { // disallowing adding objects for now (may implement a dunder method that all objects have to do so later)
                    push(vm, a->add(a, b), 0);
                } else {
                    printf("Error: Cannot add objects\n");
                }
                break;
            }
            case OP_ASSIGN: {
                char* varName = (char*)&vm->bytecode[vm->ip++];
                int isObj;
                void* value = pop(vm, &isObj);
                setGlobal(vm, varName, value, isObj);
                break;
            }
            case OP_GET_GLOBAL: {
                char* varName = (char*)&vm->bytecode[vm->ip++];
                int isObj;
                void* value = getGlobal(vm, varName, &isObj);
                push(vm, value, isObj);
                break;
            }
            case OP_CALL: {
                char* funcName = (char*)&vm->bytecode[vm->ip++];
                size_t funcOffset = getFunctionOffset(vm, funcName);
                if (funcOffset) {
                    push(vm, (void*)(uintptr_t)vm->ip, 0);  // Save return address
                    vm->ip = funcOffset;  // Jump to function start
                }
                break;
            }
            case OP_RETURN: {
                int isObj;
                void* returnValue = pop(vm, &isObj);
                vm->ip = (size_t)(uintptr_t)pop(vm, &isObj);  // Restore return address
                push(vm, returnValue, isObj);  // Push return value to caller
                break;
            }
            case OP_HALT:
                return;
        }
    }
}

/* Free VM */
void freeVM(VM* vm) {
    // Free constant table primitives
    for (int i = 0; i < vm->constantCount; i++) {
        free_primitive(vm->constants[i].value);
    }

    // Free global variables (primitives and objects)
    for (int i = 0; i < vm->globalCount; i++) {
        free(vm->globals[i].name);  // Free variable name
        if (vm->globals[i].isObject) {
            Object_destroy((Object*)vm->globals[i].value);  // Properly destroy objects
        } else {
            free_primitive((PrimitiveObject*)vm->globals[i].value); // properly free primitives
        }
    }

    vm->stackTop = 0;
    vm->constantCount = 0;
    vm->globalCount = 0;
    vm->functionCount = 0;
    vm->objectCount = 0;
}
