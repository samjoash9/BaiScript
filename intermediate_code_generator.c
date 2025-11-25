// intermediate_code_generator.c
// Corrected intermediate code generator (TAC) for BaiScript
// - Uses temp0, temp1, ...
// - Fully correct prefix/postfix semantics
// - Single assignment emission

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "intermediate_code_generator.h"

static TACInstruction *code = NULL;
TACInstruction *optimizedCode = NULL;
static int codeCount = 0;
int optimizedCount = 0;
static int tempCount = 0;

// === Utility ===
TACInstruction *getOptimizedCode(int *count)
{
    if (count)
        *count = optimizedCount;
    return optimizedCode;
}

static char *newTemp()
{
    char buf[32];
    snprintf(buf, sizeof(buf), "temp%d", tempCount++);
    return strdup(buf);
}

static void emit(const char *result, const char *arg1, const char *op, const char *arg2)
{
    TACInstruction *tmp = realloc(code, sizeof(TACInstruction) * (codeCount + 1));
    if (!tmp)
    {
        fprintf(stderr, "Memory allocation failed in emit()\n");
        exit(1);
    }
    code = tmp;

    snprintf(code[codeCount].result, sizeof(code[codeCount].result), "%s", result ? result : "");
    snprintf(code[codeCount].arg1, sizeof(code[codeCount].arg1), "%s", arg1 ? arg1 : "");
    snprintf(code[codeCount].op, sizeof(code[codeCount].op), "%s", op ? op : "");
    snprintf(code[codeCount].arg2, sizeof(code[codeCount].arg2), "%s", arg2 ? arg2 : "");
    codeCount++;
}

// === Expression Generator with Correct Prefix/Postfix ===
static char *generateExpression(ASTNode *node)
{
    if (!node)
        return NULL;

    // Leaf node (variable or literal)
    if (!node->left && !node->right)
        return strdup(node->value ? node->value : "");

    // Assignment
    if (node->type == NODE_ASSIGNMENT && node->left && node->right)
    {
        char *lhs_name = strdup(node->left->value);
        char *rhs_val = generateExpression(node->right);
        emit(lhs_name, rhs_val, "=", NULL);
        free(rhs_val);
        return lhs_name;
    }

    // Postfix (++ / --)
    if (node->type == NODE_POSTFIX_OP && node->left)
    {
        char *var = generateExpression(node->left);
        char *tmp = newTemp();

        // Postfix returns original value, then increments/decrements
        emit(tmp, var, "=", NULL); // temp = var
        if (strcmp(node->value, "++") == 0)
            emit(var, var, "+", "1"); // var = var + 1
        else if (strcmp(node->value, "--") == 0)
            emit(var, var, "-", "1"); // var = var - 1

        free(var);
        return tmp;
    }

    // Prefix / Unary (++ / -- / + / -)
    if (node->type == NODE_UNARY_OP && node->left)
    {
        char *opnd = generateExpression(node->left);

        if (strcmp(node->value, "++") == 0)
        {
            emit(opnd, opnd, "+", "1");
            return opnd; // prefix returns incremented value
        }
        else if (strcmp(node->value, "--") == 0)
        {
            emit(opnd, opnd, "-", "1");
            return opnd; // prefix returns decremented value
        }
        else if (strcmp(node->value, "-") == 0)
        {
            char *tmp = newTemp();
            emit(tmp, "0", "-", opnd);
            free(opnd);
            return tmp;
        }
        else if (strcmp(node->value, "+") == 0)
        {
            return opnd;
        }
        return opnd;
    }

    // Binary operations
    if (node->left && node->right)
    {
        char *left_val = generateExpression(node->left);
        char *right_val = generateExpression(node->right);
        char *tmp = newTemp();
        emit(tmp, left_val, node->value, right_val);
        free(left_val);
        free(right_val);
        return tmp;
    }

    return strdup(node->value ? node->value : "");
}

static void generateDeclarationList(ASTNode *node)
{
    if (!node)
        return;

    // If node is a wrapper (multiple declarations), recurse
    if (node->type == NODE_DECLARATION && node->value && strcmp(node->value, "DECL") == 0)
    {
        generateDeclarationList(node->left);
        generateDeclarationList(node->right);
        return;
    }

    // If node is a single INIT_DECL
    if (node->type == NODE_DECLARATION && node->value && strcmp(node->value, "INIT_DECL") == 0)
    {
        if (node->left && node->left->type == NODE_IDENTIFIER)
        {
            char *ident = strdup(node->left->value);
            char *rhs = NULL;
            if (node->right)
                rhs = generateExpression(node->right);
            else
                rhs = strdup("0");

            emit(ident, rhs, "=", NULL);

            free(rhs);
            free(ident);
        }
        return;
    }

    // Safety: recurse to left/right if any
    generateDeclarationList(node->left);
    generateDeclarationList(node->right);
}

// === AST Walker / Code Generator ===
static void generateCode(ASTNode *node)
{
    if (!node)
        return;

    switch (node->type)
    {
    case NODE_START:
        generateCode(node->left);
        break;
    case NODE_STATEMENT_LIST:
        generateCode(node->left);
        generateCode(node->right);
        break;
    case NODE_STATEMENT:
        generateCode(node->left);
        break;
    case NODE_DECLARATION:
    {
        generateDeclarationList(node->left);
        break;
    }
    case NODE_ASSIGNMENT:
    case NODE_EXPRESSION:
    case NODE_POSTFIX_OP:
    case NODE_UNARY_OP:
    {
        char *res = generateExpression(node);
        if (res)
            free(res);
        break;
    }
    default:
        break;
    }
}

// === Optimization: inline temporaries ===
static void removeRedundantTemporaries()
{
    if (codeCount == 0)
    {
        optimizedCode = NULL;
        optimizedCount = 0;
        return;
    }

    optimizedCode = malloc(sizeof(TACInstruction) * codeCount);
    if (!optimizedCode)
    {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }

    int j = 0;
    for (int i = 0; i < codeCount; i++)
    {
        TACInstruction *cur = &code[i];

        // Check if this line is a temp assigned and immediately copied
        int inlined = 0;
        if (strncmp(cur->result, "temp", 4) == 0 && strlen(cur->op) > 0)
        {
            // Look ahead to find single copy
            for (int k = i + 1; k < codeCount; k++)
            {
                TACInstruction *next = &code[k];
                if (strcmp(next->arg1, cur->result) == 0 && strcmp(next->op, "=") == 0)
                {
                    // Inline temp: next.result = cur.arg1 op cur.arg2
                    snprintf(optimizedCode[j].result, sizeof(optimizedCode[j].result), "%s", next->result);
                    snprintf(optimizedCode[j].arg1, sizeof(optimizedCode[j].arg1), "%s", cur->arg1);
                    snprintf(optimizedCode[j].op, sizeof(optimizedCode[j].op), "%s", cur->op);
                    snprintf(optimizedCode[j].arg2, sizeof(optimizedCode[j].arg2), "%s", cur->arg2);
                    j++;
                    inlined = 1;

                    // Skip next line because it’s merged
                    i = k;
                    break;
                }
                // If temp is used in another place, do not inline
                if (strcmp(next->arg1, cur->result) != 0 && strcmp(next->arg2, cur->result) != 0)
                    continue;
            }
        }

        if (!inlined)
        {
            optimizedCode[j++] = *cur;
        }
    }

    optimizedCount = j;
}

// === Display ===
static void displayTAC()
{
    printf("===== INTERMEDIATE CODE (TAC) =====\n");
    for (int i = 0; i < codeCount; i++)
    {
        TACInstruction *inst = &code[i];
        if (strcmp(inst->op, "=") == 0 && strlen(inst->arg2) == 0)
            printf("%s = %s\n", inst->result, inst->arg1);
        else if (strlen(inst->op) == 0)
            printf("%s = %s\n", inst->result, inst->arg1);
        else
            printf("%s = %s %s %s\n", inst->result, inst->arg1, inst->op, inst->arg2);
    }
    printf("===== INTERMEDIATE CODE (TAC) END =====\n\n");
}

static void displayOptimizedTAC()
{
    printf("===== OPTIMIZED CODE =====\n");
    for (int i = 0; i < optimizedCount; i++)
    {
        TACInstruction *inst = &optimizedCode[i];
        if (strcmp(inst->op, "=") == 0 && strlen(inst->arg2) == 0)
            printf("%s = %s\n", inst->result, inst->arg1);
        else if (strlen(inst->op) == 0)
            printf("%s = %s\n", inst->result, inst->arg1);
        else
            printf("%s = %s %s %s\n", inst->result, inst->arg1, inst->op, inst->arg2);
    }
    printf("===== OPTIMIZED CODE END =====\n\n");
}

// === Public Interface ===
void generate_intermediate_code(ASTNode *root)
{
    if (code)
        free(code);
    if (optimizedCode)
        free(optimizedCode);
    code = NULL;
    optimizedCode = NULL;
    codeCount = 0;
    optimizedCount = 0;
    tempCount = 0;

    if (root)
        generateCode(root);

    displayTAC();
    removeRedundantTemporaries();
    displayOptimizedTAC();
}
