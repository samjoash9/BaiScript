#include "machine_code_generator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int start_code_counter = 0;

const char *R_TYPE[R_TYPE_COUNT] = {"daddu", "dsub", "dmult", "ddiv", "mflo"};
const char *I_TYPE[I_TYPE_COUNT] = {"daddiu", "ld", "sd"};

/* ===================== DATA SYMBOLS ===================== */

typedef struct
{
    char label[32];
    int address;
} DataSymbol;

DataSymbol data_symbols[100];
int data_symbol_count = 0;
int current_data_address = 0xFFF8;

/* ===================== MACHINE CODE STORAGE ===================== */

typedef struct
{
    char assembly[64];
    char machine_bin[64];
    unsigned int machine_hex;
} MachineCodeEntry;

MachineCodeEntry machine_code_list[500];
int machine_code_count = 0;

/* ===================== HELPERS ===================== */

void trim(char *str)
{
    char *end;
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    *(end + 1) = 0;
}

void convert_to_binary(int num, int bits, char *output)
{
    output[bits] = '\0';
    for (int i = bits - 1; i >= 0; i--)
    {
        output[i] = (num & 1) ? '1' : '0';
        num >>= 1;
    }
}

int parse_register(const char *token)
{
    if (token[0] == 'r' || token[0] == 'R')
        return atoi(token + 1);
    return 0;
}

int get_opcode(const char *mnemonic)
{
    if (!strcmp(mnemonic, "daddu")) return 0x00;
    if (!strcmp(mnemonic, "dsub"))  return 0x00;
    if (!strcmp(mnemonic, "dmult")) return 0x00;
    if (!strcmp(mnemonic, "ddiv"))  return 0x00;
    if (!strcmp(mnemonic, "mflo"))  return 0x00;
    if (!strcmp(mnemonic, "daddiu")) return 0x19;
    if (!strcmp(mnemonic, "ld"))    return 0x37;
    if (!strcmp(mnemonic, "sd"))    return 0x3F;
    return 0;
}

int get_funct(const char *mnemonic)
{
    if (!strcmp(mnemonic, "daddu")) return 0x2D;
    if (!strcmp(mnemonic, "dsub"))  return 0x2E;
    if (!strcmp(mnemonic, "dmult")) return 0x1C;
    if (!strcmp(mnemonic, "ddiv"))  return 0x1E;
    if (!strcmp(mnemonic, "mflo"))  return 0x12;
    return 0;
}

/* ===================== REMOVE .data / .code ===================== */

void remove_data_and_code_section()
{
    int i = 0;
    while (i < assembly_code_count)
    {
        char *line = assembly_code[i].assembly;

        char label[32];
        if (sscanf(line, "%31[^:]:", label) == 1)
        {
            trim(label);
            strcpy(data_symbols[data_symbol_count].label, label);
            data_symbols[data_symbol_count].address = current_data_address;
            data_symbol_count++;
            current_data_address += 8;
        }

        if (strstr(assembly_code[i].assembly, ".code"))
        {
            start_code_counter = i + 1;
            break;
        }
        i++;
    }

    int j = 0;
    for (i = start_code_counter; i < assembly_code_count; i++, j++)
        strcpy(assembly_code[j].assembly, assembly_code[i].assembly);

    assembly_code_count = j;
}

/* ===================== MACHINE CODE GENERATOR ===================== */

void convert_to_machine_code()
{
    char mnemonic[32], operands[128];
    char bin_opcode[7], bin_rs[6], bin_rt[6], bin_rd[6], bin_shamt[6], bin_funct[7], bin_imm[17];
    char full_bin[64];

    for (int i = 0; i < assembly_code_count; i++)
    {
        assembly_code[i].assembly[strcspn(assembly_code[i].assembly, "\n")] = '\0';

        if (strstr(assembly_code[i].assembly, ";")) continue;

        if (sscanf(assembly_code[i].assembly, "%31s %[^\n]", mnemonic, operands) < 1)
            continue;

        trim(operands);

        int opcode = get_opcode(mnemonic);
        int funct = get_funct(mnemonic);
        int rs = 0, rt = 0, rd = 0, imm = 0;
        char *tok;

        /* ----- Operand Parsing ----- */
        if (!strcmp(mnemonic, "mflo"))
        {
            tok = strtok(operands, ", ");
            if (tok) rd = parse_register(tok);
        }
        else if (!strcmp(mnemonic, "dmult") || !strcmp(mnemonic, "ddiv"))
        {
            tok = strtok(operands, ", ");
            if (tok) rs = parse_register(tok);
            tok = strtok(NULL, ", ");
            if (tok) rt = parse_register(tok);
        }
        else if (opcode == 0) /* R-type */
        {
            tok = strtok(operands, ", ");
            if (tok) rd = parse_register(tok);
            tok = strtok(NULL, ", ");
            if (tok) rs = parse_register(tok);
            tok = strtok(NULL, ", ");
            if (tok) rt = parse_register(tok);
        }
        else /* I-type */
        {
            tok = strtok(operands, ", ");
            if (tok) rt = parse_register(tok);

            tok = strtok(NULL, ", ");
            if (tok)
            {
                char *paren = strchr(tok, '(');
                if (paren)
                {
                    *paren = '\0';
                    trim(tok);

                    char *base = paren + 1;
                    base[strcspn(base, ")")] = 0;

                    rs = parse_register(base);

                    int found = 0;
                    for (int d = 0; d < data_symbol_count; d++)
                    {
                        if (!strcmp(tok, data_symbols[d].label))
                        {
                            imm = data_symbols[d].address;
                            found = 1;
                            break;
                        }
                    }

                    if (!found) imm = atoi(tok);
                }
                else
                {
                    rs = parse_register(tok);
                    tok = strtok(NULL, ", ");
                    if (tok) imm = atoi(tok);
                }
            }
        }

        /* ----- Convert to Binary ----- */
        convert_to_binary(opcode, 6, bin_opcode);
        convert_to_binary(rs, 5, bin_rs);
        convert_to_binary(rt, 5, bin_rt);
        convert_to_binary(rd, 5, bin_rd);
        convert_to_binary(0, 5, bin_shamt);
        convert_to_binary(funct, 6, bin_funct);
        convert_to_binary(imm & 0xFFFF, 16, bin_imm);

        if (opcode == 0)
            snprintf(full_bin, sizeof(full_bin), "%s%s%s%s%s%s",
                     bin_opcode, bin_rs, bin_rt, bin_rd, bin_shamt, bin_funct);
        else
            snprintf(full_bin, sizeof(full_bin), "%s%s%s%s",
                     bin_opcode, bin_rs, bin_rt, bin_imm);

        /* ----- Convert binary to hex ----- */
        unsigned int hex_val = 0;
        for (int b = 0; b < strlen(full_bin); b++)
            hex_val = (hex_val << 1) | (full_bin[b] == '1');

        /* ----- STORE in machine_code_list[] ----- */
        strcpy(machine_code_list[machine_code_count].assembly, assembly_code[i].assembly);
        strcpy(machine_code_list[machine_code_count].machine_bin, full_bin);
        machine_code_list[machine_code_count].machine_hex = hex_val;
        machine_code_count++;

        /* Console Output */
        printf("%-25s -> %s (0x%08X)\n",
            assembly_code[i].assembly, full_bin, hex_val);
    }
}

/* ===================== WRITE TO FILE ===================== */

void output_machine_file()
{
    FILE *f = fopen("output_machine.txt", "w");
    if (!f) { printf("ERROR: Cannot write output file!\n"); return; }

    for (int i = 0; i < machine_code_count; i++)
    {
        fprintf(f, "%-25s -> %s (0x%08X)\n",
                machine_code_list[i].assembly,
                machine_code_list[i].machine_bin,
                machine_code_list[i].machine_hex);
    }

    fclose(f);
}

/* ===================== MAIN ENTRY ===================== */

void generate_machine_code()
{
    remove_data_and_code_section();
    convert_to_machine_code();
    output_machine_file();
}
