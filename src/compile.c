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

#include "compile.h"
#include "lexer.h"
#include "syntax_words.h"

#include <stdio.h>
#include <string.h>

typedef struct
{
  char name[64];
  TokenType type;
} LocalVariable;

static int isFunctionCall(Token *tokens, int i, int count)
{
  return i + 1 < count && tokens[i].type == TOKEN_KEYWORD &&
         strcmp(tokens[i + 1].value, "(") == 0;
}

static int generateCall(Token *tokens, int i, int count, FILE *out, int indent)
{
  if (indent)
  {
    fprintf(out, "    ");
  }
  fprintf(out, "%s(", tokens[i].value);
  i += 2;

  while (i < count && strcmp(tokens[i].value, ")") != 0)
  {
    if (tokens[i].type == TOKEN_STRING)
    {
      fprintf(out, "\"%s\"", tokens[i].value);
    }
    else
    {
      fprintf(out, "%s", tokens[i].value);
    }
    if (strcmp(tokens[i].value, ",") == 0)
    {
      fprintf(out, " ");
    }
    i++;
  }

  fprintf(out, ");\n");
  return i < count ? i + 1 : i;
}

static int parameterIndex(const char parameters[][64], int parameterCount, const char *name)
{
  for (int i = 0; i < parameterCount; i++)
  {
    if (strcmp(parameters[i], name) == 0)
    {
      return i;
    }
  }
  return -1;
}

static TokenType variableType(TokenType type)
{
  if (type == TOKEN_INT)
    return TOKEN_VAR_INT;
  if (type == TOKEN_FLOAT)
    return TOKEN_VAR_FLOAT;
  if (type == TOKEN_STRING)
    return TOKEN_VAR_STRING;
  if (type == TOKEN_BOOL)
    return TOKEN_VAR_BOOL;
  return type;
}

static const char *cType(TokenType type)
{
  if (type == TOKEN_VAR_FLOAT)
    return "double";
  if (type == TOKEN_VAR_STRING)
    return "char*";
  if (type == TOKEN_VAR_BOOL)
    return "bool";
  return "int";
}

static TokenType lookupLocalType(const LocalVariable locals[], int localCount,
                                 const char parameters[][64],
                                 const TokenType parameterTypes[],
                                 int parameterCount, const char *name)
{
  for (int i = localCount - 1; i >= 0; i--)
  {
    if (strcmp(locals[i].name, name) == 0)
    {
      return locals[i].type;
    }
  }
  int parameter = parameterIndex(parameters, parameterCount, name);
  return parameter >= 0 ? parameterTypes[parameter] : TOKEN_KEYWORD;
}

static TokenType inferExpressionType(Token *tokens, int start, int end,
                                     const LocalVariable locals[],
                                     int localCount,
                                     const char parameters[][64],
                                     const TokenType parameterTypes[],
                                     int parameterCount)
{
  TokenType result = TOKEN_KEYWORD;
  for (int i = start; i < end; i++)
  {
    TokenType type = variableType(tokens[i].type);
    if (type == TOKEN_KEYWORD)
    {
      type = lookupLocalType(locals, localCount, parameters, parameterTypes,
                             parameterCount, tokens[i].value);
    }
    if (type == TOKEN_VAR_STRING || type == TOKEN_VAR_BOOL)
    {
      return type;
    }
    if (type == TOKEN_VAR_FLOAT)
    {
      result = TOKEN_VAR_FLOAT;
    }
    if (type == TOKEN_VAR_INT && result == TOKEN_KEYWORD)
    {
      result = TOKEN_VAR_INT;
    }
  }
  return result == TOKEN_KEYWORD ? TOKEN_VAR_INT : result;
}

static void emitExpression(Token *tokens, int start, int end, FILE *out)
{
  for (int i = start; i < end; i++)
  {
    if (i > start && strcmp(tokens[i].value, ")") != 0 &&
        strcmp(tokens[i - 1].value, "(") != 0 &&
        strcmp(tokens[i].value, ",") != 0)
    {
      fprintf(out, " ");
    }
    if (tokens[i].type == TOKEN_STRING)
    {
      fprintf(out, "\"%s\"", tokens[i].value);
    }
    else
    {
      fprintf(out, "%s", tokens[i].value);
    }
    if (strcmp(tokens[i].value, ",") == 0)
    {
      fprintf(out, " ");
    }
  }
}

static void emitPrint(Token *tokens, int start, int end, TokenType type,
                      FILE *out)
{
  if (type == TOKEN_VAR_STRING)
  {
    fprintf(out, "    printf(\"%%s\\n\", ");
    emitExpression(tokens, start, end, out);
    fprintf(out, ");\n");
  }
  else if (type == TOKEN_VAR_FLOAT)
  {
    fprintf(out, "    printf(\"%%f\\n\", ");
    emitExpression(tokens, start, end, out);
    fprintf(out, ");\n");
  }
  else if (type == TOKEN_VAR_BOOL)
  {
    fprintf(out, "    printf(\"%%s\\n\", (");
    emitExpression(tokens, start, end, out);
    fprintf(out, ") ? \"true\" : \"false\");\n");
  }
  else
  {
    fprintf(out, "    printf(\"%%d\\n\", ");
    emitExpression(tokens, start, end, out);
    fprintf(out, ");\n");
  }
}

static void inferParameterTypes(Token *tokens, int count,
                                const char *functionName,
                                TokenType parameterTypes[], int parameterCount)
{
  for (int i = 0; i + 1 < count; i++)
  {
    if (strcmp(tokens[i].value, functionName) != 0 ||
        strcmp(tokens[i + 1].value, "(") != 0 ||
        (i > 0 && tokens[i - 1].type == TOKEN_FUNC))
    {
      continue;
    }

    int argument = 0;
    int depth = 1;
    for (int j = i + 2; j < count && depth > 0; j++)
    {
      if (strcmp(tokens[j].value, "(") == 0)
      {
        depth++;
      }
      else if (strcmp(tokens[j].value, ")") == 0)
      {
        depth--;
      }
      else if (depth == 1 && strcmp(tokens[j].value, ",") == 0)
      {
        argument++;
      }
      else if (depth == 1 && argument < parameterCount &&
               parameterTypes[argument] == TOKEN_KEYWORD)
      {
        TokenType inferred = variableType(tokens[j].type);
        if (inferred == TOKEN_VAR_INT || inferred == TOKEN_VAR_FLOAT ||
            inferred == TOKEN_VAR_STRING || inferred == TOKEN_VAR_BOOL)
        {
          parameterTypes[argument] = inferred;
        }
      }
    }
  }
}

void generateFunctions(Token *tokens, int count, FILE *out)
{
  for (int i = 0; i < count; i++)
  {
    if (tokens[i].type == TOKEN_FUNC)
    {
      if (i + 2 >= count || strcmp(tokens[i + 2].value, "(") != 0)
      {
        fprintf(stderr, "Error: Expected '(' after function name\n");
        continue;
      }

      char functionName[64];
      strcpy(functionName, tokens[i + 1].value);
      i += 3; // Skip 'func', function name, and '('

      char parameters[64][64];
      TokenType parameterTypes[64];
      int parameterCount = 0;
      while (i < count && strcmp(tokens[i].value, ")") != 0)
      {
        if (strcmp(tokens[i].value, ",") == 0)
        {
          i++;
          continue;
        }
        if (parameterCount < 64)
        {
          strcpy(parameters[parameterCount], tokens[i].value);
          parameterTypes[parameterCount++] = TOKEN_KEYWORD;
        }
        i++;
      }

      inferParameterTypes(tokens, count, functionName, parameterTypes,
                          parameterCount);
      fprintf(out, "void %s(", functionName);
      for (int parameter = 0; parameter < parameterCount; parameter++)
      {
        if (parameter > 0)
        {
          fprintf(out, ", ");
        }
        fprintf(out, "%s %s", cType(parameterTypes[parameter]),
                parameters[parameter]);
      }
      fprintf(out, ") {\n");
      i++; // Skip ')'

      LocalVariable locals[64];
      int localCount = 0;
      while (i < count && strcmp(tokens[i].value, KEYWORD_END) != 0)
      {
        int lineEnd = i + 1;
        while (lineEnd < count && tokens[lineEnd].line == tokens[i].line)
        {
          lineEnd++;
        }

        if (tokens[i].type == TOKEN_EXPOSE_C)
        {
          // The following TOKEN_C lines are emitted verbatim.
        }
        else if (tokens[i].type == TOKEN_C)
        {
          fprintf(out, "%s\n", tokens[i].value);
        }
        else if (strcmp(tokens[i].value, KEYWORD_PRINT) == 0)
        {
          if (i + 1 >= lineEnd)
          {
            fprintf(stderr, "Error: print statement missing argument\n");
          }
          else
          {
            TokenType type =
                inferExpressionType(tokens, i + 1, lineEnd, locals, localCount,
                                    parameters, parameterTypes, parameterCount);
            emitPrint(tokens, i + 1, lineEnd, type, out);
          }
        }
        else if (strcmp(tokens[i].value, KEYWORD_PRINTNL) == 0)
        {
          if (i + 1 >= lineEnd)
          {
            fprintf(stderr, "Error: print statement missing argument\n");
          }
          else
          {
            TokenType type =
                inferExpressionType(tokens, i + 1, lineEnd, locals, localCount,
                                    parameters, parameterTypes, parameterCount);
            emitPrint(tokens, i + 1, lineEnd, type, out);
          }
        }
        else if (strcmp(tokens[i].value, KEYWORD_VAR) == 0)
        {
          if (i + 3 >= lineEnd || strcmp(tokens[i + 2].value, "=") != 0)
          {
            fprintf(stderr, "Error: Expected 'let name = expression'\n");
          }
          else
          {
            TokenType type =
                inferExpressionType(tokens, i + 3, lineEnd, locals, localCount,
                                    parameters, parameterTypes, parameterCount);
            fprintf(out, "    %s %s = ", cType(type), tokens[i + 1].value);
            emitExpression(tokens, i + 3, lineEnd, out);
            fprintf(out, ";\n");
            if (localCount < 64)
            {
              strcpy(locals[localCount].name, tokens[i + 1].value);
              locals[localCount++].type = type;
            }
          }
        }
        else
        {
          fprintf(out, "    ");
          emitExpression(tokens, i, lineEnd, out);
          fprintf(out, ";\n");
        }
        i = lineEnd;
      }
      fprintf(out, "}\n");
    }
  }
}

void generate(Token *tokens, int count, FILE *out)
{
  // Add includes
  fprintf(out, "#include <stdio.h>\n#include <stdbool.h>\n\n");
  // Phase 1: Generate all functions first
  generateFunctions(tokens, count, out);

  // Phase 2: Generate main code, skipping function definitions
  fprintf(out, "\nint main() {\n"); // Start main function
  int i = 0;
  while (i < count)
  {
    if (tokens[i].type == TOKEN_FUNC)
    {
      // Skip entire function definition (already generated in Phase 1)
      i += 2; // Skip 'func' and function name
      while (i < count && strcmp(tokens[i].value, ")") != 0)
      {
        i++;
      }
      if (i < count)
      {
        i++; // Skip ')'
      }
      while (i < count && strcmp(tokens[i].value, KEYWORD_END) != 0)
      {
        i++; // Skip body
      }
      if (i < count)
      {
        i++; // Skip 'end'
      }
      continue;
    }
    // Existing logic for main code (if, while, print, var, etc.)
    if (tokens[i].type == TOKEN_EXPOSE_C)
    {
      i++;
    }
    else if (tokens[i].type == TOKEN_C)
    {
      fprintf(out, "%s\n", tokens[i].value);
      i++;
    }
    else if (strcmp(tokens[i].value, KEYWORD_IF) == 0)
    {
      fprintf(out, "if (");
      i++;
    }
    else if (strcmp(tokens[i].value, KEYWORD_ELSE) == 0)
    {
      fprintf(out, "} else {\n");
      i++;
    }
    else if (strcmp(tokens[i].value, KEYWORD_WHILE) == 0)
    {
      fprintf(out, "while (");
      i++;
      while (strcmp(tokens[i].value, ")"))
      {
        fprintf(out, "%s", tokens[i].value);
        i++;
      }
      fprintf(out, ") {\n");
      i++;

      // } else if (strcmp(tokens[i].value, KEYWORD_FOR) == 0) {
      //     fprintf(out, "for (int %s = 0; %s; %s++", tokens[i+1].value,
      //     tokens[i+2].value, tokens[i+1].value); i++;
    }
    else if (strcmp(tokens[i].value, KEYWORD_INPUT) == 0)
    {
      fprintf(out, "char *%s;\n", tokens[i + 2].value);
      fprintf(out, "printf(\"%s\");\n", tokens[i + 1].value);
      fprintf(out, "scanf(\"%%%s\", %s);\n",
              tokens[i + 2].type == TOKEN_VAR_INT     ? "d"
              : tokens[i + 1].type == TOKEN_VAR_FLOAT ? "f"
                                                      : "s",
              tokens[i + 2].value);
      i += 3;
    }
    else if (strcmp(tokens[i].value, KEYWORD_END) == 0)
    {
      fprintf(out, "}\n");
      i++;
    }
    else if (strcmp(tokens[i].value, KEYWORD_START) == 0)
    {
      fprintf(out, ") {\n");
      i++;
    }
    else if (isFunctionCall(tokens, i, count))
    {
      i = generateCall(tokens, i, count, out, 0);
    }
    else if (tokens[i].type == TOKEN_VAR_INT ||
             tokens[i].type == TOKEN_VAR_FLOAT ||
             tokens[i].type == TOKEN_VAR_STRING ||
             tokens[i].type == TOKEN_VAR_BOOL)
    {
      fprintf(out, "%s", tokens[i].value);
      i++;
    }
    else if (tokens[i].type == TOKEN_SINGLE_OP)
    {
      fprintf(out, " %s", tokens[i].value);
      i++;
    }
    else if (tokens[i].type == TOKEN_COMPARE_OP)
    {
      fprintf(out, " %s ", tokens[i].value);
      i++;
    }
    else if (tokens[i].type == TOKEN_OP)
    {
      fprintf(out, "%s;\n", tokens[i].value);
      i++;
    }
    else if (tokens[i].type == TOKEN_INT || tokens[i].type == TOKEN_FLOAT ||
             tokens[i].type == TOKEN_BOOL)
    {
      fprintf(out, "%s", tokens[i].value);
      i++;
    }
    else if (tokens[i].type == TOKEN_STRING)
    {
      fprintf(out, "\"%s\"", tokens[i].value);
      i++;
    }
    else if (strcmp(tokens[i].value, KEYWORD_PRINT) == 0)
    {
      if (i + 1 >= count)
      {
        fprintf(stderr, "Error: print statement missing argument\n");
        i++;
        continue;
      }
      if (tokens[i + 1].type == TOKEN_STRING)
      {
        fprintf(out, "printf(\"%s\");\n", tokens[i + 1].value);
        i += 2;
      }
      else if (tokens[i + 1].type == TOKEN_INT ||
               tokens[i + 1].type == TOKEN_OP)
      {
        fprintf(out, "printf(\"%%d\", ");
        while (tokens[i + 1].type == TOKEN_INT ||
               tokens[i + 1].type == TOKEN_OP)
        {
          fprintf(out, "%s", tokens[i + 1].value);
          i++;
        }
        fprintf(out, ");\n");
        i++;
      }
      else if (tokens[i + 1].type == TOKEN_VAR_INT)
      {
        fprintf(out, "printf(\"%%d\", %s);\n", tokens[i + 1].value);
        i += 2;
      }
      else if (tokens[i + 1].type == TOKEN_VAR_FLOAT)
      {
        fprintf(out, "printf(\"%%f\", %s);\n", tokens[i + 1].value);
        i += 2;
      }
      else if (tokens[i + 1].type == TOKEN_VAR_STRING)
      {
        fprintf(out, "printf(\"%%s\", %s);\n", tokens[i + 1].value);
        i += 2;
      }
      else if (tokens[i + 1].type == TOKEN_VAR_BOOL)
      {
        fprintf(out, "printf(\"%%s\", %s ? \"true\" : \"false\");\n",
                tokens[i + 1].value);
        i += 2;
      }
      else
      {
        fprintf(stderr, "Warning: Unknown print argument type at token %d AKA %s\n", i, tokens[i + 1].value);
        i++;
      }
    }
    else if (strcmp(tokens[i].value, KEYWORD_PRINTNL) == 0)
    {
      if (i + 1 >= count)
      {
        fprintf(stderr, "Error: print statement missing argument\n");
        i++;
        continue;
      }
      if (tokens[i + 1].type == TOKEN_STRING)
      {
        fprintf(out, "printf(\"%s\");\n", tokens[i + 1].value);
        i += 2;
      }
      else if (tokens[i + 1].type == TOKEN_INT ||
               tokens[i + 1].type == TOKEN_OP)
      {
        fprintf(out, "printf(\"%%d\\n\", ");
        while (tokens[i + 1].type == TOKEN_INT ||
               tokens[i + 1].type == TOKEN_OP)
        {
          fprintf(out, "%s", tokens[i + 1].value);
          i++;
        }
        fprintf(out, ");\n");
        i++;
      }
      else if (tokens[i + 1].type == TOKEN_VAR_INT)
      {
        fprintf(out, "printf(\"%%d\\n\", %s);\n", tokens[i + 1].value);
        i += 2;
      }
      else if (tokens[i + 1].type == TOKEN_VAR_FLOAT)
      {
        fprintf(out, "printf(\"%%f\\n\", %s);\n", tokens[i + 1].value);
        i += 2;
      }
      else if (tokens[i + 1].type == TOKEN_VAR_STRING)
      {
        fprintf(out, "printf(\"%%s\\n\", %s);\n", tokens[i + 1].value);
        i += 2;
      }
      else if (tokens[i + 1].type == TOKEN_VAR_BOOL)
      {
        fprintf(out, "printf(\"%%s\\n\", %s ? \"true\" : \"false\");\n", tokens[i + 1].value);
        i += 2;
      }
      else
      {
        fprintf(stderr, "Warning: Unknown print argument type at token %d AKA %s\n", i, tokens[i + 1].value);
        i++;
      }
    }
    else if (strcmp(tokens[i].value, KEYWORD_VAR) == 0)
    {
      if (tokens[i + 3].type == TOKEN_STRING)
      {
        fprintf(out, "char* %s = \"%s\";\n", tokens[i + 1].value, tokens[i + 3].value);
        i += 4;
      }
      else if (tokens[i + 3].type == TOKEN_INT ||
               tokens[i + 3].type == TOKEN_OP)
      {
        fprintf(out, "int %s = %s;\n", tokens[i + 1].value, tokens[i + 3].value);
        i += 4;
      }
      else if (tokens[i + 3].type == TOKEN_FLOAT)
      {
        fprintf(out, "double %s = %s;\n", tokens[i + 1].value, tokens[i + 3].value);
        i += 4;
      }
      else if (tokens[i + 3].type == TOKEN_BOOL)
      {
        fprintf(out, "bool %s = %s;\n", tokens[i + 1].value, tokens[i + 3].value);
        i += 4;
      }
      else
      {
        i++;
      }
    }
    else
    {
      i++;
    }
  }
  fprintf(out, "    return 0;\n}\n"); // End main function
}
