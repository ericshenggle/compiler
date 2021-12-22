//
// Created by ycg on 2021/11/9.
//

#ifndef COMPILER_MIPS_H
#define COMPILER_MIPS_H

#include <deque>
#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>

#define IMMEDIATE 1
#define reg 2
#define OUTPUT_MIPS

using std::deque;
using std::vector;
using std::string;
using std::pair;
using std::strtof;
using std::map;
using std::ofstream;
using std::endl;

struct symMap {
    int sp;             //栈地址
    bool isAddress;     //是否为数组地址
    int sReg;           //全局寄存器
    string name;
};

class Map {
public:
    void insert(const symMap& symMap) {
        maps.emplace_back(symMap);
    }
    void pop(int i) {
        if (!maps.empty()) {
            int tmp = -1;
            for (int j = 0; j < maps.size(); ++j) {
                if (maps[j].sp < i) {
                    tmp = j;
                    break;
                }
            }
            if (tmp != -1) {
                maps.erase(maps.begin() + tmp, maps.end());
            }
        }
    }
    bool findSpOrSReg(const string& ident, int *spOrReg, bool *isAddress) {         //查找是在栈中还是寄存器中，并返回是否为数组地址
        for (auto p = maps.rbegin(); p != maps.rend(); ++p) {
            const symMap& symMap = *p;
            if (symMap.name == ident) {
                *isAddress = symMap.isAddress;
                if (symMap.sReg != -1) {
                    *spOrReg = symMap.sReg;
                    return true;
                } else {
                    *spOrReg = symMap.sp;
                    return true;
                }
            }
        }
        return false;
    }
    void insertSp1(int sReg, int sp) {                   //寄存器保存至栈
        for (auto p = maps.rbegin(); p != maps.rend(); ++p) {
            symMap &symMap = *p;
            if (symMap.sReg == sReg) {
                symMap.sp = sp;
                symMap.sReg = -1;
                break;
            }
        }
    }
    void insertsReg(int sReg, int sp, int newSp) {                 //栈保存至寄存器
        for (auto p = maps.rbegin(); p != maps.rend(); ++p) {
            symMap &symMap = *p;
            if (symMap.sp == sp) {
                symMap.sReg = sReg;
                symMap.sp = newSp;
                break;
            }
        }
    }

    vector<symMap> maps;

};
extern ofstream mipsfile;

void mips(deque<vector<string>> &deque);
void to_mips(vector<string> &vector1);
void to_const(vector<string> &vector);
void to_var(vector<string> &vector);
void to_string(vector<string> &vector);
void to_ret(vector<string> &vector);
void to_push(vector<string> &vector);
void to_call(vector<string> &vector);
void to_expr(vector<string> &vector);
void to_output(vector<string> &vector);
void to_param(vector<string> &vector);
void to_branch(vector<string> &vector);
void to_set(vector<string> &vector);
void pushExpr(vector<string> &vector, const string& s);
string getExpr(string s, int *type, bool l);

#endif //COMPILER_MIPS_H
