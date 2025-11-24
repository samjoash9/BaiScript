%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

int yylex(void);
extern int parse_failed;
extern ASTNode *root;
extern void yyerror(const char *s);

/* Track real line number per input line */
int lineCount = 1;
%}

/* Semantic value types */
%union {
    char *str;       /* for token text */
    ASTNode *node;   /* for AST nodes */
}

/* Tokens */
%token KUAN ENTEGER CHAROT
%token PRENT
%token EXCLAM
%token PLUS MINUS MUL DIV
%token PLUSPLUS MINUSMINUS
%token LPAREN RPAREN COMMA
%token NEWLINE

%token <str> IDENTIFIER INT_LITERAL CHAR_LITERAL STRING_LITERAL
%token <str> PLUS_EQUAL MINUS_EQUAL MUL_EQUAL DIV_EQUAL EQUAL

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
    {
        root = new_node(NODE_START, "START", $1, NULL, lineCount);
    }
;

/* List of statements (can be empty) */
STATEMENT_LIST:
      STATEMENT STATEMENT_LIST
      { $$ = new_node(NODE_STATEMENT_LIST, "STMT_LIST", $1, $2, lineCount); }
    | /* empty */
      { $$ = NULL; }
;

/* Statements must end with EXCLAM '!' */
STATEMENT:
      DECLARATION EXCLAM       { $$ = new_node(NODE_STATEMENT, "DECL_STMT", $1, NULL, lineCount); }
    | ASSIGNMENT EXCLAM        { $$ = new_node(NODE_STATEMENT, "ASSIGN_STMT", $1, NULL, lineCount); }
    | SIMPLE_EXPR EXCLAM       { $$ = new_node(NODE_STATEMENT, "EXPR_STMT", $1, NULL, lineCount); }
    | PRINTING EXCLAM          { $$ = new_node(NODE_STATEMENT, "PRINT_STMT", $1, NULL, lineCount); }
    | EXCLAM                   { $$ = new_node(NODE_STATEMENT, "EMPTY!", NULL, NULL, lineCount); }
    | error NEWLINE            { yyerror("Invalid statement"); yyerrok; ++lineCount; islexerror=0; }
    | NEWLINE                  { ++lineCount; }
;

/* Printing */
PRINTING:
    PRENT PRINT_LIST
    { $$ = new_node(NODE_PRINTING, "PRINT", $2, NULL, lineCount); }
;

PRINT_LIST:
      PRINT_ITEM PRINT_LIST_PRIME
      { $$ = new_node(NODE_PRINT_ITEM, "PRINT_LIST", $1, $2, lineCount); }
    | /* empty */
      { $$ = NULL; }
;

PRINT_LIST_PRIME:
      COMMA PRINT_ITEM PRINT_LIST_PRIME
      { $$ = new_node(NODE_PRINT_ITEM, "PRINT_ITEM", $2, $3, lineCount); }
    | /* empty */
      { $$ = NULL; }
;

PRINT_ITEM:
      SIMPLE_EXPR              { $$ = $1; }
    | STRING_LITERAL           { $$ = new_node(NODE_LITERAL, $1, NULL, NULL, lineCount); }
;

/* Declarations */
DECLARATION:
    DATATYPE INIT_DECLARATOR_LIST
    {
        $$ = new_node(NODE_DECLARATION, $1 ? $1->value : "TYPE", $2, NULL, lineCount);
        if ($1) { free($1->value); free($1); }
    }
;

DATATYPE:
      CHAROT   { $$ = new_node(NODE_DATATYPE, "CHAROT", NULL, NULL, lineCount); }
    | ENTEGER  { $$ = new_node(NODE_DATATYPE, "ENTEGER", NULL, NULL, lineCount); }
    | KUAN     { $$ = new_node(NODE_DATATYPE, "KUAN", NULL, NULL, lineCount); }
;

INIT_DECLARATOR_LIST:
      INIT_DECLARATOR
      { $$ = $1; }
    | INIT_DECLARATOR COMMA INIT_DECLARATOR_LIST
      { $$ = new_node(NODE_DECLARATION, "DECL", $1, $3, lineCount); }
;

INIT_DECLARATOR:
      DECLARATOR
      { $$ = $1; }
    | DECLARATOR EQUAL SIMPLE_EXPR
      { $$ = new_node(NODE_DECLARATION, "INIT_DECL", $1, $3, lineCount); }
;

DECLARATOR:
      IDENTIFIER
      { $$ = new_node(NODE_IDENTIFIER, $1, NULL, NULL, lineCount); }
    | LPAREN DECLARATOR RPAREN
      { $$ = $2; }
;

/* Assignment */
ASSIGNMENT:
      IDENTIFIER ASSIGN_OP ASSIGNMENT
      {
          ASTNode *id = new_node(NODE_IDENTIFIER, $1, NULL, NULL, lineCount);
          $$ = new_node(NODE_ASSIGNMENT, $2->value, id, $3, lineCount);
          free($2->value); free($2);
      }
    | IDENTIFIER ASSIGN_OP SIMPLE_EXPR
      {
          ASTNode *id = new_node(NODE_IDENTIFIER, $1, NULL, NULL, lineCount);
          $$ = new_node(NODE_ASSIGNMENT, $2->value, id, $3, lineCount);
          free($2->value); free($2);
      }
;

/* ASSIGN_OP returns a node carrying the operator text */
ASSIGN_OP:
      EQUAL        { $$ = new_node(NODE_UNKNOWN, $1, NULL, NULL, lineCount); }
    | PLUS_EQUAL   { $$ = new_node(NODE_UNKNOWN, $1, NULL, NULL, lineCount); }
    | MINUS_EQUAL  { $$ = new_node(NODE_UNKNOWN, $1, NULL, NULL, lineCount); }
    | DIV_EQUAL    { $$ = new_node(NODE_UNKNOWN, $1, NULL, NULL, lineCount); }
    | MUL_EQUAL    { $$ = new_node(NODE_UNKNOWN, $1, NULL, NULL, lineCount); }
;

/* Expressions */
SIMPLE_EXPR:
      ADD_EXPR { $$ = $1; }
;

ADD_EXPR:
      ADD_EXPR PLUS TERM   { $$ = new_node(NODE_EXPRESSION, "+", $1, $3, lineCount); }
    | ADD_EXPR MINUS TERM  { $$ = new_node(NODE_EXPRESSION, "-", $1, $3, lineCount); }
    | TERM                 { $$ = $1; }
;

TERM:
      TERM MUL FACTOR      { $$ = new_node(NODE_TERM, "*", $1, $3, lineCount); }
    | TERM DIV FACTOR      { $$ = new_node(NODE_TERM, "/", $1, $3, lineCount); }
    | FACTOR               { $$ = $1; }
;

FACTOR:
      UNARY      
    | POSTFIX    
;

UNARY:
      PLUS FACTOR          { $$ = new_node(NODE_UNARY_OP, "+", $2, NULL, lineCount); }
    | MINUS FACTOR         { $$ = new_node(NODE_UNARY_OP, "-", $2, NULL, lineCount); }
    | PLUSPLUS POSTFIX     { $$ = new_node(NODE_UNARY_OP, "++", $2, NULL, lineCount); }
    | MINUSMINUS POSTFIX   { $$ = new_node(NODE_UNARY_OP, "--", $2, NULL, lineCount); }
;

POSTFIX:
      PRIMARY POSTFIX_OPT
      {
          if ($2) {
              $$ = new_node(NODE_POSTFIX_OP, $2->value, $1, NULL, lineCount);
              free($2->value); free($2);
          } else {
              $$ = $1;
          }
      }
;

POSTFIX_OPT:
      /* empty */ { $$ = NULL; }
    | PLUSPLUS   { $$ = new_node(NODE_UNKNOWN, "++", NULL, NULL, lineCount); }
    | MINUSMINUS { $$ = new_node(NODE_UNKNOWN, "--", NULL, NULL, lineCount); }
;

PRIMARY:
      IDENTIFIER    { $$ = new_node(NODE_IDENTIFIER, $1, NULL, NULL, lineCount); }
    | INT_LITERAL   { $$ = new_node(NODE_LITERAL, $1, NULL, NULL, lineCount); }
    | CHAR_LITERAL  { $$ = new_node(NODE_LITERAL, $1, NULL, NULL, lineCount); }
    | LPAREN SIMPLE_EXPR RPAREN { $$ = $2; }
;

%%

/* Error handler */
void yyerror(const char *s) {
    if (islexerror == 0) {               // only log if no previous error
        FILE *out = fopen("output_print.txt", "w"); // overwrite
        if (out) {
            fprintf(out, "[PARSE] Syntax Invalid [line:%d]\n", lineCount);
            fclose(out);
        }
        islexerror = 1;                  // mark that an error was logged
    }
    parse_failed = 1;
}

