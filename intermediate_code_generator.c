// intermediate_code_generator.c
// BaiScript TAC generator — clean, single-assignment, prefix/postfix correct

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

// === Utilities ===
TACInstruction *getOptimizedCode(int *count)
{
    if (count)
        *count = optimizedCount;
    return optimizedCode;
}

static int is_char_literal(const char *s, char *out)
{
    if (!s || !out) return 0;
    size_t len = strlen(s);
    if (len >= 3 && s[0] == '\'' && s[len - 1] == '\'')
    {
        if (len == 3) { *out = s[1]; return 1; }
        if (len == 4 && s[1] == '\\')
        {
            switch(s[2])
            {
                case 'n': *out='\n'; return 1;
                case 't': *out='\t'; return 1;
                case 'r': *out='\r'; return 1;
                case '0': *out='\0'; return 1;
                case '\\': *out='\\'; return 1;
                case '\'': *out='\''; return 1;
                case '"': *out='"'; return 1;
                default: return 0;
            }
        }
    }
    return 0;
}

static char *newTemp()
{
    char buf[32];
    snprintf(buf, sizeof(buf), "temp%d", tempCount++);
    return strdup(buf);
}

static void emit(const char *result, const char *arg1, const char *op, const char *arg2)
{
    TACInstruction *tmp = realloc(code, sizeof(TACInstruction)*(codeCount+1));
    if (!tmp) { fprintf(stderr,"Memory allocation failed in emit()\n"); exit(1); }
    code = tmp;
    snprintf(code[codeCount].result,sizeof(code[codeCount].result),"%s",result?result:"");
    snprintf(code[codeCount].arg1,sizeof(code[codeCount].arg1),"%s",arg1?arg1:"");
    snprintf(code[codeCount].op,sizeof(code[codeCount].op),"%s",op?op:"");
    snprintf(code[codeCount].arg2,sizeof(code[codeCount].arg2),"%s",arg2?arg2:"");
    codeCount++;
}

static char *generateExpression(ASTNode *node, int used_in_expr)
{
    if (!node) return NULL;

    // Leaf node
    if (!node->left && !node->right)
    {
        if (node->value)
        {
            char cval;
            if (is_char_literal(node->value,&cval))
            {
                char buf[32]; snprintf(buf,sizeof(buf),"%d",(int)cval);
                return strdup(buf);
            }
            return strdup(node->value);
        }
        return strdup("");
    }

    // Assignment / compound assignment
    if (node->type == NODE_ASSIGNMENT && node->left && node->right)
    {
        char *lhs = strdup(node->left->value);
        char *rhs = generateExpression(node->right, 1);
        if (!rhs) rhs = strdup("0");

        if (strcmp(node->value,"+=")==0) emit(lhs,lhs,"+",rhs);
        else if (strcmp(node->value,"-=")==0) emit(lhs,lhs,"-",rhs);
        else if (strcmp(node->value,"*=")==0) emit(lhs,lhs,"*",rhs);
        else if (strcmp(node->value,"/=")==0) emit(lhs,lhs,"/",rhs);
        else emit(lhs,rhs,"=",NULL);

        free(lhs); free(rhs);
        return strdup(node->left->value);
    }

    // Postfix ++ / --
    if (node->type == NODE_POSTFIX_OP && node->left)
    {
        char *var = generateExpression(node->left, 1);

        char *tmp = used_in_expr ? newTemp() : strdup(var);

        if (used_in_expr) emit(tmp,var,"=",NULL);
        if (strcmp(node->value,"++")==0) emit(var,var,"+","1");
        else if (strcmp(node->value,"--")==0) emit(var,var,"-","1");

        free(var);
        return tmp;
    }

    // Unary / prefix ++ / -- / + / -
    if (node->type == NODE_UNARY_OP && node->left)
    {
        char *opnd = generateExpression(node->left, 1);

        if (strcmp(node->value,"++")==0) { emit(opnd,opnd,"+","1"); return used_in_expr ? opnd : strdup(opnd); }
        if (strcmp(node->value,"--")==0) { emit(opnd,opnd,"-","1"); return used_in_expr ? opnd : strdup(opnd); }
        if (strcmp(node->value,"-")==0) { char *tmp=newTemp(); emit(tmp,"0","-",opnd); free(opnd); return tmp; }
        if (strcmp(node->value,"+")==0) { return opnd; }
        return opnd;
    }

    // Binary operation
    if (node->left && node->right)
    {
        char *left_val = generateExpression(node->left, 1);
        char *right_val = generateExpression(node->right, 1);
        char *tmp = newTemp();
        emit(tmp,left_val,node->value,right_val);
        free(left_val); free(right_val);
        return tmp;
    }

    return strdup(node->value?node->value:"");
}

// === Declaration List Generator ===
static void generateDeclarationList(ASTNode *node)
{
    if (!node) return;

    if (node->type == NODE_DECLARATION && node->value && strcmp(node->value,"INIT_DECL")==0)
    {
        char *ident = strdup(node->left->value);
        char *rhs = node->right ? generateExpression(node->right, 1) : strdup("0");
        emit(ident,rhs,"=",NULL);
        free(rhs); free(ident);
        return;
    }

    generateDeclarationList(node->left);
    generateDeclarationList(node->right);
}

// Updated generateCode to correctly handle declarations inside statements
static void generateCode(ASTNode *node)
{
    if (!node) return;

    switch(node->type)
    {
        case NODE_START:
            generateCode(node->left);
            break;

        case NODE_STATEMENT_LIST:
            generateCode(node->left);
            generateCode(node->right);
            break;

        case NODE_STATEMENT:
            if (!node->left) break;

            // Declaration
            if (node->left->type == NODE_DECLARATION)
                generateDeclarationList(node->left);

            // Print statement
            else if (node->value && strcmp(node->value, "PRINT_STMT") == 0)
            {
                ASTNode *print_node = node->left; // NODE_PRINTING
                if (print_node && print_node->type == NODE_PRINTING)
                {
                    ASTNode *plist = print_node->left; // first PRINT_ITEM
                    while (plist)
                    {
                        if (plist->type == NODE_PRINT_ITEM && plist->left)
                        {
                            generateExpression(plist->left, 1);
                        }
                        plist = plist->right;
                    }
                }
            }

            // Normal expression / assignment
            else
                generateExpression(node->left, 1);
            break;

        case NODE_DECLARATION:
            generateDeclarationList(node);
            break;

        case NODE_ASSIGNMENT:
        case NODE_EXPRESSION:
        case NODE_POSTFIX_OP:
        case NODE_UNARY_OP:
            { 
                char *res = generateExpression(node, 1);
                if (res) free(res);
            } 
            break;

        default:
            break;
    }
}


// === Optimization ===
static void removeRedundantTemporaries()
{
    if (codeCount == 0) { optimizedCode = NULL; optimizedCount = 0; return; }

    optimizedCode = malloc(sizeof(TACInstruction) * codeCount);
    if (!optimizedCode) { fprintf(stderr, "Out of memory\n"); exit(1); }

    int j = 0;

    for (int i = 0; i < codeCount; i++)
    {
        TACInstruction *cur = &code[i];

        // Only consider temp assignments
        if (strncmp(cur->result, "temp", 4) == 0)
        {
            int usageCount = 0;
            int lastUseIndex = -1;

            // Count how many times this temp is used
            for (int k = 0; k < codeCount; k++)
            {
                if (strcmp(code[k].arg1, cur->result) == 0 ||
                    strcmp(code[k].arg2, cur->result) == 0)
                {
                    usageCount++;
                    lastUseIndex = k;
                }
            }

            if (usageCount == 0)
            {
                // Hanging temp: never used -> skip
                continue;
            }
            else if (usageCount == 1 && lastUseIndex > i)
            {
                // Inline temp into its single usage
                TACInstruction *next = &code[lastUseIndex];

                if (strcmp(next->arg1, cur->result) == 0) 
                    snprintf(next->arg1, sizeof(next->arg1), "%s", cur->arg1);
                if (strcmp(next->arg2, cur->result) == 0) 
                    snprintf(next->arg2, sizeof(next->arg2), "%s", cur->arg2);

                // Skip this temp assignment
                continue;
            }
        }

        // Keep instruction
        optimizedCode[j++] = *cur;
    }

    optimizedCount = j;
}

// === Display ===
static void displayTAC()
{
    printf("===== INTERMEDIATE CODE (TAC) =====\n");
    for(int i=0;i<codeCount;i++)
    {
        TACInstruction *inst=&code[i];
        if(strcmp(inst->op,"=")==0 && strlen(inst->arg2)==0)
            printf("%s = %s\n",inst->result,inst->arg1);
        else if(strlen(inst->op)==0)
            printf("%s = %s\n",inst->result,inst->arg1);
        else
            printf("%s = %s %s %s\n",inst->result,inst->arg1,inst->op,inst->arg2);
    }
    printf("===== INTERMEDIATE CODE (TAC) END =====\n\n");
}

static void displayOptimizedTAC()
{
    printf("===== OPTIMIZED CODE =====\n");
    for(int i=0;i<optimizedCount;i++)
    {
        TACInstruction *inst=&optimizedCode[i];
        if(strcmp(inst->op,"=")==0 && strlen(inst->arg2)==0)
            printf("%s = %s\n",inst->result,inst->arg1);
        else if(strlen(inst->op)==0)
            printf("%s = %s\n",inst->result,inst->arg1);
        else
            printf("%s = %s %s %s\n",inst->result,inst->arg1,inst->op,inst->arg2);
    }
    printf("===== OPTIMIZED CODE END =====\n\n");
}

// === Public Interface ===
void generate_intermediate_code(ASTNode *root)
{
    if(code) free(code);
    if(optimizedCode) free(optimizedCode);
    code=NULL; optimizedCode=NULL;
    codeCount=0; optimizedCount=0; tempCount=0;

    if(root) generateCode(root);

    displayTAC();
    removeRedundantTemporaries();
    displayOptimizedTAC();
}
