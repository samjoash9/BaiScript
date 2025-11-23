#include <stdio.h>
#include <stdlib.h>

#include "ast.h"      // MUST BE FIRST
#include "yacc.tab.h" // MUST BE AFTER ast.h
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

int main()
{
    // === STEP 0: OPEN SOURCE FILE ===
    yyin = fopen("input.txt", "r");
    if (yyin == NULL)
    {
        printf("Error: unable to open input.txt\n");
        return 1;
    }

    printf("=== BAI SCRIPT IS PARSING! ===\n\n");
    parse_failed = 0;

    int result = yyparse();

    if (result == 0 && !parse_failed)
    {
        printf("[PARSE] Accepted\n\n");

        printf("== AST ==\n");
        print_ast(root, 0);
    }
    else
    {
        printf("[PARSE] Rejected\n");
    }

    printf("\n=== BAI SCRIPT IS PARSED! ===\n");


    // === STEP 2: SEMANTIC ANALYSIS ===

    printf("\n=== BAI SCRIPT SEMANTIC ANALYSIS ===\n\n");
    if (result == 0 && !parse_failed)
    {
        // === SEMANTIC ANALYSIS ===
        int sem_errors = semantic_analyzer();  // analyze AST

        if (sem_errors == 0)
        {
            printf("[MAIN] Semantic analysis passed.\n");
        }
        else
        {
            printf("[MAIN] Semantic analysis failed with %d error(s).\n", sem_errors);
        }
    }
    printf("\n=== BAI SCRIPT SEMANTIC ANALYSIS ENDED ===\n\n");
    
    
    // === STEP 3: PRINT INTERMEDIATE CODE GENERATOR ===
    printf("\n=== BAI SCRIPT INTERMEDIATE CODE GENERATION ===\n\n");
    if (result == 0 && !parse_failed)
    {
        generate_intermediate_code(root);
    }
    printf("\n=== BAI SCRIPT INTERMEDIATE CODE GENERATION ENDED ===\n\n");


    // === STEP 4: TARGET CODE GENERATION ===
    printf("\n=== BAI SCRIPT TARGET CODE GENERATION ===\n\n");
    if (result == 0 && !parse_failed)
    {
        generate_target_code();
    }
    printf("\n=== BAI SCRIPT TARGET CODE GENERATION ENDED ===\n\n");


    // === STEP 5: MACHINE CODE GENERATION ===
    printf("\n=== BAI SCRIPT MACHINE CODE GENERATION ===\n\n");
    if (result == 0 && !parse_failed)
    {
        generate_machine_code();
    }
    printf("\n=== BAI SCRIPT MACHINE CODE GENERATION ENDED ===\n\n");

    printf("\n=== BAI SCRIPT SYMBOL TABLE ===\n\n");
    print_symbol_table();
    fclose(yyin);
    return 0;
}
