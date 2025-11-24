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

static FILE *out_file = NULL; 

/* Deferred postfix ops */
typedef struct DeferredOp {
    KnownVar *kv;
    int delta; // +1 for ++, -1 for --
    struct DeferredOp *next;
} DeferredOp;

static DeferredOp *deferred_head = NULL;

/* ----------------------------
   Helpers: error/warning
---------------------------- */

static void sem_record_error(ASTNode *node, const char *fmt, ...)
{
    sem_errors++;

    if (!out_file) return;

    fprintf(out_file, "[SEM ERROR] ");

    va_list ap;
    va_start(ap, fmt);
    vfprintf(out_file, fmt, ap); 
    va_end(ap);

    if (node) fprintf(out_file, " [line:%d]\n", node->line);
}

static void sem_record_warning(ASTNode *node, const char *fmt, ...)
{
    sem_warnings++;

    if (!out_file) return;

    fprintf(out_file, "[SEM WARNING] ");

    va_list ap;
    va_start(ap, fmt);
    vfprintf(out_file, fmt, ap);
    va_end(ap);

    if (node) fprintf(out_file, " [line:%d]\n", node->line);
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
   Deferred postfix helpers
---------------------------- */
static void push_deferred_op(KnownVar *kv, int delta)
{
    DeferredOp *d = (DeferredOp*)malloc(sizeof(DeferredOp));
    if (!d) return;
    d->kv = kv;
    d->delta = delta;
    d->next = deferred_head;
    deferred_head = d;
}

static void apply_deferred_ops(void)
{
    if (!deferred_head) return;

    // Reverse the list for FIFO
    DeferredOp *prev = NULL, *cur = deferred_head;
    while (cur) { DeferredOp *n = cur->next; cur->next = prev; prev = cur; cur = n; }
    DeferredOp *r = prev;

    for (DeferredOp *p = r; p; p = p->next)
    {
        KnownVar *kv = p->kv;
        if (!kv) continue;
        if (!kv->initialized) {
            sem_record_error(kv->temp.node, "Postfix operation on uninitialized variable '%s'", kv->name);
            continue;
        }
        long before = kv->temp.is_constant ? kv->temp.int_value : 0;
        long after = before + p->delta;
        kv->temp.is_constant = 1;
        kv->temp.int_value = after;
        kv->initialized = 1;

        int idx = find_symbol(kv->name);
        if (idx != -1) {
            symbol_table[idx].initialized = 1;
            snprintf(symbol_table[idx].value_str, SYMBOL_VALUE_MAX, "%ld", after);
        }
    }

    // free list
    DeferredOp *it = r;
    while (it) { DeferredOp *n = it->next; free(it); it = n; }
    deferred_head = NULL;
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
    k->temp.node = NULL;
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
            case '"': *out = '"'; return 1;
            default: return 0;
        }
    }
    return 0;
}

/* ----------------------------
   Expression evaluation
---------------------------- */

static SEM_TEMP evaluate_expression(ASTNode *node);
static SEM_TEMP eval_factor(ASTNode *node);
static SEM_TEMP eval_term(ASTNode *node);
static SEM_TEMP eval_additive(ASTNode *node);

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
        if (symbol_table[idx].initialized) {
            long vv = 0;
            if (try_parse_int(symbol_table[idx].value_str, &vv)) {
                new_kv->temp.is_constant = 1;
                new_kv->temp.int_value = vv;
            }
        }
        return placeholder;
    }

    return evaluate_expression(node);
}

static SEM_TEMP eval_term(ASTNode *node)
{
    if (!node) return sem_new_temp(SEM_TYPE_UNKNOWN);
    if (node->type != NODE_TERM) return eval_factor(node);

    if (!node->left || !node->right) {
        if (node->left) evaluate_expression(node->left);
        if (node->right) evaluate_expression(node->right);
        return sem_new_temp(SEM_TYPE_UNKNOWN);
    }

    SEM_TEMP L = eval_term(node->left);
    SEM_TEMP R = eval_factor(node->right);
    const char *op = node->value ? node->value : "";

    if (L.is_constant && R.is_constant && op[0] != '\0')
    {
        long val = 0;
        if (strcmp(op, "*") == 0)
        {
            val = L.int_value * R.int_value;
        }
        else if (strcmp(op, "/") == 0)
        {
            if (R.int_value == 0)
            {
                sem_record_error(node, "Division by zero");
                val = 0; // fallback
            }
            else
            {
                val = L.int_value / R.int_value;
            }
        }
        else {
            goto no_fold_term;
        }
        SEM_TEMP t = sem_new_temp(SEM_TYPE_INT);
        t.is_constant = 1;
        t.int_value = val;
        t.node = node;
        return t;
    }

no_fold_term:
    {
        SEM_TEMP res = sem_new_temp(SEM_TYPE_INT);
    res.node = node;
    return res;
    }
}

static SEM_TEMP eval_additive(ASTNode *node)
{
    if (!node) return sem_new_temp(SEM_TYPE_UNKNOWN);
    if (node->type != NODE_EXPRESSION) return eval_term(node);

    if (!node->left || !node->right) {
        if (node->left) evaluate_expression(node->left);
        if (node->right) evaluate_expression(node->right);
        return sem_new_temp(SEM_TYPE_UNKNOWN);
    }

    SEM_TEMP L = eval_additive(node->left);
    SEM_TEMP R = eval_term(node->right);
    const char *op = node->value ? node->value : "";

    if (L.is_constant && R.is_constant && op[0] != '\0')
    {
        long val = 0;
        if (strcmp(op, "+") == 0) val = L.int_value + R.int_value;
        else if (strcmp(op, "-") == 0) val = L.int_value - R.int_value;
        else goto no_fold_add;
        SEM_TEMP t = sem_new_temp(SEM_TYPE_INT);
        t.is_constant = 1;
        t.int_value = val;
        t.node = node;
        return t;
    }

no_fold_add:
    {
        SEM_TEMP res = sem_new_temp(SEM_TYPE_INT);
    res.node = node;
    return res;
    }
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
            // prefix operators: e.g. ++a, --a, unary -
            const char *op = node->value ? node->value : "";
            if (op && (strcmp(op, "++") == 0 || strcmp(op, "--") == 0)) {
                ASTNode *target = node->left;
                if (!target || target->type != NODE_IDENTIFIER) {
                    sem_record_error(node, "Prefix %s applied to non-identifier", op);
                    return sem_new_temp(SEM_TYPE_UNKNOWN);
                }
                const char *name = target->value;
                KnownVar *kv = sem_find_var(name);
                if (!kv) kv = sem_add_var(name, SEM_TYPE_INT);
                if (!kv->initialized) {
                    sem_record_error(target, "Prefix %s on uninitialized variable '%s'", op, name);
                    // but still mark initialized and continue
                    kv->initialized = 1;
                    kv->temp.is_constant = 1;
                    kv->temp.int_value = 0;
                }
                int delta = (strcmp(op, "++") == 0) ? 1 : -1;
                long newval = (kv->temp.is_constant ? kv->temp.int_value : 0) + delta;
                kv->temp.is_constant = 1;
                kv->temp.int_value = newval;
                kv->initialized = 1;

                int idx = find_symbol(kv->name);
                if (idx != -1) {
                    symbol_table[idx].initialized = 1;
                    snprintf(symbol_table[idx].value_str, SYMBOL_VALUE_MAX, "%ld", newval);
                }

                SEM_TEMP r = sem_new_temp(SEM_TYPE_INT);
                r.is_constant = 1;
                r.int_value = newval;
                r.node = node;
                return r;
            }
            // unary minus and other unary ops
            {
                SEM_TEMP t = evaluate_expression(node->left);
                if (t.is_constant && op && op[0] != '\0' && strcmp(op, "-") == 0)
                {
                    SEM_TEMP r = sem_new_temp(t.type);
                    r.is_constant = 1;
                    r.int_value = -t.int_value;
                    r.node = node;
                    return r;
                }
                return t;
            }
        }
        case NODE_POSTFIX_OP:
        {
            // postfix op: return old value but schedule increment/decrement
            const char *op = node->value ? node->value : "";
            ASTNode *target = node->left;
            if (!target || target->type != NODE_IDENTIFIER) {
                sem_record_error(node, "Postfix %s applied to non-identifier", op);
                return sem_new_temp(SEM_TYPE_UNKNOWN);
            }
            const char *name = target->value;
            KnownVar *kv = sem_find_var(name);
            if (!kv) kv = sem_add_var(name, SEM_TYPE_INT);

            // old value to return
            SEM_TEMP old = kv->temp;
            old.node = node;
            if (!kv->initialized) {
                // per C semantics using uninitialized is undefined; here we record error
                sem_record_error(target, "Use of uninitialized variable '%s' in postfix operation", name);
                old.is_constant = 0;
            }

            int delta = 0;
            if (strcmp(op, "++") == 0) delta = 1;
            else if (strcmp(op, "--") == 0) delta = -1;
            if (delta != 0) {
                push_deferred_op(kv, delta);
            }
            return old;
        }
        case NODE_IDENTIFIER:
        case NODE_LITERAL: return eval_factor(node);
        default:
            if (node->left) evaluate_expression(node->left);
            if (node->right) evaluate_expression(node->right);
            return sem_new_temp(SEM_TYPE_UNKNOWN);
    }
}

/* ----------------------------
   Buffers for output
---------------------------- */
#define PRINT_BUFFER_SIZE 8192
static char print_buffer[PRINT_BUFFER_SIZE];
static size_t print_offset = 0;

static void buffer_print(const char *s)
{
    if (!s) return;
    size_t len = strlen(s);
    if (print_offset + len < PRINT_BUFFER_SIZE - 2)
    {
        strcpy(print_buffer + print_offset, s);
        print_offset += len;
    }
}


/* ----------------------------
   Handle print statements (buffered)
---------------------------- */
static void handle_print(ASTNode *print_node)
{
    if (!print_node) return;

    ASTNode *item = print_node->left;
    while (item)
    {
        ASTNode *expr = item->left ? item->left : item;
        char tempbuf[1024] = {0};

        if (expr->type == NODE_LITERAL)
        {
            const char *val = expr->value;
            size_t len = strlen(val);
            if (len >= 2 && val[0] == '"' && val[len-1] == '"')
                snprintf(tempbuf, sizeof(tempbuf), "%.*s", (int)(len-2), val+1);
            else
                snprintf(tempbuf, sizeof(tempbuf), "%s", val);
        }
        else
        {
            SEM_TEMP val = evaluate_expression(expr);
            if (val.is_constant) snprintf(tempbuf, sizeof(tempbuf), "%ld", val.int_value);
        }

        buffer_print(tempbuf);
        item = item->right;
    }

    buffer_print("\n");
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

                /* Attach declaration AST node for correct warning line */
                kv->temp.node = decls;

                /* Mark default initialization */
                kv->initialized = 1;
                kv->temp.is_constant = 1;
                kv->temp.int_value = 0;

                int idx = find_symbol(name);
                if (idx != -1)
                {
                    symbol_table[idx].initialized = 1;
                    symbol_table[idx].value_str[0] = '\0';
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

                /* Attach declaration AST node for correct warning line */
                kv->temp.node = idnode;

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

    int is_compound = assign_node->value && strlen(assign_node->value) == 2; // e.g. "+="
    if (is_compound && !kv->initialized)
        sem_record_error(lhs, "Compound assignment to uninitialized variable '%s'", name);

    if (is_compound && kv->initialized && rhs_temp.is_constant)
    {
        const char *op = assign_node->value; // "+=", "-=", "*=", "/="
        long lhs_val = kv->temp.is_constant ? kv->temp.int_value : 0;
        long newval = lhs_val;
        if (strcmp(op, "+=") == 0) newval = lhs_val + rhs_temp.int_value;
        else if (strcmp(op, "-=") == 0) newval = lhs_val - rhs_temp.int_value;
        else if (strcmp(op, "*=") == 0) newval = lhs_val * rhs_temp.int_value;
        else if (strcmp(op, "/=") == 0) {
            if (rhs_temp.int_value == 0) {
                sem_record_error(assign_node, "Division by zero in compound assignment");
                newval = 0;
            } else newval = lhs_val / rhs_temp.int_value;
        }
        kv->initialized = 1;
        kv->temp.is_constant = 1;
        kv->temp.int_value = newval;
        int idx = find_symbol(name);
        if (idx != -1) {
            symbol_table[idx].initialized = 1;
            snprintf(symbol_table[idx].value_str, SYMBOL_VALUE_MAX, "%ld", newval);
        }
    }
    else
    {
        // simple assignment or compound with non-constant RHS
        kv->initialized = 1;
        kv->temp.is_constant = rhs_temp.is_constant;
        kv->temp.int_value = rhs_temp.is_constant ? rhs_temp.int_value : 0;

        int idx = find_symbol(name);
        if (idx != -1) {
            symbol_table[idx].initialized = 1;
            if (kv->temp.is_constant) snprintf(symbol_table[idx].value_str, SYMBOL_VALUE_MAX, "%ld", kv->temp.int_value);
        }
    }
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
        case NODE_STATEMENT:
            analyze_node(node->left);
            apply_deferred_ops();
            break;
        case NODE_DECLARATION:
            handle_declaration(node);
            apply_deferred_ops();
            break;
        case NODE_ASSIGNMENT:
            handle_assignment(node);
            apply_deferred_ops();
            break;
        case NODE_PRINTING:
        case NODE_PRINT_ITEM:
            handle_print(node);
            apply_deferred_ops();
            break;
        default:
            evaluate_expression(node);
            break;
    }
}

/* ----------------------------
   Check unused
---------------------------- */

static void check_unused_variables(void)
{
    for (KnownVar *k = known_vars_head; k; k = k->next)
    {
        ASTNode *node = k->temp.node;
        if (!k->used)
            sem_record_warning(node, "Variable '%s' declared but never used", k->name);
    }
}

/* ----------------------------
   Public API
---------------------------- */

/* ----------------------------
   Semantic analyzer
---------------------------- */
int semantic_analyzer(void)
{
    // overwrite old file
    out_file = fopen("output_print.txt", "w");
    if (!out_file) { fprintf(stderr, "[SEM] Failed to open output_print.txt\n"); return 0; }

    sem_errors = 0; sem_warnings = 0;
    sem_next_temp_id = 1;
    sem_temps_count = 0; sem_ops_count = 0;
    print_offset = 0;
    print_buffer[0] = '\0';

    // free known vars
    KnownVar *k = known_vars_head;
    while (k) { KnownVar *n = k->next; free(k->name); free(k); k = n; }
    known_vars_head = NULL;

    if (!root) { fprintf(stderr, "[SEM] No AST\n"); fclose(out_file); return 0; }

    // Traverse AST
    analyze_node(root);


    // If errors exist, discard buffered prints
    if (sem_errors > 0)
    {
        print_offset = 0;
        print_buffer[0] = '\0';
    }
    else
    {
        // No errors: flush buffered prints
        if (print_offset > 0) fprintf(out_file, "%s", print_buffer);
    }


    if (sem_errors == 0){
        // Write semantic analysis summary
        fprintf(out_file, "\n\n=== COMPILATION SUCCESSFULL ===\n\n");
        check_unused_variables();
        fprintf(out_file, "[SEM] Analysis completed: %d semantic error(s), %d warning(s)\n", sem_errors, sem_warnings);
    }

    fclose(out_file);
    out_file = NULL;

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

    DeferredOp *d = deferred_head;
    while (d) { DeferredOp *n = d->next; free(d); d = n; }
    deferred_head = NULL;

    sem_errors = 0; sem_warnings = 0;
}