
#include "semantic_analyzer.h"
#include "ast.h"
#include "symbol_table.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

/* ----------------------------
   Internal structures
   ---------------------------- */

static SEM_TEMP *sem_temps = NULL;
static size_t sem_temps_capacity = 0;
static size_t sem_temps_count = 0;
static int sem_next_temp_id = 1;

static SEM_OP *sem_ops = NULL;
static size_t sem_ops_capacity = 0;
static size_t sem_ops_count = 0;

static KnownVar *known_vars_head = NULL;

static int sem_errors = 0;
static int sem_warnings = 0;

/* ----------------------------
   Helpers: error/warning
   ---------------------------- */

static void sem_record_error(ASTNode *node, const char *fmt, ...)
{
    sem_errors++;
    fprintf(stderr, "[SEM ERROR] ");
    if (node) fprintf(stderr, "(node='%s') ", node->value ? node->value : "NULL");
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    if (node) fprintf(stderr, " [line:%d]", node->line);
    fprintf(stderr, "\n");
}

static void sem_record_warning(ASTNode *node, const char *fmt, ...)
{
    sem_warnings++;
    fprintf(stderr, "[SEM WARNING] ");
    if (node) fprintf(stderr, "(node='%s') ", node->value ? node->value : "NULL");
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    if (node) fprintf(stderr, " [line:%d]", node->line);
    fprintf(stderr, "\n");
}

/* ----------------------------
   Dynamic arrays helpers
   ---------------------------- */

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
   Public helpers
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
    KnownVar *existing = sem_find_var(name);
    if (existing) return existing;

    KnownVar *k = (KnownVar *)malloc(sizeof(KnownVar));
    if (!k) return NULL;
    k->name = strdup(name);
    k->temp = sem_new_temp(type);
    k->initialized = 0;
    k->used = 0;
    k->next = known_vars_head;
    known_vars_head = k;

    int idx = find_symbol(name);
    if (idx == -1)
    {
        const char *dtype_str = "KUAN";
        switch(type)
        {
            case SEM_TYPE_INT:  dtype_str = "ENTEGER"; break;
            case SEM_TYPE_CHAR: dtype_str = "CHAROT"; break;
            default: dtype_str = "KUAN"; break;
        }
        add_symbol(name, dtype_str, 0, NULL);
    }

    return k;
}

SEM_TYPE sem_type_from_string(const char *s)
{
    if (!s) return SEM_TYPE_UNKNOWN;
    if (strcmp(s, "ENTEGER") == 0 || strcmp(s, "int") == 0) return SEM_TYPE_INT;
    if (strcmp(s, "CHAROT") == 0 || strcmp(s, "char") == 0) return SEM_TYPE_CHAR;
    if (strcmp(s, "KUAN") == 0) return SEM_TYPE_UNKNOWN;
    return SEM_TYPE_UNKNOWN;
}

/* ----------------------------
   Constant helpers
   ---------------------------- */

static int try_parse_int(const char *s, long *out)
{
    if (!s || !out) return 0;
    char *end;
    long v = strtol(s, &end, 10);
    if (end != s && *end == '\0') { *out = v; return 1; }
    return 0;
}

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
   Expression evaluation
   ---------------------------- */

static SEM_TEMP evaluate_expression(ASTNode *node);

static SEM_TEMP eval_factor(ASTNode *node)
{
    if (!node) return sem_new_temp(SEM_TYPE_UNKNOWN);

    if (node->type == NODE_LITERAL)
    {
        long v;
        SEM_TEMP t;
        if (try_parse_int(node->value, &v))
        {
            t = sem_new_temp(SEM_TYPE_INT);
            t.is_constant = 1;
            t.int_value = v;
            t.node = node;
            return t;
        }
        if (try_parse_char_literal(node->value, &v))
        {
            t = sem_new_temp(SEM_TYPE_CHAR);
            t.is_constant = 1;
            t.int_value = v;
            t.node = node;
            return t;
        }
        return sem_new_temp(SEM_TYPE_UNKNOWN);
    }

    if (node->type == NODE_IDENTIFIER)
    {
        const char *name = node->value;
        if (!name) return sem_new_temp(SEM_TYPE_UNKNOWN);

        KnownVar *kv = sem_find_var(name);
        if (kv)
        {
            kv->used = 1;
            if (!kv->initialized)
                sem_record_error(node, "Use of uninitialized variable '%s'", name);
            SEM_TEMP t = kv->temp;
            t.node = node;
            return t;
        }

        int idx = find_symbol(name);
        if (idx == -1)
        {
            sem_record_error(node, "Undeclared identifier '%s'", name);
            return sem_new_temp(SEM_TYPE_UNKNOWN);
        }

        SEM_TYPE stype = sem_type_from_string(symbol_table[idx].datatype);
        SEM_TEMP placeholder = sem_new_temp(stype);
        placeholder.node = node;

        KnownVar *new_kv = sem_add_var(name, stype);
        new_kv->used = 1;
        new_kv->initialized = symbol_table[idx].initialized;
        return placeholder;
    }

    return evaluate_expression(node);
}

static SEM_TEMP eval_term(ASTNode *node)
{
    if (!node) return sem_new_temp(SEM_TYPE_UNKNOWN);
    if (node->type != NODE_TERM) return eval_factor(node);

    SEM_TEMP L = eval_term(node->left);
    SEM_TEMP R = eval_factor(node->right);
    const char *op = node->value ? node->value : "";

    if (L.is_constant && R.is_constant && op)
    {
        long val = 0;
        if (strcmp(op, "*") == 0) val = L.int_value * R.int_value;
        else if (strcmp(op, "/") == 0) val = R.int_value ? L.int_value / R.int_value : 0;
        SEM_TEMP t = sem_new_temp(SEM_TYPE_INT);
        t.is_constant = 1;
        t.int_value = val;
        t.node = node;
        return t;
    }

    SEM_TEMP res = sem_new_temp(SEM_TYPE_INT);
    res.node = node;
    return res;
}

static SEM_TEMP eval_additive(ASTNode *node)
{
    if (!node) return sem_new_temp(SEM_TYPE_UNKNOWN);
    if (node->type != NODE_EXPRESSION) return eval_term(node);

    SEM_TEMP L = eval_additive(node->left);
    SEM_TEMP R = eval_term(node->right);
    const char *op = node->value ? node->value : "";

    if (L.is_constant && R.is_constant && op)
    {
        long val = 0;
        if (strcmp(op, "+") == 0) val = L.int_value + R.int_value;
        else if (strcmp(op, "-") == 0) val = L.int_value - R.int_value;
        SEM_TEMP t = sem_new_temp(SEM_TYPE_INT);
        t.is_constant = 1;
        t.int_value = val;
        t.node = node;
        return t;
    }

    SEM_TEMP res = sem_new_temp(SEM_TYPE_INT);
    res.node = node;
    return res;
}

static SEM_TEMP evaluate_expression(ASTNode *node)
{
    if (!node) return sem_new_temp(SEM_TYPE_UNKNOWN);

    switch (node->type)
    {
        case NODE_TERM: return eval_term(node);
        case NODE_EXPRESSION: return eval_additive(node);
        case NODE_UNARY_OP:
        {
            SEM_TEMP t = evaluate_expression(node->left);
            if (t.is_constant && node->value)
            {
                SEM_TEMP r = sem_new_temp(t.type);
                r.is_constant = 1;
                r.int_value = strcmp(node->value, "-") == 0 ? -t.int_value : t.int_value;
                r.node = node;
                return r;
            }
            return t;
        }
        case NODE_POSTFIX_OP: return evaluate_expression(node->left);
        case NODE_IDENTIFIER:
        case NODE_LITERAL: return eval_factor(node);
        default:
            if (node->left) evaluate_expression(node->left);
            if (node->right) evaluate_expression(node->right);
            return sem_new_temp(SEM_TYPE_UNKNOWN);
    }
}

/* ----------------------------
   Declaration & assignment
   ---------------------------- */

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

            KnownVar *kv = sem_find_var(name);
            if (kv)
            {
                sem_record_error(decls, "Redeclaration of variable '%s'", name);
            }
            else
            {
                kv = sem_add_var(name, dtype);

                /* Mark default initialization */
                kv->initialized = 1;
                kv->temp.is_constant = 1;
                kv->temp.int_value = dtype == SEM_TYPE_CHAR ? 0 : 0;

                int idx = find_symbol(name);
                if (idx != -1)
                {
                    symbol_table[idx].initialized = 1;
                    snprintf(symbol_table[idx].value_str, SYMBOL_VALUE_MAX, "%ld", kv->temp.int_value);
                }
            }
        }
        else if (decls->type == NODE_DECLARATION)
        {
            ASTNode *idnode = decls->left;
            ASTNode *init_expr = decls->right;
            if (!idnode || idnode->type != NODE_IDENTIFIER) { decls = decls->right; continue; }

            const char *name = idnode->value;
            KnownVar *kv = sem_find_var(name);
            if (kv) sem_record_error(idnode, "Redeclaration of variable '%s'", name);
            else
            {
                kv = sem_add_var(name, dtype);
                SEM_TEMP val = evaluate_expression(init_expr);

                kv->initialized = 1;
                kv->temp.is_constant = val.is_constant;
                kv->temp.int_value = val.is_constant ? val.int_value : 0;

                int idx = find_symbol(name);
                if (idx != -1)
                {
                    symbol_table[idx].initialized = 1;
                    if (val.is_constant) snprintf(symbol_table[idx].value_str, SYMBOL_VALUE_MAX, "%ld", val.int_value);
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
    if (!lhs || lhs->type != NODE_IDENTIFIER) { sem_record_error(assign_node, "Invalid LHS"); return; }

    const char *name = lhs->value;
    KnownVar *kv = sem_find_var(name);
    if (!kv) kv = sem_add_var(name, SEM_TYPE_INT);

    SEM_TEMP rhs_temp = evaluate_expression(rhs);

    int is_compound = assign_node->value && strlen(assign_node->value) == 2;
    if (is_compound && !kv->initialized)
        sem_record_error(lhs, "Compound assignment to uninitialized variable '%s'", name);

    kv->initialized = 1;
    kv->temp.is_constant = rhs_temp.is_constant;
    kv->temp.int_value = rhs_temp.is_constant ? rhs_temp.int_value : 0;

    int idx = find_symbol(name);
    if (idx != -1) symbol_table[idx].initialized = 1;
}

/* ----------------------------
   AST traversal
   ---------------------------- */

static void analyze_node(ASTNode *node)
{
    if (!node) return;
    switch (node->type)
    {
        case NODE_START: analyze_node(node->left); break;
        case NODE_STATEMENT_LIST:
            analyze_node(node->left);
            analyze_node(node->right);
            break;
        case NODE_STATEMENT: analyze_node(node->left); break;
        case NODE_DECLARATION: handle_declaration(node); break;
        case NODE_ASSIGNMENT: handle_assignment(node); break;
        case NODE_PRINTING: analyze_node(node->left); break;
        default: evaluate_expression(node); break;
    }
}

/* ----------------------------
   Check unused
   ---------------------------- */

static void check_unused_variables(void)
{
    for (KnownVar *k = known_vars_head; k; k = k->next)
        if (!k->used)
            sem_record_warning(k->temp.node ? k->temp.node : NULL, "Variable '%s' declared but never used", k->name);
}

/* ----------------------------
   Public API
   ---------------------------- */

int semantic_analyzer(void)
{
    sem_errors = 0; sem_warnings = 0;
    sem_next_temp_id = 1;
    sem_temps_count = 0; sem_ops_count = 0;

    KnownVar *k = known_vars_head;
    while (k) { KnownVar *n = k->next; free(k->name); free(k); k = n; }
    known_vars_head = NULL;

    if (!root) { fprintf(stderr, "[SEM] No AST\n"); return 0; }

    analyze_node(root);
    check_unused_variables();

    printf("[SEM] Analysis completed: %d semantic error(s), %d warning(s)\n", sem_errors, sem_warnings);
    return sem_errors;
}

int semantic_error_count(void) { return sem_errors; }

void sem_cleanup(void)
{
    if (sem_temps) { free(sem_temps); sem_temps = NULL; sem_temps_count = 0; sem_temps_capacity = 0; }
    if (sem_ops) { free(sem_ops); sem_ops = NULL; sem_ops_count = 0; sem_ops_capacity = 0; }
    KnownVar *k = known_vars_head;
    while (k) { KnownVar *n = k->next; free(k->name); free(k); k = n; }
    known_vars_head = NULL;
    sem_errors = 0; sem_warnings = 0;
}
