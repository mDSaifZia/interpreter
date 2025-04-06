#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "vm/vm.h"

int compile_ir(const char *input_path, const char *output_path); // From ir_compiler.c

int main(int argc, char const *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input.bytecode>\n", argv[0]);
        return 1;
    }

    const char *input_ir = argv[1];

    // Generate output .bin path by replacing extension
    char *output_bin = malloc(strlen(input_ir) + 5); // ".bin" + null
    if (!output_bin) {
        perror("Memory allocation failed");
        return 1;
    }

    strcpy(output_bin, input_ir);
    char *dot = strrchr(output_bin, '.');
    if (dot) strcpy(dot, ".bin");
    else strcat(output_bin, ".bin");

    // Compile IR to binary
    if (compile_ir(input_ir, output_bin) != 0) {
        printf("Compilation failed.\n");
        free(output_bin);
        return 1;
    }

    // Run compiled bytecode
    VM *vm = initVM();
    run(vm, output_bin);

    // just for testing
    char* var_name1 = "test";

    GlobalEntry* test = hashmap_get(vm->globals, var_name1);
    printf("%s = %ld\n", var_name1, ((int_Object*)test->value)->value);

    // char* var_name2 = "test2";
    // GlobalEntry* test2 = hashmap_get(vm->globals, var_name2);
    // printf("%s = %ld\n", var_name2, ((int_Object*)test2->value)->value);

    free(output_bin);
    return 0;
}

