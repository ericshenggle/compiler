#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <cstdio>
#include <cstring>
#include <cstring>
#include "main.h"

//#define OUTPUT_Lexer

#define END -2
#define NO_MATCH -1
#define IDSY 0
#define INTSY 1
#define FORMATSY 2
#define SERSY_main 3
#define SERSY_const 4
#define SERSY_int 5
#define SERSY_break 6
#define SERSY_continue 7
#define SERSY_if 8
#define SERSY_else 9
#define SERSY_while 10
#define SERSY_getint 11
#define SERSY_printf 12
#define SERSY_return 13
#define SERSY_void 14
#define NOT 15
#define AND 16
#define OR 17
#define PLUS 18
#define MINU 19
#define MULT 20
#define DIV 21
#define MOD 22
#define LSS 23
#define LEQ 24
#define GRE 25
#define GEQ 26
#define EQL 27
#define NEQ 28
#define ASSGIN 29
#define SEMICN 30
#define COMMA 31
#define LPARENT 32
#define RPARENT 33
#define LBRACK 34
#define RBRACK 35
#define LBRACE 36
#define RBRACE 37

#ifndef COMPILER_LEXER_H
#define COMPILER_LEXER_H

struct Format_string {
    int exp_num;
    bool is_wrong;
};

extern Format_string* format_string;

void error_Lexer();
int reServer();
void getSym();
void getSym2();
int jump_annotation(char **pString);
void output_Lexer();
void output_Lexer2();

#endif // COMPILER_LEXER_H