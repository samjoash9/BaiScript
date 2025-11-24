#include <stdio.h>
#include <stdlib.h>

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

int main()
{

    // === STEP 0: OPEN SOURCE FILE ===
    yyin = fopen("input.txt", "r");
    if (!yyin)
    {
        printf("Error: unable to open input.txt\n");
        return 1;
        
    }

    printf("=== BaiScript IS PARSING! ===\n\n");
  
    parse_failed = 0;  // reset before starting
    islexerror = 0; // reset before starting
    int result = yyparse();

    if (result == 0 && !parse_failed)
    {
        // Emtying all the files at the start
        write_error_file("output_assembly.txt", "");
        write_error_file("output_machine.txt", "");
        write_error_file("output_tac.txt", "");
        write_error_file("output_print.txt", "");
        printf("[PARSE] Accepted\n\n");
        printf("== AST ==\n");
        print_ast(root, 0);
    }
    else
    {
        write_error_file("output_assembly.txt", "No assembly generated due to parse errors.");
        write_error_file("output_machine.txt", "No machine code generated due to parse errors.");
        write_error_file("output_tac.txt", "No TAC generated due to parse errors.");
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
        write_error_file("output_tac.txt", "No TAC generated due to semantic errors.");
    }
    else if (result == 0 && !parse_failed)
    {
        generate_intermediate_code(root);
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
        write_error_file("output_assembly.txt", "No assembly generated due to semantic errors.");
    }
    else if (result == 0 && !parse_failed)
    {
        generate_target_code();
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
        write_error_file("output_machine.txt", "No machine code generated due to semantic errors.");
    }
    else if (result == 0 && !parse_failed)
    {
        generate_machine_code();
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

    return sem_errors > 0 ? 1 : 0;
}
