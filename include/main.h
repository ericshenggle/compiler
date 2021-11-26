//
// Created by yangcg on 2021/10/11.
//

#ifndef COMPILER_MAIN_H
#define COMPILER_MAIN_H
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <cstdio>
#include <cstring>
#include <cstring>
#include "ast.h"

#define MAX_WORD 256

using std::ofstream;
using std::string;
using std::vector;

extern vector<string> category;
extern ofstream outfile, errorfile;   /* NOLINT */

extern int cnt, cnt_output;
extern int num;
extern bool is_annotation;
extern int line;
extern char token[MAX_WORD];
extern int sym;
extern char *str;
extern bool main_already;

#endif //COMPILER_MAIN_H
