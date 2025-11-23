%{
/* Prologue: includes and forward declarations */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"           /* ASTNode, NodeType, new_node, extern ASTNode *root; */

int yylex(void);          /* lexer prototype */
extern int parse_failed;  /* defined in main.c */
void yyerror(const char *s);
%}

/* Semantic value types */
%union {
    char *str;       /* for token text */
    ASTNode *node;   /* for AST nodes */
}

/* Tokens (must match lex.l) */
%token KUAN ENTEGER CHAROT
%token PRENT
%token EXCLAM
%token PLUS_EQUAL MINUS_EQUAL DIV_EQUAL MUL_EQUAL EQUAL
%token PLUS MINUS MUL DIV
%token PLUSPLUS MINUSMINUS
%token LPAREN RPAREN COMMA

%token <str> IDENTIFIER INT_LITERAL CHAR_LITERAL STRING_LITERAL

/* Nonterminals that produce AST nodes */
%type <node> S STATEMENT_LIST STATEMENT
%type <node> PRINTING PRINT_LIST PRINT_LIST_PRIME PRINT_ITEM
%type <node> DECLARATION DATATYPE INIT_DECLARATOR_LIST INIT_DECLARATOR DECLARATOR
%type <node> ASSIGNMENT ASSIGN_OP
%type <node> SIMPLE_EXPR ADD_EXPR TERM FACTOR UNARY POSTFIX POSTFIX_OPT PRIMARY

%start S

%%

/* Program */
S:
    STATEMENT_LIST
    { root = new_node(NODE_START, "START", $1, NULL); }
;

/* list of statements (can be empty) */
STATEMENT_LIST:
      STATEMENT STATEMENT_LIST
        { $$ = new_node(NODE_STATEMENT_LIST, "STMT_LIST", $1, $2); }
    | /* empty */
        { $$ = NULL; }
;

/* statements: everything must end with EXCLAM '!' except a bare EXCLAM */
STATEMENT:
      DECLARATION EXCLAM
        { $$ = new_node(NODE_STATEMENT, "DECL_STMT", $1, NULL); }
    | ASSIGNMENT EXCLAM
        { $$ = new_node(NODE_STATEMENT, "ASSIGN_STMT", $1, NULL); }
    | SIMPLE_EXPR EXCLAM
        { $$ = new_node(NODE_STATEMENT, "EXPR_STMT", $1, NULL); }
    | PRINTING EXCLAM
        { $$ = new_node(NODE_STATEMENT, "PRINT_STMT", $1, NULL); }
    | EXCLAM
        { $$ = new_node(NODE_STATEMENT, "EMPTY!", NULL, NULL); }
;

/* Printing */
PRINTING:
    PRENT PRINT_LIST
        { $$ = new_node(NODE_PRINTING, "PRINT", $2, NULL); }
;

PRINT_LIST:
      PRINT_ITEM PRINT_LIST_PRIME
        { $$ = new_node(NODE_PRINT_ITEM, "PRINT_LIST", $1, $2); }
    | /* empty */
        { $$ = NULL; }
;

PRINT_LIST_PRIME:
      COMMA PRINT_ITEM PRINT_LIST_PRIME
        { $$ = new_node(NODE_PRINT_ITEM, "PRINT_ITEM", $2, $3); }
    | /* empty */
        { $$ = NULL; }
;

PRINT_ITEM:
      SIMPLE_EXPR
        { $$ = $1; }
    | STRING_LITERAL
        { $$ = new_node(NODE_LITERAL, $1, NULL, NULL); }
;

/* Declarations */
DECLARATION:
    DATATYPE INIT_DECLARATOR_LIST
        {
            /* make declaration node with datatype label */
            $$ = new_node(NODE_DECLARATION, $1 ? $1->value : "TYPE", $2, NULL);
            if ($1) { free($1->value); free($1); }  /* we duplicated datatype text in new_node */
        }
;

DATATYPE:
      CHAROT   { $$ = new_node(NODE_DATATYPE, "CHAROT", NULL, NULL); }
    | ENTEGER  { $$ = new_node(NODE_DATATYPE, "ENTEGER", NULL, NULL); } /* matches lexer spelling */
    | KUAN     { $$ = new_node(NODE_DATATYPE, "KUAN", NULL, NULL); }
;

INIT_DECLARATOR_LIST:
      INIT_DECLARATOR
        { $$ = $1; }
    | INIT_DECLARATOR COMMA INIT_DECLARATOR_LIST
        { $$ = new_node(NODE_DECLARATION, "DECL", $1, $3); }
;

INIT_DECLARATOR:
      DECLARATOR
        { $$ = $1; }
    | DECLARATOR EQUAL SIMPLE_EXPR
        { $$ = new_node(NODE_DECLARATION, "INIT_DECL", $1, $3); }
;

DECLARATOR:
      IDENTIFIER
        { $$ = new_node(NODE_IDENTIFIER, $1, NULL, NULL); }
    | LPAREN DECLARATOR RPAREN
        { $$ = $2; }
;

/* Assignment */
ASSIGNMENT:
      IDENTIFIER ASSIGN_OP ASSIGNMENT
        {
            ASTNode *id = new_node(NODE_IDENTIFIER, $1, NULL, NULL);
            $$ = new_node(NODE_ASSIGNMENT, $2 ? $2->value : "ASSIGN", id, $3);
            if ($2) { free($2->value); free($2); } /* free temporary assign-op node */
        }
    | IDENTIFIER ASSIGN_OP SIMPLE_EXPR
        {
            ASTNode *id = new_node(NODE_IDENTIFIER, $1, NULL, NULL);
            $$ = new_node(NODE_ASSIGNMENT, $2 ? $2->value : "ASSIGN", id, $3);
            if ($2) { free($2->value); free($2); }
        }
;

/* ASSIGN_OP returns a small node carrying the operator text */
ASSIGN_OP:
      EQUAL        { $$ = new_node(NODE_UNKNOWN, "=", NULL, NULL); }
    | PLUS_EQUAL   { $$ = new_node(NODE_UNKNOWN, "+=", NULL, NULL); }
    | MINUS_EQUAL  { $$ = new_node(NODE_UNKNOWN, "-=", NULL, NULL); }
    | DIV_EQUAL    { $$ = new_node(NODE_UNKNOWN, "/=", NULL, NULL); }
    | MUL_EQUAL    { $$ = new_node(NODE_UNKNOWN, "*=", NULL, NULL); }
;

/* Expressions */
SIMPLE_EXPR:
    ADD_EXPR   { $$ = $1; }
;

ADD_EXPR:
      ADD_EXPR PLUS TERM
        { $$ = new_node(NODE_EXPRESSION, "+", $1, $3); }
    | ADD_EXPR MINUS TERM
        { $$ = new_node(NODE_EXPRESSION, "-", $1, $3); }
    | TERM
        { $$ = $1; }
;

TERM:
      TERM MUL FACTOR
        { $$ = new_node(NODE_TERM, "*", $1, $3); }
    | TERM DIV FACTOR
        { $$ = new_node(NODE_TERM, "/", $1, $3); }
    | FACTOR
        { $$ = $1; }
;

/* UNARY & POSTFIX */
FACTOR:
      UNARY
        { $$ = $1; }
    | POSTFIX
        { $$ = $1; }
;

UNARY:
      PLUS FACTOR
        { $$ = new_node(NODE_UNARY_OP, "+", $2, NULL); }
    | MINUS FACTOR
        { $$ = new_node(NODE_UNARY_OP, "-", $2, NULL); }
    | PLUSPLUS POSTFIX
        { $$ = new_node(NODE_UNARY_OP, "++", $2, NULL); }
    | MINUSMINUS POSTFIX
        { $$ = new_node(NODE_UNARY_OP, "--", $2, NULL); }
;

POSTFIX:
      PRIMARY POSTFIX_OPT
        {
            if ($2) {
                $$ = new_node(NODE_POSTFIX_OP, $2->value, $1, NULL);
                free($2->value); free($2);
            } else {
                $$ = $1;
            }
        }
;

POSTFIX_OPT:
      /* empty */ { $$ = NULL; }
    | PLUSPLUS   { $$ = new_node(NODE_UNKNOWN, "++", NULL, NULL); }
    | MINUSMINUS { $$ = new_node(NODE_UNKNOWN, "--", NULL, NULL); }
;

PRIMARY:
      IDENTIFIER
        { $$ = new_node(NODE_IDENTIFIER, $1, NULL, NULL); }
    | INT_LITERAL
        { $$ = new_node(NODE_LITERAL, $1, NULL, NULL); }
    | CHAR_LITERAL
        { $$ = new_node(NODE_LITERAL, $1, NULL, NULL); }
    | LPAREN SIMPLE_EXPR RPAREN
        { $$ = $2; }
;

%%

/* Error handler */
void yyerror(const char *s) {
    parse_failed = 1;
    fprintf(stderr, "[PARSE] Rejected (%s)\n", s ? s : "syntax error");
}
