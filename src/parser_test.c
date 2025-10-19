#include <stdio.h>
#include "scanner.h"
#include "ast.h"

void test(char *input)
{
    char **tokens = scan(input);
    printf("Tokens are: \n");
    print_tokens(tokens);
    printf("\nAST is: \n");
    Cmd *ast = parse(tokens);
    print_cmd(ast);


    free_tokens(tokens);
    free_cmd(ast);
    printf("\n");
}

//using raw strings from gnu expansion for tests, so ignore error squigglies
int main() {

     test(R"((ls -l | wc -w) && echo hello)");
    
     test(R"(ls -l | wc -w && echo hello &)");
    
     test(R"(echo hi >out.txt)");
    
     test(R"(ls -a | grep -E ^\d\d\d > out.txt)");
    
     test(R"(echo hello || (echo there | grep -E ^the))");

    test(R"(;)");
    
    return 0;
}