//
// Created by ycg on 2021/11/9.
//
#include "include/mips.h"
bool global = true;

vector<string> text_;
vector<string> data_int;
vector<string> data_string;
vector<string> func_;

vector<int> mapSp;
int sp = 0, paramSp1 = 0, paramSp2 = 0;
int sReg[8];
int tReg[8];
int aReg[4];
Map *mySpMap;
ofstream mipsfile;         /* NOLINT */
int param_num = 0, paramnum = 0;
int tmp = 0;
vector<map<int, string>> stores;
int start = 0;
bool isMain = false, isSave = false;

void MipsFile() {
#ifdef OUTPUT_MIPS
    mipsfile.open("mips.txt", std::ios::trunc);
    if (!mipsfile.is_open())
    {
        std::cerr << "Open midCode failed" << endl;
        exit(-1);
    }
    mipsfile << ".data" << endl;
    for (const auto&i : data_int) {
        mipsfile << i << endl;
    }
    for (const auto&i : data_string) {
        mipsfile << i << endl;
    }
    mipsfile << ".text" << endl;
    for (const auto&i : text_) {
        mipsfile << i << endl;
    }
    for (const auto&i : func_) {
        mipsfile << i << endl;
    }
#endif
}

void flush_s() {            //清空全局寄存器
    for (int i = 0; i < 8; i++) {
        sReg[i] = 0;
    }
}

void flush_t() {            //清空临时寄存器
    for (int i = 0; i < 8; i++) {
        tReg[i] = 0;
    }
}

void flush_a() {            //清空函数参数寄存器
    for (int i = 0; i < 4; i++) {
        aReg[i] = 0;
    }
}

int getS() {                //获取空闲全局寄存器
    for (int i = 0; i < 8; i++) {
        if (sReg[i] == 0) {
            sReg[i] = 1;
            return i;
        }
    }
    return -1;
}

void pushSp() {             //作用域进栈
    mapSp.emplace_back(sp);
}

void popSp() {              //作用域退栈
    sp = mapSp.back();
    mySpMap->pop(sp);
    mapSp.pop_back();
    flush_s();
    flush_t();
}

bool isImm(const string& s) {       //是否为立即数
    return isdigit(s[0]) || s[0] == '-';
}

map<int, string> storeReg() {       //保存上下文
    map<int, string> store;
    for (int i = 0; i < 8; i++) {
        if (tReg[i] != 0) {
            sp -= 4;
            store[sp] = "t" + std::to_string(i);
            text_.emplace_back("sw\t$"+ store[sp] + ", " + std::to_string(sp) + "($sp)");
        }
    }
    for (int i = 0; i < 8; i++) {
        if (sReg[i] != 0) {
            sp -= 4;
            store[sp] = "s" + std::to_string(i);
            mySpMap->insertSp1(i, sp);
            text_.emplace_back("sw\t$"+ store[sp] + ", " + std::to_string(sp) + "($sp)");
        }
    }
    flush_s();
    flush_t();
    return store;
}

int storeReg1() {       //保存上下文
    int sp1 = sp;
    for (int i = 0; i < 8; i++) {
        if (tReg[i] != 0) {
            sp1 -= 4;
        }
    }
    for (int i = 0; i < 8; i++) {
        if (sReg[i] != 0) {
            sp1 -= 4;
        }
    }
    return -sp1;
}

int storeRa(map<int, string> &store) {          //保存ra寄存器
    sp -= 4;
    store[sp] = "ra";
    text_.emplace_back("sw\t$ra, " + std::to_string(sp) + "($sp)");
    return -sp;
}

void loadReg(const map<int, string>& store) {           //恢复上下文
    for(const auto &i : store) {
        if (i.second[0] == 't') {
            tReg[i.second[1] - '0'] = 1;
        } else if (i.second[0] == 's') {
            sReg[i.second[1] - '0'] = 1;
        }
        sp += 4;
        text_.emplace_back("lw\t$"+ i.second + ", " + std::to_string(i.first) + "($sp)");
        if (i.second[0] == 's') {
            mySpMap->insertsReg(i.second[1] - '0', i.first, mapSp.back() - 4);
        }
    }
}

string arrLengthString(const string& i) {       //获取数组长度
    char *end = nullptr;
    int intStr = strtol(i.c_str(), &end, 10);
    return std::to_string(intStr * 4);
}

int arrLengthInt(const string& i) {             //获取数组长度
    char *end = nullptr;
    int intStr = strtol(i.c_str(), &end, 10);
    return intStr * 4;
}

void mips(deque<vector<string>>& deque) {
    mySpMap = new Map();
    for (auto &i: deque) {
        if (!i.empty()) {
            to_mips(i);
        }
    }
    delete mySpMap;
    MipsFile();
}

void to_mips(vector<string> &vector1) {
    string str1 = vector1[0];
    if (str1 == "const" && vector1[1] != "=") {
        to_const(vector1);
    } else if (str1 == "var" && vector1[1] != "=") {
        to_var(vector1);
    } else if (str1 == "string" && vector1[1] != "=") {
        to_string(vector1);
    } else if ((str1 == "int" || str1 == "void") && vector1[1] != "=") {
        if (vector1[1] == "main") {
            isMain = true;
        }
        start = text_.size();
        pushSp();
        global = false;
        param_num = 0;
        paramSp2 = 0;
        char *end = nullptr;
        paramnum = strtol(vector1[2].c_str(), &end, 10);
        text_.emplace_back(vector1[1] + ":");
        if (paramnum > 4) {
            sp -= (paramnum - 4) * 4;
        }
    } else if (str1 == "endFunc" && vector1.size() == 1) {
        if (!isMain) {
            func_.insert(func_.end(), text_.begin() + start, text_.end());
            func_.emplace_back("jr\t$ra");
            text_.erase(text_.begin() + start, text_.end());
        }
        popSp();
    } else if (str1 == "goto" && vector1[1] != "=") {
        text_.emplace_back("j\t" + vector1[1]);
    } else if (str1 == "output" && vector1[1] != "=") {
        to_output(vector1);
    } else if (str1 == "ret" && (vector1.size() == 1 || vector1[1] != "=")) {
        if (isMain) {
            text_.emplace_back("li\t$v0, 10");
            text_.emplace_back("syscall");
        } else {
            to_ret(vector1);
        }
    } else if (str1 == "push" && vector1[1] != "=") {
        if (!isSave) {
            tmp = storeReg1() + 4;
            isSave = true;
        }
        to_push(vector1);
    } else if (str1 == "call" && vector1[1] != "=") {
        stores.emplace_back(storeReg());
        tmp = storeRa(stores.back());
        text_.emplace_back("addi\t$sp, $sp, -" + std::to_string(tmp));

        to_call(vector1);
        flush_a();
        paramSp1 = 0;
        text_.emplace_back("addi\t$sp, $sp, " + std::to_string(tmp));
        loadReg(stores.back());
        stores.pop_back();
        isSave = false;
    } else if (str1 == "param" && vector1[1] != "=") {
        to_param(vector1);
    } else if ((str1 == "beqz" || str1 == "bnez") && vector1[1] != "=") {
        to_branch(vector1);
    } else if ((str1 == "slt" || str1 == "sle" || str1 == "seq" || str1 == "sne") && vector1[1] != "=") {
        to_set(vector1);
    } else if ((str1 == "if" || str1 == "while") && vector1[1] != "=") {
        stores.emplace_back(storeReg());
        pushSp();
        text_.emplace_back(vector1[1]);
    } else if (str1 == "else" && vector1[1] != "=") {
        popSp();
        pushSp();
        text_.emplace_back(vector1[1]);
    } else if (str1 == "end" && vector1[1] != "=") {
        text_.emplace_back(vector1[1]);
        popSp();
        loadReg(stores.back());
        stores.pop_back();
    } else if (str1 == "beginBlock" && vector1.size() == 1) {
        stores.emplace_back(storeReg());
        pushSp();
    } else if (str1 == "endBlock" && vector1.size() == 1) {
        popSp();
        loadReg(stores.back());
        stores.pop_back();
    } else if (vector1.size() > 1 && vector1[1] == "=") {
        to_expr(vector1);
    } else {
        text_.emplace_back(str1);
    }
}

void to_const(vector<string> &vector) {
    if (global) {               //全局
        if (vector[1] == "arrayInt") {
            data_int.emplace_back(vector[2] + ":.space\t" + arrLengthString(vector[3]));
        } else if (vector[1] == "int") {
            data_int.emplace_back(vector[2] + ":.word\t" + vector[4]);
        }
    } else {
        if (vector[1] == "arrayInt") {
            sp -= arrLengthInt(vector[3]);
            mySpMap->insert({sp, false, -1, vector[2]});
        } else if (vector[1] == "int") {
            int i = getS();
            if (i != -1) {                  //分配为全局寄存器
                mySpMap->insert({mapSp.back() - 4, false, i, vector[2]});
                text_.emplace_back("li\t$s" + std::to_string(i) + ", " + vector[4]);
            } else {                        //分配栈中
                sp -= 4;
                mySpMap->insert({sp, false, -1, vector[2]});
                text_.emplace_back("li\t$t8, " + vector[4]);
                text_.emplace_back("sw\t$t8, " + std::to_string(sp) + "($sp)");
            }
        }
    }
}

void to_var(vector<string> &vector) {
    if (global) {                   //全局
        if (vector[1] == "arrayInt") {
            data_int.emplace_back(vector[2] + ":.space\t" + arrLengthString(vector[3]));
        } else if (vector[1] == "int") {
            data_int.emplace_back(vector[2] + ":.word\t" + vector[4]);
        }
    } else {
        if (vector[1] == "arrayInt") {
            sp -= arrLengthInt(vector[3]);
            mySpMap->insert({sp, false, -1, vector[2]});
        } else if (vector[1] == "int") {
            int i = getS();
            if (i != -1) {                                  //分配为全局寄存器
                mySpMap->insert({mapSp.back() - 4, false, i, vector[2]});
                if (vector.size() == 5) {                   //是否初始化
                    if (isImm(vector[4])) {              //初始化立即数
                        text_.emplace_back("li\t$s" + std::to_string(i) + ", " + vector[4]);
                    } else if (vector[4][0] == 't' && isdigit(vector[4][1]) && vector[4].length() == 2) {       //初始化临时寄存器
                        tReg[vector[4][1] - '0'] = 0;
                        text_.emplace_back("move\t$s" + std::to_string(i) + ", $" + vector[4]);
                    } else {                                //初始化为标识符
                        string str = vector[4];
                        int j = 0;
                        size_t pos = str.find('[');
                        string length;
                        bool isArray = false;
                        bool isAddress = false;
                        if (pos != string::npos) {
                            size_t pos1 = str.find(']');
                            string str1 = str.substr(0, pos);
                            length = str.substr(pos + 1, pos1 - pos - 1);
                            str = str1;
                            isArray = true;
                        }
                        if (mySpMap->findSpOrSReg(str, &j, &isAddress)) {
                            if (j >= 0) {                   //全局寄存器
                                text_.emplace_back("move\t$s" + std::to_string(i) + ", $s" + std::to_string(j));
                            } else {
                                if (isArray) {              //是否为数组
                                    if (isAddress) {        //是否为地址
                                        if (isImm(length)) {
                                            text_.emplace_back("lw\t$t8, " + std::to_string(j) + "($sp)");
                                            text_.emplace_back("lw\t$s" + std::to_string(i) + ", " + arrLengthString(length) + "($t8)");
                                        } else {
                                            int a;
                                            string string1 = getExpr(length, &a, false);
                                            text_.emplace_back("lw\t$t8, " + std::to_string(j) + "($sp)");
                                            text_.emplace_back("addu\t$t8, $t8, " + string1);
                                            text_.emplace_back("lw\t$s" + std::to_string(i) + ", ($t8)");
                                        }
                                    } else {
                                        if (isImm(length)) {
                                            int t = j + arrLengthInt(length);
                                            text_.emplace_back("lw\t$s" + std::to_string(i) + ", " + std::to_string(t) + "($sp)");
                                        } else {
                                            int a;
                                            string string1 = getExpr(length, &a, false);
                                            text_.emplace_back("addiu\t$t8, $sp, " + std::to_string(j));
                                            text_.emplace_back("addu\t$t8, $t8, " + string1);
                                            text_.emplace_back("lw\t$s" + std::to_string(i) + ", ($t8)");
                                        }
                                    }
                                } else {
                                    text_.emplace_back("lw\t$s" + std::to_string(i) + ", " + std::to_string(j) + "($sp)");
                                }
                            }
                        } else {
                            if (isArray) {
                                if (isImm(length)) {
                                    text_.emplace_back("la\t$t8, " + str);
                                    text_.emplace_back("lw\t$s" + std::to_string(i) + ", " + arrLengthString(length) + "($t8)");
                                } else {
                                    int a;
                                    string string1 = getExpr(length, &a, false);
                                    text_.emplace_back("la\t$t8, " + str);
                                    text_.emplace_back("addu\t$t8, $t8, " + string1);
                                    text_.emplace_back("lw\t$s" + std::to_string(i) + ", ($t8)");
                                }
                            } else {
                                text_.emplace_back("lw\t$s" + std::to_string(i) + ", " + str);
                            }
                        }
                    }
                }
            } else {                    //分配栈中
                sp -= 4;
                mySpMap->insert({sp, false, -1, vector[2]});
                if (vector.size() == 5) {
                    if (isImm(vector[4])) {
                        text_.emplace_back("li\t$t8, " + vector[4]);
                        text_.emplace_back("sw\t$t8, " + std::to_string(sp) + "($sp)");
                    } else if (vector[4][0] == 't' && isdigit(vector[4][1]) && vector[4].length() == 2) {
                        tReg[vector[4][1] - '0'] = 0;
                        text_.emplace_back("move\t$t8, $" + vector[4] );
                        text_.emplace_back("sw\t$t8, " + std::to_string(sp) + "($sp)");
                    } else {
                        string str = vector[4];
                        int j = 0;
                        size_t pos = str.find('[');
                        string length;
                        bool isArray = false;
                        bool isAddress = false;
                        if (pos != string::npos) {
                            size_t pos1 = str.find(']');
                            string str1 = str.substr(0, pos);
                            length = str.substr(pos + 1, pos1 - pos - 1);
                            str = str1;
                            isArray = true;
                        }
                        if (mySpMap->findSpOrSReg(str, &j, &isAddress)) {
                            if (j >= 0) {
                                text_.emplace_back("move\t$t8, $s" + std::to_string(j));
                            } else {
                                if (isArray) {
                                    if (isAddress) {
                                        if (isImm(length)) {
                                            text_.emplace_back("lw\t$t8, " + std::to_string(j) + "($sp)");
                                            text_.emplace_back("lw\t$t8, " + arrLengthString(length) + "($t8)");
                                        } else {
                                            int a;
                                            string string1 = getExpr(length, &a, false);
                                            text_.emplace_back("lw\t$t8, " + std::to_string(j) + "($sp)");
                                            text_.emplace_back("addu\t$t8, $t8, " + string1);
                                            text_.emplace_back("lw\t$t8, ($t8)");
                                        }
                                    } else {
                                        if (isImm(length)) {
                                            int t = j + arrLengthInt(length);
                                            text_.emplace_back("lw\t$t8, " + std::to_string(t) + "($sp)");
                                        } else {
                                            int a;
                                            string string1 = getExpr(length, &a, false);
                                            text_.emplace_back("addiu\t$t8, $sp, " + std::to_string(j));
                                            text_.emplace_back("addu\t$t8, $t8, " + string1);
                                            text_.emplace_back("lw\t$t8, ($t8)");
                                        }
                                    }
                                } else {
                                    text_.emplace_back("lw\t$t8, " + std::to_string(j) + "($sp)");
                                }
                            }
                        } else {
                            if (isArray) {
                                if (isImm(length)) {
                                    text_.emplace_back("la\t$t8, " + str);
                                    text_.emplace_back("lw\t$t8, " + arrLengthString(length) + "($t8)");
                                } else {
                                    int a;
                                    string string1 = getExpr(length, &a, false);
                                    text_.emplace_back("la\t$t8, " + str);
                                    text_.emplace_back("addu\t$t8, $t8, " + string1);
                                    text_.emplace_back("lw\t$t8, ($t8)");
                                }
                            } else {
                                text_.emplace_back("lw\t$t8, " + str);
                            }
                        }
                        text_.emplace_back("sw\t$t8, " + std::to_string(sp) + "($sp)");
                    }
                }
            }
        }
    }
}

void to_string(vector<string> &vector) {
    data_string.emplace_back(vector[1] + ":.asciiz\t" + vector[2]);
}

void to_output(vector<string> &vector) {
    if (vector.size() == 2) {
        string str = vector[1];
        if (isImm(str)) {
            text_.emplace_back("li\t$a0, " + str);
        } else if (vector[1][0] == 't' && isdigit(vector[1][1]) && vector[1].length() == 2) {
            tReg[vector[1][1] - '0'] = 0;
            text_.emplace_back("move\t$a0, $" + str);
        } else {
            int i = 0;
            size_t pos = str.find('[');
            string length;
            bool isArray = false;
            bool isAddress = false;
            if (pos != string::npos) {
                size_t pos1 = str.find(']');
                string str1 = str.substr(0, pos);
                length = str.substr(pos + 1, pos1 - pos - 1);
                str = str1;
                isArray = true;
            }
            if (mySpMap->findSpOrSReg(str, &i, &isAddress)) {
                if (i >= 0) {
                    text_.emplace_back("move\t$a0, $s" + std::to_string(i));
                } else {
                    if (isArray) {
                        if (isAddress) {
                            if (isImm(length)) {
                                text_.emplace_back("lw\t$t8, " + std::to_string(i) + "($sp)");
                                text_.emplace_back("lw\t$a0, " + arrLengthString(length) + "($t8)");
                            } else {
                                int a;
                                string string1 = getExpr(length, &a, false);
                                text_.emplace_back("lw\t$t8, " + std::to_string(i) + "($sp)");
                                text_.emplace_back("addu\t$t8, $t8, " + string1);
                                text_.emplace_back("lw\t$a0, ($t8)");
                            }
                        } else {
                            if (isImm(length)) {
                                int t = i + arrLengthInt(length);
                                text_.emplace_back("lw\t$a0, " + std::to_string(t) + "($sp)");
                            } else {
                                int a;
                                string string1 = getExpr(length, &a, false);
                                text_.emplace_back("addiu\t$t8, $sp, " + std::to_string(i));
                                text_.emplace_back("addu\t$t8, $t8, " + string1);
                                text_.emplace_back("lw\t$a0, ($t8)");
                            }
                        }
                    } else {
                        text_.emplace_back("lw\t$a0, " + std::to_string(i) + "($sp)");
                    }
                }
            } else {
                if (isArray) {
                    if (isImm(length)) {
                        text_.emplace_back("la\t$t8, " + str);
                        text_.emplace_back("lw\t$a0, " + arrLengthString(length) + "($t8)");
                    } else {
                        int a;
                        string string1 = getExpr(length, &a, false);
                        text_.emplace_back("la\t$t8, " + str);
                        text_.emplace_back("addu\t$t8, $t8, " + string1);
                        text_.emplace_back("lw\t$a0, ($t8)");
                    }
                } else {
                    text_.emplace_back("lw\t$a0, " + str);
                }
            }
        }
        text_.emplace_back("li\t$v0, 1");
        text_.emplace_back("syscall");
    } else {
        text_.emplace_back("la\t$a0, " + vector[2]);
        text_.emplace_back("li\t$v0, 4");
        text_.emplace_back("syscall");
    }
}

void to_ret(vector<string> &vector) {
    if (vector.size() == 1) {
        text_.emplace_back("jr\t$ra");
    } else {
        string str = vector[1];
        if (isImm(str)) {
            text_.emplace_back("li\t$v1, " + str);
        } else if (vector[1][0] == 't' && isdigit(vector[1][1]) && vector[1].length() == 2) {
            tReg[vector[1][1] - '0'] = 0;
            text_.emplace_back("move\t$v1, $" + str);
        } else {
            int i = 0;
            size_t pos = str.find('[');
            string length;
            bool isArray = false;
            bool isAddress = false;
            if (pos != string::npos) {
                size_t pos1 = str.find(']');
                string str1 = str.substr(0, pos);
                length = str.substr(pos + 1, pos1 - pos - 1);
                str = str1;
                isArray = true;
            }
            if (mySpMap->findSpOrSReg(str, &i, &isAddress)) {
                if (i >= 0) {
                    text_.emplace_back("move\t$v1, $s" + std::to_string(i));
                } else {
                    if (isArray) {
                        if (isAddress) {
                            if (isImm(length)) {
                                text_.emplace_back("lw\t$t8, " + std::to_string(i) + "($sp)");
                                text_.emplace_back("lw\t$v1, " + arrLengthString(length) + "($t8)");
                            } else {
                                int a;
                                string string1 = getExpr(length, &a, false);
                                text_.emplace_back("lw\t$t8, " + std::to_string(i) + "($sp)");
                                text_.emplace_back("addu\t$t8, $t8, " + string1);
                                text_.emplace_back("lw\t$v1, ($t8)");
                            }
                        } else {
                            if (isImm(length)) {
                                int t = i + arrLengthInt(length);
                                text_.emplace_back("lw\t$v1, " + std::to_string(t) + "($sp)");
                            } else {
                                int a;
                                string string1 = getExpr(length, &a, false);
                                text_.emplace_back("addiu\t$t8, $sp, " + std::to_string(i));
                                text_.emplace_back("addu\t$t8, $t8, " + string1);
                                text_.emplace_back("lw\t$v1, ($t8)");
                            }
                        }
                    } else {
                        text_.emplace_back("lw\t$v1, " + std::to_string(i) + "($sp)");
                    }
                }
            } else {
                if (isArray) {
                    if (isImm(length)) {
                        text_.emplace_back("la\t$t8, " + str);
                        text_.emplace_back("lw\t$v1, " + arrLengthString(length) + "($t8)");
                    } else {
                        int a;
                        string string1 = getExpr(length, &a, false);
                        text_.emplace_back("la\t$t8, " + str);
                        text_.emplace_back("addu\t$t8, $t8, " + string1);
                        text_.emplace_back("lw\t$v1, ($t8)");
                    }
                } else {
                    text_.emplace_back("lw\t$v1, " + str);
                }
            }
        }
        text_.emplace_back("jr\t$ra");
    }
}

void to_push(vector<string> &vector) {
    int a = arrLengthInt(vector[3]) / 4;
    string type = vector[2];
    if (a < 4) {                    // 分配函数参数寄存器
        string str = vector[1];
        if (isImm(str)) {
            text_.emplace_back("li\t$a" + std::to_string(a) + ", " + str);
        } else if (vector[1][0] == 't' && isdigit(vector[1][1]) && vector[1].length() == 2) {
            tReg[vector[1][1] - '0'] = 0;
            text_.emplace_back("move\t$a" + std::to_string(a) + ", $" + str );
        } else {
            int i = 0;
            size_t pos = str.find('[');
            string length;
            bool isArray = false;
            bool isAddress = false;
            if (pos != string::npos) {
                size_t pos1 = str.find(']');
                string str1 = str.substr(0, pos);
                length = str.substr(pos + 1, pos1 - pos - 1);
                str = str1;
                isArray = true;
            }
            if (mySpMap->findSpOrSReg(str, &i, &isAddress)) {
                if (i >= 0) {
                    text_.emplace_back("move\t$a" + std::to_string(a) + ", $s" + std::to_string(i));
                } else {
                    if (type == "int") {                //参数为int，传值
                        if (isArray) {
                            if (isAddress) {
                                if (isImm(length)) {
                                    text_.emplace_back("lw\t$t8, " + std::to_string(i) + "($sp)");
                                    text_.emplace_back("lw\t$a" + std::to_string(a) + ", " + arrLengthString(length) + "($t8)");
                                } else {
                                    int b;
                                    string string1 = getExpr(length, &b, false);
                                    text_.emplace_back("lw\t$t8, " + std::to_string(i) + "($sp)");
                                    text_.emplace_back("addu\t$t8, $t8, " + string1);
                                    text_.emplace_back("lw\t$a" + std::to_string(a) + ", ($t8)");
                                }
                            } else {
                                if (isImm(length)) {
                                    int t = i + arrLengthInt(length);
                                    text_.emplace_back("lw\t$a" + std::to_string(a) + ", " + std::to_string(t) + "($sp)");
                                } else {
                                    int b;
                                    string string1 = getExpr(length, &b, false);
                                    text_.emplace_back("addiu\t$t8, $sp, " + std::to_string(i));
                                    text_.emplace_back("addu\t$t8, $t8, " + string1);
                                    text_.emplace_back("lw\t$a" + std::to_string(a) + ", ($t8)");
                                }
                            }
                        } else {
                            text_.emplace_back("lw\t$a" + std::to_string(a) + ", " + std::to_string(i) + "($sp)");
                        }
                    } else {                            //参数为array，传地址
                        if (isArray) {
                            if (isAddress) {
                                if (isImm(length)) {
                                    text_.emplace_back("lw\t$t8, " + std::to_string(i) + "($sp)");
                                    text_.emplace_back("addiu\t$a" + std::to_string(a) + ", $t8, " + arrLengthString(length));
                                } else {
                                    int b;
                                    string string1 = getExpr(length, &b, false);
                                    text_.emplace_back("lw\t$t8, " + std::to_string(i) + "($sp)");
                                    text_.emplace_back("addu\t$a" + std::to_string(a) + ", $t8, " + string1);
                                }
                            } else {
                                if (isImm(length)) {
                                    int t = i + arrLengthInt(length);
                                    text_.emplace_back("addiu\t$a" + std::to_string(a) + ", $sp, " + std::to_string(t));
                                } else {
                                    int b;
                                    string string1 = getExpr(length, &b, false);
                                    text_.emplace_back("addiu\t$t8, $sp, " + std::to_string(i));
                                    text_.emplace_back("addu\t$a" + std::to_string(a) + ", $t8, " + string1);
                                }
                            }
                        } else {
                            if (isAddress) {
                                text_.emplace_back("lw\t$t8, " + std::to_string(i) + "($sp)");
                                text_.emplace_back("move\t$a" + std::to_string(a) + ", $t8");
                            } else {
                                text_.emplace_back("addiu\t$a" + std::to_string(a) + ", $sp, " + std::to_string(i));
                            }
                        }
                    }
                }
            } else {
                if (type == "int") {
                    if (isArray) {
                        if (isImm(length)) {
                            text_.emplace_back("la\t$t8, " + str);
                            text_.emplace_back("lw\t$a" + std::to_string(a) + ", " + arrLengthString(length) + "($t8)");
                        } else {
                            int b;
                            string string1 = getExpr(length, &b, false);
                            text_.emplace_back("la\t$t8, " + str);
                            text_.emplace_back("addu\t$t8, $t8, " + string1);
                            text_.emplace_back("lw\t$a" + std::to_string(a) + ", ($t8)");
                        }
                    } else {
                        text_.emplace_back("lw\t$a" + std::to_string(a) + ", " + str);
                    }
                } else if (type == "arrayInt") {
                    if (isArray) {
                        if (isImm(length)) {
                            text_.emplace_back("la\t$t8, " + str);
                            text_.emplace_back("addiu\t$a" + std::to_string(a) + ", $t8, " + arrLengthString(length));
                        } else {
                            int b;
                            string string1 = getExpr(length, &b, false);
                            text_.emplace_back("la\t$t8, " + str);
                            text_.emplace_back("addu\t$a" + std::to_string(a) + ", $t8, " + string1);
                        }
                    } else {
                        text_.emplace_back("la\t$a" + std::to_string(a) + ", " + str);
                    }
                }
            }
        }
    } else {                    //将函数参数保存至下一个作用域的栈中
        paramSp1 -= 4;
        string str = vector[1];
        if (isImm(str)) {
            text_.emplace_back("li\t$t8, " + str);
            text_.emplace_back("sw\t$t8, " + std::to_string(paramSp1 - tmp) + "($sp)");
        } else if (vector[1][0] == 't' && isdigit(vector[1][1]) && vector[1].length() == 2) {
            tReg[vector[1][1] - '0'] = 0;
            text_.emplace_back("sw\t$" + str + ", " + std::to_string(paramSp1 - tmp) + "($sp)");
        } else {
            int i = 0;
            size_t pos = str.find('[');
            string length;
            bool isArray = false;
            bool isAddress = false;
            if (pos != string::npos) {
                size_t pos1 = str.find(']');
                string str1 = str.substr(0, pos);
                length = str.substr(pos + 1, pos1 - pos - 1);
                str = str1;
                isArray = true;
            }
            if (mySpMap->findSpOrSReg(str, &i, &isAddress)) {
                if (i >= 0) {
                    text_.emplace_back("sw\t$s" + std::to_string(i) + ", " + std::to_string(paramSp1 - tmp) + "($sp)");
                } else {
                    if (type == "int") {
                        if (isArray) {
                            if (isAddress) {
                                if (isImm(length)) {
                                    text_.emplace_back("lw\t$t8, " + std::to_string(i) + "($sp)");
                                    text_.emplace_back("lw\t$t8, " + arrLengthString(length) + "($t8)");
                                } else {
                                    int b;
                                    string string1 = getExpr(length, &b, false);
                                    text_.emplace_back("lw\t$t8, " + std::to_string(i) + "($sp)");
                                    text_.emplace_back("addu\t$t8, $t8, " + string1);
                                    text_.emplace_back("lw\t$t8, ($t8)");
                                }
                            } else {
                                if (isImm(length)) {
                                    int t = i + arrLengthInt(length);
                                    text_.emplace_back("lw\t$t8, " + std::to_string(t) + "($sp)");
                                } else {
                                    int b;
                                    string string1 = getExpr(length, &b, false);
                                    text_.emplace_back("addiu\t$t8, $sp, " + std::to_string(i));
                                    text_.emplace_back("addu\t$t8, $t8, " + string1);
                                    text_.emplace_back("lw\t$t8, ($t8)");
                                }
                            }
                        } else {
                            text_.emplace_back("lw\t$t8, " + std::to_string(i) + "($sp)");
                        }
                        text_.emplace_back("sw\t$t8, " + std::to_string(paramSp1 - tmp) + "($sp)");
                    } else {
                        if (isArray) {
                            if (isAddress) {
                                if (isImm(length)) {
                                    text_.emplace_back("lw\t$t8, " + std::to_string(i) + "($sp)");
                                    text_.emplace_back("addiu\t$t8, $t8, " + arrLengthString(length));
                                } else {
                                    int b;
                                    string string1 = getExpr(length, &b, false);
                                    text_.emplace_back("lw\t$t8, " + std::to_string(i) + "($sp)");
                                    text_.emplace_back("addu\t$t8, $t8, " + string1);
                                }
                            } else {
                                if (isImm(length)) {
                                    int t = i + arrLengthInt(length);
                                    text_.emplace_back("addiu\t$t8, $sp, " + std::to_string(t));
                                } else {
                                    int b;
                                    string string1 = getExpr(length, &b, false);
                                    text_.emplace_back("addiu\t$t8, $sp, " + std::to_string(i));
                                    text_.emplace_back("addu\t$t8, $t8, " + string1);
                                }
                            }
                        } else {
                            if (isAddress) {
                                text_.emplace_back("lw\t$t8, " + std::to_string(i) + "($sp)");
                            } else {
                                text_.emplace_back("addiu\t$t8, $sp, " + std::to_string(i));
                            }
                        }
                        text_.emplace_back("sw\t$t8, " + std::to_string(paramSp1 - tmp) + "($sp)");
                    }
                }
            } else {
                if (type == "int") {
                    if (isArray) {
                        if (isImm(length)) {
                            text_.emplace_back("la\t$t8, " + str);
                            text_.emplace_back("lw\t$t8, " + arrLengthString(length) + "($t8)");
                        } else {
                            int b;
                            string string1 = getExpr(length, &b, false);
                            text_.emplace_back("la\t$t8, " + str);
                            text_.emplace_back("addu\t$t8, $t8, " + string1);
                            text_.emplace_back("lw\t$t8, ($t8)");
                        }
                    } else {
                        text_.emplace_back("lw\t$t8, " + str);
                    }
                    text_.emplace_back("sw\t$t8, " + std::to_string(paramSp1 - tmp) + "($sp)");
                } else if (type == "arrayInt") {
                    if (isArray) {
                        if (isImm(length)) {
                            text_.emplace_back("la\t$t8, " + str);
                            text_.emplace_back("addiu\t$t8, $t8, " + arrLengthString(length));
                        } else {
                            int b;
                            string string1 = getExpr(length, &b, false);
                            text_.emplace_back("la\t$t8, " + str);
                            text_.emplace_back("addu\t$t8, $t8, " + string1);
                        }
                    } else {
                        text_.emplace_back("la\t$t8, " + str);
                    }
                    text_.emplace_back("sw\t$t8, " + std::to_string(paramSp1 - tmp) + "($sp)");
                }
            }
        }
    }
}

void to_call(vector<string> &vector1) {
    text_.emplace_back("jal\t" + vector1[1]);
}

void to_param(vector<string> &vector) {
    if (param_num < 4) {
        if (vector[1] == "arrayInt") {
            sp -= 4;
            mySpMap->insert({sp, true, -1, vector[2]});
            text_.emplace_back("sw\t$a" + std::to_string(param_num) + ", " + std::to_string(sp) + "($sp)");
        } else if (vector[1] == "int") {
            int i = getS();
            if (i != -1) {
                mySpMap->insert({mapSp.back() - 4, false, i, vector[2]});
                text_.emplace_back("move\t$s" + std::to_string(i) + ", $a" + std::to_string(param_num));
            } else {
                sp -= 4;
                mySpMap->insert({sp, false, -1, vector[2]});
                text_.emplace_back("sw\t$a" + std::to_string(param_num) + ", " + std::to_string(sp) + "($sp)");
            }
        }
        param_num++;
    } else {
        paramSp2 -= 4;
        if (vector[1] == "arrayInt") {
            mySpMap->insert({paramSp2, true, -1, vector[2]});
        } else if (vector[1] == "int") {
            int i = getS();
            if (i != -1) {
                mySpMap->insert({mapSp.back() - 4, false, i, vector[2]});
                text_.emplace_back("lw\t$s" + std::to_string(i) + ", " + std::to_string(paramSp2) + "($sp)");
            } else {
                mySpMap->insert({paramSp2, false, -1, vector[2]});
            }
        }
    }
}

string getExpr(string s, int *type, bool l) {           //将当前操作数转为寄存器，包括立即数、全局寄存器、临时寄存器和标识符
    if (isdigit(s[0]) || s[0] == '-') {
        if (s == "0") {
            *type = reg;
            return "$zero";
        }
        *type = IMMEDIATE;
        return s;
    } else if (s[0] == 't' && isdigit(s[1]) && s.length() == 2) {
        tReg[s[1] - '0'] = 0;
        *type = reg;
        return "$" + s;
    } else if (s == "RET") {
        *type = reg;
        return "$v1";
    } else if (s == "getint") {
        text_.emplace_back("li\t$v0, 5");
        text_.emplace_back("syscall");
        *type = reg;
        return "$v0";
    } else {
        int i = 0;
        size_t pos = s.find('[');
        string length;
        bool isArray = false;
        bool isAddress = false;
        if (pos != string::npos) {
            size_t pos1 = s.find(']');
            string str1 = s.substr(0, pos);
            length = s.substr(pos + 1, pos1 - pos - 1);
            s = str1;
            isArray = true;
        }
        if (mySpMap->findSpOrSReg(s, &i, &isAddress)) {
            if (i >= 0) {
                *type = reg;
                return "$s" + std::to_string(i);
            } else {
                if (isArray) {
                    if (isAddress) {
                        if (l) {
                            if (isImm(length)) {
                                text_.emplace_back("lw\t$t8, " + std::to_string(i) + "($sp)");
                                text_.emplace_back("lw\t$t8, " + arrLengthString(length) + "($t8)");
                            } else {
                                int a;
                                string string1 = getExpr(length, &a, false);
                                text_.emplace_back("lw\t$t8, " + std::to_string(i) + "($sp)");
                                text_.emplace_back("addu\t$t8, $t8, " + string1);
                                text_.emplace_back("lw\t$t8, ($t8)");
                            }
                        } else {
                            if (isImm(length)) {
                                text_.emplace_back("lw\t$t9, " + std::to_string(i) + "($sp)");
                                text_.emplace_back("lw\t$t9, " + arrLengthString(length) + "($t9)");
                            } else {
                                int a;
                                string string1 = getExpr(length, &a, true);
                                text_.emplace_back("lw\t$t9, " + std::to_string(i) + "($sp)");
                                text_.emplace_back("addu\t$t9, $t9, " + string1);
                                text_.emplace_back("lw\t$t9, ($t9)");
                            }
                        }
                    } else {
                        if (l) {
                            if (isImm(length)) {
                                int t = i + arrLengthInt(length);
                                text_.emplace_back("lw\t$t8, " + std::to_string(t) + "($sp)");
                            } else {
                                int a;
                                string string1 = getExpr(length, &a, false);
                                text_.emplace_back("addiu\t$t8, $sp, " + std::to_string(i));
                                text_.emplace_back("addu\t$t8, $t8, " + string1);
                                text_.emplace_back("lw\t$t8, ($t8)");
                            }
                        } else {
                            if (isImm(length)) {
                                int t = i + arrLengthInt(length);
                                text_.emplace_back("lw\t$t9, " + std::to_string(t) + "($sp)");
                            } else {
                                int a;
                                string string1 = getExpr(length, &a, true);
                                text_.emplace_back("addiu\t$t9, $sp, " + std::to_string(i));
                                text_.emplace_back("addu\t$t9, $t9, " + string1);
                                text_.emplace_back("lw\t$t9, ($t9)");
                            }
                        }
                    }
                } else {
                    if (l) {
                        text_.emplace_back("lw\t$t8, " + std::to_string(i) + "($sp)");
                    } else {
                        text_.emplace_back("lw\t$t9, " + std::to_string(i) + "($sp)");
                    }
                }
                *type = reg;
            }
        } else {
            if (isArray) {
                if (l) {
                    if (isImm(length)) {
                        text_.emplace_back("la\t$t8, " + s);
                        text_.emplace_back("lw\t$t8, " + arrLengthString(length) + "($t8)");
                    } else {
                        int a;
                        string string1 = getExpr(length, &a, false);
                        text_.emplace_back("la\t$t8, " + s);
                        text_.emplace_back("addu\t$t8, $t8, " + string1);
                        text_.emplace_back("lw\t$t8, ($t8)");
                    }
                } else {
                    if (isImm(length)) {
                        text_.emplace_back("la\t$t9, " + s);
                        text_.emplace_back("lw\t$t9, " + arrLengthString(length) + "($t9)");
                    } else {
                        int a;
                        string string1 = getExpr(length, &a, true);
                        text_.emplace_back("la\t$t9, " + s);
                        text_.emplace_back("addu\t$t9, $t9, " + string1);
                        text_.emplace_back("lw\t$t9, ($t9)");
                    }
                }
            } else {
                if (l) {
                    text_.emplace_back("lw\t$t8, " + s);
                } else {
                    text_.emplace_back("lw\t$t9, " + s);
                }
            }
            *type = reg;
        }
        if (l) {
            return "$t8";
        } else {
            return "$t9";
        }
    }
}

void pushExpr(vector<string> &vector, const string& s) {            //+、-、*、/、%、！等基本表达式，并将结果保存至寄存器s中
    if (vector.size() == 4) {
        int i;
        string str = getExpr(vector[3], &i, true);
        if (i == IMMEDIATE) {
            if (vector[2] == "-") {
                text_.emplace_back("li\t$" + s + ", -" + str);
            } else if (vector[2] == "!") {
                text_.emplace_back("li\t$t8, " + str);
                text_.emplace_back("seq\t$" + s + ", $t8, $zero");
            }
        } else {
            if (vector[2] == "-") {
                text_.emplace_back("subu\t$" + s + ", $zero, " + str);
            } else if (vector[2] == "!") {
                text_.emplace_back("seq\t$" + s + ", " + str + ", $zero");
            }
        }
    } else if (vector.size() == 5) {
        int i, j;
        string string1 = getExpr(vector[2], &i, true);
        string string2 = getExpr(vector[4], &j, false);
        if (vector[3] == "+") {
            if (i == IMMEDIATE && j == IMMEDIATE) {
                text_.emplace_back("li\t$t8, " + string1);
                text_.emplace_back("addiu\t$" + s + ", $t8, " + string2);
            } else if (i == IMMEDIATE && j == reg) {
                text_.emplace_back("addiu\t$" + s + ", " + string2 + ", " + string1);
            } else if (i == reg && j == IMMEDIATE) {
                text_.emplace_back("addiu\t$" + s + ", " + string1 + ", " + string2);
            } else {
                text_.emplace_back("addu\t$" + s + ", " + string1 + ", " + string2);
            }
        } else if (vector[3] == "-") {
            if (i == IMMEDIATE && j == IMMEDIATE) {
                text_.emplace_back("li\t$t9, " + string1);
                text_.emplace_back("subiu\t$" + s + ", $t9, " + string2);
            } else if (i == IMMEDIATE && j == reg) {
                text_.emplace_back("li\t$t8, " + string1);
                text_.emplace_back("subu\t$" + s + ", $t8, " + string2);
            } else if (i == reg && j == IMMEDIATE) {
                text_.emplace_back("subiu\t$" + s + ", " + string1 + ", " + string2);
            } else {
                text_.emplace_back("subu\t$" + s + ", " + string1 + ", " + string2);
            }
        } else if (vector[3] == "*") {
            if (i == IMMEDIATE && j == IMMEDIATE) {
                text_.emplace_back("li\t$t8, " + string1);
                text_.emplace_back("li\t$t9, " + string2);
                text_.emplace_back("mult\t$t8, $t9");
                text_.emplace_back("mflo\t$" + s);
            } else if (i == IMMEDIATE && j == reg) {
                text_.emplace_back("li\t$t8, " + string1);
                text_.emplace_back("mult\t$t8, " + string2);
                text_.emplace_back("mflo\t$" + s);
            } else if (i == reg && j == IMMEDIATE) {
                text_.emplace_back("li\t$t9, " + string2);
                text_.emplace_back("mult\t" + string1 + ", $t9");
                text_.emplace_back("mflo\t$" + s);
            } else {
                text_.emplace_back("mult\t" + string1 + ", " + string2);
                text_.emplace_back("mflo\t$" + s);
            }
        } else if (vector[3] == "/" || vector[3] == "%") {
            if (i == IMMEDIATE && j == IMMEDIATE) {
                text_.emplace_back("li\t$t8, " + string1);
                text_.emplace_back("li\t$t9, " + string2);
                text_.emplace_back("div\t$t8, $t9");
            } else if (i == IMMEDIATE && j == reg) {
                text_.emplace_back("li\t$t8, " + string1);
                text_.emplace_back("div\t$t8, " + string2);
            } else if (i == reg && j == IMMEDIATE) {
                text_.emplace_back("li\t$t9, " + string2);
                text_.emplace_back("div\t" + string1 + ", $t9");
            } else {
                text_.emplace_back("div\t" + string1 + ", " + string2);
            }
            if (vector[3] == "/") {
                text_.emplace_back("mflo\t$" + s);
            } else {
                text_.emplace_back("mfhi\t$" + s);
            }
        }
    }
}

void to_expr(vector<string> &vector) {
    if (vector[0][0] == 't' && isdigit(vector[0][1]) && vector[0].length() == 2) {
        tReg[vector[0][1] - '0'] = 1;
        if (vector.size() == 3) {
            int i;
            string str = getExpr(vector[2], &i, true);
            if (i == IMMEDIATE) {
                text_.emplace_back("li\t$" + vector[0] + ", " + str);
            } else {
                text_.emplace_back("move\t$" + vector[0] + ", " + str);
            }
        } else {
            pushExpr(vector, vector[0]);
        }
    } else {
        string s = vector[0];
        int i = 0;
        size_t pos = s.find('[');
        string length;
        bool isArray = false;
        bool isAddress = false;
        if (pos != string::npos) {
            size_t pos1 = s.find(']');
            string str1 = s.substr(0, pos);
            length = s.substr(pos + 1, pos1 - pos - 1);
            s = str1;
            isArray = true;
        }
        if (mySpMap->findSpOrSReg(s, &i, &isAddress)) {
            if (i >= 0) {
                if (vector.size() == 3) {
                    int j;
                    string str = getExpr(vector[2], &j, true);
                    if (j == IMMEDIATE) {
                        text_.emplace_back("li\t$s" + std::to_string(i) + ", " + str);
                    } else {
                        text_.emplace_back("move\t$s" + std::to_string(i) + ", " + str);
                    }
                } else {
                    pushExpr(vector, "s" + std::to_string(i));
                }
            } else {
                if (isArray) {
                    if (isAddress) {
                        if (vector.size() == 3) {
                            int j;
                            string str = getExpr(vector[2], &j, false);
                            if (j == IMMEDIATE) {
                                text_.emplace_back("li\t$t9, " + str);
                                if (isImm(length)) {
                                    text_.emplace_back("lw\t$t8, " + std::to_string(i) + "($sp)");
                                    text_.emplace_back("sw\t$t9, " + arrLengthString(length) + "($t8)");
                                } else {
                                    int a;
                                    string string1 = getExpr(length, &a, true);
                                    text_.emplace_back("lw\t$t8, " + std::to_string(i) + "($sp)");
                                    text_.emplace_back("addu\t$t8, $t8, " + string1);
                                    text_.emplace_back("sw\t$t9, ($t8)");
                                }
                            } else {
                                if (isImm(length)) {
                                    text_.emplace_back("lw\t$t8, " + std::to_string(i) + "($sp)");
                                    text_.emplace_back("sw\t" + str + ", " + arrLengthString(length) + "($t8)");
                                } else {
                                    int a;
                                    string string1 = getExpr(length, &a, true);
                                    text_.emplace_back("lw\t$t8, " + std::to_string(i) + "($sp)");
                                    text_.emplace_back("addu\t$t8, $t8, " + string1);
                                    text_.emplace_back("sw\t" + str + ", ($t8)");
                                }
                            }
                        } else {
                            pushExpr(vector, "t9");
                            if (isImm(length)) {
                                text_.emplace_back("lw\t$t8, " + std::to_string(i) + "($sp)");
                                text_.emplace_back("sw\t$t9, " + arrLengthString(length) + "($t8)");
                            } else {
                                int a;
                                string string1 = getExpr(length, &a, true);
                                text_.emplace_back("lw\t$t8, " + std::to_string(i) + "($sp)");
                                text_.emplace_back("addu\t$t8, $t8, " + string1);
                                text_.emplace_back("sw\t$t9, ($t8)");
                            }
                        }
                    } else {
                        if (vector.size() == 3) {
                            int j;
                            string str = getExpr(vector[2], &j, false);
                            if (j == IMMEDIATE) {
                                text_.emplace_back("li\t$t9, " + str);
                                if (isImm(length)) {
                                    int t = i + arrLengthInt(length);
                                    text_.emplace_back("sw\t$t9, " + std::to_string(t) + "($sp)");
                                } else {
                                    int a;
                                    string string1 = getExpr(length, &a, true);
                                    text_.emplace_back("addiu\t$t8, $sp, " + std::to_string(i));
                                    text_.emplace_back("addu\t$t8, $t8, " + string1);
                                    text_.emplace_back("sw\t$t9, ($t8)");
                                }
                            } else {
                                if (isImm(length)) {
                                    int t = i + arrLengthInt(length);
                                    text_.emplace_back("sw\t" + str + ", " + std::to_string(t) + "($sp)");
                                } else {
                                    int a;
                                    string string1 = getExpr(length, &a, true);
                                    text_.emplace_back("addiu\t$t8, $sp, " + std::to_string(i));
                                    text_.emplace_back("addu\t$t8, $t8, " + string1);
                                    text_.emplace_back("sw\t" + str + ", ($t8)");
                                }
                            }
                        } else {
                            pushExpr(vector, "t9");
                            if (isImm(length)) {
                                int t = i + arrLengthInt(length);
                                text_.emplace_back("sw\t$t9, " + std::to_string(t) + "($sp)");
                            } else {
                                int a;
                                string string1 = getExpr(length, &a, true);
                                text_.emplace_back("addiu\t$t8, $sp, " + std::to_string(i));
                                text_.emplace_back("addu\t$t8, $t8, " + string1);
                                text_.emplace_back("sw\t$t9, ($t8)");
                            }
                        }
                    }
                } else {
                    if (vector.size() == 3) {
                        int j;
                        string str = getExpr(vector[2], &j, false);
                        if (j == IMMEDIATE) {
                            text_.emplace_back("li\t$t9, " + str);
                            text_.emplace_back("sw\t$t9, " + std::to_string(i) + "($sp)");
                        } else {
                            text_.emplace_back("sw\t" + str + ", " + std::to_string(i) + "($sp)");
                        }
                    } else {
                        pushExpr(vector, "t9");
                        text_.emplace_back("sw\t$t9, " + std::to_string(i) + "($sp)");
                    }
                }
            }
        } else {
            if (isArray) {
                if (vector.size() == 3) {
                    int j;
                    string str = getExpr(vector[2], &j, false);
                    if (j == IMMEDIATE) {
                        text_.emplace_back("li\t$t9, " + str);
                        if (isImm(length)) {
                            text_.emplace_back("la\t$t8, " + s);
                            text_.emplace_back("sw\t$t9, " + arrLengthString(length) + "($t8)");
                        } else {
                            int a;
                            string string1 = getExpr(length, &a, true);
                            text_.emplace_back("la\t$t8, " + s);
                            text_.emplace_back("addu\t$t8, $t8, " + string1);
                            text_.emplace_back("sw\t$t9, ($t8)");
                        }
                    } else {
                        if (isImm(length)) {
                            text_.emplace_back("la\t$t8, " + s);
                            text_.emplace_back("sw\t" + str + ", " + arrLengthString(length) + "($t8)");
                        } else {
                            int a;
                            string string1 = getExpr(length, &a, true);
                            text_.emplace_back("la\t$t8, " + s);
                            text_.emplace_back("addu\t$t8, $t8, " + string1);
                            text_.emplace_back("sw\t" + str + ", ($t8)");
                        }
                    }
                } else {
                    pushExpr(vector, "t9");
                    if (isImm(length)) {
                        text_.emplace_back("la\t$t8, " + s);
                        text_.emplace_back("sw\t$t9, " + arrLengthString(length) + "($t8)");
                    } else {
                        int a;
                        string string1 = getExpr(length, &a, true);
                        text_.emplace_back("la\t$t8, " + s);
                        text_.emplace_back("addu\t$t8, $t8, " + string1);
                        text_.emplace_back("sw\t$t9, ($t8)");
                    }
                }
            } else {
                if (vector.size() == 3) {
                    int j;
                    string str = getExpr(vector[2], &j, false);
                    if (j == IMMEDIATE) {
                        text_.emplace_back("li\t$t9, " + str);
                        text_.emplace_back("sw\t$t9, " + s);
                    } else {
                        text_.emplace_back("sw\t" + str + ", " + s);
                    }
                } else {
                    pushExpr(vector, "t9");
                    text_.emplace_back("sw\t$t9, " + s);
                }
            }
        }
    }
}

void to_branch(vector<string> &vector) {
    int i;
    string str = getExpr(vector[1], &i, true);
    if (i == IMMEDIATE) {
        text_.emplace_back("li\t$t8, " + str);
        text_.emplace_back(vector[0] + "\t$t8, " + vector[2]);
    } else {
        text_.emplace_back(vector[0] + "\t" + str + ", " + vector[2]);
    }
}

void to_set(vector<string> &vector) {
    int i,j;
    string str1 = getExpr(vector[2], &i, true);
    string str2 = getExpr(vector[3], &j, false);
    if (i == IMMEDIATE && j == IMMEDIATE) {
        text_.emplace_back("li\t$t8, " + str1);
        if (vector[0] == "slt") {
            text_.emplace_back("slti\t$" + vector[1] + ", $t8, " + str2);
        } else {
            text_.emplace_back("li\t$t9, " + str2);
            text_.emplace_back(vector[0] + "\t$" + vector[1] + ", $t8, $t9");
        }
    } else if (i == IMMEDIATE && j == reg) {
        text_.emplace_back("li\t$t8, " + str1);
        text_.emplace_back(vector[0] + "\t$" + vector[1] + ", $t8, " + str2);
    } else if (i == reg && j == IMMEDIATE) {
        if (vector[0] == "slt") {
            text_.emplace_back("slti\t$" + vector[1] + ", " + str1 + ", " + str2);
        } else {
            text_.emplace_back("li\t$t9, " + str2);
            text_.emplace_back(vector[0] + "\t$" + vector[1] + ", " + str1 + ", $t9");
        }
    } else {
        text_.emplace_back(vector[0] + "\t$" + vector[1] + ", " + str1 + ", " + str2);
    }
}
