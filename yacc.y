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
%token PRENT
%token EXCLAM
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

/* Top-level: build root and return it so main can print it */
S:
    STATEMENT_LIST
    { 
        root = new_node(NODE_START, "START", $1, NULL, lineCount);
        $$ = root;
    }
;

/* Statement list: left-recursive list (or empty) */
/* Produces a chain using right pointers for siblings */
STATEMENT_LIST:
      STATEMENT STATEMENT_LIST
      {
          ASTNode *head = $1;
          if ($2) head->right = $2;
          $$ = head;
      }
    | /* empty */ { $$ = NULL; }
;

/* Statement: declaration/assignment/expression/print */
STATEMENT:
      DECLARATION EXCLAM       { $$ = new_node(NODE_STATEMENT, "DECL_STMT", $1, NULL, lineCount); }
    | ASSIGNMENT EXCLAM        { $$ = new_node(NODE_STATEMENT, "ASSIGN_STMT", $1, NULL, lineCount); }
    | SIMPLE_EXPR EXCLAM       { $$ = new_node(NODE_STATEMENT, "EXPR_STMT", $1, NULL, lineCount); }
    | PRINTING EXCLAM          { $$ = new_node(NODE_STATEMENT, "PRINT_STMT", $1, NULL, lineCount); }
    | EXCLAM                   { $$ = new_node(NODE_STATEMENT, "EMPTY!", NULL, NULL, lineCount); }
    | error NEWLINE            { yyerror("Invalid statement"); yyerrok; ++lineCount; islexerror=0; $$ = NULL; }
    | NEWLINE                  { ++lineCount; $$ = NULL; }
;

/* Printing (PRINT expr,...) */
PRINTING:
    PRENT PRINT_LIST
    { $$ = $2; }
;

PRINT_LIST:
      PRINT_ITEM PRINT_LIST_PRIME
      {
          ASTNode *head = $1;
          if ($2) head->right = $2;
          $$ = head;
      }
    | /* empty */ { $$ = NULL; }
;

PRINT_LIST_PRIME:
      COMMA PRINT_ITEM PRINT_LIST_PRIME
      {
          ASTNode *head = $2;
          if ($3) head->right = $3;
          $$ = head;
      }
    | /* empty */ { $$ = NULL; }
;

PRINT_ITEM:
      SIMPLE_EXPR     { $$ = $1; }
    | STRING_LITERAL  { $$ = new_node(NODE_LITERAL, $1, NULL, NULL, lineCount); }
;

/* --------------------
   Declarations
   -------------------- */

/* Declaration: datatype + list of declarators.
   We traverse the INIT_DECLARATOR_LIST (which is a right-chained list)
   and add each symbol. For AST we create NODE_DECL entries with left=initializer.
*/
DECLARATION:
    DATATYPE INIT_DECLARATOR_LIST
    {
        ASTNode *decl_list_out = NULL; /* list of NODE_DECL nodes to attach under NODE_DECLARATION */
        ASTNode *prev_out = NULL;

        ASTNode *iter = $2; /* iterates the list returned by INIT_DECLARATOR_LIST.
                              Each node is a NODE_DECL whose left child is the initializer (or NULL),
                              and whose right child is the NEXT declarator in the list. */

        while (iter) {
            int initialized = (iter->left != NULL) ? 1 : 0;

            /* pick initializer string for symbol table if it's a literal; otherwise empty.
               We keep it simple and pass empty string for now (as before). */
            const char *init_val = "";

            /* Add to symbol table: name, datatype, initialized, value-string */
            add_symbol(iter->value, $1->value, initialized, init_val);

            /* For output AST we create a NODE_DECL node whose left child is the initializer expression */
            ASTNode *outnode = new_node(NODE_DECL, iter->value, iter->left, NULL, lineCount);

            if (!decl_list_out) decl_list_out = outnode;
            else prev_out->right = outnode;

            prev_out = outnode;

            iter = iter->right; /* advance along declarator list */
        }

        /* Final wrapper with datatype as value and decl_list_out as left child */
        $$ = new_node(NODE_DECLARATION, $1->value, decl_list_out, NULL, lineCount);

        /* free datatype temp */
        free($1->value);
        free($1);
    }
;

/* Datatype nodes (value stores token string like "ENTEGER" etc.) */
DATATYPE:
      CHAROT   { $$ = new_node(NODE_DATATYPE, "CHAROT", NULL, NULL, lineCount); }
    | ENTEGER  { $$ = new_node(NODE_DATATYPE, "ENTEGER", NULL, NULL, lineCount); }
    | KUAN     { $$ = new_node(NODE_DATATYPE, "KUAN", NULL, NULL, lineCount); }
;

/* Init declarator list: returns a chain of NODE_DECL nodes
   where each node's right points to the next declarator in list.
*/
INIT_DECLARATOR_LIST:
      INIT_DECLARATOR
      { $$ = $1; }
    | INIT_DECLARATOR COMMA INIT_DECLARATOR_LIST
      {
          ASTNode *first = $1;
          /* $1 is a NODE_DECL; its right should point to the subsequent list head */
          first->right = $3;
          $$ = first;
      }
;

/* Init declarator: either identifier or identifier = initializer.
   We use NODE_DECL for the syntactic declarator: its left child is the initializer (if any).
*/
INIT_DECLARATOR:
      DECLARATOR EQUAL SIMPLE_EXPR
      {
          /* DECLARATOR ($1) is a NODE_IDENTIFIER; take its value as the variable name */
          $$ = new_node(NODE_DECL, $1->value, $3, NULL, lineCount);
          free($1->value);
          free($1);
      }
    | DECLARATOR
      {
          $$ = new_node(NODE_DECL, $1->value, NULL, NULL, lineCount);
          free($1->value);
          free($1);
      }
;

/* Declarator returns an identifier node (kept temporary here) */
DECLARATOR:
      IDENTIFIER { $$ = new_node(NODE_IDENTIFIER, $1, NULL, NULL, lineCount); }
    | LPAREN DECLARATOR RPAREN { $$ = $2; }
;

/* --------------------
   Assignments & expressions (unchanged)
   -------------------- */

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
      TERM MUL FACTOR      { $$ = new_node(NODE_TERM, "*", $1, $3, lineCount); }
    | TERM DIV FACTOR      { $$ = new_node(NODE_TERM, "/", $1, $3, lineCount); }
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
      {
          if ($2)
              $$ = new_node(NODE_POSTFIX_OP, $2->value, $1, NULL, lineCount);
          else
              $$ = $1;
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

void yyerror(const char *s) {
    if (islexerror == 0) {
        FILE *out = fopen("output_print.txt", "w");
        if (out) {
            fprintf(out, "[PARSE] Syntax Invalid [line:%d]\n", lineCount);
            fclose(out);
        }
        islexerror = 1;
    }
    parse_failed = 1;
}
