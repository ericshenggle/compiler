//
// Created by ycg on 2021/11/2.
//

#include "include/ast.h"

#define register_num 8

int temp[register_num];
deque<vector<string>> midCode;/* NOLINT */
int loop_statement = 0;
int if_statement = 0;
int print = 0;
int reg = 0;
int label = 0;
bool OPTIMIZE = true;
ofstream midCodefile;/* NOLINT */
vector<int> loop;

int toInt(const string& i) {             //字符串转int
    char *end = nullptr;
    int intStr = strtol(i.c_str(), &end, 10);
    return intStr;
}

void midCodeFile() {
    midCodefile.open("midCode.txt", std::ios::trunc);
    if (!midCodefile.is_open())
    {
        std::cerr << "Open midCode failed" << endl;
        exit(-1);
    }
}

int getTemp() {                         // 当寄存器不够时超时
    while (temp[reg] != 0) {
        reg++;
        reg %= register_num;
    }
    temp[reg] = 1;
    return reg;
}

initVal_class::~initVal_class() {
    for (const auto &i : initVals) {
        delete i;
    }
}

SubDecl::~SubDecl() {
    delete dim;
    dim = nullptr;
    delete initVal;
    initVal = nullptr;
}

void SubDecl::mid_code(bool is_const) {
    Symbol* pid = this->init(is_const, is_global);
    if (is_const) {
        if (dim == nullptr) {
            vector<string> code;
            code.emplace_back("const");
            code.emplace_back("int");
            code.emplace_back(ident);
            code.emplace_back("=");
            code.emplace_back(std::to_string(pid->value));
            midCode.emplace_back(code);
        } else {
            vector<string> code;
            code.emplace_back("const");
            code.emplace_back("arrayInt");
            code.emplace_back(ident);
            Array_data *p = ((Array_data*) (pid->data));
            if (pid->dimension == 1) {
                code.emplace_back(std::to_string(p->length[0]));
                midCode.emplace_back(code);
            } else {
                code.emplace_back(std::to_string(p->length[0]*p->length[1]));
                midCode.emplace_back(code);
            }
            for (int i = 0; i < p->var.size(); ++i) {
                vector<string> code1;
                code1.emplace_back(ident + "[" + std::to_string(i) + "]");
                code1.emplace_back("=");
                code1.emplace_back(std::to_string(p->var[i]));
                midCode.emplace_back(code1);
            }
        }
    } else {
        if (is_global) {
            if (dim == nullptr) {
                vector<string> code;
                code.emplace_back("var");
                code.emplace_back("int");
                code.emplace_back(ident);
                code.emplace_back("=");
                code.emplace_back(std::to_string(pid->value));
                midCode.emplace_back(code);
            } else {
                Array_data *p = ((Array_data*) (pid->data));
                vector<string> code;
                code.emplace_back("var");
                code.emplace_back("arrayInt");
                code.emplace_back(ident);
                if (pid->dimension == 1) {
                    code.emplace_back(std::to_string(p->length[0]));
                    midCode.emplace_back(code);
                } else {
                    code.emplace_back(std::to_string(p->length[0]*p->length[1]));
                    midCode.emplace_back(code);
                }
                if (initVal != nullptr) {
                    for (int i = 0; i < p->var.size(); ++i) {
                        vector<string> code1;
                        code1.emplace_back(ident + "[" + std::to_string(i) + "]");
                        code1.emplace_back("=");
                        code1.emplace_back(std::to_string(p->var[i]));
                        midCode.emplace_back(code1);
                    }
                }
            }
        } else {
            if (dim == nullptr) {
                if (initVal != nullptr) {
                    if (initVal->is_expr) {
                        int a = -1;
                        string i = initVal->expr->mid_code(&a);
                        if (a != -1) {
                            temp[a] = 0;
                        }
                        vector<string> code;
                        code.emplace_back("var");
                        code.emplace_back("int");
                        code.emplace_back(ident);
                        code.emplace_back("=");
                        code.emplace_back(i);
                        midCode.emplace_back(code);
                    }
                } else {
                    vector<string> code;
                    code.emplace_back("var");
                    code.emplace_back("int");
                    code.emplace_back(ident);
                    midCode.emplace_back(code);
                }
            } else {
                Array_data *p = ((Array_data*) (pid->data));
                vector<string> code;
                code.emplace_back("var");
                code.emplace_back("arrayInt");
                code.emplace_back(ident);
                if (pid->dimension == 1) {
                    int n = 0;
                    code.emplace_back(std::to_string(p->length[0]));
                    midCode.emplace_back(code);
                    if (initVal != nullptr) {
                        for (const auto &i : initVal->initVals) {
                            if (i->is_expr) {
                                int a = -1;
                                string j = i->expr->mid_code(&a);
                                if (a != -1) {
                                    temp[a] = 0;
                                }
                                vector<string> code1;
                                code1.emplace_back(ident + "[" + std::to_string(n++) + "]");
                                code1.emplace_back("=");
                                code1.emplace_back(j);
                                midCode.emplace_back(code1);
                            }
                        }
                    }
                } else {
                    code.emplace_back(std::to_string(p->length[0]*p->length[1]));
                    midCode.emplace_back(code);
                    int n = 0;
                    if (initVal != nullptr) {
                        for (const auto &i : initVal->initVals) {
                            for (const auto &j : i->initVals) {
                                if (j->is_expr) {
                                    int a = -1;
                                    string k = j->expr->mid_code(&a);
                                    if (a != -1) {
                                        temp[a] = 0;
                                    }
                                    vector<string> code1;
                                    code1.emplace_back(ident + "[" + std::to_string(n++) + "]");
                                    code1.emplace_back("=");
                                    code1.emplace_back(k);
                                    midCode.emplace_back(code1);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

Symbol * SubDecl::init(bool is_const, bool global) const {
    if (global) {
        Symbol* pid = symbolTable_Var->find(ident);
        if (pid->type == const_t) {
            if (initVal != nullptr && initVal->is_expr) {
                pid->value = initVal->expr->returnValue();
            } else {
                pid->value = 0;
            }
        } else if (pid->type == const_array_t){
            Array_data *p = ((Array_data*) (pid->data));
            if (pid->dimension == 1) {
                p->length[0] = dim->exprs[0]->returnValue();
                for (int i = 0; i < p->length[0]; ++i) {
                    p->var.emplace_back(0);
                }
                int i = 0;
                if (initVal != nullptr) {
                    for (const auto &j: initVal->initVals) {
                        p->var[i] = j->expr->returnValue();
                        i++;
                    }
                }
            } else {
                p->length[0] = dim->exprs[0]->returnValue();
                p->length[1] = dim->exprs[1]->returnValue();
                for (int i = 0; i < p->length[0]*p->length[1]; ++i) {
                    p->var.emplace_back(0);
                }
                int k = 0;
                if (initVal != nullptr) {
                    for (const auto &i: initVal->initVals) {
                        for (const auto &j: i->initVals) {
                            p->var[k] = j->expr->returnValue();
                            k++;
                        }
                    }
                }
            }
        } else if (pid->type == int_t) {
            if (initVal != nullptr && initVal->is_expr) {
                pid->value = initVal->expr->returnValue();
            } else {
                pid->value = 0;
            }
        } else if (pid->type == int_array_t) {
            Array_data *p = ((Array_data*) (pid->data));
            if (pid->dimension == 1) {
                p->length[0] = dim->exprs[0]->returnValue();
                for (int i = 0; i < p->length[0]; ++i) {
                    p->var.emplace_back(0);
                }
                int i = 0;
                if (initVal != nullptr) {
                    for (const auto &j: initVal->initVals) {
                        p->var[i] = j->expr->returnValue();
                        i++;
                    }
                }
            } else {
                p->length[0] = dim->exprs[0]->returnValue();
                p->length[1] = dim->exprs[1]->returnValue();
                for (int i = 0; i < p->length[0]*p->length[1]; ++i) {
                    p->var.emplace_back(0);
                }
                int k = 0;
                if (initVal != nullptr) {
                    for (const auto &i: initVal->initVals) {
                        for (const auto &j: i->initVals) {
                            p->var[k] = j->expr->returnValue();
                            k++;
                        }
                    }
                }
            }
        }
        return pid;
    } else {
        auto temp_symbol = new Symbol();
        temp_symbol->str = ident;
        if (is_const) {
            if (dim == nullptr) {
                temp_symbol->type = const_t;
                if (initVal != nullptr && initVal->is_expr) {
                    temp_symbol->value = initVal->expr->returnValue();
                }
                temp_symbol->dimension = 0;
            } else {
                auto p = new Array_data();
                temp_symbol->type = const_array_t;
                temp_symbol->data = p;
                if (dim->exprs.size() == 1) {
                    temp_symbol->dimension = 1;
                    p->length[0] = dim->exprs[0]->returnValue();
                    if (initVal != nullptr) {
                        for (const auto &i: initVal->initVals) {
                            p->var.emplace_back(i->expr->returnValue());
                        }
                    }
                } else {
                    temp_symbol->dimension = 2;
                    p->length[0] = dim->exprs[0]->returnValue();
                    p->length[1] = dim->exprs[1]->returnValue();
                    if (initVal != nullptr) {
                        for (const auto &i: initVal->initVals) {
                            for (const auto &j: i->initVals) {
                                p->var.emplace_back(j->expr->returnValue());
                            }
                        }
                    }
                }
            }
        } else {
            if (dim == nullptr) {
                temp_symbol->type = int_t;
                temp_symbol->dimension = 0;
            } else {
                auto p = new Array_data();
                temp_symbol->type = int_array_t;
                temp_symbol->data = p;
                if (dim->exprs.size() == 1) {
                    temp_symbol->dimension = 1;
                    p->length[0] = dim->exprs[0]->returnValue();
                } else {
                    temp_symbol->dimension = 2;
                    p->length[0] = dim->exprs[0]->returnValue();
                    p->length[1] = dim->exprs[1]->returnValue();
                }
            }
        }
        symbolTable_Var->insert(temp_symbol);
        return temp_symbol;
    }
    
}

decl_class::~decl_class() {
    for (const auto &i: subDecls) {
        delete i;
    }
}

void decl_class::mid_code() {
    for (const auto &i : subDecls) {
        i->mid_code(is_const);
    }
}

compUnit::~compUnit() {
    for (const auto &i: decls) {
        delete i;
    }
    for (const auto &i: funDefs) {
        delete i;
    }
}

void compUnit::mid_code() {
    for (const auto &i : decls) {
        i->mid_code();
    }
    for (const auto &i : funDefs) {
        i->mid_code();
    }
#ifdef OUTPUT_MidCode
    midCodeFile();
    for (const auto& i : midCode) {
        for (const auto& j : i) {
            midCodefile << j << " ";
        }
        midCodefile << endl;
    }
#endif
}


Expr::~Expr() {
    delete lVal;
    lVal = nullptr;
    delete assignExpr;
    assignExpr = nullptr;
    delete unaryExpr;
    unaryExpr = nullptr;
    delete binaryExpr;
    binaryExpr = nullptr;
    delete callExpr;
    callExpr = nullptr;
    delete literalExpr;
    literalExpr = nullptr;
}

std::basic_string<char> Expr::mid_code(int *pInt) const {
    switch (exprType) {
        case ::literalExpr:
            return literalExpr->mid_code();
        case ::unaryExpr:
            return unaryExpr->mid_code(pInt);
        case ::binaryExpr:
            return binaryExpr->mid_code(pInt);
        case ::callExpr:
            return callExpr->mid_code(pInt);
        case ::lval:
            return lVal->mid_code(pInt);
        case ::assignExpr:
        case ::empty:
            return string("null");
    }
    return string("null");
}

int Expr::returnValue() const {
    switch (exprType) {
        case ::literalExpr:
            return literalExpr->value;
        case ::unaryExpr:
            return unaryExpr->returnValue();
        case ::binaryExpr:
            return binaryExpr->returnValue();
        case ::lval:
            return lVal->returnValue();
        case ::callExpr:
        case ::assignExpr:
        case ::empty:
            break;
    }
    return 0;
}

lVal_class::~lVal_class() {
    delete dim;
    dim = nullptr;
}

std::basic_string<char> lVal_class::mid_code(int *pInt) const {
    Symbol* pid = symbolTable_Var->find(ident);
    if (pid->dimension == 0) {
        return ident;
    } else {
        if (pid->dimension == 1) {
            if (dim->exprs.size() == 1) {
                string str = dim->exprs[0]->mid_code(pInt);
                if (isdigit(str[0])) {
                    return ident + "[" + str + "]";
                } else {
                    int t1 = getTemp();
                    string str1 = "t" + std::to_string(t1);
                    if (*pInt != -1) {
                        temp[*pInt] = 0;
                    }
                    *pInt = t1;
                    vector<string> code;
                    code.emplace_back(str1);
                    code.emplace_back("=");
                    code.emplace_back(str);
                    code.emplace_back("*");
                    code.emplace_back("4");
                    midCode.emplace_back(code);
                    return ident + "[" + str1 + "]";
                }
            } else {
                return ident;
            }
        } else {
            if (!dim->exprs.empty()) {
                Array_data *p = ((Array_data*) (pid->data));
                int a = -1;
                string i = dim->exprs[0]->mid_code(&a);
                int t1 = getTemp();
                string str = "t" + std::to_string(t1);
                if (a != -1) {
                    temp[a] = 0;
                }
                vector<string> code;
                code.emplace_back(str);
                code.emplace_back("=");
                code.emplace_back(i);
                code.emplace_back("*");
                code.emplace_back(std::to_string(p->length[1]));
                midCode.emplace_back(code);
                if (dim->exprs.size() == 2) {
                    int b = -1;
                    string j = dim->exprs[1]->mid_code(&b);

                    int t = getTemp();
                    string str1 = "t" + std::to_string(t);
                    if (b != -1) {
                        temp[b] = 0;
                    }
                    temp[t1] = 0;
                    vector<string> code1;
                    code1.emplace_back(str1);
                    code1.emplace_back("=");
                    code1.emplace_back(str);
                    code1.emplace_back("+");
                    code1.emplace_back(j);
                    midCode.emplace_back(code1);
                    *pInt = getTemp();
                    string str2 = "t" + std::to_string(*pInt);
                    temp[t] = 0;
                    vector<string> code2;
                    code2.emplace_back(str2);
                    code2.emplace_back("=");
                    code2.emplace_back(str1);
                    code2.emplace_back("*");
                    code2.emplace_back("4");
                    midCode.emplace_back(code2);
                    return ident + "[" + str2 + "]";
                } else {
                    int t2 = getTemp();
                    string str1 = "t" + std::to_string(t2);
                    if (t1 != -1) {
                        temp[t1] = 0;
                    }
                    *pInt = t2;
                    vector<string> code1;
                    code1.emplace_back(str1);
                    code1.emplace_back("=");
                    code1.emplace_back(str);
                    code1.emplace_back("*");
                    code1.emplace_back("4");
                    midCode.emplace_back(code1);
                    return ident + "[" + str1 + "]";
                }
            } else {
                return ident;
            }
        }
    }
}

int lVal_class::returnValue() const {
    Symbol* pid = symbolTable_Var->find(ident);
    if (pid->type == const_t || pid->type == int_t) {
        return pid->value;
    } else if (pid->type == const_array_t){
        Array_data *p = ((Array_data*) (pid->data));
        if (pid->dimension == 1) {
            return p->var[dim->exprs[0]->returnValue()];
        } else {
            return p->var[dim->exprs[0]->returnValue() * p->length[1] + dim->exprs[1]->returnValue()];
        }
    } else if (pid->type == int_array_t) {
        Array_data *p = ((Array_data*) (pid->data));
        if (!p->var.empty()) {
            if (pid->dimension == 1) {
                return p->var[dim->exprs[0]->returnValue()];
            } else {
                return p->var[dim->exprs[0]->returnValue() * p->length[1] + dim->exprs[1]->returnValue()];
            }
        }
    }
    return 0;
}

AssignExpr::~AssignExpr() {
    delete l;
    l = nullptr;
    delete r;
    r = nullptr;
}

void AssignExpr::mid_code() const {
    int a = -1, b = -1;
    string i = l->mid_code(&a);
    vector<string> code;
    code.emplace_back(i);
    code.emplace_back("=");
    if (r != nullptr) {
        string j = r->mid_code(&b);
        code.emplace_back(j);
        if (b != -1) {
            temp[b] = 0;
        }
    } else {
        code.emplace_back("getint");
    }
    if (a != -1) {
        temp[a] = 0;
    }
    midCode.emplace_back(code);
}

UnaryExpr::~UnaryExpr() {
    delete expr;
    expr = nullptr;
}

std::basic_string<char> UnaryExpr::mid_code(int *pInt) const {
    if (unaryOp == Neg) {
        vector<string> code;
        int a = -1;
        string i = expr->mid_code(&a);
        *pInt = getTemp();
        string str = "t" + std::to_string(*pInt);
        if (a != -1) {
            temp[a] = 0;
        }
        code.emplace_back(str);
        code.emplace_back("=");
        code.emplace_back("-");
        code.emplace_back(i);
        midCode.emplace_back(code);
        return str;
    } else if (unaryOp == Not) {
        vector<string> code;
        int a = -1;
        string i = expr->mid_code(&a);
        *pInt = getTemp();
        string str = "t" + std::to_string(*pInt);
        if (a != -1) {
            temp[a] = 0;
        }
        code.emplace_back(str);
        code.emplace_back("=");
        code.emplace_back("!");
        code.emplace_back(i);
        midCode.emplace_back(code);
        return str;
    } else {
        return expr->mid_code(pInt);
    }
}

int UnaryExpr::returnValue() const {
    if (unaryOp == Neg) {
        return -expr->returnValue();
    } else if (unaryOp == Not) {
        return !expr->returnValue();
    } else {
        return expr->returnValue();
    }
}

BinaryExpr::~BinaryExpr() {
    delete l;
    l = nullptr;
    delete r;
    r = nullptr;
}

std::basic_string<char> BinaryExpr::mid_code(int *pInt) const {
    vector<string> code;
    int a = -1, b = -1;
    string i = l->mid_code(&a);
    string j = r->mid_code(&b);
    if ((i[0] == '-' || isdigit(i[0])) && (j[0] == '-' || isdigit(j[0])) && OPTIMIZE) {
        int k;
        switch (binaryOp) {
            case Add:
                k = toInt(i) + toInt(j);
                break;
            case Sub:
                k = toInt(i) - toInt(j);
                break;
            case Mul:
                k = toInt(i) * toInt(j);
                break;
            case Div:
                k = toInt(i) / toInt(j);
                break;
            case Mod:
                k = toInt(i) % toInt(j);
                break;
        }
        return std::to_string(k);
    } else {
        *pInt = getTemp();
        string str = "t" + std::to_string(*pInt);
        if (a != -1) {
            temp[a] = 0;
        }
        if (b != -1) {
            temp[b] = 0;
        }
        code.emplace_back(str);
        code.emplace_back("=");
        code.emplace_back(i);
        switch (binaryOp) {
            case Add:
                code.emplace_back("+");
                break;
            case Sub:
                code.emplace_back("-");
                break;
            case Mul:
                code.emplace_back("*");
                break;
            case Div:
                code.emplace_back("/");
                break;
            case Mod:
                code.emplace_back("%");
                break;
        }
        code.emplace_back(j);
        midCode.emplace_back(code);
        return str;
    }
}

int BinaryExpr::returnValue() const {
    switch (binaryOp) {
        case Add:
            return l->returnValue() + r->returnValue();
        case Sub:
            return l->returnValue() - r->returnValue();
        case Mul:
            return l->returnValue() * r->returnValue();
        case Div:
            return l->returnValue() / r->returnValue();
        case Mod:
            return l->returnValue() % r->returnValue();
    }
    return 0;
}

CallExpr::~CallExpr() {
    for (const auto &i : params) {
        delete i;
    }
}

std::basic_string<char> CallExpr::mid_code(int *pInt) const {
    Symbol *pid = symbolTable_Func->find(ident);
    Func_data *p = ((Func_data *) (pid->data));
    int num = 0;

    vector<vector<string>> midCode1;
    for (const auto &i: params) {
        if (i->exprType == callExpr) {
            vector<string> code1;
            code1.emplace_back("push");
            int a = -1;
            string str = i->mid_code(&a);
            code1.emplace_back(str);
            code1.emplace_back(std::to_string(a));
            if (p->params[num] == int_param) {
                code1.emplace_back("int");
            } else {
                code1.emplace_back("arrayInt");
            }
            code1.emplace_back(std::to_string(num));
            midCode1.emplace_back(code1);
        }
        num++;
    }
    num = 0;
    for (const auto &i: params) {
        if (i->exprType != callExpr) {
            vector<string> code;
            code.emplace_back("push");
            int a = -1;
            string str = i->mid_code(&a);
            code.emplace_back(str);
            if (a != -1) {
                temp[a] = 0;
            }
            if (isdigit(str[0]) || str[0] == '-') {
                code.emplace_back("number");
            } else if (p->params[num] == int_param) {
                code.emplace_back("int");
            } else {
                code.emplace_back("arrayInt");
            }
            code.emplace_back(std::to_string(num));
            midCode.emplace_back(code);
        } else {
            if (isdigit(midCode1.front()[2][0])) {
                int a = midCode1.front()[2][0] - '0';
                temp[a] = 0;
            }
            vector<string> code1 = midCode1.front();
            code1.erase(code1.begin() + 2, code1.begin() + 3);
            midCode.emplace_back(code1);
            midCode1.erase(midCode1.begin(), midCode1.begin() + 1);
        }
        num++;
    }
    vector<string> code1;
    code1.emplace_back("call");
    code1.emplace_back(ident);
    midCode.emplace_back(code1);
    if (pid->type == function_int_t) {
        vector<string> code2;
        *pInt = getTemp();
        string str = "t" + std::to_string(*pInt);
        code2.emplace_back(str);
        code2.emplace_back("=");
        code2.emplace_back("RET");
        midCode.emplace_back(code2);
        return str;
    } else {
        return "null";
    }
}

FuncParam::~FuncParam() {
    delete dim;
    dim = nullptr;
}

void FuncParam::mid_code() const {
    vector<string> code;
    auto pid = new Symbol();
    code.emplace_back("param");
    pid->type = int_t;
    pid->str = ident;
    pid->dimension = 0;
    if (dim != nullptr) {
        code.emplace_back("arrayInt");
        pid->type = int_array_t;
        auto *p = new Array_data();
        pid->data = p;
        if (dim->exprs.empty()) {
            pid->dimension = 1;
        } else {
            pid->dimension = 2;
            p->length[1] = dim->exprs[0]->returnValue();
        }
    } else {
        switch (paramType) {
            case INT:
                code.emplace_back("int");
                break;
            case VOID:
                code.emplace_back("void");
                break;
        }
    }
    code.emplace_back(ident);
    midCode.emplace_back(code);
    symbolTable_Var->insert(pid);
}

funcDef::~funcDef() {
    delete body;
    body = nullptr;
    for (const auto &i : params) {
        delete i;
    }
}

void funcDef::mid_code() {
    symbolTable_Var->push();
    vector<string> code;
    switch (funcType) {
        case INT:
            code.emplace_back("int");
            break;
        case VOID:
            code.emplace_back("void");
            break;
    }
    code.emplace_back(ident);
    code.emplace_back(std::to_string(params.size()));
    midCode.emplace_back(code);

    for (const auto &i: params) {
        i->mid_code();
    }
    body->mid_code(false);
    vector<string> code1;
    code1.emplace_back("endFunc");
    midCode.emplace_back(code1);
    symbolTable_Var->pop();
}

blockItem::~blockItem() {
    delete stmt;
    stmt = nullptr;
    delete decl;
    decl = nullptr;
}

void blockItem::mid_code() const {
    if (isStmt) {
        stmt->mid_code(true);
    } else {
        decl->mid_code();
    }
}

IfStmt::~IfStmt() {
    delete then_block;
    then_block = nullptr;
    delete else_block;
    else_block = nullptr;
}

void IfStmt::mid_code() const {
    if_statement++;
    vector<string> code;
    code.emplace_back("if");
    code.emplace_back("if_begin_" + std::to_string(if_statement) + ":");
    midCode.emplace_back(code);
    string else_if = "if_else_" + std::to_string(if_statement);
    string end_if = "if_end_" + std::to_string(if_statement);
    int a = -1;
    bool isLabel = false;
    string str = cond->mid_code(&a);
    if (cond->condOp == Or) {
        vector<string> code1;
        code1.emplace_back("goto");
        if (else_block != nullptr) {
            code1.emplace_back(else_if);
        } else {
            code1.emplace_back(end_if);
        }
        midCode.emplace_back(code1);
        vector<string> code2;
        code2.emplace_back(str + ":");
        midCode.emplace_back(code2);
        then_block->mid_code(false);
    } else if (cond->condOp == And) {
        then_block->mid_code(false);
        isLabel = true;
    } else {
        vector<string> code1;
        code1.emplace_back("beqz");
        code1.emplace_back(str);
        if (else_block != nullptr) {
            code1.emplace_back(else_if);
        } else {
            code1.emplace_back(end_if);
        }
        midCode.emplace_back(code1);
        if (a != -1) {
            temp[a] = 0;
        }
        then_block->mid_code(false);
    }
    if (else_block != nullptr) {
        vector<string> code1;
        code1.emplace_back("goto");
        code1.emplace_back(end_if);
        midCode.emplace_back(code1);
        vector<string> code2;
        code2.emplace_back("else");
        code2.emplace_back(else_if + ":");
        midCode.emplace_back(code2);
        if (isLabel) {
            vector<string> code3;
            code3.emplace_back(str + ":");
            midCode.emplace_back(code3);
        }
        else_block->mid_code(false);
        vector<string> code3;
        code3.emplace_back("end");
        code3.emplace_back(end_if + ":");
        midCode.emplace_back(code3);
    } else {
        if (isLabel) {
            vector<string> code2;
            code2.emplace_back(str + ":");
            midCode.emplace_back(code2);
        }
        vector<string> code1;
        code1.emplace_back("end");
        code1.emplace_back(end_if + ":");
        midCode.emplace_back(code1);
    }
}

BlockStmt::~BlockStmt() {
    for (const auto &i: blockItems) {
        delete i;
    }
}

void BlockStmt::mid_code(bool isFuncBlock) {
    symbolTable_Var->push();
    if (isFuncBlock) {
        vector<string> code;
        code.emplace_back("beginBlock");
        midCode.emplace_back(code);
        for (const auto &i: blockItems) {
            i->mid_code();
        }
        vector<string> code1;
        code1.emplace_back("endBlock");
        midCode.emplace_back(code1);
    } else {
        for (const auto &i: blockItems) {
            i->mid_code();
        }
    }
    symbolTable_Var->pop();
}

WhileStmt::~WhileStmt() {
    delete body;
    body = nullptr;
}

void WhileStmt::mid_code() const {
    loop_statement++;
    loop.emplace_back(loop_statement);
    vector<string> code;
    string begin_while = "loop_begin_" + std::to_string(loop_statement);
    code.emplace_back("while");
    code.emplace_back(begin_while + ":");
    midCode.emplace_back(code);
    string end_while = "loop_end_" + std::to_string(loop_statement);
    int a = -1;
    bool isLabel = false;
    string str = cond->mid_code(&a);
    if (cond->condOp == Or) {
        vector<string> code1;
        code1.emplace_back("goto");
        code1.emplace_back(end_while);
        midCode.emplace_back(code1);
        vector<string> code2;
        code2.emplace_back(str + ":");
        midCode.emplace_back(code2);
        body->mid_code(false);
    } else if (cond->condOp == And) {
        body->mid_code(false);
        isLabel = true;
    } else {
        vector<string> code1;
        code1.emplace_back("beqz");
        code1.emplace_back(str);
        code1.emplace_back(end_while);
        midCode.emplace_back(code1);
        if (a != -1) {
            temp[a] = 0;
        }
        body->mid_code(false);
    }
    vector<string> code2;
    code2.emplace_back("goto");
    code2.emplace_back(begin_while);
    midCode.emplace_back(code2);
    if (isLabel) {
        vector<string> code3;
        code3.emplace_back(str + ":");
        midCode.emplace_back(code3);
    }
    vector<string> code3;
    code3.emplace_back("end");
    code3.emplace_back(end_while + ":");
    midCode.emplace_back(code3);
    loop.pop_back();
}

ReturnStmt::~ReturnStmt() {
    delete expr;
    expr = nullptr;
}

void ReturnStmt::mid_code() const {
    vector<string> code;
    if (expr != nullptr) {
        int a = -1;
        string str = expr->mid_code(&a);
        if (a != -1) {
            temp[a] = 0;
        }
        code.emplace_back("ret");
        code.emplace_back(str);
    } else {
        code.emplace_back("ret");
    }
    midCode.emplace_back(code);
}

PrintStmt::~PrintStmt() {
    for (const auto &i: exprs) {
        delete i;
    }
}

void CutString(string line, char a, vector<Expr*> &exprs)
{
    //首字母为a，剔除首字母
    if (line.empty())
    {
        return;
    }
    int i = 0;
    size_t pos = 0;
    line.erase(line.begin());
    line.erase(line.end() - 1);
    if (line[0] == a)
    {
        line.erase(0, 2);
        vector<string> code;
        code.emplace_back("output");
        int b = -1;
        code.emplace_back(exprs[i]->mid_code(&b));
        if (b != -1) {
            temp[b] = 0;
        }
        midCode.emplace_back(code);
        i++;
    }

    while (pos < line.length())
    {
        size_t curpos = pos;
        pos = line.find(a, curpos);
        if (pos == string::npos)
        {
            pos = line.length();
            vector<string> code;
            code.emplace_back("string");
            string str = "print_" + std::to_string(print) + "_" + std::to_string(i);
            code.emplace_back(str);
            code.emplace_back("\"" + line.substr(curpos, pos - curpos) + "\"");
            midCode.emplace_front(code);
            vector<string> code1;
            code1.emplace_back("output");
            code1.emplace_back("string");
            code1.emplace_back(str);
            midCode.emplace_back(code1);
        } else {
            vector<string> code;
            code.emplace_back("string");
            string str = "print_" + std::to_string(print) + "_" + std::to_string(i);
            code.emplace_back(str);
            code.emplace_back("\"" + line.substr(curpos, pos - curpos) + "\"");
            midCode.emplace_front(code);
            vector<string> code1;
            code1.emplace_back("output");
            code1.emplace_back("string");
            code1.emplace_back(str);
            midCode.emplace_back(code1);
            vector<string> code2;
            code2.emplace_back("output");
            int b = -1;
            code2.emplace_back(exprs[i]->mid_code(&b));
            if (b != -1) {
                temp[b] = 0;
            }
            midCode.emplace_back(code2);
        }
        pos += 2;
        i++;
    }
}

void PrintStmt::mid_code() {
    print++;
    CutString(formatString, '%', exprs);
}

stmt_class::~stmt_class() {
    delete expr;
    expr = nullptr;
    delete blockStmt;
    blockStmt = nullptr;
    delete ifStmt;
    ifStmt = nullptr;
    delete whileStmt;
    whileStmt = nullptr;
    delete returnStmt;
    returnStmt = nullptr;
    delete printStmt;
    printStmt = nullptr;
}

void stmt_class::mid_code(bool isFuncBlock) const {
    switch (stmtType) {
        case LVALASSIGN:
        case LVALGETINT:
            expr->assignExpr->mid_code();
            break;
        case EXP: {
            int a = -1;
            expr->mid_code(&a);
            if (a != -1) {
                temp[a] = 0;
            }
            break;
        }
        case EMPTY:
            break;
        case BLOCK:
            blockStmt->mid_code(isFuncBlock);
            break;
        case IFSTMT:
            ifStmt->mid_code();
            break;
        case WHILESTMT:
            whileStmt->mid_code();
            break;
        case BREAK:{
            vector<string> code;
            code.emplace_back("goto");
            code.emplace_back("loop_end_" + std::to_string(loop.back()));
            midCode.emplace_back(code);
            break;
        }
        case CONTINUE:{
            vector<string> code;
            code.emplace_back("goto");
            code.emplace_back("loop_begin_" + std::to_string(loop.back()));
            midCode.emplace_back(code);
            break;
        }
        case RETURNSTMT:
            returnStmt->mid_code();
            break;
        case PRINTSTMT:
            printStmt->mid_code();
            break;
    }
}

std::basic_string<char> LiteralExpr::mid_code() const {
    return std::to_string(value);
}

Condition::~Condition() {
    delete l;
    l = nullptr;
    delete r;
    r = nullptr;
    delete expr;
    expr = nullptr;
}

std::basic_string<char> Condition::mid_code(int *pInt) const {
    switch (condOp) {
        case Gre:{
            int a = -1, b= -1;
            string i = l->mid_code(&a);
            string j = r->mid_code(&b);
            int t = getTemp();
            if (pInt != nullptr) {
                *pInt = t;
            }
            string str = "t" + std::to_string(t);
            vector<string> code;
            code.emplace_back("slt");
            code.emplace_back(str);
            code.emplace_back(j);
            code.emplace_back(i);
            midCode.emplace_back(code);
            if (a != -1) {
                temp[a] = 0;
            }
            if (b != -1) {
                temp[b] = 0;
            }
            return str;
        }
        case Lss:{
            int a = -1, b= -1;
            string i = l->mid_code(&a);
            string j = r->mid_code(&b);
            int t = getTemp();
            if (pInt != nullptr) {
                *pInt = t;
            }
            string str = "t" + std::to_string(t);
            vector<string> code;
            code.emplace_back("slt");
            code.emplace_back(str);
            code.emplace_back(i);
            code.emplace_back(j);
            midCode.emplace_back(code);
            if (a != -1) {
                temp[a] = 0;
            }
            if (b != -1) {
                temp[b] = 0;
            }
            return str;
        }
        case Geq:{
            int a = -1, b= -1;
            string i = l->mid_code(&a);
            string j = r->mid_code(&b);
            int t = getTemp();
            if (pInt != nullptr) {
                *pInt = t;
            }
            string str = "t" + std::to_string(t);
            vector<string> code;
            code.emplace_back("sle");
            code.emplace_back(str);
            code.emplace_back(j);
            code.emplace_back(i);
            midCode.emplace_back(code);
            if (a != -1) {
                temp[a] = 0;
            }
            if (b != -1) {
                temp[b] = 0;
            }
            return str;
        }
        case Leq:{
            int a = -1, b= -1;
            string i = l->mid_code(&a);
            string j = r->mid_code(&b);
            int t = getTemp();
            if (pInt != nullptr) {
                *pInt = t;
            }
            string str = "t" + std::to_string(t);
            vector<string> code;
            code.emplace_back("sle");
            code.emplace_back(str);
            code.emplace_back(i);
            code.emplace_back(j);
            midCode.emplace_back(code);
            if (a != -1) {
                temp[a] = 0;
            }
            if (b != -1) {
                temp[b] = 0;
            }
            return str;
        }
        case Eql:{
            int a = -1, b= -1;
            string i = l->mid_code(&a);
            string j = r->mid_code(&b);
            int t = getTemp();
            if (pInt != nullptr) {
                *pInt = t;
            }
            string str = "t" + std::to_string(t);
            vector<string> code;
            code.emplace_back("seq");
            code.emplace_back(str);
            code.emplace_back(i);
            code.emplace_back(j);
            midCode.emplace_back(code);
            if (a != -1) {
                temp[a] = 0;
            }
            if (b != -1) {
                temp[b] = 0;
            }
            return str;
        }
        case Neq:{
            int a = -1, b= -1;
            string i = l->mid_code(&a);
            string j = r->mid_code(&b);
            int t = getTemp();
            if (pInt != nullptr) {
                *pInt = t;
            }
            string str = "t" + std::to_string(t);
            vector<string> code;
            code.emplace_back("sne");
            code.emplace_back(str);
            code.emplace_back(i);
            code.emplace_back(j);
            midCode.emplace_back(code);
            if (a != -1) {
                temp[a] = 0;
            }
            if (b != -1) {
                temp[b] = 0;
            }
            return str;
        }
        case And:{
            string str;
            if (l->condOp != And) {
                str = "label_" + std::to_string(label++);
                int a = -1;
                string i = l->mid_code(&a);
                if (a != -1) {
                    temp[a] = 0;
                }
                vector<string> code;
                code.emplace_back("beqz");
                code.emplace_back(i);
                code.emplace_back(str);
                midCode.emplace_back(code);
            } else {
                str = l->mid_code(nullptr);
            }
            int b = -1;
            string j = r->mid_code(&b);
            vector<string> code1;
            code1.emplace_back("beqz");
            code1.emplace_back(j);
            code1.emplace_back(str);
            midCode.emplace_back(code1);
            if (b != -1) {
                temp[b] = 0;
            }
            return str;
        }
        case Or:{
            string str;
            if (l->condOp == Or) {
                str = l->mid_code(nullptr);
            } else {
                str = "label_" + std::to_string(label++);
                if (l->condOp == And) {
                    string string1 = l->mid_code(nullptr);
                    vector<string> code;
                    code.emplace_back("goto");
                    code.emplace_back(str);
                    midCode.emplace_back(code);
                    vector<string> code1;
                    code1.emplace_back(string1 + ":");
                    midCode.emplace_back(code1);
                } else {
                    int a = -1;
                    string i = l->mid_code(&a);
                    if (a != -1) {
                        temp[a] = 0;
                    }
                    vector<string> code;
                    code.emplace_back("bnez");
                    code.emplace_back(i);
                    code.emplace_back(str);
                    midCode.emplace_back(code);
                }
            }
            if (r->condOp != Or && r->condOp == And) {
                string string1 = r->mid_code(nullptr);
                vector<string> code;
                code.emplace_back("goto");
                code.emplace_back(str);
                midCode.emplace_back(code);
                vector<string> code1;
                code1.emplace_back(string1 + ":");
                midCode.emplace_back(code1);
            } else if (r->condOp != Or && r->condOp != And) {
                int a = -1;
                string i = r->mid_code(&a);
                if (a != -1) {
                    temp[a] = 0;
                }
                vector<string> code;
                code.emplace_back("bnez");
                code.emplace_back(i);
                code.emplace_back(str);
                midCode.emplace_back(code);
            }
            return str;
        }
        case NotCond:
            return expr->mid_code(pInt);
    }
    return string ("null");
}
