//
// Created by yangcg on 2021/10/11.
//
#include "main.h"
#include "Lexer.h"
#include "SymbolType.h"


#ifndef COMPILER_PARSER_H
#define COMPILER_PARSER_H

//#define OUTPUT_Parser
//#define OUTPUT_Parser_WRONG

void error_Parser();
void error(int, char);
void SymbolTable_push();
void SymbolTable_pop();

compUnit* CompUnit();
decl_class * Decl(bool is_global);
decl_class* ConstDecl(bool is_global);
void BType();
SubDecl * ConstDef(bool is_global);
initVal_class * ConstInitVal();
decl_class * VarDecl(bool is_global);
decl_class * VarDecl2(string& ident, bool is_global);
SubDecl * VarDef(bool is_global);
SubDecl * VarDef2(string& ident, bool is_global);
initVal_class * InitVal();
funcDef * FuncDef();
funcDef * FuncDef2(string& ident);
funcDef * FuncDef3();
funcDef * MainFuncDef();
void FuncType();
vector<FuncParam*> FuncFParams(Func_data *p);
FuncParam* FuncFParam(Func_data *p);
BlockStmt * Block();
blockItem * BlockItem();
stmt_class * Stmt();
Expr * Exp();
Condition * Cond();
lVal_class * LVal();
int LVal2(Dim **pDim);
Expr * PrimaryExp();
Dim * PrimaryExp2(Symbol *p);
LiteralExpr * Number();
Expr * UnaryExp();
UnaryOp UnaryOp();
vector<Expr *> FuncRParams(Func_data *pid);
Expr * MulExp();
Expr * AddExp();
Condition * RelExp();
Condition * EqExp();
Condition * LAndExp();
Condition * LOrExp();
Expr * ConstExp();
void NextExp(Expr **pExpr);

#endif //COMPILER_PARSER_H
