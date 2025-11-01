#include <stdio.h>
#include "scanner.h"
#include "parser.h"

void test(char *input)
{
    char **tokens = scan(input);
    printf("Tokens are: \n");
    print_tokens(tokens);
    printf("\n");

    Cmd *ast = parse(tokens);
    printf("AST is: \n");
    print_cmd(ast);
    printf("\n");

    free_tokens(tokens);
    free_cmd(ast);
}

//using raw strings from gnu expansion for tests, so ignore error squigglies
int main() {

    test(R"(echo hello world)");

    test(R"(ls -a | grep -E ^s)");

    test(R"(echo a && echo b)");

    test(R"(echo c || echo d)");

    test(R"(ech e; echo f)");

    test(R"(echo hi >out.txt)");

    test(R"(echo hi | cat > out.txt)");

    test(R"((ls -l | wc -w) && echo hello)");

    test(R"((echo hello &) | (echo hi &))");

    test(R"(ls -l | wc -w && echo hello &)");

    test(R"(ls -a | grep -E ^\d\d\d > out.txt)");

    test(R"(echo hello || (echo there | grep -E ^the))");

    test(R"(;)");

    test(R"(()");

    test(R"())");

    test(R"((ls -s)");

    test(R"(ls -a | )");

    test(R"(cd .. && )");
    
    return 0;
}