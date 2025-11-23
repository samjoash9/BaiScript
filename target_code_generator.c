#include "target_code_generator.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

Register registers[MAX_REGISTERS];
Data data_storage[MAX_DATA];
int data_count = 0;

int assembly_code_count = 0;
ASSEMBLY assembly_code[MAX_ASSEMBLY_CODE];

// === UTILITY ===
void add_assembly_line(const char *format, ...)
{
    if (assembly_code_count >= MAX_ASSEMBLY_CODE)
        return;

    va_list args;
    va_start(args, format);
    vsprintf(assembly_code[assembly_code_count++].assembly, format, args);
    va_end(args);
}

void display_assembly_code()
{
    for (int i = 0; i < assembly_code_count; i++)
    {
        char *line = assembly_code[i].assembly;
        if (i == assembly_code_count - 1)
        {
            size_t len = strlen(line);
            if (len > 0 && line[len - 1] == '\n')
                line[len - 1] = '\0';
        }
        printf("%s", line);
    }
}

void initialize_registers()
{
    for (int i = 0; i < MAX_REGISTERS; i++)
    {
        sprintf(registers[i].name, "r%d", i + 1);
        registers[i].used = 0;
        registers[i].assigned_temp[0] = '\0';
    }
}

void add_to_data_storage(char *data)
{
    strcpy(data_storage[data_count++].data, data);
}

int is_in_data_storage(char *data)
{
    if (data_count == 0)
        return 0;
    for (int i = 0; i < data_count; i++)
    {
        if (strcmp(data_storage[i].data, data) == 0)
            return 1;
    }
    return 0;
}

Register *get_available_register()
{
    for (int i = 0; i < MAX_REGISTERS; i++)
    {
        if (registers[i].used == 0)
        {
            return &registers[i];
        }
    }
    return NULL;
}

int is_tac_temporary(char *tac)
{
    if (strncmp(tac, "temp", 4) != 0)
        return 0;
    for (int i = 4; tac[i] != '\0'; i++)
        if (!isdigit((unsigned char)tac[i]))
            return 0;
    return strlen(tac) > 4;
}

Register *find_temp_reg(char *temp)
{
    for (int i = 0; i < MAX_REGISTERS; i++)
    {
        if (registers[i].used && strcmp(registers[i].assigned_temp, temp) == 0)
            return &registers[i];
    }
    return NULL;
}

int is_digit(char *value)
{
    if (!value || !*value)
        return 0;
    if (*value == '-')
        value++;
    if (!*value) return 0;
    while (*value)
    {
        if (!isdigit(*value)) return 0;
        value++;
    }
    return 1;
}

// === CHAROT CONVERSION ===
void convert_char_to_int(char *arg)
{
    if (!arg || arg[0] == '\0') return;

    // Case 1: literal char like 'a'
    if (arg[0] == '\'' && arg[2] == '\'' && arg[3] == '\0')
    {
        int val = (int)arg[1];
        sprintf(arg, "%d", val);
        return;
    }

    // Case 2: variable of type CHAROT
    int idx = find_symbol(arg);
    if (idx >= 0 && strcmp(symbol_table[idx].datatype, "CHAROT") == 0)
    {
        if (symbol_table[idx].initialized && strlen(symbol_table[idx].value_str) == 1)
        {
            sprintf(arg, "%d", (int)symbol_table[idx].value_str[0]);
        }
    }
}

// === DATA SECTION ===
void generate_data_section()
{
    add_assembly_line(".data\n");
    for (int i = 0; i < symbol_count; i++)
    {
        if (is_tac_temporary(symbol_table[i].name)) continue;
        add_assembly_line("%s: .word64 0\n", symbol_table[i].name);
        add_to_data_storage(symbol_table[i].name);
    }
}

// === TAC COMMENT ===
void display_tac_as_comment(TACInstruction ins)
{
    if (strlen(ins.arg2) == 0)
        add_assembly_line("; %s = %s\n", ins.result, ins.arg1);
    else
        add_assembly_line("; %s = %s %s %s\n", ins.result, ins.arg1, ins.op, ins.arg2);
}

// === PERFORM OPERATION ===
void perform_operation(char *result, char *arg1, char *op, char *arg2, Register *reg1, Register *reg2, Register *reg3, int is_for_temporary)
{
    if (strcmp(op, "+") == 0)
        add_assembly_line("daddu %s, %s, %s\n", reg3->name, reg1->name, reg2->name);
    else if (strcmp(op, "-") == 0)
        add_assembly_line("dsub %s, %s, %s\n", reg3->name, reg1->name, reg2->name);
    else if (strcmp(op, "*") == 0)
    {
        add_assembly_line("dmult %s, %s\n", reg1->name, reg2->name);
        add_assembly_line("mflo %s\n", reg3->name);
    }
    else if (strcmp(op, "/") == 0)
    {
        add_assembly_line("ddiv %s, %s\n", reg1->name, reg2->name);
        add_assembly_line("mflo %s\n", reg3->name);
    }

    if (!is_for_temporary)
    {
        add_assembly_line("sd %s, %s(r0)\n", reg3->name, result);
        if (reg1) { reg1->used = 0; reg1->assigned_temp[0] = '\0'; }
        if (reg2) { reg2->used = 0; reg2->assigned_temp[0] = '\0'; }
        if (reg3) { reg3->used = 0; reg3->assigned_temp[0] = '\0'; }
    }
    else
    {
        strcpy(reg3->assigned_temp, result);
        reg1->used = 0;
        reg2->used = 0;
    }
}

// === CODE SECTION ===
void generate_code_section()
{
    add_assembly_line("\n.code\n");
    for (int i = 0; i < optimizedCount; i++)
    {
        TACInstruction ins = optimizedCode[i];
        display_tac_as_comment(ins);

        // Convert CHAROT arguments to integers
        convert_char_to_int(ins.arg1);
        convert_char_to_int(ins.arg2);

        Register *reg1 = get_available_register();
        reg1->used = 1;
        Register *reg2 = get_available_register();
        reg2->used = 1;
        Register *reg3 = get_available_register();
        reg3->used = 1;

        // Assignment with operation
        if (strlen(ins.arg2) > 0)
        {
            if (is_digit(ins.arg1))
                add_assembly_line("daddiu %s, r0, %s\n", reg1->name, ins.arg1);
            else
                add_assembly_line("ld %s, %s(r0)\n", reg1->name, ins.arg1);

            if (is_digit(ins.arg2))
                add_assembly_line("daddiu %s, r0, %s\n", reg2->name, ins.arg2);
            else
                add_assembly_line("ld %s, %s(r0)\n", reg2->name, ins.arg2);

            perform_operation(ins.result, ins.arg1, ins.op, ins.arg2, reg1, reg2, reg3, is_tac_temporary(ins.result));
        }
        else // simple assignment
        {
            if (is_digit(ins.arg1))
                add_assembly_line("daddiu %s, r0, %s\n", reg1->name, ins.arg1);
            else
                add_assembly_line("ld %s, %s(r0)\n", reg1->name, ins.arg1);

            if (is_in_data_storage(ins.result))
                add_assembly_line("sd %s, %s(r0)\n", reg1->name, ins.result);
            else
                strcpy(reg1->assigned_temp, ins.result);
        }

        reg1->used = 0;
        reg2->used = 0;
        reg3->used = 0;
        add_assembly_line("\n");
    }
}

// === OUTPUT FILE ===
void output_assembly_file()
{
    FILE *file = fopen("output_assembly.txt", "w");
    if (!file) { printf("Error creating output file\n"); return; }
    for (int i = 0; i < assembly_code_count; i++)
        fprintf(file, "%s", assembly_code[i].assembly);
    fclose(file);
}

// === TARGET CODE GENERATION ===
void generate_target_code()
{
    initialize_registers();
    generate_data_section();
    generate_code_section();
    display_assembly_code();
    output_assembly_file();
}
