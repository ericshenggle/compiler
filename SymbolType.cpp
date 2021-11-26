//
// Created by yangcg on 2021/10/15.
//

#include "include/SymbolType.h"

SymbolMap::~SymbolMap() {
    for (const auto &p : Symbol_map) {
        Symbol* pid = p.second;
        if (pid->type == const_array_t || pid->type == int_array_t) {
            auto pidd = (Array_data *)pid->data;
            delete pidd;
        } else if (pid->type == function_int_t || pid->type == function_void_t){
            auto pidd = (Func_data *)pid->data;
            delete(pidd);
        }
        delete pid;
    }
}

map<string,Symbol*>& SymbolMap::getAll() {
    return Symbol_map;
}

Symbol* SymbolMap::find(const string &str) const {
    auto p = Symbol_map.find(str);
    if (p != Symbol_map.end())
        return (*p).second;
    return nullptr;
}

bool SymbolMap::insert(Symbol* symbol) {
    string str = symbol->str;
    auto p = Symbol_map.find(str);
    if (p != Symbol_map.end()) {
        return false;
    }
    Symbol_map[str] = symbol;
    return true;
}

map<string,Symbol*>& SymbolTable::getAll(int level) {
    return Symbol_stack[level].getAll();
}

Symbol* SymbolTable::find(const string &str) const {
    for (auto p = Symbol_stack.rbegin(); p != Symbol_stack.rend(); ++p) {
        const SymbolMap& symbolMap = *p;
        Symbol* pid = symbolMap.find(str);
        if (pid != nullptr) {
            return pid;
        }
    }
    return nullptr;
}

bool SymbolTable::insert(Symbol* symbol) {
    SymbolMap& symbolMap = Symbol_stack.back();
    return symbolMap.insert(symbol);
}

void SymbolTable::push() {
    Symbol_stack.emplace_back(SymbolMap());
}

void SymbolTable::pop() {
    Symbol_stack.pop_back();
}

int SymbolTable::getLevel() {
    return (int)Symbol_stack.size() - 1;
}