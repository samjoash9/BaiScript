#ifndef AST_H
#define AST_H

typedef enum NodeType {
    NODE_START,
    NODE_STATEMENT_LIST,
    NODE_STATEMENT,
    NODE_PRINTING,
    NODE_PRINT_ITEM,
    NODE_DECLARATION,
    NODE_DATATYPE,
    NODE_IDENTIFIER,
    NODE_LITERAL,
    NODE_ASSIGNMENT,
    NODE_UNKNOWN,
    NODE_EXPRESSION,
    NODE_TERM,
    NODE_UNARY_OP,
    NODE_POSTFIX_OP,
    NODE_FACTOR 
} NodeType;


typedef struct ASTNode {
    NodeType type;
    char *value;
    struct ASTNode *left;
    struct ASTNode *right;
    int line;  // <-- added line number
} ASTNode;

extern ASTNode *root;

extern int islexerror;

ASTNode *new_node(NodeType type, const char *val, ASTNode *l, ASTNode *r, int line);
void print_ast(ASTNode *node, int level);

#endif
