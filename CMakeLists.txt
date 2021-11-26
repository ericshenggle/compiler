cmake_minimum_required(VERSION 3.13)
project(compiler)

set(CMAKE_CXX_STANDARD 11)

add_executable(compiler include/main.h main.cpp include/Lexer.h Lexer.cpp include/Parser.h Parser.cpp include/SymbolType.h SymbolType.cpp ast.cpp include/ast.h include/mips.h mips.cpp)