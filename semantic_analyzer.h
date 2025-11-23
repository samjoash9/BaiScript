#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include <stddef.h>
#include "ast.h"

/* ----------------------------
Semantic Types & Temp Records
---------------------------- */
typedef enum {
SEM_TYPE_UNKNOWN,
SEM_TYPE_INT,
SEM_TYPE_CHAR
} SEM_TYPE;

typedef struct {
int id;             /* unique temp id */
SEM_TYPE type;      /* type of this temp */
int is_constant;    /* boolean: constant or not */
long int_value;     /* value if constant */
ASTNode *node;      /* optional: originating AST node */
} SEM_TEMP;

/* ----------------------------
Known Variable (tracked in analyzer)
---------------------------- */
typedef struct KnownVar {
char *name;           /* variable name */
SEM_TEMP temp;        /* temp representing this variable */
int initialized;      /* boolean: has been initialized */
int used;             /* boolean: has been used */
struct KnownVar *next;
} KnownVar;

/* ----------------------------
Semantic Operation Records
---------------------------- */
typedef enum {
SEMOP_DECL,          /* declaration */
SEMOP_STORE_CONST,   /* store constant */
SEMOP_ASSIGN,        /* assignment dst = src */
SEMOP_BINARY,        /* dst = src1 op src2 */
SEMOP_UNARY          /* dst = op src1 */
} SEMOP_TYPE;

typedef struct {
SEMOP_TYPE type;
int dst_temp;
int src1_temp;
int src2_temp;
char op[8];           /* operator text: "+", "-", etc */
long const_val;       /* value if storing constant */
ASTNode *node;        /* AST node origin */
} SEM_OP;

/* ----------------------------
Public API
---------------------------- */

/* Analyze the global AST root; returns number of semantic errors (0 = success) */
extern int semantic_analyzer(void);

/* Returns number of semantic errors recorded */
int semantic_error_count(void);

/* Returns read-only array of temps, and sets count */
const SEM_TEMP *sem_get_temps(size_t *out_count);

/* Returns read-only array of semantic operations, and sets count */
const SEM_OP *sem_get_ops(size_t *out_count);

/* Cleanup internal structures (free memory) */
void sem_cleanup(void);

/* Utility: create a new temp of given type */
SEM_TEMP sem_new_temp(SEM_TYPE type);

/* Variable lookup/addition in analyzer */
KnownVar* sem_find_var(const char *name);
KnownVar* sem_add_var(const char *name, SEM_TYPE type);

/* Map datatype string token to SEM_TYPE */
SEM_TYPE sem_type_from_string(const char *s);

/* Optional: semantic annotation / textual notes */
void sem_emit(const char *fmt, ...);

#endif /* SEMANTIC_ANALYZER_H */
