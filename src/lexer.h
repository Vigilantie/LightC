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

#pragma once

#include "stdio.h"

typedef enum
{
  TOKEN_KEYWORD, // print
  TOKEN_STRING,  // "Hello"
  TOKEN_INT,     // 9
  TOKEN_FLOAT,   // 3.14
  TOKEN_BOOL,    // true/false
  TOKEN_OP,      // + - * /
  TOKEN_ASSIGN,
  TOKEN_IF,
  TOKEN_ELSE,
  TOKEN_WHILE,
  TOKEN_FOR,
  TOKEN_FUNC,
  TOKEN_RETURN,
  TOKEN_INPUT,
  TOKEN_END,        // end
  TOKEN_SINGLE_OP,  // +=, -=, etc.
  TOKEN_COMPARE_OP, // ==, !=, <=, >=, &&, ||

  TOKEN_VAR_BOOL,
  TOKEN_VAR_FLOAT,
  TOKEN_VAR_STRING,
  TOKEN_VAR_INT,
  TOKEN_C,
  TOKEN_EXPOSE_C,
  TOKEN_COMMIT
} TokenType;

typedef struct
{
  TokenType type;
  char value[512]; // the actual text, or a complete line for raw C
  int line;
} Token;

int lexer(FILE *in, Token **tokens, int *capacity);
void assignVarTypes(Token **tokens, int count);
void generate(Token *tokens, int count, FILE *out);
