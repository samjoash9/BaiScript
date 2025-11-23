#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stddef.h>

#define SYMBOL_NAME_MAX 64
#define SYMBOL_VALUE_MAX 64

typedef enum {
    SYM_TYPE_KUAN,
    SYM_TYPE_ENTEGER,
    SYM_TYPE_CHAROT
} SYM_TYPE;

typedef struct {
    char name[SYMBOL_NAME_MAX];
    char datatype[16];          /* original type token: ENTEGER, CHAROT, etc */
    int initialized;            /* 0 = no, 1 = yes */
    char value_str[SYMBOL_VALUE_MAX]; /* optional constant value as string */
} SymbolEntry;

extern SymbolEntry *symbol_table;
extern size_t symbol_count;
extern size_t symbol_capacity;

/* Symbol table operations */
int add_symbol(const char *name, const char *datatype, int initialized, const char *value_str);
int find_symbol(const char *name);
void clear_symbol_table(void);
void print_symbol_table(void);

#endif /* SYMBOL_TABLE_H */
