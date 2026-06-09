/*
    LightC - the official compiler for the Light programming language

    Copyright (C) 2026  Jacob T. Ward

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/

// std
#include <ctype.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>

#include "compile.h"
#include "lexer.h"

#define NDEBUG 1

typedef enum { T_INT, T_FLOAT, T_STRING, T_BOOL } VarKind;
typedef enum { DOT_LIGHT, DOT_СВОД } FileExtention;

typedef struct VarNode {
    char name[64];
    VarKind kind;
    struct VarNode* next;
} VarNode;

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "");
    if (argc < 1) {
        fprintf(stderr, "Usage: lightc [options] <file.light/file.cвет>\nOr run lightc --help for "
                        "more info.\n");
        return 1;
    }

    char* input_file = NULL;
    char command_buffer[512] = "";

    // Find the input file and collect command options
    for (int i = 1; i < argc; i++) {
        size_t len = strlen(argv[i]);
        if (len > 6 && strcmp(argv[i] + len - 6, ".light") == 0) {
            input_file = argv[i];
        } else if (len > 8 && strcmp(argv[i] + len - 8, ".cвет") == 0) {
            input_file = argv[i];
        } else {
            // Collect as command option
            if (strlen(command_buffer) > 0) {
                strncat(command_buffer, " ", sizeof(command_buffer) - strlen(command_buffer) - 1);
            }
            strncat(command_buffer, argv[i], sizeof(command_buffer) - strlen(command_buffer) - 1);
        }
    }

    if (!input_file) {
        fprintf(stderr, "Error: No input file with .light or .cвет extension found.\n");
        return 1;
    }
    size_t len = strlen(input_file);
    int file_extention;
    if (strcmp(input_file + len - 6, ".light") == 0) {
        file_extention = DOT_LIGHT;
    } else if (strcmp(input_file + len - 8, ".cвет") == 0) {
        file_extention = DOT_СВОД;
    } else {
        fprintf(stderr, "Error: Input file must end with .light or .cвет\n");
        return 1;
    }

    int subtract_num_chars;
    if (file_extention == DOT_LIGHT)
        subtract_num_chars = 6;
    else
        subtract_num_chars = 8;

    char output_c[256], output_exe[256];
    strcpy(output_c, input_file);
    strcpy(output_c + len - subtract_num_chars, ".c");
    strcpy(output_exe, input_file);
    output_exe[len - subtract_num_chars] = 0;

    FILE* in = fopen(input_file, "r");
    FILE* out = fopen(output_c, "w");

    if (!in || !out) {
        fprintf(stderr, "Error opening files.\n");
        return 1;
    }

    int capacity = 10;
    Token* tokens = malloc(capacity * sizeof(Token));
    int count = lexer(in, &tokens, &capacity);

    assignVarTypes(&tokens, count);
    generate(tokens, count, out);
    free(tokens);

    fclose(in);
    fclose(out);

    char cmd[512];
    snprintf(cmd, sizeof(cmd), "clang %s %s -o %s", command_buffer, output_c, output_exe);
    printf("Compiling with command: %s\n", cmd);
    int result = system(cmd);

#ifndef NDEBUG
    char cleanup_cmd[512];
    snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm %s", output_c);
    system(cleanup_cmd);
#endif

    if (result == 0) {
        printf("Success! Run: ./%s\n", output_exe);
    } else {
        fprintf(stderr, "Clang build failed\n");
    }

    return 0;
}

//
