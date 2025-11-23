#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

ASTNode *root = NULL;

/*
 * Creates a new AST node.
 * type  - enum NodeType
 * val   - string value (copied internally)
 * l, r  - left/right child pointers
 */
ASTNode *new_node(NodeType type, const char *val, ASTNode *l, ASTNode *r)
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

    // Duplicate string value if provided
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

/*
 * Helper function: prints indentation.
 */
static void indent(int level)
{
    for (int i = 0; i < level; i++)
        printf("  ");
}

/*
 * Recursively prints the AST structure.
 */
void print_ast(ASTNode *node, int level)
{
    if (!node)
        return;

    indent(level);

    const char *val = node->value ? node->value : "NULL";
    printf("Node(type=%d, value=%s)\n", node->type, val);

    print_ast(node->left, level + 1);
    print_ast(node->right, level + 1);
}
