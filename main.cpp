#include "include/Lexer.h"
#include "include/Parser.h"
#include "include/mips.h"

using std::cerr;
using std::endl;
using std::ofstream;
using std::ifstream;

ofstream outfile, errorfile;       /* NOLINT */
char *str;
vector<string> category = {         /* NOLINT */
        "IDENFR", "INTCON", "STRCON", "MAINTK", "CONSTTK", "INTTK", "BREAKTK", "CONTINUETK",
        "IFTK", "ELSETK", "WHILETK", "GETINTTK", "PRINTFTK", "RETURNTK", "VOIDTK",
        "NOT", "AND", "OR", "PLUS", "MINU",  "MULT", "DIV", "MOD", "LSS",
        "LEQ", "GRE", "GEQ", "EQL", "NEQ", "ASSIGN", "SEMICN", "COMMA", "LPARENT", "RPARENT",
        "LBRACK", "RBRACK", "LBRACE", "RBRACE"
};
bool is_annotation = false;
int line = 0;
int cnt = 0, cnt_output = 0;
char token[MAX_WORD];
int sym = -1;
int num = 0;

char * ReadFile(long *length)
{
    FILE *pfile;
    char *data;
    pfile = fopen("testfile.txt", "r");
    if (pfile == nullptr) {
        cerr << "Open testfile failed" << endl;
        exit(-1);
    }
    fseek(pfile, 0, SEEK_END);
    *length = ftell(pfile);
    data = (char *)malloc((*length + 1) * sizeof(char));
    memset(data, 0, (*length + 1) * sizeof(char));
    rewind(pfile);
    fread(data, 1, (size_t) *length, pfile);
    fclose(pfile);
    return data;
}

void WriteFile() {
    outfile.open("output.txt", std::ios::trunc);
    if (!outfile.is_open())
    {
        cerr << "Open output failed" << endl;
        exit(-1);
    }
}

void errorFile() {
    errorfile.open("error.txt", std::ios::trunc);
    if (!errorfile.is_open())
    {
        cerr << "Open error failed" << endl;
        exit(-1);
    }
}

void parser() {
    symbolTable_Var = new SymbolTable();
    symbolTable_Func = new SymbolTable();
    SymbolTable_push();
    compUnit* compunit = CompUnit();
    compunit->mid_code();
    SymbolTable_pop();
    delete symbolTable_Var;
    delete symbolTable_Func;
    mips(midCode);
    delete compunit;
}

int main() {
    long length = 0;
    char *data;
    data = ReadFile(&length);
    str = data;
#ifdef OUTPUT_Lexer
#ifdef OUTPUT_Parser
    WriteFile();
#endif
#endif
#ifdef OUTPUT_Parser_WRONG
    errorFile();
#endif
    if (*str != '\0') {
        line++;
    }
    parser();
    free(data);
    outfile.close();
    errorfile.close();
    return 0;
}