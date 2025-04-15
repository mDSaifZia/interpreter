#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h> 
#include "vm/vm.h"

int compile_ir(const char *input_path, const char *output_path);

int main(int argc, char const *argv[]) {
    int keep_ir = 0;
    int keep_bin = 0;
    const char *source_file = NULL;
    char *bytecode_file = NULL;
    char *output_bin = NULL;
    VM *vm = NULL;

    if (argc < 2 || argc > 4) {
        fprintf(stderr, "Usage: %s [-keep_ir] [-keep_bin] <source_file.rtsk>\n", argv[0]);
        goto cleanup;
    }

    // Parse args
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-keep_ir") == 0) {
            keep_ir = 1;
        } else if (strcmp(argv[i], "-keep_bin") == 0) {
            keep_bin = 1;
        } else if (!source_file) {
            source_file = argv[i];
        } else {
            fprintf(stderr, "Error: Unrecognized or duplicate argument: %s\n", argv[i]);
            goto cleanup;
        }
    }

    if (!source_file) {
        fprintf(stderr, "Error: No source file provided.\n");
        goto cleanup;
    }

    const char *ext = strrchr(source_file, '.');
    if (!ext || strcmp(ext, ".rtsk") != 0) {
        fprintf(stderr, "Error: Provided source file is not a .rtsk file.\n");
        goto cleanup;
    }

    // Prep bytecode and bin paths (this will be wherever the user currently is)
    bytecode_file = malloc(strlen(source_file) + 10); // .bytecode
    output_bin    = malloc(strlen(source_file) + 10); // .rtskbin
    if (!bytecode_file || !output_bin) {
        perror("malloc failed");
        goto cleanup;
    }

    strcpy(bytecode_file, source_file);
    char *dot = strrchr(bytecode_file, '.');
    if (dot) strcpy(dot, ".bytecode");
    else strcat(bytecode_file, ".bytecode");

    strcpy(output_bin, source_file);
    dot = strrchr(output_bin, '.');
    if (dot) strcpy(dot, ".rtskbin");
    else strcat(output_bin, ".rtskbin");

    // Remove any outdated files from previous run
    remove(bytecode_file);
    remove(output_bin);

    // Construct absolute path to frontend_manager.py 
    char path_buffer[512];
    strncpy(path_buffer, argv[0], sizeof(path_buffer));
    path_buffer[sizeof(path_buffer)-1] = '\0';

    char *exec_dir = dirname(path_buffer);
    char python_command[1024];
    snprintf(python_command, sizeof(python_command),
             "python \"%s/FrontEndParts/frontend_manager.py\" -i \"%s\"",exec_dir, source_file);

    // Run the Python frontend
    int result = system(python_command);
    if (result != 0) {
        fprintf(stderr, "Error: Failed to generate IR from source file.\n");
        goto cleanup;
    }

    // Compile IR to binary
    if (compile_ir(bytecode_file, output_bin) != 0) {
        fprintf(stderr, "IR Compilation failed.\n");
        goto cleanup;
    }

    // Run VM
    vm = initVM();
    if (!vm) {
        fprintf(stderr, "VM initialization failed.\n");
        goto cleanup;
    }

    run(vm, output_bin);

cleanup:
    if (bytecode_file && !keep_ir) {
        remove(bytecode_file);
    }
    if (output_bin && !keep_bin) {
        remove(output_bin);
    }

    free(bytecode_file);
    free(output_bin);
    return 0;
}
