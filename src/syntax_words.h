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

#ifndef SYNTAX_WORDS_H
#define SYNTAX_WORDS_H

#ifdef LANG_RU
    #define KEYWORD_PRINT "вывод"
    #define KEYWORD_IF "если"
    #define KEYWORD_ELSE "иначе"
    #define KEYWORD_WHILE "пока"
    #define KEYWORD_FOR "для"
    #define KEYWORD_FUNC "функция"
    #define KEYWORD_RETURN "возврат"
    #define KEYWORD_INPUT "ввод"
    #define KEYWORD_VAR "переменная"
    #define KEYWORD_END "конец"
    #define KEYWORD_TRUE "истина"
    #define KEYWORD_FALSE "ложь"
    #define KEYWORD_START "начало"
#else
    #define KEYWORD_PRINT "print"
    #define KEYWORD_PRINTNL "printnl"
    #define KEYWORD_IF "if"
    #define KEYWORD_ELSE "else"
    #define KEYWORD_WHILE "while"
    #define KEYWORD_FOR "for"
    #define KEYWORD_FUNC "func"
    #define KEYWORD_FUNC_START '('
    #define KEYWORD_FUNC_END ')'
    #define KEYWORD_RETURN "return"
    #define KEYWORD_INPUT "input"
    #define KEYWORD_VAR "let"
    #define KEYWORD_END "end"
    #define KEYWORD_TRUE "true"
    #define KEYWORD_FALSE "false"
    #define KEYWORD_START "start"
#endif

#endif // SYNTAX_WORDS_H
