#include <stdio.h>
#include <stdlib.h>

#include "ast.h"      // MUST BE FIRST
#include "yacc.tab.h" // MUST BE AFTER ast.h

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

    printf("=== BAI SCRIPT IS PARSING! ===\n");
    parse_failed = 0;

    int result = yyparse();

    if (result == 0 && !parse_failed)
    {
        printf("[PARSE] Accepted\n");

        print_ast(root, 0);
    }
    else
    {
        printf("[PARSE] Rejected\n");
    }

    //

    fclose(yyin);
    return 0;
}
