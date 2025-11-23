%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ast.h"
#include "yacc.tab.h"

extern void yyerror(const char *s);
extern int lineCount;
%}

%x COMMENT
%option noyywrap
%option yylineno

%%

[ \t\r]+              ;  // skip whitespace

"//".*                { printf("[LEX] COMMENT LINE\n"); }
"/*"                  { printf("[LEX] COMMENT START\n"); BEGIN(COMMENT); }
<COMMENT>"*/"         { printf("[LEX] COMMENT END\n"); BEGIN(INITIAL); }
<COMMENT>\n           { yylineno++; }
<COMMENT>.            ;  // ignore other comment chars


"KUAN"                { printf("[LEX] KUAN\n"); return KUAN; }
"ENTEGER"             { printf("[LEX] ENTEGER\n"); return ENTEGER; }
"CHAROT"              { printf("[LEX] CHAROT\n"); return CHAROT; }
"PRENT"               { printf("[LEX] PRENT\n"); return PRENT; }


"+="                  { yylval.str = strdup(yytext); printf("[LEX] PLUS_EQUAL (+=)\n"); return PLUS_EQUAL; }
"-="                  { yylval.str = strdup(yytext); printf("[LEX] MINUS_EQUAL (-=)\n"); return MINUS_EQUAL; }
"/="                  { yylval.str = strdup(yytext); printf("[LEX] DIV_EQUAL (/=)\n"); return DIV_EQUAL; }
"*="                  { yylval.str = strdup(yytext); printf("[LEX] MUL_EQUAL (*=)\n"); return MUL_EQUAL; }
"++"                  { yylval.str = strdup(yytext); printf("[LEX] PLUSPLUS (++)\n"); return PLUSPLUS; }
"--"                  { yylval.str = strdup(yytext); printf("[LEX] MINUSMINUS (--)\n"); return MINUSMINUS; }


"+"                   { printf("[LEX] PLUS (+)\n"); return PLUS; }
"-"                   { printf("[LEX] MINUS (-)\n"); return MINUS; }
"*"                   { printf("[LEX] MUL (*)\n"); return MUL; }
"/"                   { printf("[LEX] DIV (/)\n"); return DIV; }
"="                   { yylval.str = strdup(yytext); printf("[LEX] EQUAL (=)\n"); return EQUAL; }
"!"                   { printf("[LEX] EXCLAM (!)\n"); return EXCLAM; }
"("                   { printf("[LEX] LPAREN\n"); return LPAREN; }
")"                   { printf("[LEX] RPAREN\n"); return RPAREN; }
","                   { printf("[LEX] COMMA\n"); return COMMA; }


[0-9]+                { yylval.str = strdup(yytext); printf("[LEX] INT_LITERAL (%s)\n", yytext); return INT_LITERAL; }
\'[^\']\'             { yylval.str = strdup(yytext); printf("[LEX] CHAR_LITERAL (%s)\n", yytext); return CHAR_LITERAL; }
\"[^\"]*\"             { yylval.str = strdup(yytext); printf("[LEX] STRING_LITERAL (%s)\n", yytext); return STRING_LITERAL; }


[A-Za-z_][A-Za-z0-9_]* {
    yylval.str = strdup(yytext);
    printf("[LEX] IDENTIFIER (%s)\n", yytext);
    return IDENTIFIER;
}


\n                    { lineCount++; printf("[LEX] NEWLINE\n"); }


.                     { printf("[LEX] INVALID (%s)\n", yytext); yyerror("Invalid character"); }

%%

