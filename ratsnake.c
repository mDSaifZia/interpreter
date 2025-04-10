#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "vm/vm.h"

int compile_ir(const char *input_path, const char *output_path); // forward declaration

int main(int argc, char const *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <source_file.rs>\n", argv[0]);
        return 1;
    }

    const char *source_file = argv[1];

    // run frontend manager
    char command[512];
    snprintf(command, sizeof(command), "python FrontEndParts/frontend_manager.py -i %s", source_file);
    int result = system(command);
    if (result != 0) {
        fprintf(stderr, "Error: Failed to generate IR from source file.\n");
        return 1;
    }

    // Get the file names of the bytecode file
    char *bytecode_file = malloc(strlen(source_file) + 10); // .bytecode + null/padding
    if (!bytecode_file) {
        perror("malloc failed");
        return 1;
    }
    strcpy(bytecode_file, source_file);
    char *dot = strrchr(bytecode_file, '.');
    if (dot) strcpy(dot, ".bytecode");
    else strcat(bytecode_file, ".bytecode");

    char *output_bin = malloc(strlen(source_file) + 5); // .bin + null/padding
    if (!output_bin) {
        perror("malloc failed");
        free(bytecode_file);
        return 1;
    }
    strcpy(output_bin, source_file);
    dot = strrchr(output_bin, '.');
    if (dot) strcpy(dot, ".bin");
    else strcat(output_bin, ".bin");

    // Step 3: Compile .bytecode to binary .bin for vm to run
    if (compile_ir(bytecode_file, output_bin) != 0) {
        fprintf(stderr, "IR Compilation failed.\n");
        free(bytecode_file);
        free(output_bin);
        return 1;
    }

    // Step 4: Run the VM on the binary
    VM *vm = initVM();
    run(vm, output_bin);

    // Step 5: we could clean up the bytecode and binary but i'm lazy

    free(bytecode_file);
    free(output_bin);
    return 0;
}

