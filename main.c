#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "yacc.tab.h"
#include "semantic_analyzer.h"
#include "intermediate_code_generator.h"
#include "target_code_generator.h"
#include "machine_code_generator.h"
#include "symbol_table.h"

extern int yyparse(void);
extern int parse_failed;
extern FILE *yyin;
extern ASTNode *root;

int parse_failed = 0;
int sem_errors = 0;

void write_error_file(const char *filename, const char *msg)
{
    FILE *f = fopen(filename, "w");
    if (f)
    {
        fprintf(f, "%s\n", msg);
        fclose(f);
    }
}

void write_machine_error_files(const char *error_msg)
{
    // Write the same error message to all machine code output files
    // This ensures the Electron process finds consistent error messages
    write_error_file("output_machine_assembly.txt", error_msg);
    write_error_file("output_machine_bin.txt", error_msg);
    write_error_file("output_machine_hex.txt", error_msg);
    // Also write to the legacy single file for backward compatibility
    write_error_file("output_machine.txt", error_msg);
}

void write_assembly_error_file(const char *error_msg)
{
    write_error_file("output_assembly.txt", error_msg);
}

void write_tac_error_file(const char *error_msg)
{
    write_error_file("output_tac.txt", error_msg);
}

void write_print_error_file(const char *error_msg)
{
    write_error_file("output_print.txt", error_msg);
}

void initialize_output_files()
{
    // Clear all output files at start
    write_error_file("output_assembly.txt", "");
    write_error_file("output_machine_assembly.txt", "");
    write_error_file("output_machine_bin.txt", "");
    write_error_file("output_machine_hex.txt", "");
    write_error_file("output_machine.txt", "");
    write_error_file("output_tac.txt", "");
    write_error_file("output_print.txt", "");
}

int main()
{
    // === STEP 0: OPEN SOURCE FILE ===
    yyin = fopen("input.txt", "r");
    if (!yyin)
    {
        printf("Error: unable to open input.txt\n");
        // Write error to all output files
        write_assembly_error_file("Error: unable to open input.txt");
        write_machine_error_files("Error: unable to open input.txt");
        write_tac_error_file("Error: unable to open input.txt");
        write_print_error_file("Error: unable to open input.txt");
        return 1;
    }

    printf("=== BaiScript IS PARSING! ===\n\n");

    parse_failed = 0; // reset before starting
    islexerror = 0;   // reset before starting

    // Initialize all output files to empty
    initialize_output_files();

    int result = yyparse();

    if (result == 0 && !parse_failed)
    {
        printf("[PARSE] Accepted\n\n");
        printf("== AST ==\n");
        print_ast(root, 0);
    }
    else
    {
        const char *parse_error_msg = "No assembly generated due to parse errors.";
        printf("[PARSE] Failed - writing error messages to output files\n");
        write_assembly_error_file(parse_error_msg);
        write_machine_error_files("No machine code generated due to parse errors.");
        write_tac_error_file("No TAC generated due to parse errors.");
        write_print_error_file("[MAIN] Compilation failed due to parse errors.");
    }

    printf("\n=== BaiScript IS PARSED! ===\n");

    // === STEP 2: SEMANTIC ANALYSIS ===
    printf("\n=== BaiScript SEMANTIC ANALYSIS ===\n\n");

    if (result == 0 && !parse_failed)
    {
        // analyze AST
        sem_errors = semantic_analyzer(); // assign to global

        if (sem_errors == 0)
        {
            printf("[MAIN] Semantic analysis passed.\n");
        }
        else
        {
            printf("[MAIN] Semantic analysis failed with %d error(s).\n", sem_errors);
            // Write semantic errors to output files
            char sem_error_msg[256];
            snprintf(sem_error_msg, sizeof(sem_error_msg),
                     "No assembly generated due to %d semantic error(s).", sem_errors);
            write_assembly_error_file(sem_error_msg);
            write_machine_error_files("No machine code generated due to semantic errors.");
            write_tac_error_file("No TAC generated due to semantic errors.");
        }
    }
    else
    {
        printf("[MAIN] Skipping semantic analysis due to parse errors.\n");
    }

    printf("\n=== BaiScript SEMANTIC ANALYSIS ENDED ===\n\n");

    // === STEP 3: INTERMEDIATE CODE GENERATION ===
    printf("\n=== BaiScript INTERMEDIATE CODE GENERATION ===\n\n");

    if (sem_errors > 0)
    {
        printf("[MAIN] Skipping intermediate code generation due to semantic errors.\n");
        // Error messages already written in semantic analysis step
    }
    else if (result == 0 && !parse_failed)
    {
        generate_intermediate_code(root);
        printf("[MAIN] Intermediate code generation completed.\n");
    }
    else
    {
        printf("[MAIN] Skipping intermediate code generation due to parse errors.\n");
    }

    printf("\n=== BaiScript INTERMEDIATE CODE GENERATION ENDED ===\n\n");

    // === STEP 4: TARGET CODE GENERATION ===
    printf("\n=== BaiScript TARGET CODE GENERATION ===\n\n");

    if (sem_errors > 0)
    {
        printf("[MAIN] Skipping target code generation due to semantic errors.\n");
        // Error messages already written in semantic analysis step
    }
    else if (result == 0 && !parse_failed)
    {
        generate_target_code();
        printf("[MAIN] Target code generation completed.\n");
    }
    else
    {
        printf("[MAIN] Skipping target code generation due to parse errors.\n");
    }

    printf("\n=== BaiScript TARGET CODE GENERATION ENDED ===\n\n");

    // === STEP 5: MACHINE CODE GENERATION ===
    printf("\n=== BaiScript MACHINE CODE GENERATION ===\n\n");

    if (sem_errors > 0)
    {
        printf("[MAIN] Skipping machine code generation due to semantic errors.\n");
        // Error messages already written in semantic analysis step
    }
    else if (result == 0 && !parse_failed)
    {
        generate_machine_code();
        printf("[MAIN] Machine code generation completed.\n");
    }
    else
    {
        printf("[MAIN] Skipping machine code generation due to parse errors.\n");
    }

    printf("\n=== BaiScript MACHINE CODE GENERATION ENDED ===\n\n");

    // === SYMBOL TABLE ===
    printf("\n=== BaiScript SYMBOL TABLE ===\n\n");
    print_symbol_table();

    fclose(yyin);

    // Return appropriate exit code
    if (result != 0 || parse_failed || sem_errors > 0)
    {
        printf("\n\n[MAIN] Compilation failed with errors\nn");
        return 1;
    }
    else
    {
        printf("\n\n[MAIN] Compilation successful\n\n");
        return 0;
    }
}