#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

int islexerror = 0;

ASTNode *root = NULL;

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
    n->line = line; // store line number

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

static void indent(int level)
{
    for (int i = 0; i < level; i++)
        printf("  ");
}

// Pretty print AST with type names
void print_ast(ASTNode *node, int indent)
{
    if (!node)
        return;

    // Print indentation
    for (int i = 0; i < indent; i++)
        printf("  ");

    // Print node value and type
    printf("(%s: ", node->value ? node->value : "NULL");

    switch (node->type)
    {
    case NODE_START:
        printf("START");
        break;
    case NODE_STATEMENT_LIST:
        printf("STATEMENT_LIST");
        break;
    case NODE_STATEMENT:
        printf("STATEMENT");
        break;
    case NODE_PRINTING:
        printf("PRINTING");
        break;
    case NODE_PRINT_ITEM:
        printf("PRINT_ITEM");
        break;
    case NODE_DECLARATION:
        printf("DECL");
        break;
    case NODE_DATATYPE:
        printf("DATATYPE");
        break;
    case NODE_IDENTIFIER:
        printf("IDENTIFIER");
        break;
    case NODE_LITERAL:
        printf("LITERAL");
        break;
    case NODE_ASSIGNMENT:
        printf("ASSIGNMENT");
        break;
    case NODE_UNKNOWN:
        printf("UNKNOWN");
        break;
    case NODE_EXPRESSION:
        printf("EXPRESSION");
        break;
    case NODE_TERM:
        printf("TERM");
        break;
    case NODE_UNARY_OP:
        printf("UNARY_OP");
        break;
    case NODE_POSTFIX_OP:
        printf("POSTFIX_OP");
        break;
    case NODE_FACTOR:
        printf("FACTOR");
        break;
    default:
        printf("OTHER");
        break;
    }

    printf(")\n");

    // Recursively print children
    print_ast(node->left, indent + 1);
    print_ast(node->right, indent + 1);
}
