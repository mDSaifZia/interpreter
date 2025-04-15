#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "vm/vm.h"

// for windows and linux and mac
ssize_t portable_getline(char **lineptr, size_t *n, FILE *stream) {
    if (!lineptr || !n || !stream) return -1;

    if (*lineptr == NULL || *n == 0) {
        *n = 128;
        *lineptr = malloc(*n);
        if (!*lineptr) return -1;
    }

    size_t i = 0;
    int c;

    while ((c = fgetc(stream)) != EOF) {
        if (i + 1 >= *n) {
            size_t new_size = *n * 2;
            char *new_ptr = realloc(*lineptr, new_size);
            if (!new_ptr) return -1;
            *lineptr = new_ptr;
            *n = new_size;
        }

        (*lineptr)[i++] = (char)c;
        if (c == '\n') break;
    }

    if (i == 0 && c == EOF) return -1;

    (*lineptr)[i] = '\0';
    return (ssize_t)i;
}

// Write helpers
void write_uint8(FILE *f, uint8_t val) { fwrite(&val, 1, 1, f); }
void write_uint16(FILE *f, uint16_t val) { fwrite(&val, 2, 1, f); }
void write_int32(FILE *f, int32_t val) { fwrite(&val, 4, 1, f); }
void write_int64(FILE *f, int64_t val) { fwrite(&val, 8, 1, f); }
void write_double(FILE *f, double val) { fwrite(&val, 8, 1, f); }

int map_opcode(const char *token) {
    #define MATCH(x) if (strcmp(token, x) == 0) return x
    if (strcmp(token, "__NULL__") == 0) return _NULL_;
    if (strcmp(token, "OP_ADD") == 0) return OP_ADD;
    if (strcmp(token, "OP_SUB") == 0) return OP_SUB;
    if (strcmp(token, "OP_MUL") == 0) return OP_MUL;
    if (strcmp(token, "OP_DIV") == 0) return OP_DIV;
    if (strcmp(token, "OP_GET_GLOBAL") == 0) return OP_GET_GLOBAL;
    if (strcmp(token, "OP_SET_GLOBAL") == 0) return OP_SET_GLOBAL;
    if (strcmp(token, "OP_CALL") == 0) return OP_CALL;
    if (strcmp(token, "OP_RETURN") == 0) return OP_RETURN;
    if (strcmp(token, "OP_HALT") == 0) return OP_HALT;
    // if (strcmp(token, "OP_JMP") == 0) return OP_JMP;
    // if (strcmp(token, "OP_JMPIF") == 0) return OP_JMPIF;
    if (strcmp(token, "OP_FUNCDEF") == 0) return OP_FUNCDEF;
    if (strcmp(token, "OP_ENDFUNC") == 0) return OP_ENDFUNC;
    if (strcmp(token, "OP_CLASSDEF") == 0) return OP_CLASSDEF;
    if (strcmp(token, "OP_ENDCLASS") == 0) return OP_ENDCLASS;
    if (strcmp(token, "OP_BLSHIFT") == 0) return OP_BLSHIFT;
    if (strcmp(token, "OP_BRSHIFT") == 0) return OP_BRSHIFT;
    if (strcmp(token, "OP_BXOR") == 0) return OP_BXOR;
    if (strcmp(token, "OP_BOR") == 0) return OP_BOR;
    if (strcmp(token, "OP_BAND") == 0) return OP_BAND;
    if (strcmp(token, "OP_GET_LOCAL") == 0) return OP_GET_LOCAL;
    if (strcmp(token, "OP_SET_LOCAL") == 0) return OP_SET_LOCAL;
    if (strcmp(token, "OP_PRINT") == 0) return OP_PRINT;
    if (strcmp(token, "OP_INPUT") == 0) return OP_INPUT;
    if (strcmp(token, "OP_POP") == 0) return OP_POP;
    if (strcmp(token, "OP_MOD") == 0) return OP_MOD;
    if (strcmp(token, "OP_EQ") == 0) return OP_EQ;
    if (strcmp(token, "OP_NEQ") == 0) return OP_NEQ;
    if (strcmp(token, "OP_GT") == 0) return OP_GT;
    if (strcmp(token, "OP_GEQ") == 0) return OP_GEQ;
    if (strcmp(token, "OP_LT") == 0) return OP_LT;
    if (strcmp(token, "OP_LEQ") == 0) return OP_LEQ;
    if (strcmp(token, "OP_LOGICAL_AND") == 0) return OP_LOGICAL_AND;
    if (strcmp(token, "OP_LOGICAL_OR") == 0) return OP_LOGICAL_OR;
    if (strcmp(token, "OP_LOGICAL_NOT") == 0) return OP_LOGICAL_NOT;
    return -1;
}

// void write_id_or_str(FILE *out, const char *token, const char *len_str, const char *val) {
//     uint16_t len = (uint16_t)atoi(len_str);
//     if (strcmp(token, "STR") == 0) {
//         write_uint8(out, STR);
//         uint32_t len32 = (uint32_t)len;
//         // if (len32 == 0) {
//         //     printf(" writing 0 length string\n");
//         // }
//         fwrite(&len32, sizeof(uint32_t), 1, out);
//         fwrite(val, sizeof(char), len, out);
//     } else {
//         write_uint8(out, ID); // for both ID and IDFUNC
//         write_uint16(out, len);
//         fwrite(val, sizeof(char), len, out);
//     }
// }

#define GREEN "\033[0;32m"
#define WHITE "\033[0m"

int compile_ir(const char *input_path, const char *output_path) {
    FILE *in = fopen(input_path, "rb");
    if (!in) {
        perror("Failed to open input");
        return 1;
    }

    FILE *out = fopen(output_path, "wb+");
    if (!out) {
        perror("Failed to open output");
        fclose(in);
        return 1;
    }

    // printf("Opened input: %s\n", input_path);
    // printf("Opened output: %s\n", output_path);

    // Write placeholder header
    BytecodeHeader hdr = {0};
    fwrite(&hdr, sizeof(hdr), 1, out);

    long byte_offset = sizeof(BytecodeHeader); // 64
    hdr.execution_section_start = (uint32_t)byte_offset;

    int64_t func_start = 0;
    int64_t func_end = 0;

    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int lineno = 1;

    while ((read = portable_getline(&line, &len, in)) != -1) {
        char *token = strtok(line, " \t\r\n");

        if (!token || token[0] == '#') {
            lineno++;
            continue;
        }

        // printf(GREEN "[Line %d] Token: %s\n" WHITE, lineno, token);

        if (strcmp(token, "INT") == 0) {
            char *arg = strtok(NULL, " \t\r\n");
            int64_t val = atoll(arg);
            // printf("  INT: %s %ld\n", token, val);
            write_uint8(out, INT); byte_offset += 1;
            write_int64(out, val);  byte_offset += 8;

        } else if (strcmp(token, "FLOAT") == 0) {
            char *arg = strtok(NULL, " \t\r\n");
            double val = atof(arg);
            // printf("  FLOAT: %s %.8f\n", token, val);
            write_uint8(out, FLOAT); byte_offset += 1;
            write_double(out, val);  byte_offset += 8;

        } else if (strcmp(token, "BOOL") == 0) {
            char *arg = strtok(NULL, " \t\r\n");
            int val = atoi(arg);
            // printf("  BOOL: %s %d\n", token, val);
            write_uint8(out, BOOL); byte_offset += 1;
            write_uint8(out, val);  byte_offset += 1;

        } else if (strcmp(token, "STR") == 0 || strcmp(token, "ID") == 0 || strcmp(token, "IDFUNC") == 0) {
            char *len_str = strtok(NULL, " \t\r\n");
            char *val = strtok(NULL, "\n");

            if (!len_str) {
                fprintf(stderr, "  Error: Missing length or value on line %d\n", lineno);
                exit(EXIT_FAILURE);
            } else {
                // printf("  %s len=%s val=%s\n", token, len_str, val);
                uint16_t len16 = atoi(len_str);
                if (strcmp(token, "STR") == 0) {
                    write_uint8(out, STR);              byte_offset += 1;
                    uint32_t len32 = len16;
                    fwrite(&len32, sizeof(uint32_t), 1, out); byte_offset += 4;
                    fwrite(val, sizeof(char), len16, out);   byte_offset += len16;

                } else {
                    write_uint8(out, ID);               byte_offset += 1;
                    fwrite(&len16, sizeof(uint16_t), 1, out); byte_offset += 2;
                    fwrite(val, sizeof(char), len16, out);   byte_offset += len16;

                }
            }
        } else if (strcmp(token, "LOCAL") == 0) {
            char *arg = strtok(NULL, " \t\r\n");
            uint16_t idx = atoi(arg);
            // printf("  LOCAL idx: %d\n", idx);
            write_uint8(out, LOCAL); byte_offset += 1;
            write_uint16(out, idx);  byte_offset += 2;

        } else if (strcmp(token, "OP_JMP") == 0 || strcmp(token, "OP_JMPIF") == 0) {
            char *arg = strtok(NULL, " \t\r\n");
            int32_t offset = atoi(arg);
            // printf("  %s offset: %d\n", token, offset);
            // printf("OP_JMP: %x, OP_JMPIF: %x\n",OP_JMP,OP_JMPIF);
            write_uint8(out, strcmp(token, "OP_JMP") == 0 ? OP_JMP : OP_JMPIF); byte_offset += 1;
            write_int32(out, offset); byte_offset += 4;

        } else if (strcmp(token, "NUMARGS") == 0 || strcmp(token, "NUMVARS") == 0) {
            char *arg = strtok(NULL, " \t\r\n");
            uint16_t count = atoi(arg);
            // printf("  %s count: %d\n", token, count);
            write_uint16(out, count); byte_offset += 2;
            
        } else {
            int op = map_opcode(token);
            if (op != -1) {
                if (strcmp(token, "OP_FUNCDEF") == 0 && func_start == 0) {
                    func_start = byte_offset;
                }
                if (strcmp(token, "OP_ENDFUNC") == 0) {
                    func_end = byte_offset+1;
                }
                // printf("  Writing opcode: %s (%d)\n", token, op);
                write_uint8(out, op); byte_offset += 1;
            } else {
                fprintf(stderr, "Unknown token on line %d: %s\n", lineno, token);
                free(line);
                fclose(in);
                fclose(out);
                return 1;
            }
        }

        // printf(" Byte offset: %ld\n", byte_offset);
        lineno++;
    }

    free(line);
    fclose(in);

    // Finish writing output body
    fflush(out);

    // Set header values
    hdr.func_section_start = (uint32_t)func_start;
    hdr.func_section_end = (uint32_t)func_end;
    hdr.class_section_start = 0;
    hdr.class_section_end = 0;

    // Patch the header at the beginning
    fseek(out, 0, SEEK_SET);
    fwrite(&hdr, sizeof(hdr), 1, out);
    fclose(out);

    // printf(GREEN "Patched header written:\n" WHITE);
    // printf("  execution_section_start = %lu\n", hdr.execution_section_start);
    // printf("  func_section_start      = %lu\n", hdr.func_section_start);
    // printf("  func_section_end        = %lu\n", hdr.func_section_end);
    // printf("  Compilation complete.\n");

    return 0;
}


