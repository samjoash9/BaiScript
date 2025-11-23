/*
  semantic.c
  Semantic analyzer for BaiScript (works with your ast.h, semantic_analyzer.h, symbol_table.h)
  - Enforces: declared-before-use (error), initialized-before-use (error)
  - Warns: unused variables
  - Allows implicit char->int promotion
  - Does constant folding when possible
  - Saves generated semantic operations (operations list) for future TAC generation
*/

#include "semantic_analyzer.h"
#include "ast.h"
#include "symbol_table.h" /* expected to provide find_symbol(), symbol_table[], symbol_count */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

/* ----------------------------
   Internal structures
   ---------------------------- */

/* dynamic arrays for temps and ops */
static SEM_TEMP *sem_temps = NULL;
static size_t sem_temps_capacity = 0;
static size_t sem_temps_count = 0;
static int sem_next_temp_id = 1;

static SEM_OP *sem_ops = NULL;
static size_t sem_ops_capacity = 0;
static size_t sem_ops_count = 0;

/* KnownVar (keeps track of declared variables, their temp, init/used flags) */
static KnownVar *known_vars_head = NULL;

/* error/warning counters */
static int sem_errors = 0;
static int sem_warnings = 0;

/* ----------------------------
   Helpers: memory / arrays
   ---------------------------- */

static void sem_record_error(ASTNode *node, const char *fmt, ...)
{
    sem_errors++;
    fprintf(stderr, "[SEM ERROR] ");
    if (node)
        fprintf(stderr, "(node='%s') ", node->value ? node->value : "NULL");

    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    if (node)
        fprintf(stderr, " [line:%d]", node->line);

    fprintf(stderr, "\n");
}

static void sem_record_warning(ASTNode *node, const char *fmt, ...)
{
    sem_warnings++;
    fprintf(stderr, "[SEM WARNING] ");
    if (node)
        fprintf(stderr, "(node='%s') ", node->value ? node->value : "NULL");

    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    if (node)
        fprintf(stderr, " [line:%d]", node->line);

    fprintf(stderr, "\n");
}

static int ensure_temp_capacity(void)
{
    if (sem_temps_count + 1 > sem_temps_capacity)
    {
        size_t newcap = sem_temps_capacity == 0 ? 256 : sem_temps_capacity * 2;
        SEM_TEMP *nb = (SEM_TEMP *)realloc(sem_temps, newcap * sizeof(SEM_TEMP));
        if (!nb) return 0;
        sem_temps = nb;
        sem_temps_capacity = newcap;
    }
    return 1;
}

static int ensure_ops_capacity(void)
{
    if (sem_ops_count + 1 > sem_ops_capacity)
    {
        size_t newcap = sem_ops_capacity == 0 ? 256 : sem_ops_capacity * 2;
        SEM_OP *nb = (SEM_OP *)realloc(sem_ops, newcap * sizeof(SEM_OP));
        if (!nb) return 0;
        sem_ops = nb;
        sem_ops_capacity = newcap;
    }
    return 1;
}

/* ----------------------------
   Public helpers (declared in header)
   ---------------------------- */

SEM_TEMP sem_new_temp(SEM_TYPE type)
{
    if (!ensure_temp_capacity())
    {
        SEM_TEMP t = {0, SEM_TYPE_UNKNOWN, 0, 0, NULL};
        return t;
    }
    SEM_TEMP t;
    t.id = sem_next_temp_id++;
    t.type = type;
    t.is_constant = 0;
    t.int_value = 0;
    t.node = NULL;
    sem_temps[sem_temps_count++] = t;
    return t;
}

KnownVar* sem_find_var(const char *name)
{
    for (KnownVar *k = known_vars_head; k; k = k->next)
        if (strcmp(k->name, name) == 0)
            return k;
    return NULL;
}

KnownVar* sem_add_var(const char *name, SEM_TYPE type)
{
    /* if already exists in known_vars, return it */
    KnownVar *existing = sem_find_var(name);
    if (existing) return existing;

    /* Create new KnownVar */
    KnownVar *k = (KnownVar *)malloc(sizeof(KnownVar));
    if (!k) return NULL;
    k->name = strdup(name);
    k->temp = sem_new_temp(type);
    k->initialized = 0;
    k->used = 0;
    k->next = known_vars_head;
    known_vars_head = k;

    /* Also add to dynamic symbol_table if not already present */
    int idx = find_symbol(name);
    if (idx == -1)
    {
        const char *dtype_str = "KUAN";
        switch(type)
        {
            case SEM_TYPE_INT:  dtype_str = "ENTEGER"; break;
            case SEM_TYPE_CHAR: dtype_str = "CHAROT"; break;
            default:            dtype_str = "KUAN"; break;
        }
        add_symbol(name, dtype_str, 0, NULL);
    }

    return k;
}



/* sem_type_from_string: maps your DATATYPE token label to SEM_TYPE */
SEM_TYPE sem_type_from_string(const char *s)
{
    if (!s) return SEM_TYPE_UNKNOWN;
    if (strcmp(s, "ENTEGER") == 0 || strcmp(s, "int") == 0) return SEM_TYPE_INT;
    if (strcmp(s, "CHAROT") == 0 || strcmp(s, "char") == 0) return SEM_TYPE_CHAR;
    if (strcmp(s, "KUAN") == 0) return SEM_TYPE_UNKNOWN; /* generic type token — adapt if you want */
    return SEM_TYPE_UNKNOWN;
}

/* sem_emit: record a semantic operation in memory (no textual TAC emitted) */
void sem_emit(const char *fmt, ...)
{
    /* The semantic analyzer itself will use structured sem_ops instead of textual fmt.
       This utility allows you to append arbitrary textual notes if desired. We'll not use it heavily.
    */
    (void)fmt;
    /* no-op for now, or used to append textual annotation — left intentionally blank */
}

/* ----------------------------
   Constant parsing helpers
   ---------------------------- */

static int try_parse_int(const char *s, long *out)
{
    if (!s || !out) return 0;
    char *end;
    long v = strtol(s, &end, 10);
    if (end != s && *end == '\0') { *out = v; return 1; }
    return 0;
}

/* parse char literal like 'a' or '\n' */
static int try_parse_char_literal(const char *lex, long *out)
{
    if (!lex || !out) return 0;
    size_t len = strlen(lex);
    if (len < 3 || lex[0] != '\'' || lex[len-1] != '\'') return 0;
    size_t content_len = len - 2;
    if (content_len == 1) { *out = (unsigned char)lex[1]; return 1; }
    if (content_len == 2 && lex[1] == '\\')
    {
        char esc = lex[2];
        switch (esc) {
            case 'n': *out = '\n'; return 1;
            case 't': *out = '\t'; return 1;
            case 'r': *out = '\r'; return 1;
            case '0': *out = '\0'; return 1;
            case '\\': *out = '\\'; return 1;
            case '\'': *out = '\''; return 1;
            case '\"': *out = '\"'; return 1;
            default: return 0;
        }
    }
    return 0;
}

/* ----------------------------
   Try evaluate subtree to constant (folding)
   Returns 1 if constant and *out contains value.
   ---------------------------- */

static int try_eval_constant(ASTNode *node, long *out)
{
    if (!node || !out) return 0;

    if (node->type == NODE_LITERAL)
    {
        if (try_parse_int(node->value, out)) return 1;
        if (try_parse_char_literal(node->value, out)) return 1;
        return 0;
    }

    if (node->type == NODE_UNARY_OP)
    {
        long v;
        if (!try_eval_constant(node->left, &v)) return 0;
        if (node->value && strcmp(node->value, "-") == 0) { *out = -v; return 1; }
        if (node->value && strcmp(node->value, "+") == 0) { *out = v; return 1; }
        return 0;
    }

    if (node->type == NODE_TERM)
    {
        long L, R;
        if (!try_eval_constant(node->left, &L)) return 0;
        if (!try_eval_constant(node->right, &R)) return 0;
        if (node->value && strcmp(node->value, "*") == 0) { *out = L * R; return 1; }
        if (node->value && strcmp(node->value, "/") == 0) {
            if (R == 0) return 0; /* avoid division by zero at compile-time */
            *out = L / R;
            return 1;
        }
        return 0;
    }

    if (node->type == NODE_EXPRESSION)
    {
        long L, R;
        if (!try_eval_constant(node->left, &L)) return 0;
        if (!try_eval_constant(node->right, &R)) return 0;
        if (node->value && strcmp(node->value, "+") == 0) { *out = L + R; return 1; }
        if (node->value && strcmp(node->value, "-") == 0) { *out = L - R; return 1; }
        return 0;
    }

    return 0;
}

/* ----------------------------
   Core: evaluate expression and produce SEM_TEMP
   - enforces declared-before-use & initialized-before-use
   - implicit promotions: char -> int (no error)
   - creates temps and records semantic ops
   ---------------------------- */

static SEM_TEMP evaluate_expression(ASTNode *node);

static SEM_TEMP eval_factor(ASTNode *node)
{
    if (!node) return sem_new_temp(SEM_TYPE_UNKNOWN);

    if (node->type == NODE_LITERAL)
    {
        long v;
        if (try_parse_int(node->value, &v))
        {
            SEM_TEMP t = sem_new_temp(SEM_TYPE_INT);
            t.is_constant = 1;
            t.int_value = v;
            t.node = node;
            /* record op: load constant (for future TAC) */
            if (ensure_ops_capacity())
            {
                SEM_OP op = { .type = SEMOP_STORE_CONST, .dst_temp = t.id, .src1_temp = -1, .src2_temp = -1, .const_val = v, .node = node };
                sem_ops[sem_ops_count++] = op;
            }
            return t;
        }
        if (try_parse_char_literal(node->value, &v))
        {
            SEM_TEMP t = sem_new_temp(SEM_TYPE_CHAR);
            t.is_constant = 1;
            t.int_value = v;
            t.node = node;
            if (ensure_ops_capacity())
            {
                SEM_OP op = { .type = SEMOP_STORE_CONST, .dst_temp = t.id, .src1_temp = -1, .src2_temp = -1, .const_val = v, .node = node };
                sem_ops[sem_ops_count++] = op;
            }
            return t;
        }
        /* string literal or unsupported literal: treat as unknown */
        return sem_new_temp(SEM_TYPE_UNKNOWN);
    }

    if (node->type == NODE_IDENTIFIER)
    {
        const char *name = node->value;
        if (!name) return sem_new_temp(SEM_TYPE_UNKNOWN);

        /* check known-vars first (declarations we processed) */
        KnownVar *kv = sem_find_var(name);
        if (kv)
        {
            kv->used = 1;
            if (!kv->initialized)
            {
                /* You chose B = yes (must be initialized before use) => ERROR */
                sem_record_error(node, "Use of uninitialized variable '%s'", name);
            }
            SEM_TEMP t = kv->temp;
            t.node = node;
            return t;
        }

        /* fallback to symbol table provided by parser/compiler driver */
        int idx = find_symbol(name); /* expected available in symbol_table.h */
        if (idx == -1)
        {
            sem_record_error(node, "Undeclared identifier '%s'", name);
            return sem_new_temp(SEM_TYPE_UNKNOWN);
        }

        /* if symbol_table says not initialized => error (you required initialization) */
        if (!symbol_table[idx].initialized)
        {
            sem_record_error(node, "Use of uninitialized variable '%s' (declared in symbol table but not initialized)", name);
        }

        /* create known-var placeholder that mirrors symbol table entry so we can track used/initialized */
        SEM_TYPE stype = sem_type_from_string(symbol_table[idx].datatype);
        SEM_TEMP placeholder = sem_new_temp(stype);
        if (symbol_table[idx].initialized && symbol_table[idx].value_str[0] != '\0')
        {
            long vv;
            if (try_parse_int(symbol_table[idx].value_str, &vv))
            {
                placeholder.is_constant = 1;
                placeholder.int_value = vv;
            }
        }
        sem_add_var(name, stype)->initialized = symbol_table[idx].initialized;
        KnownVar *created = sem_find_var(name);
        if (created) created->used = 1;
        placeholder.node = node;
        return placeholder;
    }

    /* parentheses or other constructs: delegate */
    return evaluate_expression(node);
}

static SEM_TEMP eval_term(ASTNode *node)
{
    if (!node) return sem_new_temp(SEM_TYPE_UNKNOWN);
    if (node->type != NODE_TERM) return eval_factor(node);

    SEM_TEMP L = eval_term(node->left);
    SEM_TEMP R = eval_factor(node->right);
    const char *op = node->value ? node->value : "";

    /* division by zero detection when constant */
    if (op && strcmp(op, "/") == 0)
    {
        if (R.is_constant && R.int_value == 0)
        {
            sem_record_error(node, "Division by zero detected at compile time");
            return sem_new_temp(SEM_TYPE_UNKNOWN);
        }
        long denom;
        if (try_eval_constant(node->right, &denom) && denom == 0)
        {
            sem_record_error(node, "Division by zero detected at compile time");
            return sem_new_temp(SEM_TYPE_UNKNOWN);
        }
    }

    /* constant folding */
    if (L.is_constant && R.is_constant && op)
    {
        if (strcmp(op, "*") == 0)
        {
            long val = L.int_value * R.int_value;
            SEM_TEMP t = sem_new_temp(SEM_TYPE_INT);
            t.is_constant = 1; t.int_value = val; t.node = node;
            if (ensure_ops_capacity())
            {
                SEM_OP oprec = { .type = SEMOP_BINARY, .dst_temp = t.id, .src1_temp = L.id, .src2_temp = R.id, .const_val = val, .node = node };
                strncpy(oprec.op, "*", sizeof(oprec.op)-1);
                sem_ops[sem_ops_count++] = oprec;
            }
            return t;
        }
        if (strcmp(op, "/") == 0)
        {
            if (R.int_value == 0) return sem_new_temp(SEM_TYPE_UNKNOWN);
            long val = L.int_value / R.int_value;
            SEM_TEMP t = sem_new_temp(SEM_TYPE_INT);
            t.is_constant = 1; t.int_value = val; t.node = node;
            if (ensure_ops_capacity())
            {
                SEM_OP oprec = { .type = SEMOP_BINARY, .dst_temp = t.id, .src1_temp = L.id, .src2_temp = R.id, .const_val = val, .node = node };
                strncpy(oprec.op, "/", sizeof(oprec.op)-1);
                sem_ops[sem_ops_count++] = oprec;
            }
            return t;
        }
    }

    /* create result temp (int) and record op for future */
    SEM_TEMP res = sem_new_temp(SEM_TYPE_INT);
    res.node = node;
    if (ensure_ops_capacity())
    {
        SEM_OP oprec = { .type = SEMOP_BINARY, .dst_temp = res.id, .src1_temp = L.id, .src2_temp = R.id, .node = node };
        strncpy(oprec.op, op, sizeof(oprec.op)-1);
        sem_ops[sem_ops_count++] = oprec;
    }
    return res;
}

static SEM_TEMP eval_additive(ASTNode *node)
{
    if (!node) return sem_new_temp(SEM_TYPE_UNKNOWN);
    if (node->type != NODE_EXPRESSION) return eval_term(node);

    SEM_TEMP L = eval_additive(node->left);
    SEM_TEMP R = eval_term(node->right);
    const char *op = node->value ? node->value : "";

    /* constant folding */
    if (L.is_constant && R.is_constant && op)
    {
        if (strcmp(op, "+") == 0)
        {
            long val = L.int_value + R.int_value;
            SEM_TEMP t = sem_new_temp(SEM_TYPE_INT);
            t.is_constant = 1; t.int_value = val; t.node = node;
            if (ensure_ops_capacity())
            {
                SEM_OP oprec = { .type = SEMOP_BINARY, .dst_temp = t.id, .src1_temp = L.id, .src2_temp = R.id, .const_val = val, .node = node };
                strncpy(oprec.op, "+", sizeof(oprec.op)-1);
                sem_ops[sem_ops_count++] = oprec;
            }
            return t;
        }
        if (strcmp(op, "-") == 0)
        {
            long val = L.int_value - R.int_value;
            SEM_TEMP t = sem_new_temp(SEM_TYPE_INT);
            t.is_constant = 1; t.int_value = val; t.node = node;
            if (ensure_ops_capacity())
            {
                SEM_OP oprec = { .type = SEMOP_BINARY, .dst_temp = t.id, .src1_temp = L.id, .src2_temp = R.id, .const_val = val, .node = node };
                strncpy(oprec.op, "-", sizeof(oprec.op)-1);
                sem_ops[sem_ops_count++] = oprec;
            }
            return t;
        }
    }

    SEM_TEMP res = sem_new_temp(SEM_TYPE_INT);
    res.node = node;
    if (ensure_ops_capacity())
    {
        SEM_OP oprec = { .type = SEMOP_BINARY, .dst_temp = res.id, .src1_temp = L.id, .src2_temp = R.id, .node = node };
        strncpy(oprec.op, op, sizeof(oprec.op)-1);
        sem_ops[sem_ops_count++] = oprec;
    }
    return res;
}

static SEM_TEMP evaluate_expression(ASTNode *node)
{
    if (!node) return sem_new_temp(SEM_TYPE_UNKNOWN);

    switch (node->type)
    {
        case NODE_FACTOR:
            return evaluate_expression(node->left);
        case NODE_TERM:
            return eval_term(node);
        case NODE_EXPRESSION:
            return eval_additive(node);
        case NODE_UNARY_OP:
        {
            SEM_TEMP t = evaluate_expression(node->left);
            /* apply unary if constant */
            if (t.is_constant && node->value)
            {
                if (strcmp(node->value, "+") == 0) return t;
                if (strcmp(node->value, "-") == 0)
                {
                    SEM_TEMP r = sem_new_temp(t.type);
                    r.is_constant = 1;
                    r.int_value = -t.int_value;
                    r.node = node;
                    if (ensure_ops_capacity())
                    {
                        SEM_OP oprec = { .type = SEMOP_UNARY, .dst_temp = r.id, .src1_temp = t.id, .src2_temp = -1, .node = node };
                        strncpy(oprec.op, node->value, sizeof(oprec.op)-1);
                        sem_ops[sem_ops_count++] = oprec;
                    }
                    return r;
                }
            }
            SEM_TEMP r = sem_new_temp(t.type);
            r.node = node;
            if (ensure_ops_capacity())
            {
                SEM_OP oprec = { .type = SEMOP_UNARY, .dst_temp = r.id, .src1_temp = t.id, .src2_temp = -1, .node = node };
                strncpy(oprec.op, node->value ? node->value : "", sizeof(oprec.op)-1);
                sem_ops[sem_ops_count++] = oprec;
            }
            return r;
        }
        case NODE_POSTFIX_OP:
        {
            /* ++/-- on an expression (likely identifier) */
            SEM_TEMP base = evaluate_expression(node->left);
            SEM_TEMP r = sem_new_temp(base.type);
            r.node = node;
            if (ensure_ops_capacity())
            {
                SEM_OP oprec = { .type = SEMOP_UNARY, .dst_temp = r.id, .src1_temp = base.id, .src2_temp = -1, .node = node };
                strncpy(oprec.op, node->value ? node->value : "", sizeof(oprec.op)-1);
                sem_ops[sem_ops_count++] = oprec;
            }
            return r;
        }

        case NODE_IDENTIFIER:
        case NODE_LITERAL:
            return eval_factor(node);

        default:
            /* recurse children if unknown */
            if (node->left) evaluate_expression(node->left);
            if (node->right) evaluate_expression(node->right);
            return sem_new_temp(SEM_TYPE_UNKNOWN);
    }
}

/* ----------------------------
   Declarations & assignments analysis
   ---------------------------- */


/* --- Updated handle_declaration() --- */
static void handle_declaration(ASTNode *decl_node)
{
    SEM_TYPE dtype = sem_type_from_string(decl_node->value);
    ASTNode *decls = decl_node->left;

    while (decls)
    {
        if (decls->type == NODE_IDENTIFIER)
        {
            const char *name = decls->value;
            if (!name) { decls = decls->right; continue; }

            if (sem_find_var(name))
            {
                sem_record_error(decls, "Redeclaration of variable '%s'", name);
            }
            else
            {
                KnownVar *kv = sem_add_var(name, dtype);

                /* record declaration op */
                if (ensure_ops_capacity())
                {
                    SEM_OP op = { .type = SEMOP_DECL, .dst_temp = kv->temp.id, .node = decls };
                    sem_ops[sem_ops_count++] = op;
                }
            }
        }
        else if (decls->type == NODE_DECLARATION)
        {
            ASTNode *idnode = decls->left;
            ASTNode *init_expr = decls->right;
            if (!idnode || idnode->type != NODE_IDENTIFIER) { decls = decls->right; continue; }

            const char *name = idnode->value;
            if (sem_find_var(name))
            {
                sem_record_error(idnode, "Redeclaration of variable '%s'", name);
            }
            else
            {
                KnownVar *kv = sem_add_var(name, dtype);
                SEM_TEMP val = evaluate_expression(init_expr);

                if (val.is_constant)
                {
                    kv->initialized = 1;
                    kv->temp.is_constant = 1;
                    kv->temp.int_value = val.int_value;
                    /* Update symbol table */
                    int idx = find_symbol(name);
                    if (idx != -1)
                    {
                        symbol_table[idx].initialized = 1;
                        snprintf(symbol_table[idx].value_str, SYMBOL_VALUE_MAX, "%ld", val.int_value);
                    }

                    /* record op */
                    if (ensure_ops_capacity())
                    {
                        SEM_OP op = { .type = SEMOP_STORE_CONST, .dst_temp = kv->temp.id, .const_val = val.int_value, .node = idnode };
                        sem_ops[sem_ops_count++] = op;
                    }
                }
                else
                {
                    kv->initialized = 1;
                    int idx = find_symbol(name);
                    if (idx != -1) symbol_table[idx].initialized = 1;

                    if (ensure_ops_capacity())
                    {
                        SEM_OP op = { .type = SEMOP_ASSIGN, .dst_temp = kv->temp.id, .src1_temp = val.id, .node = idnode };
                        sem_ops[sem_ops_count++] = op;
                    }
                }
            }
        }

        decls = decls->right;
    }
}


static void handle_assignment(ASTNode *assign_node)
{
    ASTNode *lhs = assign_node->left;
    ASTNode *rhs = assign_node->right;

    if (!lhs || lhs->type != NODE_IDENTIFIER)
    {
        sem_record_error(assign_node, "Invalid LHS in assignment");
        return;
    }

    const char *name = lhs->value;
    KnownVar *kv = sem_find_var(name);

    if (!kv)
    {
        int idx = find_symbol(name);
        if (idx == -1)
        {
            sem_record_error(lhs, "Assignment to undeclared variable '%s'", name);
            return;
        }
        else
        {
            SEM_TYPE stype = sem_type_from_string(symbol_table[idx].datatype);
            kv = sem_add_var(name, stype);
            kv->initialized = symbol_table[idx].initialized;
        }
    }

    SEM_TEMP rhs_temp = evaluate_expression(rhs);

    /* simple = with constant */
    if (assign_node->value && strcmp(assign_node->value, "=") == 0 && rhs_temp.is_constant)
    {
        kv->initialized = 1;
        kv->temp.is_constant = 1;
        kv->temp.int_value = rhs_temp.int_value;

        int idx = find_symbol(name);
        if (idx != -1)
        {
            symbol_table[idx].initialized = 1;
            snprintf(symbol_table[idx].value_str, SYMBOL_VALUE_MAX, "%ld", rhs_temp.int_value);
        }

        if (ensure_ops_capacity())
        {
            SEM_OP op = { .type = SEMOP_STORE_CONST, .dst_temp = kv->temp.id, .const_val = rhs_temp.int_value, .node = assign_node };
            sem_ops[sem_ops_count++] = op;
        }
    }
    else
    {
        /* general assignment or +=, -=, etc */
        if (ensure_ops_capacity())
        {
            SEM_OP op = { .type = SEMOP_ASSIGN, .dst_temp = kv->temp.id, .src1_temp = rhs_temp.id, .node = assign_node };
            if (assign_node->value) strncpy(op.op, assign_node->value, sizeof(op.op)-1);
            sem_ops[sem_ops_count++] = op;
        }

        kv->initialized = 1;
        int idx = find_symbol(name);
        if (idx != -1) symbol_table[idx].initialized = 1;
    }
}

/* ----------------------------
   AST Traversal / Analysis
   ---------------------------- */

static void analyze_node(ASTNode *node)
{
    if (!node) return;

    switch (node->type)
    {
        case NODE_START:
            analyze_node(node->left); /* statement list */
            break;

        case NODE_STATEMENT_LIST:
            analyze_node(node->left);
            analyze_node(node->right);
            break;

        case NODE_STATEMENT:
            /* statement wrapper: left holds actual statement node */
            analyze_node(node->left);
            break;

        case NODE_DECLARATION:
            handle_declaration(node);
            break;

        case NODE_ASSIGNMENT:
            handle_assignment(node);
            break;

        case NODE_PRINTING:
            /* printing: analyze its expressions so we catch undeclared/uninitialized uses */
            analyze_node(node->left);
            break;

        /* expressions: evaluate to catch semantic problems like undeclared/uninitialized uses
           but don't produce new semantic ops beyond evaluate_expression. */
        case NODE_EXPRESSION:
        case NODE_TERM:
        case NODE_FACTOR:
        case NODE_UNARY_OP:
        case NODE_POSTFIX_OP:
        case NODE_LITERAL:
        case NODE_IDENTIFIER:
            (void)evaluate_expression(node);
            break;

        default:
            /* generic: recurse */
            analyze_node(node->left);
            analyze_node(node->right);
            break;
    }
}

/* After analysis, warn about declared-but-never-used variables (4C: yes) */
static void check_unused_variables(void)
{
    /* check symbol_table entries as well as known_vars */
    for (KnownVar *k = known_vars_head; k; k = k->next)
    {
        if (!k->used)
        {
            sem_record_warning(k->temp.node ? k->temp.node : NULL, "Variable '%s' declared but never used", k->name);
        }
    }

    /* If your symbol_table holds variables declared by parser that we didn't touch,
       we can also warn by iterating symbol_table[]. We do a best-effort check:
    */
    for (int i = 0; i < symbol_count; ++i)
    {
        const char *name = symbol_table[i].name;
        /* skip if we already tracked it in known_vars */
        if (sem_find_var(name)) continue;
        if (!symbol_table[i].initialized && !symbol_table[i].value_str[0])
        {
            sem_record_warning(NULL, "Variable '%s' declared (in symbol table) but never initialized or used", name);
        }
    }
}

/* ----------------------------
   Public API
   ---------------------------- */

int semantic_analyzer(void)
{
    /* reset state */
    sem_errors = 0;
    sem_warnings = 0;
    sem_next_temp_id = 1;
    sem_temps_count = 0;
    sem_ops_count = 0;

    /* free known_vars if any (fresh run) */
    KnownVar *k = known_vars_head;
    while (k)
    {
        KnownVar *n = k->next;
        free(k->name);
        free(k);
        k = n;
    }
    known_vars_head = NULL;

    if (!root)
    {
        fprintf(stderr, "[SEM] No syntax tree to analyze\n");
        return 0;
    }

    analyze_node(root);

    check_unused_variables();

    if (sem_errors == 0)
    {
        printf("[SEM] Analysis completed: no semantic errors (warnings: %d)\n", sem_warnings);
    }
    else
    {
        printf("[SEM] Analysis completed: %d semantic error(s), %d warning(s)\n", sem_errors, sem_warnings);
    }

    /* Return number of semantic errors (0 == success) */
    return sem_errors;
}

int semantic_error_count(void)
{
    return sem_errors;
}

/* OPTIONAL helpers to let other phases inspect what we recorded */

/* Returns pointer to array of temps and count (read-only) */
const SEM_TEMP *sem_get_temps(size_t *out_count)
{
    if (out_count) *out_count = sem_temps_count;
    return sem_temps;
}

/* Returns pointer to ops array and count */
const SEM_OP *sem_get_ops(size_t *out_count)
{
    if (out_count) *out_count = sem_ops_count;
    return sem_ops;
}

/* Cleanup function in case caller wants to free analyzer structures */
void sem_cleanup(void)
{
    if (sem_temps) { free(sem_temps); sem_temps = NULL; sem_temps_capacity = 0; sem_temps_count = 0; }
    if (sem_ops) { free(sem_ops); sem_ops = NULL; sem_ops_capacity = 0; sem_ops_count = 0; }
    KnownVar *k = known_vars_head;
    while (k) {
        KnownVar *n = k->next;
        free(k->name);
        free(k);
        k = n;
    }
    known_vars_head = NULL;
    sem_errors = 0;
    sem_warnings = 0;
}
