//
// Created by yangcg on 2021/10/15.
//
#include <map>
#include <string>
#include <deque>
#include <vector>

#ifndef COMPILER_SYMBOLTYPE_H
#define COMPILER_SYMBOLTYPE_H

using std::string;
using std::map;
using std::deque;
using std::vector;

enum SymbolType
{
    int_t, const_t, int_array_t, const_array_t, function_int_t, function_void_t
};

enum FuncParamType
{
    int_param, int_array1_param, int_array2_param, void_param
};

typedef struct {
    int length[2];
    vector<int> var;
}Array_data;

typedef struct {
    int num;
    FuncParamType params[20];
}Func_data;

struct Symbol {
    int dimension;
    SymbolType type;
    void* data;
    int value;
    string str;
};

class SymbolMap
{
public:
    SymbolMap() = default;
    ~SymbolMap();
    map<string,Symbol*>& getAll();
    Symbol* find(const string& str) const;
    bool insert(Symbol* symbol);

private:
    map<string, Symbol*> Symbol_map;
};

class SymbolTable
{
public:
    SymbolTable() = default;
    map<string,Symbol*>& getAll(int level);
    Symbol* find(const string& str) const;
    bool insert(Symbol* symbol);
    void push();
    void pop();
    int getLevel();

private:
    deque<SymbolMap> Symbol_stack;
};

#endif //COMPILER_SYMBOLTYPE_H
