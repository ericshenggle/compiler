//
// Created by yangcg on 2021/10/11.
//

#include "include/Parser.h"
#include "include/SymbolType.h"
#include <vector>
#include <algorithm>

using std::endl;
using std::pair;
using std::vector;
using std::make_pair;

bool main_already = false;
bool has_return = false;
SymbolTable *symbolTable_Var;
SymbolTable *symbolTable_Func;
Symbol *temp_symbol;
int temp_line;
FuncParamType temp_param_type;
bool temp_return;
int while_block = 0, block = 0;
vector<pair<int, char>> vect;
bool is_wrong = false;

bool cmp(const pair<int, char> & v1, const pair<int, char> & v2) {
    return v1.first < v2.first;
}

void output_Parser_wrong() {
#ifdef OUTPUT_Parser_WRONG
    sort(vect.begin(), vect.end(), cmp);
    for(auto &it : vect) {
        errorfile << it.first << " " << it.second << endl;
    }
    std::cout << "InValid Syntax" << endl;
#endif
}

void error_Parser() {
#ifdef OUTPUT_Parser
    outfile << "invalid Grammar:" << token << " line:" << line << endl;
#endif
    exit(1);
}

void error(int line1, char code) {
    is_wrong = true;
    vect.emplace_back(make_pair(line1, code));
}

void SymbolTable_push() {
    symbolTable_Func->push();
    symbolTable_Var->push();
}

void SymbolTable_pop() {
    symbolTable_Func->pop();
    symbolTable_Var->pop();
}

compUnit* CompUnit() {
    auto compunit = new compUnit();
    getSym2();
    while (sym == SERSY_const || sym == SERSY_int) {
        if (sym == SERSY_const) {
            decl_class* decl1 = ConstDecl(true);
            compunit->decls.emplace_back(decl1);
        } else {
            getSym();
            if (sym == IDSY) {
                string ident(token);
                temp_line = line;
                getSym2();
                if (sym == LPARENT) {
#ifdef OUTPUT_Parser
                    outfile << "<FuncType>" << endl;
#endif
                    output_Lexer2();
                    getSym();
                    temp_symbol = new Symbol();
                    temp_symbol->type = function_int_t;
                    funcDef* funcDef1 = FuncDef2(ident);
                    funcDef1->funcType = INT;
                    compunit->funDefs.emplace_back(funcDef1);
                    break;
                } else {
                    output_Lexer2();
                    decl_class* decl1 = VarDecl2(ident, true);
                    compunit->decls.emplace_back(decl1);
                }
            } else if (sym == SERSY_main) {
                getSym();
                funcDef* funcDef1 = MainFuncDef();
                compunit->funDefs.emplace_back(funcDef1);
                break;
            } else {
                error_Parser();
            }
        }
    }
    while (sym == SERSY_void || sym == SERSY_int) {
        if (main_already) {
            error_Parser();
        }
        if (sym == SERSY_void) {
            funcDef* funcDef1 = FuncDef();
            compunit->funDefs.emplace_back(funcDef1);
        } else {
            getSym();
            if (sym == SERSY_main) {
                getSym();
                funcDef* funcDef1 = MainFuncDef();
                compunit->funDefs.emplace_back(funcDef1);
            } else {
#ifdef OUTPUT_Parser
                outfile << "<FuncType>" << endl;
#endif
                temp_symbol = new Symbol();
                temp_symbol->type = function_int_t;
                funcDef* funcDef1 = FuncDef3();
                compunit->funDefs.emplace_back(funcDef1);
            }
        }
    }
    if (sym != END) {
        error_Parser();
    }
#ifdef OUTPUT_Parser
    outfile << "<CompUnit>" << endl;
#endif
    if (is_wrong) {
        output_Parser_wrong();
        exit(0);
    }
    return compunit;
}

funcDef * MainFuncDef() {
    funcDef* funcDef1 = nullptr;
    main_already = true;
    auto *symbol = new Symbol();
    symbol->str = "main";
    int line1 = line;
    symbol->type = function_int_t;
    temp_return = true;
    symbolTable_Func->insert(symbol);
    has_return = false;
    symbolTable_Var->push();
    if (sym == LPARENT) {
        getSym();
        if (sym == RPARENT) {
            getSym();
        } else {
            error(line1, 'j');
        }
        BlockStmt* blockStmt = Block();
        funcDef1 = new funcDef(string("main"), INT, blockStmt);
    } else {
        error_Parser();
    }
    symbolTable_Var->pop();
    if (!has_return && temp_return) {
        error(temp_line, 'g');
    }
#ifdef OUTPUT_Parser
    outfile << "<MainFuncDef>" << endl;
#endif
    return funcDef1;
}

funcDef * FuncDef3() {
    funcDef* funcDef1 = nullptr;
    if (sym == IDSY) {
        string ident(token);
        temp_line = line;
        getSym();
        if (sym == LPARENT) {
            getSym();
            funcDef1 = FuncDef2(ident);
        } else {
            error_Parser();
        }
    } else {
        error_Parser();
    }
    return funcDef1;
}

funcDef * FuncDef2(string& ident) {
    funcDef* funcDef1;
    temp_symbol->str = ident;
    temp_return = temp_symbol->type == function_int_t;
    auto p = new Func_data();
    bool a;
    if (!(a = symbolTable_Func->insert(temp_symbol))) {
        delete temp_symbol;
        error(temp_line, 'b');
    } else {
        temp_symbol->data = p;
    }
    p->num = 0;
    has_return = false;
    symbolTable_Var->push();
    if (sym == RPARENT) {
        getSym();
        BlockStmt* blockStmt = Block();
        funcDef1 = new funcDef(ident, INT, blockStmt);
    } else {
        int line1 = line;
        vector<FuncParam*> params = FuncFParams(p);
        if (sym == RPARENT) {
            getSym();
        } else {
            error(line1, 'j');
        }
        BlockStmt* blockStmt = Block();
        funcDef1 = new funcDef(ident, INT, blockStmt);
        funcDef1->params = params;
    }
    if (!a) {
        delete p;
    }
    symbolTable_Var->pop();
    if (!has_return && temp_return) {
        error(temp_line, 'g');
    }
#ifdef OUTPUT_Parser
    outfile << "<FuncDef>" << endl;
#endif
    return funcDef1;
}

funcDef * FuncDef() {
    temp_symbol = new Symbol();
    FuncType();
    funcDef* funcDef1 = FuncDef3();
    funcDef1->funcType = VOID;
    return funcDef1;
}

void FuncType() {
    if (sym == SERSY_void) {
        temp_symbol->type = function_void_t;
        getSym();
    } else if (sym == SERSY_int) {
        temp_symbol->type = function_int_t;
        getSym();
    } else {
        error_Parser();
    }
#ifdef OUTPUT_Parser
    outfile << "<FuncType>" << endl;
#endif
}

vector<FuncParam*> FuncFParams(Func_data *p) {
    vector<FuncParam*> params;
    FuncParam* param = FuncFParam(p);
    params.emplace_back(param);
    while (sym == COMMA) {
        getSym();
        param = FuncFParam(p);
        params.emplace_back(param);
    }
#ifdef OUTPUT_Parser
    outfile << "<FuncFParams>" << endl;
#endif
    return params;
}

FuncParam* FuncFParam(Func_data *p) {
    FuncParam* funcParam = nullptr;
    p->num++;
    BType();
    if (sym == IDSY) {
        temp_symbol = new Symbol();
        temp_symbol->str = token;
        string ident(token);
        int line1 = line;
        bool a;
        if (!(a = symbolTable_Var->insert(temp_symbol))) {
            error(temp_line, 'b');
        }
        getSym();
        if (sym == LBRACK) {
            getSym();
            if (sym == RBRACK) {
                getSym();
            } else {
                error(line1, 'k');
            }
            auto* dim = new Dim();
            if (sym == LBRACK) {
                getSym();
                int line2 = line;
                Expr* expr = ConstExp();
                dim->exprs.emplace_back(expr);
                if (sym == RBRACK) {
                    getSym();
                } else {
                    error(line2, 'k');
                }
                temp_symbol->dimension = 2;
                temp_symbol->type = int_array_t;
                p->params[p->num - 1] = int_array2_param;
                funcParam = new FuncParam(ident, dim, INT);
            } else {
                temp_symbol->dimension = 1;
                temp_symbol->type = int_array_t;
                p->params[p->num - 1] = int_array1_param;
                funcParam = new FuncParam(ident, dim, INT);
            }
        } else {
            temp_symbol->dimension = 0;
            temp_symbol->type = int_t;
            p->params[p->num - 1] = int_param;
            funcParam = new FuncParam(ident, nullptr, INT);
        }
        if (!a) {
            delete temp_symbol;
        }
    } else {
        error_Parser();
    }
#ifdef OUTPUT_Parser
    outfile << "<FuncFParam>" << endl;
#endif
    return funcParam;
}

decl_class * Decl(bool is_global) {
    decl_class* decl1 = nullptr;
    if (sym == SERSY_const) {
        decl1 = ConstDecl(is_global);
    } else if (sym == SERSY_int) {
        decl1 = VarDecl(is_global);
    } else {
        error_Parser();
    }
    return decl1;
}

void BType() {
    if (sym == SERSY_int) {
        getSym();
    } else {
        error_Parser();
    }
}

decl_class * VarDecl2(string& ident, bool is_global) {
    auto* decl1 = new decl_class(false);
    SubDecl* subDecl1 = VarDef2(ident, is_global);
    decl1->subDecls.emplace_back(subDecl1);
    while (sym == COMMA) {
        getSym();
        subDecl1 = VarDef(is_global);
        decl1->subDecls.emplace_back(subDecl1);
    }
    if (sym == SEMICN) {
        getSym();
    } else {
        error(temp_line, 'i');
    }
#ifdef OUTPUT_Parser
    outfile << "<VarDecl>" << endl;
#endif
    return decl1;
}

decl_class * VarDecl(bool is_global) {
    auto* decl1 = new decl_class(false);
    BType();
    SubDecl* subDecl1 = VarDef(is_global);
    decl1->subDecls.emplace_back(subDecl1);
    while (sym == COMMA) {
        getSym();
        subDecl1 = VarDef(is_global);
        decl1->subDecls.emplace_back(subDecl1);
    }
    if (sym == SEMICN) {
        getSym();
    } else {
        error(line, 'i');
        error_Parser();
    }
#ifdef OUTPUT_Parser
    outfile << "<VarDecl>" << endl;
#endif
    return decl1;
}

SubDecl * VarDef2(string& ident, bool is_global) {
    auto subDecl = new SubDecl(ident, is_global);
    temp_symbol = new Symbol();
    temp_symbol->str = ident;
    temp_symbol->type = int_t;
    bool a;
    if (!(a = symbolTable_Var->insert(temp_symbol))) {
        error(temp_line, 'b');
    }
    if (sym == LBRACK) {
        auto p = new Array_data();
        temp_symbol->data = p;
        temp_symbol->type = int_array_t;
        auto* dim = new Dim();
        while (sym == LBRACK) {
            getSym();
            int line1 = line;
            Expr* expr = ConstExp();
            dim->exprs.emplace_back(expr);
            if (sym == RBRACK) {
                getSym();
            } else {
                error(line1, 'k');
            }
            temp_symbol->dimension++;
        }
        subDecl->dim = dim;
    }
    if (sym == ASSGIN) {
        getSym();
        initVal_class* initVal1 = InitVal();
        subDecl->initVal = initVal1;
    }
    if (!a) {
        delete temp_symbol;
    }
#ifdef OUTPUT_Parser
    outfile << "<VarDef>" << endl;
#endif
    return subDecl;
}

SubDecl * VarDef(bool is_global) {
    SubDecl* subDecl = nullptr;
    if (sym == IDSY) {
        string ident(token);
        temp_line = line;
        getSym();
        subDecl = VarDef2(ident, is_global);
    } else {
        error_Parser();
    }
    return subDecl;
}

decl_class* ConstDecl(bool is_global) {
    auto decl1 = new decl_class(true);
    temp_line = line;
    if (sym == SERSY_const) {
        getSym();
        BType();
        SubDecl* subDecl = ConstDef(is_global);
        decl1->subDecls.emplace_back(subDecl);
        while (sym == COMMA) {
            getSym();
            subDecl = ConstDef(is_global);
            decl1->subDecls.emplace_back(subDecl);
        }
    } else {
        error_Parser();
    }
    if (sym == SEMICN) {
        getSym();
    } else {
        error(temp_line, 'i');
    }
#ifdef OUTPUT_Parser
    outfile << "<ConstDecl>" << endl;
#endif
    return decl1;
}

SubDecl * ConstDef(bool is_global) {
    SubDecl *subDecl = nullptr;
    temp_symbol = new Symbol();
    auto array_data = new Array_data();
    temp_symbol->data = array_data;
    if (sym == IDSY) {
        temp_symbol->str = token;
        subDecl = new SubDecl(string(token), is_global);
        temp_line = line;
        bool a;
        if (!(a = symbolTable_Var->insert(temp_symbol))) {
            error(temp_line, 'b');
        }
        getSym();
        if (sym == LBRACK) {
            temp_symbol->type = const_array_t;
            auto* dim = new Dim();
            while (sym == LBRACK) {
                getSym();
                int line1 = line;
                Expr* expr = ConstExp();
                dim->exprs.emplace_back(expr);
                if (sym == RBRACK) {
                    getSym();
                } else {
                    error(line1, 'k');
                }
                temp_symbol->dimension++;
            }
            subDecl->dim = dim;
        } else {
            temp_symbol->type = const_t;
        }
        if (sym == ASSGIN) {
            getSym();
            initVal_class* initVal1 = ConstInitVal();
            subDecl->initVal = initVal1;
        } else {
            error_Parser();
        }
        if (!a) {
            delete temp_symbol;
        }
    } else {
        error_Parser();
    }
#ifdef OUTPUT_Parser
    outfile << "<ConstDef>" << endl;
#endif
    return subDecl;
}

initVal_class * InitVal() {
    initVal_class* initVal1;
    if (sym == LBRACE) {
        getSym();
        if (sym == RBRACE) {
            getSym();
            initVal1 = new initVal_class(false, nullptr);
        } else {
            initVal1 = new initVal_class(false, nullptr);
            initVal_class* initVal2 = InitVal();
            initVal1->initVals.emplace_back(initVal2);
            while (sym == COMMA) {
                getSym();
                initVal2 = InitVal();
                initVal1->initVals.emplace_back(initVal2);
            }
            if (sym == RBRACE) {
                getSym();
            } else {
                error_Parser();
            }
        }
    } else {
        Expr* expr = Exp();
        initVal1 = new initVal_class(true, expr);
    }
#ifdef OUTPUT_Parser
    outfile << "<InitVal>" << endl;
#endif
    return initVal1;
}

initVal_class * ConstInitVal() {
    initVal_class* initVal1;
    if (sym == LBRACE) {
        getSym();
        if (sym == RBRACE) {
            getSym();
            initVal1 = new initVal_class(false, nullptr);
        } else {
            initVal1 = new initVal_class(false, nullptr);
            initVal_class* initVal2 = ConstInitVal();
            initVal1->initVals.emplace_back(initVal2);
            while (sym == COMMA) {
                getSym();
                initVal2 = ConstInitVal();
                initVal1->initVals.emplace_back(initVal2);
            }
            if (sym == RBRACE) {
                getSym();
            } else {
                error_Parser();
            }
        }
    } else {
        Expr* expr = ConstExp();
        initVal1 = new initVal_class(true, expr);
    }
#ifdef OUTPUT_Parser
    outfile << "<ConstInitVal>" << endl;
#endif
    return initVal1;
}

Expr * Exp() {
    Expr* expr = AddExp();
#ifdef OUTPUT_Parser
    outfile << "<Exp>" << endl;
#endif
    return expr;
}

Expr * ConstExp() {
    Expr* expr = AddExp();
#ifdef OUTPUT_Parser
    outfile << "<ConstExp>" << endl;
#endif
    return expr;
}

Expr * AddExp() {
    Expr* expr1 = MulExp();
    Expr* expr2;

    while (sym == PLUS || sym == MINU) {
#ifdef OUTPUT_Parser
        outfile << "<AddExp>" << endl;
#endif
        int temp_sym = sym;
        getSym();
        expr2 = MulExp();
        if (temp_sym == PLUS) {
            expr1 = new Expr(binaryExpr, new BinaryExpr(Add, expr1, expr2));
        } else {
            expr1 = new Expr(binaryExpr, new BinaryExpr(Sub, expr1, expr2));
        }
    }
#ifdef OUTPUT_Parser
    outfile << "<AddExp>" << endl;
#endif
    return expr1;
}

Expr * MulExp() {
    Expr* expr1 = UnaryExp();
    Expr* expr2;

    while (sym == MULT || sym == DIV || sym == MOD) {
#ifdef OUTPUT_Parser
        outfile << "<MulExp>" << endl;
#endif
        int temp_sym = sym;
        getSym();
        expr2 = UnaryExp();
        if (temp_sym == MULT) {
            expr1 = new Expr(binaryExpr, new BinaryExpr(Mul, expr1, expr2));
        } else if (temp_sym == DIV) {
            expr1 = new Expr(binaryExpr, new BinaryExpr(Div, expr1, expr2));
        } else {
            expr1 = new Expr(binaryExpr, new BinaryExpr(Mod, expr1, expr2));
        }
    }
#ifdef OUTPUT_Parser
    outfile << "<MulExp>" << endl;
#endif
    return expr1;
}

Expr * UnaryExp() {
    Expr* expr = nullptr;
    if (sym == PLUS || sym == MINU || sym == NOT) {
        enum UnaryOp unaryOp = UnaryOp();
        Expr* expr1 = UnaryExp();
        expr = new Expr(unaryExpr);
        expr->unaryExpr = new UnaryExpr(unaryOp, expr1);
    } else if (sym == IDSY) {
        string ident(token);
        temp_line = line;
        getSym();
        if (sym == LPARENT) {
            expr = new Expr(callExpr);
            expr->callExpr = new CallExpr(ident);
            auto p = symbolTable_Func->find(ident);
            if (p == nullptr) {
                error(temp_line, 'c');
            }
            Func_data *pid = nullptr;
            FuncParamType type = temp_param_type;
            if (p != nullptr) {
                if (p->type == function_void_t) {
                    type = void_param;
                }
                pid = (Func_data *)p->data;
            }
            getSym();
            if (sym == RPARENT) {
                if (pid != nullptr && pid->num != 0) {
                    error(temp_line, 'd');
                }
                getSym();
            } else {
                int line1 = line;
                expr->callExpr->params = FuncRParams(pid);
                if (sym == RPARENT) {
                    getSym();
                } else {
                    error(line1, 'j');
                }
            }
            temp_param_type = type;
        } else {
            auto p = symbolTable_Var->find(ident);
            expr = new Expr(lval);
            if (p == nullptr) {
                error(temp_line, 'c');
            }
            Dim* dim = PrimaryExp2(p);
            auto lVal1 = new lVal_class(ident, dim);
            expr->lVal = lVal1;
        }
    } else if (sym == LPARENT || sym == INTSY){
        expr = PrimaryExp();
    } else {
        error_Parser();
    }
#ifdef OUTPUT_Parser
    outfile << "<UnaryExp>" << endl;
#endif
    return expr;
}

vector<Expr *> FuncRParams(Func_data *pid) {
    int line1 = temp_line;
    int a = 0, b = 0;
    if (pid != nullptr) {
        b = pid->num;
    }
    vector<Expr *> params;
    temp_param_type = int_param;
    Expr* expr = Exp();
    params.emplace_back(expr);
    a++;
    if (pid != nullptr && a <= b && temp_param_type != pid->params[a - 1]) {
        error(line1, 'e');
    }
    while (sym == COMMA) {
        getSym();
        temp_param_type = int_param;
        expr = Exp();
        params.emplace_back(expr);
        a++;
        if (pid != nullptr && a <= b && temp_param_type != pid->params[a - 1]) {
            error(line1, 'e');
        }
    }
    if (pid != nullptr && (a < b || a > b)) {
        error(line1, 'd');
    }
#ifdef OUTPUT_Parser
    outfile << "<FuncRParams>" << endl;
#endif
    return params;
}

enum UnaryOp UnaryOp() {
    enum UnaryOp unaryOp = Pos;
    if (sym == PLUS) {
        getSym();
        unaryOp = Pos;
    } else if (sym == MINU) {
        getSym();
        unaryOp = Neg;
    } else if (sym == NOT) {
        getSym();
        unaryOp = Not;
    } else {
        error_Parser();
    }
#ifdef OUTPUT_Parser
    outfile << "<UnaryOp>" << endl;
#endif
    return unaryOp;
}

Dim * PrimaryExp2(Symbol *p) {
    SymbolType symbolType;
    int dimen = 0;
    if (p != nullptr) {
        symbolType = p->type;
        if (symbolType == int_array_t) {
            dimen = p->dimension;
        }
    }
    auto* dim = new Dim();
    int a = LVal2(&dim);
    int b = dimen - a;
    if (b == 1) {
        temp_param_type = int_array1_param;
    } else if (b == 2) {
        temp_param_type = int_array2_param;
    }
#ifdef OUTPUT_Parser
    outfile << "<PrimaryExp>" << endl;
#endif
    return dim;
}

Expr * PrimaryExp() {
    int line1 = line;
    Expr* expr = nullptr;
    if (sym == LPARENT) {
        getSym();
        expr = Exp();
        if (sym == RPARENT) {
            getSym();
        } else {
            error(line1, 'j');
        }
    } else if (sym == INTSY) {
        expr = new Expr(literalExpr);
        LiteralExpr* literalExpr1 = Number();
        expr->literalExpr = literalExpr1;
    } else if (sym == IDSY) {
        expr = new Expr(lval);
        lVal_class* lVal1 = LVal();
        expr->lVal = lVal1;
    } else {
        error_Parser();
    }
#ifdef OUTPUT_Parser
    outfile << "<PrimaryExp>" << endl;
#endif
    return expr;
}

LiteralExpr * Number() {
    LiteralExpr* literalExpr1 = nullptr;
    if (sym == INTSY) {
        literalExpr1 = new LiteralExpr(num);
        getSym();
    } else {
        error_Parser();
    }
#ifdef OUTPUT_Parser
    outfile << "<Number>" << endl;
#endif
    return literalExpr1;
}

int LVal2(Dim **pDim) {
    int a = 0;
    while (sym == LBRACK) {
        getSym();
        int line1 = line;
        Expr* expr = Exp();
        (*pDim)->exprs.emplace_back(expr);
        a++;
        if (sym == RBRACK) {
            getSym();
        } else {
            error(line1, 'k');
        }
    }
#ifdef OUTPUT_Parser
    outfile << "<LVal>" << endl;
#endif
    return a;
}

lVal_class * LVal() {
    lVal_class* lVal1 = nullptr;
    if (sym == IDSY) {
        string ident(token);
        auto* dim = new Dim();
        temp_line = line;
        if (symbolTable_Var->find(ident) == nullptr) {
            error(temp_line, 'c');
        }
        getSym();
        LVal2(&dim);
        lVal1 = new lVal_class(ident, dim);
    } else {
        error_Parser();
    }
    return lVal1;
}

BlockStmt * Block() {
    vector<blockItem*> blockItems;
    block++;
    if (sym == LBRACE) {
        getSym();
        while (sym != RBRACE) {
            blockItem* blockItem1 = BlockItem();
            blockItems.emplace_back(blockItem1);
        }
        if (sym == RBRACE) {
            temp_line = line;
            getSym();
        } else {
            error_Parser();
        }
    } else {
        error_Parser();
    }
    block--;
    auto* blockStmt = new BlockStmt(blockItems);
#ifdef OUTPUT_Parser
    outfile << "<Block>" << endl;
#endif
    return blockStmt;
}

blockItem * BlockItem() {
    blockItem* blockItem1;
    if (sym == SERSY_const || sym == SERSY_int) {
        decl_class* decl1 = Decl(false);
        blockItem1 = new blockItem(false, nullptr, decl1);
    } else {
        stmt_class* stmt1 = Stmt();
        blockItem1 = new blockItem(true, stmt1, nullptr);
    }
    return blockItem1;
}

stmt_class * Stmt() {
    auto* stmt1 = new stmt_class();
    has_return = false;
    if (sym == LBRACE) {
        stmt1->stmtType = BLOCK;
        symbolTable_Var->push();
        BlockStmt* blockStmt = Block();
        stmt1->blockStmt = blockStmt;
        symbolTable_Var->pop();
    } else if (sym == SERSY_if) {
        stmt1->stmtType = IFSTMT;
        getSym();
        if (sym == LPARENT) {
            getSym();
            temp_line = line;
            Condition* cond = Cond();
            if (sym == RPARENT) {
                getSym();
            } else {
                error(temp_line, 'j');
            }
            stmt_class* then_block = Stmt();
            if (sym == SERSY_else) {
                getSym();
                stmt_class* else_block = Stmt();
                stmt1->ifStmt = new IfStmt(cond, then_block, else_block);
            } else {
                stmt1->ifStmt = new IfStmt(cond, then_block);
            }
        } else {
            error_Parser();
        }
    } else if (sym == SERSY_while) {
        stmt1->stmtType = WHILESTMT;
        getSym();
        if (sym == LPARENT) {
            getSym();
            temp_line = line;
            Condition* cond = Cond();
            if (sym == RPARENT) {
                getSym();
            } else {
                error(temp_line, 'j');
            }
            while_block++;
            stmt_class* body = Stmt();
            while_block--;
            stmt1->whileStmt = new WhileStmt(cond, body);
        } else {
            error_Parser();
        }
    } else if (sym == SERSY_break || sym == SERSY_continue) {
        if (sym == SERSY_break) {
            stmt1->stmtType = BREAK;
        } else {
            stmt1->stmtType = CONTINUE;
        }
        if (while_block == 0) {
            error(line, 'm');
        }
        temp_line = line;
        getSym();
        if (sym == SEMICN) {
            getSym();
        } else {
            error(temp_line, 'i');
        }
    } else if (sym == SERSY_return) {
        stmt1->stmtType = RETURNSTMT;
        if (block == 1) {
            has_return = true;
        }
        temp_line = line;
        getSym();
        if (sym == SEMICN) {
            getSym();
            stmt1->returnStmt = new ReturnStmt();
        } else {
            int line1 = line;
            Expr* expr = Exp();
            stmt1->returnStmt = new ReturnStmt(expr);
            if (!temp_return) {
                error(temp_line, 'f');
            }
            if (sym == SEMICN) {
                getSym();
            } else {
                error(line1, 'i');
            }
        }
    } else if (sym == SERSY_printf) {
        stmt1->stmtType = PRINTSTMT;
        temp_line = line;
        getSym();
        if (sym == LPARENT) {
            getSym();
            if (sym == FORMATSY) {
                string formatString(token);
                int line1 = line;
                if (format_string->is_wrong) {
                    error(line, 'a');
                }
                getSym();
                int tmp = 0;
                vector<Expr*> exprs;
                while (sym == COMMA) {
                    getSym();
                    line1 = line;
                    Expr* expr = Exp();
                    exprs.emplace_back(expr);
                    tmp++;
                }
                stmt1->printStmt = new PrintStmt(formatString, exprs);
                if (format_string->exp_num != tmp) {
                    error(temp_line, 'l');
                }
                if (sym == RPARENT) {
                    getSym();
                } else {
                    error(line1, 'j');
                    error_Parser();
                }
                if (sym == SEMICN) {
                    getSym();
                } else {
                    error(line1, 'i');
                    error_Parser();
                }
            } else {
                error_Parser();
            }
        } else {
            error_Parser();
        }
    } else if (sym == IDSY) {
        stmt1->stmtType = EXP;
        string ident(token);
        temp_line = line;
        getSym();
        Expr *expr;
        if (sym == LPARENT) {            //Exp中的Func
            expr = new Expr(callExpr);
            expr->callExpr = new CallExpr(ident);
            auto p = symbolTable_Func->find(ident);
            if (p == nullptr) {
                error(temp_line, 'c');
            }
            Func_data *pid = nullptr;
            if (p != nullptr) {
                pid = (Func_data *)p->data;
            }
            getSym();
            if (sym == RPARENT) {
                if (pid != nullptr && pid->num != 0) {
                    error(temp_line, 'd');
                }
                getSym();
            } else {
                int line1 = line;
                expr->callExpr->params = FuncRParams(pid);
                if (sym == RPARENT) {
                    getSym();
                } else {
                    error(line1, 'j');
                }
            }
#ifdef OUTPUT_Parser
            outfile << "<UnaryExp>" << endl;
#endif
            NextExp(&expr);
            stmt1->expr = expr;
        } else {
            auto p = symbolTable_Var->find(ident);
            if (p == nullptr) {
                error(temp_line, 'c');
            }
            auto* dim = new Dim();
            auto* lVal1 = new lVal_class(ident, dim);
            LVal2(&dim);
            expr = new Expr(lval);
            expr->lVal = lVal1;
            if (sym == ASSGIN) {           //LVal=Exp或LVal=getint()
                if (p != nullptr && (p->type == const_t || p->type == const_array_t)) {
                    error(temp_line, 'h');
                }
                getSym();
                if (sym == SERSY_getint) {
                    getSym();
                    if (sym == LPARENT) {
                        getSym();
                    } else {
                        error_Parser();
                    }
                    if (sym == RPARENT) {
                        getSym();
                    } else {
                        error(temp_line, 'j');
                    }
                    if (sym == SEMICN) {
                        getSym();
                    } else {
                        error(temp_line, 'i');
                    }
                    stmt1->expr = new Expr(assignExpr, new AssignExpr(expr, nullptr));
                    stmt1->stmtType = LVALGETINT;
                } else {
                    Expr* expr1 = Exp();
                    if (sym == SEMICN) {
                        getSym();
                    } else {
                        error(temp_line, 'i');
                    }
                    stmt1->expr = new Expr(assignExpr, new AssignExpr(expr, expr1));
                    stmt1->stmtType = LVALASSIGN;
                }
            } else {               //Exp中的LVal
#ifdef OUTPUT_Parser
                outfile << "<PrimaryExp>" << endl;
                outfile << "<UnaryExp>" << endl;
#endif
                NextExp(&expr);
                stmt1->expr = expr;
            }
        }
    } else {
        if (sym == SEMICN) {
            stmt1->stmtType = EMPTY;
            getSym();
        } else {
            stmt1->stmtType = EXP;
            Expr* expr = Exp();
            stmt1->expr = expr;
            if (sym == SEMICN) {
                getSym();
            } else {
                error(temp_line, 'i');
            }
        }
    }
#ifdef OUTPUT_Parser
    outfile << "<Stmt>" << endl;
#endif
    return stmt1;
}

void NextExp(Expr **pExpr) {
    int temp_sym;
    if (sym == PLUS || sym == MINU) {
        temp_sym = sym;
#ifdef OUTPUT_Parser
        outfile << "<MulExp>" << endl;
        outfile << "<AddExp>" << endl;
#endif
        getSym();
        Expr* expr1 = Exp();
        if (temp_sym == PLUS) {
            (*pExpr) = new Expr(binaryExpr, new BinaryExpr(Add, *pExpr, expr1));
        } else {
            (*pExpr) = new Expr(binaryExpr, new BinaryExpr(Sub, *pExpr, expr1));
        }
        if (sym == SEMICN) {
            getSym();
        } else {
            error(temp_line, 'i');
        }
    } else if (sym == MULT || sym == DIV || sym == MOD) {
        temp_sym = sym;
#ifdef OUTPUT_Parser
        outfile << "<MulExp>" << endl;
#endif
        getSym();
        Expr* expr1 = Exp();
        if (temp_sym == MULT) {
            (*pExpr) = new Expr(binaryExpr, new BinaryExpr(Mul, *pExpr, expr1));
        } else if (temp_sym == DIV) {
            (*pExpr) = new Expr(binaryExpr, new BinaryExpr(Div, *pExpr, expr1));
        } else {
            (*pExpr) = new Expr(binaryExpr, new BinaryExpr(Mod, *pExpr, expr1));
        }
        if (sym == SEMICN) {
            getSym();
        } else {
            error(temp_line, 'i');
        }
    } else if (sym == SEMICN) {
#ifdef OUTPUT_Parser
        outfile << "<MulExp>" << endl;
        outfile << "<AddExp>" << endl;
        outfile << "<Exp>" << endl;
#endif
        getSym();
    } else {
        error(temp_line, 'i');
    }
}

Condition * Cond() {
    Condition* condition = LOrExp();
#ifdef OUTPUT_Parser
    outfile << "<Cond>" << endl;
#endif
    return condition;
}

Condition * LOrExp() {
    Condition* cond1 = LAndExp();
    Condition* cond2;
    while (sym == OR) {
#ifdef OUTPUT_Parser
        outfile << "<LOrExp>" << endl;
#endif
        getSym();
        cond2 = LAndExp();
        cond1 = new Condition(true, cond1, cond2, Or);
    }
#ifdef OUTPUT_Parser
    outfile << "<LOrExp>" << endl;
#endif
    return cond1;
}

Condition * LAndExp() {
    Condition* cond1 = EqExp();
    Condition* cond2;

    while (sym == AND) {
#ifdef OUTPUT_Parser
        outfile << "<LAndExp>" << endl;
#endif
        getSym();
        cond2 = EqExp();
        cond1 = new Condition(true, cond1, cond2, And);
    }
#ifdef OUTPUT_Parser
    outfile << "<LAndExp>" << endl;
#endif
    return cond1;
}

Condition * EqExp() {
    Condition* cond1 = RelExp();
    Condition* cond2;
    while (sym == EQL || sym == NEQ) {
#ifdef OUTPUT_Parser
        outfile << "<EqExp>" << endl;
#endif
        int temp_sym = sym;
        getSym();
        cond2 = RelExp();
        if (temp_sym == EQL) {
            cond1 = new Condition(true, cond1, cond2, Eql);
        } else {
            cond1 = new Condition(true, cond1, cond2, Neq);
        }
    }
#ifdef OUTPUT_Parser
    outfile << "<EqExp>" << endl;
#endif
    return cond1;
}

Condition * RelExp() {
    Expr* expr1 = AddExp();
    Expr* expr2;
    auto* cond = new Condition(false, expr1);

    while (sym == LSS || sym == GRE || sym == LEQ || sym == GEQ) {
#ifdef OUTPUT_Parser
        outfile << "<RelExp>" << endl;
#endif
        int temp_sym = sym;
        getSym();
        expr2 = AddExp();
        if (temp_sym == LSS) {
            cond = new Condition(true, cond, new Condition(false, expr2), Lss);
        } else if (temp_sym == GRE) {
            cond = new Condition(true, cond, new Condition(false, expr2), Gre);
        } else if (temp_sym == LEQ) {
            cond = new Condition(true, cond, new Condition(false, expr2), Leq);
        } else {
            cond = new Condition(true, cond, new Condition(false, expr2), Geq);
        }
    }
#ifdef OUTPUT_Parser
    outfile << "<RelExp>" << endl;
#endif
    return cond;
}