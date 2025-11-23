#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

ASTNode *root = NULL;

ASTNode *new_node(NodeType type, const char *val, ASTNode *l, ASTNode *r, int line)
{
    ASTNode *n = (ASTNode *)malloc(sizeof(ASTNode));
    if (!n) { fprintf(stderr, "Memory allocation failed for AST node.\n"); exit(EXIT_FAILURE); }

    n->type = type;
    n->left = l;
    n->right = r;
    n->line = line;   // store line number

    if (val) {
        n->value = strdup(val);
        if (!n->value) { fprintf(stderr, "strdup failed.\n"); exit(EXIT_FAILURE); }
    } else {
        n->value = NULL;
    }

    return n;
}

static void indent(int level)
{
    for (int i = 0; i < level; i++) printf("  ");
}

void print_ast(ASTNode *node, int level)
{
    if (!node) return;

    indent(level);
    const char *val = node->value ? node->value : "NULL";
    printf("Node(type=%d, value=%s, line=%d)\n", node->type, val, node->line);

    print_ast(node->left, level + 1);
    print_ast(node->right, level + 1);
}
