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

#include "lexer.h"
#include "syntax_words.h"
#include "util.h"

// std
// #include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int lineEquals(const char* line, const char* expected) {
    while (*line == ' ' || *line == '\t')
        line++;
    size_t length = strlen(line);
    while (length > 0 && (line[length - 1] == ' ' || line[length - 1] == '\t'))
        length--;

    return strlen(expected) == length && strncmp(line, expected, length) == 0;
}

int lexer(FILE* in, Token** tokens, int* capacity) {
    int count = 0;
    int lineNumber = 1;
    int inRawC = 0;

    char line[512];

    while (fgets(line, sizeof(line), in)) {
        int lineTokenStart = count;
        line[strcspn(line, "\r\n")] = '\0';

        if (inRawC) {
            if (lineEquals(line, KEYWORD_END)) {
                inRawC = 0;
                lineNumber++;
                continue;
            }
            if (count >= *capacity) {
                *capacity *= 2;
                *tokens = realloc(*tokens, *capacity * sizeof(Token));
            }
            strcpy((*tokens)[count].value, line);
            (*tokens)[count].type = TOKEN_C;
            (*tokens)[count].line = lineNumber;
            count++;
            lineNumber++;
            continue;
        }

        if (lineEquals(line, "\"C\"")) {
            if (count >= *capacity) {
                *capacity *= 2;
                *tokens = realloc(*tokens, *capacity * sizeof(Token));
            }
            strcpy((*tokens)[count].value, "\"C\"");
            (*tokens)[count].type = TOKEN_EXPOSE_C;
            (*tokens)[count].line = lineNumber;
            count++;
            inRawC = 1;
            lineNumber++;
            continue;
        }

        int i = 0;
        while (line[i] != '\0') {
            if (line[i] == ' ') {
                i++; // skip spaces
            } else if (line[i] == '"') {
                int j = 0;
                i++; // skip opening quote
                while (line[i] != '\0' && line[i] != '"') {
                    if (count >= *capacity) {
                        *capacity *= 2;
                        *tokens = realloc(*tokens, *capacity * sizeof(Token));
                    }
                    (*tokens)[count].value[j] = line[i];
                    i++;
                    j++;
                }
                i++; // skip closing quote
                (*tokens)[count].value[j] = '\0';
                (*tokens)[count].type = TOKEN_STRING;
                count++;
            } else if (line[i] >= '0' && line[i] <= '9') {
                if (count >= *capacity) {
                    *capacity *= 2;
                    *tokens = realloc(*tokens, *capacity * sizeof(Token));
                }
                int j = 0;
                while (line[i] >= '0' && line[i] <= '9') {
                    (*tokens)[count].value[j] = line[i];
                    i++;
                    j++;
                }
                if (line[i] == '.') {
                    (*tokens)[count].value[j] = line[i];
                    i++;
                    j++;
                    while (line[i] >= '0' && line[i] <= '9') {
                        (*tokens)[count].value[j] = line[i];
                        i++;
                        j++;
                    }
                    (*tokens)[count].value[j] = '\0';
                    (*tokens)[count].type = TOKEN_FLOAT;
                    count++;
                } else {
                    (*tokens)[count].value[j] = '\0';
                    (*tokens)[count].type = TOKEN_INT;
                    count++;
                }

            } else if (line[i] == '*' || line[i] == '+' || line[i] == '-' || line[i] == '/' ||
                       line[i] == '%' || line[i] == '=' || line[i] == '!' || line[i] == '<' ||
                       line[i] == '>' || line[i] == '&' || line[i] == '|') {

                // 1. Ensure capacity
                if (count >= *capacity) {
                    *capacity *= 2;
                    *tokens = realloc(*tokens, *capacity * sizeof(Token));
                }

                // 2. Check for double-character operators (Lookahead)
                // Ensure we don't read past the end of the string
                int is_double_op = 0;
                if (line[i + 1] != '\0') {
                    if ((line[i] == '+' && line[i + 1] == '+') ||
                        (line[i] == '-' && line[i + 1] == '-')) {

                        // Store the double operator (e.g., "++")
                        // NOTE: Ensure Token.value is at least size 3 to hold 2 chars + null
                        // terminator
                        (*tokens)[count].value[0] = line[i];
                        (*tokens)[count].value[1] = line[i + 1];
                        (*tokens)[count].value[2] = '\0';

                        (*tokens)[count].type = TOKEN_OP; // Multi-char operator
                        i += 2;                           // Skip both characters
                        is_double_op = 1;
                    }
                }

                if (line[i + 1] != '\0') {
                    if ((line[i] == '=' && line[i + 1] == '=') ||
                        (line[i] == '!' && line[i + 1] == '=') ||
                        (line[i] == '<' && line[i + 1] == '=') ||
                        (line[i] == '>' && line[i + 1] == '=') ||
                        (line[i] == '&' && line[i + 1] == '&') ||
                        (line[i] == '|' && line[i + 1] == '|')) {

                        // Store the double operator (e.g., "++")
                        // NOTE: Ensure Token.value is at least size 3 to hold 2 chars + null
                        // terminator
                        (*tokens)[count].value[0] = line[i];
                        (*tokens)[count].value[1] = line[i + 1];
                        (*tokens)[count].value[2] = '\0';

                        (*tokens)[count].type = TOKEN_COMPARE_OP; // Multi-char operator
                        i += 2;                                   // Skip both characters
                        is_double_op = 1;
                    }
                }

                // 3. If not a double operator, store as single
                if (!is_double_op) {
                    (*tokens)[count].value[0] = line[i];
                    (*tokens)[count].value[1] = '\0';
                    (*tokens)[count].type = TOKEN_SINGLE_OP;
                    i += 1; // Skip single character
                }

                // 4. Move to next token slot
                count++;
            } else if (line[i] == '(' || line[i] == ')' || line[i] == ',' || line[i] == '{' ||
                       line[i] == '}' || line[i] == ';') {
                if (count >= *capacity) {
                    *capacity *= 2;
                    *tokens = realloc(*tokens, *capacity * sizeof(Token));
                }
                (*tokens)[count].value[0] = line[i];
                (*tokens)[count].value[1] = '\0';
                (*tokens)[count].type = TOKEN_OP;
                i++;
                count++;
            } else if (strncmp(&line[i], KEYWORD_TRUE, strlen(KEYWORD_TRUE)) == 0) {
                if (count >= *capacity) {
                    *capacity *= 2;
                    *tokens = realloc(*tokens, *capacity * sizeof(Token));
                }
                strcpy((*tokens)[count].value, "true");
                (*tokens)[count].type = TOKEN_BOOL;
                i += strlen(KEYWORD_TRUE);
                count++;
            } else if (strncmp(&line[i], KEYWORD_FALSE, strlen(KEYWORD_FALSE)) == 0) {
                if (count >= *capacity) {
                    *capacity *= 2;
                    *tokens = realloc(*tokens, *capacity * sizeof(Token));
                }
                strcpy((*tokens)[count].value, "false");
                (*tokens)[count].type = TOKEN_BOOL;
                i += strlen(KEYWORD_FALSE);
                count++;
            } else if (strncmp(&line[i], KEYWORD_IF, strlen(KEYWORD_IF)) == 0) {
                if (count >= *capacity) {
                    *capacity *= 2;
                    *tokens = realloc(*tokens, *capacity * sizeof(Token));
                }
                strcpy((*tokens)[count].value, KEYWORD_IF);
                (*tokens)[count].type = TOKEN_IF;
                i += strlen(KEYWORD_IF);
                count++;
            } else if (strncmp(&line[i], KEYWORD_FUNC, strlen(KEYWORD_FUNC)) == 0) {
                if (count >= *capacity) {
                    *capacity *= 2;
                    *tokens = realloc(*tokens, *capacity * sizeof(Token));
                }
                strcpy((*tokens)[count].value, KEYWORD_FUNC);
                (*tokens)[count].type = TOKEN_FUNC;
                i += strlen(KEYWORD_FUNC);
                count++;
                // } else if (line[i] == '/' && line[i+1] == '/') {
                //     if (count >= *capacity) {
                //         *capacity *= 2;
                //         *tokens = realloc(*tokens, *capacity * sizeof(Token));
                //     }
                //     (*tokens)[count].value[0] = '/';
                //     (*tokens)[count].value[1] = '/';
                //     (*tokens)[count].value[2] = '\0';
                //     (*tokens)[count].type = TOKEN_COMMIT;
                //     i+=2;
            } else {
                if (count >= *capacity) {
                    *capacity *= 2;
                    *tokens = realloc(*tokens, *capacity * sizeof(Token));
                }
                int j = 0;
                while (line[i] != '\0' && line[i] != ' ' && line[i] != '(' && line[i] != ')' &&
                       line[i] != ',' && line[i] != '{' && line[i] != '}' && line[i] != ';' &&
                       line[i] != '*' && line[i] != '+' && line[i] != '-' && line[i] != '/' &&
                       line[i] != '%' && line[i] != '=' && line[i] != '!' && line[i] != '<' &&
                       line[i] != '>' && line[i] != '&' && line[i] != '|') {
                    (*tokens)[count].value[j] = line[i];
                    i++;
                    j++;
                }
                (*tokens)[count].value[j] = '\0'; // null-terminate the word
                (*tokens)[count].type = TOKEN_KEYWORD;
                count++;
            }
        }
        for (int token = lineTokenStart; token < count; token++) {
            (*tokens)[token].line = lineNumber;
        }
        lineNumber++;
    }
    if (inRawC) {
        fprintf(stderr, "Error: Raw C block is missing '%s'\n", KEYWORD_END);
    }
    return count;
}

void assignVarTypes(Token** tokens, int count) {
    typedef struct {
        char name[64];
        TokenType type;
    } VarInfo;

    VarInfo vars[64];
    int varCount = 0;

    int i = 0;
    while (i < count) {
        if ((*tokens)[i].type == TOKEN_FUNC) {
            while (i < count && strcmp((*tokens)[i].value, KEYWORD_END) != 0)
                i++;
            if (i < count)
                i++;
        } else if (strcmp((*tokens)[i].value, KEYWORD_VAR) == 0) {
            if (i + 3 >= count) {
                fprintf(stderr, "Error: Unexpected end of file after '%s'\n", KEYWORD_VAR);
                i++;
                continue;
            }

            int valueIndex = i + 2;
            if (strcmp((*tokens)[i + 2].value, "=") == 0 || (*tokens)[i + 2].type == TOKEN_ASSIGN) {
                if (i + 3 >= count) {
                    fprintf(stderr, "Error: Expected value after '='\n");
                    i++;
                    continue;
                }
                valueIndex = i + 3;
            }

            TokenType varType = TOKEN_KEYWORD;
            if ((*tokens)[valueIndex].type == TOKEN_INT) {
                varType = TOKEN_VAR_INT;
            } else if ((*tokens)[valueIndex].type == TOKEN_FLOAT) {
                varType = TOKEN_VAR_FLOAT;
            } else if ((*tokens)[valueIndex].type == TOKEN_BOOL) {
                varType = TOKEN_VAR_BOOL;
            } else if ((*tokens)[valueIndex].type == TOKEN_STRING) {
                varType = TOKEN_VAR_STRING;
            }

            if (varType == TOKEN_KEYWORD) {
                fprintf(stderr, "Warning: Unknown type for variable %s\n", (*tokens)[i + 1].value);
                i++;
                continue;
            }

            (*tokens)[i + 1].type = varType;
            if (varCount < 64) {
                strcpy(vars[varCount].name, (*tokens)[i + 1].value);
                vars[varCount].type = varType;
                varCount++;
            }

            i += (valueIndex - i) + 1;
        } else {
            i++;
        }
    }

    int inFunction = 0;
    for (int j = 0; j < count; j++) {
        if ((*tokens)[j].type == TOKEN_FUNC) {
            inFunction = 1;
            continue;
        }
        if (inFunction && strcmp((*tokens)[j].value, KEYWORD_END) == 0) {
            inFunction = 0;
            continue;
        }
        if (inFunction)
            continue;
        if ((*tokens)[j].type != TOKEN_KEYWORD) {
            continue;
        }

        for (int v = 0; v < varCount; v++) {
            if (strcmp((*tokens)[j].value, vars[v].name) == 0) {
                (*tokens)[j].type = vars[v].type;
                break;
            }
        }
    }
}
