//
// Created by yangcg on 2021/10/11.
//
#include "include/Lexer.h"

using std::endl;

class Token_format{
public:
    int code = -1;
    char txt[256] = {'\0'};
    int line = 0;
};
Format_string* format_string = new Format_string;       /* NOLINT */
Token_format* token_format[100000];

void error_Lexer() {
    std::cerr << "invalid Syntax" << endl;
    exit(-1);
}

int reServer() {
    if (strcmp(token, "main") == 0) {
        return SERSY_main;
    } else if (strcmp(token, "const") == 0) {
        return SERSY_const;
    } else if (strcmp(token, "int") == 0) {
        return SERSY_int;
    } else if (strcmp(token, "break") == 0) {
        return SERSY_break;
    } else if (strcmp(token, "continue") == 0) {
        return SERSY_continue;
    } else if (strcmp(token, "if") == 0) {
        return SERSY_if;
    } else if (strcmp(token, "else") == 0) {
        return SERSY_else;
    } else if (strcmp(token, "while") == 0) {
        return SERSY_while;
    } else if (strcmp(token, "getint") == 0) {
        return SERSY_getint;
    } else if (strcmp(token, "printf") == 0) {
        return SERSY_printf;
    } else if (strcmp(token, "return") == 0) {
        return SERSY_return;
    } else if (strcmp(token, "void") == 0) {
        return SERSY_void;
    }
    return 0;
}

void getSym() {
    output_Lexer();
    getSym2();
}

void getSym2() {
    while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r') {
        if (*str == '\n') {
            line++;
        }
        str++;
    }
    char *p = str;
    memset(token, 0, MAX_WORD);
    char *tmp = token;
    if (isdigit(*p)) {
        num = 0;
        while (isdigit(*p)) {
            num *= 10;
            num += (*p - '0');
            *tmp++ = *p++;
        }
        if (isalpha(*p)) {
            error_Lexer();
        }
        sym = INTSY;
    } else if (isalpha(*p) || *p == '_') {
        bool not_server = false;
        while (isalpha(*p) || isdigit(*p) || *p == '_') {
            if (*p == '_') {
                not_server = true;
            }
            *tmp++ = *p++;
        }
        if (not_server) {
            sym = IDSY;
        } else {
            int resultValue = reServer();
            if (resultValue == 0) {
                sym = IDSY;
            } else {
                sym = resultValue;
            }
        }
    } else if (*p == '"') {
        *tmp++ = *p++;
        format_string->is_wrong = false;
        format_string->exp_num = 0;
        while (*p != '"') {
            if (*p == '\0') {
                error_Lexer();
            } else if (*p == '\\') {
                if (*(p+1) != 'n') {
                    format_string->is_wrong = true;
                }
            } else if (*p != 32 && *p != 33 && (*p < 40 || *p > 126)) {
                if (*p == '%') {
                    if (*(p+1) != 'd') {
                        format_string->is_wrong = true;
                    } else {
                        format_string->exp_num++;
                    }
                } else {
                    format_string->is_wrong = true;
                }
            }
            *tmp++ = *p++;
        }
        *tmp = *p++;
        sym = FORMATSY;
    } else if (*p == '\0'){
        sym = END;
    } else {
        switch (*p++) {
            case '!':
                *tmp++ = '!';
                if (*p++ == '=') {
                    *tmp = '=';
                    sym = NEQ;
                } else {
                    p--;
                    sym = NOT;
                }
                break;
            case '&':
                *tmp++ = '&';
                if (*p++ == '&') {
                    *tmp = '&';
                    sym = AND;
                } else {
                    error_Lexer();
                }
                break;
            case '|':
                *tmp++ = '|';
                if (*p++ == '|') {
                    *tmp = '|';
                    sym = OR;
                } else {
                    error_Lexer();
                }
                break;
            case '+':
                *tmp = '+';
                sym = PLUS;
                break;
            case '-':
                *tmp = '-';
                sym = MINU;
                break;
            case '*':
                *tmp = '*';
                sym = MULT;
                break;
            case '/':
                p--;
                if (jump_annotation(&p)) {
                    str = p;
                    getSym2();
                    return;
                } else {
                    p++;
                    *tmp = '/';
                    sym = DIV;
                }
                break;
            case '%':
                *tmp = '%';
                sym = MOD;
                break;
            case '<':
                *tmp++ = '<';
                if (*p++ == '=') {
                    *tmp = '=';
                    sym = LEQ;
                } else {
                    p--;
                    sym = LSS;
                }
                break;
            case '>':
                *tmp++ = '>';
                if (*p++ == '=') {
                    *tmp = '=';
                    sym = GEQ;
                } else {
                    p--;
                    sym = GRE;
                }
                break;
            case '=':
                *tmp++ = '=';
                if (*p++ == '=') {
                    *tmp = '=';
                    sym = EQL;
                } else {
                    p--;
                    sym = ASSGIN;
                }
                break;
            case ';':
                *tmp = ';';
                sym = SEMICN;
                break;
            case ',':
                *tmp = ',';
                sym = COMMA;
                break;
            case '(':
                *tmp = '(';
                sym = LPARENT;
                break;
            case ')':
                *tmp = ')';
                sym = RPARENT;
                break;
            case '[':
                *tmp = '[';
                sym = LBRACK;
                break;
            case ']':
                *tmp = ']';
                sym = RBRACK;
                break;
            case '{':
                *tmp = '{';
                sym = LBRACE;
                break;
            case '}':
                *tmp = '}';
                sym = RBRACE;
                break;
            default:
                sym = NO_MATCH;
                break;
        }
    }
    str = p;
    if (sym > -1) {
        auto t = new Token_format;
        t->code = sym;
        if (sym == IDSY) {
            strcpy(t->txt, token);
            if (token[0] == 't' && isdigit(token[1]) && token[2] == '\0') {
                token[2] = '0';
            }
        } else {
            strcpy(t->txt, token);
        }
        t->line = line;
        token_format[cnt++] = t;
    }
}

int jump_annotation(char **pString) {
    char *p = *pString;
    int code = 0;
    if (*p == '/') {
        p++;
        if (*p == '/') {
            while (*p != '\0') {
                if (*p == '\n') {
                    line++;
                    p++;
                    break;
                }
                p++;
            }
            code = 1;
        } else if (*p == '*') {
            p++;
            is_annotation = true;
            while (is_annotation) {
                if (*p == '*') {
                    p++;
                    if (*p == '/') {
                        p++;
                        is_annotation = false;
                    }
                } else if (*p == '\n'){
                    line++;
                    p++;
                } else {
                    p++;
                }
            }
            code = 1;
        } else {
            p--;
        }
    }
    *pString = p;
    return code;
}

void output_Lexer() {
#ifdef OUTPUT_Lexer
    if (token_format[cnt_output]->code >= 0) {
        outfile << category[token_format[cnt_output]->code] + " " + token_format[cnt_output]->txt << endl;
        cnt_output = cnt;
    }
#endif
}

void output_Lexer2() {
#ifdef OUTPUT_Lexer
    for (int i = cnt_output; i < cnt - 1; ++i) {
        outfile << category[token_format[i]->code] + " " << token_format[i]->txt << endl;
    }
    cnt_output = cnt - 1;
#endif
}


