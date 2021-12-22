//
// Created by ycg on 2021/11/2.
//

#ifndef COMPILER_AST_H
#define COMPILER_AST_H

#include <utility>
#include <vector>
#include <fstream>
#include <string>
#include <fstream>
#include <iostream>
#include "SymbolType.h"

#define OUTPUT_MidCode
extern bool OPTIMIZE;

using std::vector;
using std::string;
using std::ofstream;
using std::endl;

extern SymbolTable* symbolTable_Var;
extern SymbolTable* symbolTable_Func;
extern ofstream midCodefile;
extern deque<vector<string>> midCode;

enum ExprType
{
    literalExpr, unaryExpr, binaryExpr, callExpr, lval, assignExpr, empty
};

enum UnaryOp
{
    Neg, Pos, Not
};

enum BinaryOp
{
    Add, Sub, Mul, Div, Mod
};

enum CondOp
{
    Gre, Lss, Geq, Leq, Eql, Neq, And, Or, NotCond
};

enum StmtType
{
    LVALASSIGN, LVALGETINT, EXP, EMPTY, BLOCK, IFSTMT, WHILESTMT, BREAK, CONTINUE, RETURNSTMT, PRINTSTMT
};

enum Type
{
    INT, VOID
};

class lVal_class;
class AssignExpr;
class UnaryExpr;
class BinaryExpr;
class CallExpr;
class LiteralExpr;
class Dim;
class Expr;
class Condition;
class initVal_class;
class SubDecl;
class decl_class;
class funcDef;
class compUnit;
class FuncParam;
class blockItem;
class IfStmt;
class BlockStmt;
class WhileStmt;
class ReturnStmt;
class PrintStmt;
class stmt_class;

class Condition {
public:
    Condition(bool isCond, Condition* l, Condition* r, CondOp condOp1):
            isCond(isCond), l(l), r(r), expr(nullptr), condOp(condOp1){};
    Condition(bool isCond, Expr* expr1):
            isCond(isCond), l(nullptr), r(nullptr), expr(expr1), condOp(NotCond){};
    ~Condition();
    std::basic_string<char> mid_code(int *pInt) const;

    bool isCond;
    CondOp condOp;
    Condition* l;
    Condition* r;
    Expr* expr;
};

class lVal_class {
public:
    lVal_class(string ident, Dim* dim):
            ident(std::move(ident)), dim(dim){};
    ~lVal_class();
    int returnValue() const;
    std::basic_string<char> mid_code(int *pInt) const;

    string ident;
    Dim* dim;
};

class AssignExpr {
public:
    AssignExpr(Expr* l, Expr* r):
        l(l), r(r){};
    ~AssignExpr();
    void mid_code() const;

    Expr* l;
    Expr* r;
};

class UnaryExpr {
public:
    UnaryExpr(UnaryOp unaryOp, Expr* expr):
            unaryOp(unaryOp), expr(expr){};
    ~UnaryExpr();
    int returnValue() const;
    std::basic_string<char> mid_code(int *pInt) const;

    UnaryOp unaryOp;
    Expr* expr;
};

class BinaryExpr {
public:
    BinaryExpr(BinaryOp binaryOp, Expr* l, Expr* r):
            binaryOp(binaryOp),l(l), r(r){};
    ~BinaryExpr();
    int returnValue() const;
    std::basic_string<char> mid_code(int *pInt) const;

    BinaryOp binaryOp;
    Expr* l;
    Expr* r;
};

class CallExpr {
public:
    explicit CallExpr(string ident) :
            ident(std::move(ident)){};
    ~CallExpr();
    std::basic_string<char> mid_code(int *pInt) const;

    string ident;
    vector<Expr*> params;
};

class LiteralExpr {
public:
    explicit LiteralExpr(int value):
            value(value){};
    std::basic_string<char> mid_code() const;

    int value;
};

class Expr {
public:
    explicit Expr(ExprType exprType):
            exprType(exprType), lVal(nullptr), assignExpr(nullptr), unaryExpr(nullptr),
            binaryExpr(nullptr), callExpr(nullptr), literalExpr(nullptr){};
    Expr(ExprType exprType, BinaryExpr* binaryExpr):
            exprType(exprType), lVal(nullptr), assignExpr(nullptr), unaryExpr(nullptr),
            binaryExpr(binaryExpr), callExpr(nullptr), literalExpr(nullptr){};
    Expr(ExprType exprType, AssignExpr *assignExpr):
            exprType(exprType), lVal(nullptr), assignExpr(assignExpr), unaryExpr(nullptr),
            binaryExpr(nullptr), callExpr(nullptr), literalExpr(nullptr){};
    ~Expr();
    int returnValue() const;
    std::basic_string<char> mid_code(int *pInt) const;

    ExprType exprType;
    lVal_class* lVal;
    AssignExpr* assignExpr;
    UnaryExpr* unaryExpr;
    BinaryExpr* binaryExpr;
    CallExpr* callExpr;
    LiteralExpr* literalExpr;
};

class Dim {
public:
    Dim() = default;

    vector<Expr*> exprs;
};

class initVal_class {
public:
    initVal_class(bool is_expr, Expr* expr):
            is_expr(is_expr), expr(expr){};
    ~initVal_class();

    Expr* expr;
    bool is_expr;
    vector<initVal_class*> initVals;
};

class SubDecl {
public:
    SubDecl(string ident, bool is_global):
            ident(std::move(ident)), is_global(is_global), dim(nullptr), initVal(nullptr){};
    Symbol *init(bool is_const, bool global) const;
    ~SubDecl();
    void mid_code(bool is_const);

    string ident;
    bool is_global;
    Dim* dim;
    initVal_class* initVal;
};

class decl_class {
public:
    explicit decl_class(bool is_const):
            is_const(is_const) {};
    ~decl_class();
    void mid_code();

    bool is_const;
    vector<SubDecl*> subDecls;
};

class FuncParam {
public:
    FuncParam(string ident, Dim* dim, Type type):
            ident(std::move(ident)), dim(dim), paramType(type){};
    ~FuncParam();
    void mid_code() const;

    string ident;
    Dim* dim;
    Type paramType;
};

class funcDef {
public:
    funcDef(string ident, Type funcType, BlockStmt* body):
            ident(std::move(ident)), funcType(funcType), body(body){};
    ~funcDef();
    void mid_code();

    string ident;
    vector<FuncParam*> params;
    Type funcType;
    BlockStmt* body;
};

class blockItem {
public:
    blockItem(bool isStmt, stmt_class* stmt, decl_class* decl):
            isStmt(isStmt), stmt(stmt), decl(decl){};
    ~blockItem();
    void mid_code() const;

    bool isStmt;
    stmt_class* stmt;
    decl_class* decl;
};

class IfStmt {
public:
    IfStmt(Condition* cond, stmt_class* then_block, stmt_class* else_block):
            cond(cond), then_block(then_block), else_block(else_block){};
    IfStmt(Condition* cond, stmt_class* then_block):
            cond(cond), then_block(then_block), else_block(nullptr){};
    ~IfStmt();
    void mid_code() const;

    Condition* cond;
    stmt_class* then_block;
    stmt_class* else_block;
};

class BlockStmt {
public:
    explicit BlockStmt(vector<blockItem*> blockItems):
            blockItems(std::move(blockItems)){};
    ~BlockStmt();
    void mid_code(bool isFuncBlock);

    vector<blockItem*> blockItems;
};

class WhileStmt {
public:
    WhileStmt(Condition* cond, stmt_class* body):
        cond(cond), body(body){};
    ~WhileStmt();
    void mid_code() const;

    Condition* cond;
    stmt_class* body;
};

class ReturnStmt {
public:
    explicit ReturnStmt(Expr* expr):
            expr(expr){};
    ReturnStmt():
            expr(nullptr){};
    ~ReturnStmt();
    void mid_code() const;

    Expr* expr;
};

class PrintStmt {
public:
    PrintStmt(string formatString, vector<Expr*> exprs):
            formatString(std::move(formatString)), exprs(std::move(exprs)){};
    ~PrintStmt();
    void mid_code();

    string formatString;
    vector<Expr*> exprs;
};

class stmt_class {
public:
    stmt_class():
            stmtType(EMPTY), expr(nullptr), blockStmt(nullptr), ifStmt(nullptr),
            whileStmt(nullptr), returnStmt(nullptr), printStmt(nullptr){};
    ~stmt_class();
    void mid_code(bool isFuncBlock) const;

    StmtType stmtType;
    Expr* expr;
    BlockStmt* blockStmt;
    IfStmt* ifStmt;
    WhileStmt* whileStmt;
    ReturnStmt* returnStmt;
    PrintStmt* printStmt;

};

class compUnit {
public:
    compUnit() = default;
    ~compUnit();
    void mid_code();

    vector<decl_class*> decls;
    vector<funcDef*> funDefs;
};



#endif //COMPILER_AST_H
