#ifndef AST_H
#define AST_H

typedef enum NodeType
{
    NODE_START,
    NODE_STATEMENT_LIST,
    NODE_STATEMENT,
    NODE_PRINTING,
    NODE_PRINT_ITEM,
    NODE_DECLARATION, // semantic declaration node
    NODE_DECL,        // **this must exist**
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

typedef struct ASTNode
{
    NodeType type;         // node type
    char *value;           // textual name or literal value
    struct ASTNode *left;  // left child
    struct ASTNode *right; // right child
    int line;              // source line
} ASTNode;

extern ASTNode *root;
extern int islexerror;

// Constructor — ALWAYS use this for AST creation
ASTNode *new_node(NodeType type, const char *val, ASTNode *left, ASTNode *right, int line);

// Pretty-printer for AST
void print_ast(ASTNode *node, int level);

#endif
