#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

int islexerror = 0;
ASTNode *root = NULL;

// Create a new AST node
ASTNode *new_node(NodeType type, const char *val, ASTNode *l, ASTNode *r, int line)
{
    ASTNode *n = (ASTNode *)malloc(sizeof(ASTNode));
    if (!n)
    {
        fprintf(stderr, "Memory allocation failed for AST node.\n");
        exit(EXIT_FAILURE);
    }

    n->type = type;
    n->left = l;
    n->right = r;
    n->line = line;

    if (val)
    {
        n->value = strdup(val);
        if (!n->value)
        {
            fprintf(stderr, "strdup failed.\n");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        n->value = NULL;
    }

    return n;
}

const char *node_type_name(NodeType type)
{
    switch (type)
    {
    case NODE_START:
        return "START";
    case NODE_STATEMENT_LIST:
        return "STATEMENT_LIST";
    case NODE_STATEMENT:
        return "STATEMENT";
    case NODE_PRINTING:
        return "PRINTING";
    case NODE_PRINT_ITEM:
        return "PRINT_ITEM";
    case NODE_DECLARATION:
        return "DECLARATION";
    case NODE_DECL:
        return "DECL"; // <--- this is the key
    case NODE_DATATYPE:
        return "DATATYPE";
    case NODE_IDENTIFIER:
        return "IDENTIFIER";
    case NODE_LITERAL:
        return "LITERAL";
    case NODE_ASSIGNMENT:
        return "ASSIGNMENT";
    case NODE_UNKNOWN:
        return "UNKNOWN";
    case NODE_EXPRESSION:
        return "EXPRESSION";
    case NODE_TERM:
        return "TERM";
    case NODE_UNARY_OP:
        return "UNARY_OP";
    case NODE_POSTFIX_OP:
        return "POSTFIX_OP";
    case NODE_FACTOR:
        return "FACTOR";
    default:
        return "OTHER";
    }
}

// Pretty print AST
void print_ast(ASTNode *node, int indent)
{
    if (!node)
        return;

    // Indentation
    for (int i = 0; i < indent; i++)
        printf("  ");

    // Print node
    printf("(%s: %d) %s\n",
           node->value ? node->value : "NULL",
           node->line,
           node_type_name(node->type));

    // Print left child (deeper level)
    print_ast(node->left, indent + 1);

    // Print right sibling (same level)
    print_ast(node->right, indent);
}
