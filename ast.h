#ifndef AST_H
#define AST_H

typedef enum
{
    NODE_START,
    NODE_STATEMENT_LIST,
    NODE_STATEMENT,
    NODE_PRINTING,
    NODE_PRINT_ITEM,
    NODE_DECLARATION,
    NODE_ASSIGNMENT,
    NODE_EXPRESSION,
    NODE_TERM,
    NODE_FACTOR,
    NODE_UNARY_OP,
    NODE_POSTFIX_OP,
    NODE_LITERAL,
    NODE_IDENTIFIER,
    NODE_DATATYPE,
    NODE_UNKNOWN
} NodeType;

typedef struct ASTNode
{
    NodeType type;
    char *value;
    struct ASTNode *left;
    struct ASTNode *right;
} ASTNode;

ASTNode *new_node(NodeType type, const char *val, ASTNode *l, ASTNode *r);
void print_ast(ASTNode *node, int level);

extern ASTNode *root;

#endif
