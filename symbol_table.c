#include "symbol_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

SymbolEntry *symbol_table = NULL;
size_t symbol_count = 0;
size_t symbol_capacity = 0;

/* Ensure capacity for dynamic array */
static int ensure_symbol_capacity(void)
{
    if (symbol_count >= symbol_capacity)
    {
        size_t new_cap = (symbol_capacity == 0) ? 16 : symbol_capacity * 2;
        SymbolEntry *new_table = realloc(symbol_table, new_cap * sizeof(SymbolEntry));
        if (!new_table) return 0; // failed
        symbol_table = new_table;
        symbol_capacity = new_cap;
    }
    return 1;
}

/* Add a symbol */
int add_symbol(const char *name, const char *datatype, int initialized, const char *value_str)
{
    if (!ensure_symbol_capacity()) return -1;

    strncpy(symbol_table[symbol_count].name, name, SYMBOL_NAME_MAX-1);
    symbol_table[symbol_count].name[SYMBOL_NAME_MAX-1] = '\0';

    strncpy(symbol_table[symbol_count].datatype, datatype, sizeof(symbol_table[symbol_count].datatype)-1);
    symbol_table[symbol_count].datatype[sizeof(symbol_table[symbol_count].datatype)-1] = '\0';

    symbol_table[symbol_count].initialized = initialized;

    if (value_str)
        strncpy(symbol_table[symbol_count].value_str, value_str, SYMBOL_VALUE_MAX-1);
    else
        symbol_table[symbol_count].value_str[0] = '\0';

    symbol_table[symbol_count].value_str[SYMBOL_VALUE_MAX-1] = '\0';

    return symbol_count++;
}

/* Find a symbol by name; returns index or -1 if not found */
int find_symbol(const char *name)
{
    for (size_t i = 0; i < symbol_count; i++)
    {
        if (strcmp(symbol_table[i].name, name) == 0)
            return (int)i;
    }
    return -1;
}

/* Clear symbol table */
void clear_symbol_table(void)
{
    free(symbol_table);
    symbol_table = NULL;
    symbol_count = 0;
    symbol_capacity = 0;
}

/* Print symbol table */
void print_symbol_table(void)
{
    printf("=== SYMBOL TABLE (%zu entries) ===\n", symbol_count);
    printf("%-10s | %-10s | %-10s | %-10s\n", "Name", "Datatype", "Initialized", "Value");
    printf("---------------------------------------------\n");
    for (size_t i = 0; i < symbol_count; i++)
    {
        printf("%-10s | %-10s | %-10s | %-10s\n",
            symbol_table[i].name,
            symbol_table[i].datatype,
            symbol_table[i].initialized ? "Yes" : "No",
            symbol_table[i].value_str);
    }
    printf("=================================\n");
}
