%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "symbol_table.h"

int yylex(void);
extern int parse_failed;
extern ASTNode *root;
extern void yyerror(const char *s);

int lineCount = 1;
%}

/* Semantic value types */
%union {
    char *str;
    ASTNode *node;
}

/* Tokens */
%token KUAN ENTEGER CHAROT
%token PRENT EXCLAM
%token PLUS MINUS MUL DIV
%token PLUSPLUS MINUSMINUS
%token LPAREN RPAREN COMMA
%token NEWLINE
%token <str> IDENTIFIER INT_LITERAL CHAR_LITERAL STRING_LITERAL
%token <str> PLUS_EQUAL MINUS_EQUAL MUL_EQUAL DIV_EQUAL EQUAL

/* Nonterminals producing AST nodes */
%type <node> S STATEMENT_LIST STATEMENT
%type <node> PRINTING PRINT_LIST PRINT_LIST_PRIME PRINT_ITEM
%type <node> DECLARATION DATATYPE INIT_DECLARATOR_LIST INIT_DECLARATOR DECLARATOR
%type <node> ASSIGNMENT ASSIGN_OP
%type <node> SIMPLE_EXPR ADD_EXPR TERM FACTOR UNARY POSTFIX POSTFIX_OPT PRIMARY

%start S

%%

S:
    STATEMENT_LIST
    { 
        root = new_node(NODE_START, "START", $1, NULL, lineCount);
        $$ = root;
    }
;

STATEMENT_LIST:
      STATEMENT STATEMENT_LIST
      { ASTNode *head = $1; if ($2) head->right = $2; $$ = head; }
    | /* empty */ { $$ = NULL; }
;

STATEMENT:
      DECLARATION EXCLAM       { $$ = new_node(NODE_STATEMENT, "STMT", $1, NULL, lineCount); }
    | ASSIGNMENT EXCLAM        { $$ = new_node(NODE_STATEMENT, "STMT", $1, NULL, lineCount); }
    | SIMPLE_EXPR EXCLAM       { $$ = new_node(NODE_STATEMENT, "STMT", $1, NULL, lineCount); }
    | PRINTING EXCLAM          { $$ = new_node(NODE_STATEMENT, "STMT", $1, NULL, lineCount); }
    | EXCLAM                   { $$ = new_node(NODE_STATEMENT, "STMT", NULL, NULL, lineCount); }
    | error NEWLINE            { yyerror("Invalid statement"); yyerrok; ++lineCount; $$ = NULL; }
    | NEWLINE                  { ++lineCount; $$ = NULL; }
;

PRINTING:
    PRENT PRINT_LIST
    { $$ = $2; }
;

PRINT_LIST:
      PRINT_ITEM PRINT_LIST_PRIME
      { ASTNode *head = $1; if ($2) head->right = $2; $$ = head; }
    | /* empty */ { $$ = NULL; }
;

PRINT_LIST_PRIME:
      COMMA PRINT_ITEM PRINT_LIST_PRIME
      { ASTNode *head = $2; if ($3) head->right = $3; $$ = head; }
    | /* empty */ { $$ = NULL; }
;

PRINT_ITEM:
      SIMPLE_EXPR     { $$ = $1; }
    | STRING_LITERAL  { $$ = new_node(NODE_LITERAL, $1, NULL, NULL, lineCount); }
;

DECLARATION:
    DATATYPE INIT_DECLARATOR_LIST
    {
        ASTNode *decl_list_out = NULL;
        ASTNode *prev_out = NULL;
        ASTNode *iter = $2;

        while (iter) {
            int initialized = (iter->left != NULL);
            add_symbol(iter->value, $1->value, initialized, "");

            ASTNode *outnode = new_node(NODE_DECL, iter->value, iter->left, NULL, lineCount);
            if (!decl_list_out) decl_list_out = outnode;
            else prev_out->right = outnode;

            prev_out = outnode;
            iter = iter->right;
        }

        $$ = new_node(NODE_DECLARATION, $1->value, decl_list_out, NULL, lineCount);
        free($1->value); free($1);
    }
;

DATATYPE:
      CHAROT   { $$ = new_node(NODE_DATATYPE, "CHAR", NULL, NULL, lineCount); }
    | ENTEGER  { $$ = new_node(NODE_DATATYPE, "INT", NULL, NULL, lineCount); }
    | KUAN     { $$ = new_node(NODE_DATATYPE, "FLOAT", NULL, NULL, lineCount); }
;

INIT_DECLARATOR_LIST:
      INIT_DECLARATOR
      { $$ = $1; }
    | INIT_DECLARATOR COMMA INIT_DECLARATOR_LIST
      { $1->right = $3; $$ = $1; }
;

INIT_DECLARATOR:
      DECLARATOR EQUAL SIMPLE_EXPR
      { $$ = new_node(NODE_DECL, $1->value, $3, NULL, lineCount); free($1->value); free($1); }
    | DECLARATOR
      { $$ = new_node(NODE_DECL, $1->value, NULL, NULL, lineCount); free($1->value); free($1); }
;

DECLARATOR:
      IDENTIFIER { $$ = new_node(NODE_IDENTIFIER, $1, NULL, NULL, lineCount); }
    | LPAREN DECLARATOR RPAREN { $$ = $2; }
;

ASSIGNMENT:
      IDENTIFIER ASSIGN_OP ASSIGNMENT
      { ASTNode *id = new_node(NODE_IDENTIFIER, $1, NULL, NULL, lineCount);
        $$ = new_node(NODE_ASSIGNMENT, $2->value, id, $3, lineCount);
        free($2->value); free($2); }
    | IDENTIFIER ASSIGN_OP SIMPLE_EXPR
      { ASTNode *id = new_node(NODE_IDENTIFIER, $1, NULL, NULL, lineCount);
        $$ = new_node(NODE_ASSIGNMENT, $2->value, id, $3, lineCount);
        free($2->value); free($2); }
;

ASSIGN_OP:
      EQUAL        { $$ = new_node(NODE_UNKNOWN, $1, NULL, NULL, lineCount); }
    | PLUS_EQUAL   { $$ = new_node(NODE_UNKNOWN, $1, NULL, NULL, lineCount); }
    | MINUS_EQUAL  { $$ = new_node(NODE_UNKNOWN, $1, NULL, NULL, lineCount); }
    | DIV_EQUAL    { $$ = new_node(NODE_UNKNOWN, $1, NULL, NULL, lineCount); }
    | MUL_EQUAL    { $$ = new_node(NODE_UNKNOWN, $1, NULL, NULL, lineCount); }
;

SIMPLE_EXPR: ADD_EXPR { $$ = $1; };

ADD_EXPR:
      ADD_EXPR PLUS TERM   { $$ = new_node(NODE_EXPRESSION, "+", $1, $3, lineCount); }
    | ADD_EXPR MINUS TERM  { $$ = new_node(NODE_EXPRESSION, "-", $1, $3, lineCount); }
    | TERM                 { $$ = $1; }
;

TERM:
      TERM MUL FACTOR      { $$ = new_node(NODE_EXPRESSION, "*", $1, $3, lineCount); }
    | TERM DIV FACTOR      { $$ = new_node(NODE_EXPRESSION, "/", $1, $3, lineCount); }
    | FACTOR               { $$ = $1; }
;

FACTOR:
      UNARY
    | POSTFIX
;

UNARY:
      PLUS FACTOR        { $$ = new_node(NODE_UNARY_OP, "+", $2, NULL, lineCount); }
    | MINUS FACTOR       { $$ = new_node(NODE_UNARY_OP, "-", $2, NULL, lineCount); }
    | PLUSPLUS POSTFIX   { $$ = new_node(NODE_UNARY_OP, "++", $2, NULL, lineCount); }
    | MINUSMINUS POSTFIX { $$ = new_node(NODE_UNARY_OP, "--", $2, NULL, lineCount); }
;

POSTFIX:
      PRIMARY POSTFIX_OPT
      { if ($2) $$ = new_node(NODE_POSTFIX_OP, $2->value, $1, NULL, lineCount); else $$ = $1; }
;

POSTFIX_OPT:
      /* empty */ { $$ = NULL; }
    | PLUSPLUS   { $$ = new_node(NODE_POSTFIX_OP, "++", NULL, NULL, lineCount); }
    | MINUSMINUS { $$ = new_node(NODE_POSTFIX_OP, "--", NULL, NULL, lineCount); }
;

PRIMARY:
      IDENTIFIER    { $$ = new_node(NODE_IDENTIFIER, $1, NULL, NULL, lineCount); }
    | INT_LITERAL   { $$ = new_node(NODE_LITERAL, $1, NULL, NULL, lineCount); }
    | CHAR_LITERAL  { $$ = new_node(NODE_LITERAL, $1, NULL, NULL, lineCount); }
    | LPAREN SIMPLE_EXPR RPAREN { $$ = $2; }
;

%%

void yyerror(const char *s) {
    parse_failed = 1;
    fprintf(stderr, "[Parse Error] %s at line %d\n", s, lineCount);
}
