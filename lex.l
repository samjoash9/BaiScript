%{
    #include <stdio.h>
    #include <string.h>
    #include "ast.h"
    #include "yacc.tab.h"

    extern void yyerror(const char *s);
%}

%x COMMENT
%option yylineno

%%

[ \t\r]+      ;

"//".*        { printf("[LEX] COMMENT LINE\n"); }

"/*"          { printf("[LEX] COMMENT START\n"); BEGIN(COMMENT); }
<COMMENT>"*/" { printf("[LEX] COMMENT END\n"); BEGIN(INITIAL); }
<COMMENT>.    ;
<COMMENT>\n   ;

"KUAN"      { printf("[LEX] KUAN\n"); return KUAN; }
"ENTEGER"   { printf("[LEX] ENTEGER\n"); return ENTEGER; }
"CHAROT"    { printf("[LEX] CHAROT\n"); return CHAROT; }
"PRENT"     { printf("[LEX] PRENT\n"); return PRENT; }

"+="        { printf("[LEX] PLUS_EQUAL (+=)\n"); yylval.str = strdup(yytext); return PLUS_EQUAL; }
"-="        { printf("[LEX] MINUS_EQUAL (-=)\n"); yylval.str = strdup(yytext); return MINUS_EQUAL; }
"/="        { printf("[LEX] DIV_EQUAL (/=)\n"); yylval.str = strdup(yytext); return DIV_EQUAL; }
"*="        { printf("[LEX] MUL_EQUAL (*=)\n"); yylval.str = strdup(yytext); return MUL_EQUAL; }

"++"        { printf("[LEX] PLUSPLUS (++)\n"); yylval.str = strdup(yytext); return PLUSPLUS; }
"--"        { printf("[LEX] MINUSMINUS (--)\n"); yylval.str = strdup(yytext); return MINUSMINUS; }

"+"         { printf("[LEX] PLUS (+)\n"); return PLUS; }
"-"         { printf("[LEX] MINUS (-)\n"); return MINUS; }
"*"         { printf("[LEX] MUL (*)\n"); return MUL; }
"/"         { printf("[LEX] DIV (/)\n"); return DIV; }
"="         { printf("[LEX] EQUAL (=)\n"); yylval.str = strdup(yytext); return EQUAL; }

"!"         { printf("[LEX] EXCLAM (!)\n"); return EXCLAM; }
"("         { printf("[LEX] LPAREN\n"); return LPAREN; }
")"         { printf("[LEX] RPAREN\n"); return RPAREN; }
","         { printf("[LEX] COMMA\n"); return COMMA; }

[0-9][0-9]* {
    printf("[LEX] INT_LITERAL (%s)\n", yytext);
    yylval.str = strdup(yytext);
    return INT_LITERAL;
}

\'[^\']\' {
    printf("[LEX] CHAR_LITERAL (%s)\n", yytext);
    yylval.str = strdup(yytext);
    return CHAR_LITERAL;
}

\"[^"]*\" {
    printf("[LEX] STRING_LITERAL (%s)\n", yytext);
    yylval.str = strdup(yytext);
    return STRING_LITERAL;
}

[A-Za-z_][A-Za-z0-9_]* {
    printf("[LEX] IDENTIFIER (%s)\n", yytext);
    yylval.str = strdup(yytext);
    return IDENTIFIER;
}

\n {
    printf("[LEX] NEWLINE\n");
}

. {
    printf("[LEX] INVALID (%s)\n", yytext);
    yyerror("Invalid character");
}

%%

int yywrap(void) { return 1; }
