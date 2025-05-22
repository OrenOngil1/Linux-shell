#include "../unity/src/unity.h"
#include "../src/parser.h"
#include <string.h>

void setUp(void) {}

void tearDown(void) {}

void test_basic()
{
    char *line = "echo hello";
    ParsedCmdLine *pLine = parseCmdLine(line);
    TEST_ASSERT_NOT_NULL(pLine);
    TEST_ASSERT_NOT_NULL(pLine->args);
    TEST_ASSERT_EQUAL(2, pLine->argCount);
    TEST_ASSERT_EQUAL_CHAR_ARRAY("echo", pLine->args[0] , strlen("echo"));
    TEST_ASSERT_EQUAL_CHAR_ARRAY("hello", pLine->args[1], strlen("hello"));
    TEST_ASSERT_NULL(pLine->args[2]);
    TEST_ASSERT_NULL(pLine->inputRedirect);
    TEST_ASSERT_NULL(pLine->outputRedirect);
    TEST_ASSERT_EQUAL_CHAR_ARRAY(line, pLine->str, strlen(line));
    TEST_ASSERT_NULL(pLine->next);
    freeCmdLine(pLine);
}

void test_sanity_check()
{
    char *line = NULL;
    ParsedCmdLine *pLine = parseCmdLine(line);
    TEST_ASSERT_NULL(pLine);
}

void test_sanity_check2()
{
    char *line = "";
    ParsedCmdLine *pLine = parseCmdLine(line);
    TEST_ASSERT_NULL(pLine);

}

void test_input_redirect()
{
    char *line = "cat < in.txt";
    ParsedCmdLine *pLine = parseCmdLine(line);
    TEST_ASSERT_NOT_NULL(pLine);
    TEST_ASSERT_NOT_NULL(pLine->args);
    TEST_ASSERT_EQUAL(3, pLine->argCount);
    TEST_ASSERT_EQUAL_CHAR_ARRAY("cat", pLine->args[0] , strlen("cat"));
    TEST_ASSERT_EQUAL_CHAR_ARRAY("<", pLine->args[1] , strlen("<"));
    TEST_ASSERT_EQUAL_CHAR_ARRAY("in.txt", pLine->args[2] , strlen("in.txt"));
    TEST_ASSERT_NULL(pLine->args[3]);
    TEST_ASSERT_NOT_NULL(pLine->inputRedirect);
    TEST_ASSERT_EQUAL_CHAR_ARRAY("in.txt", pLine->inputRedirect, strlen("in.txt"));
    TEST_ASSERT_NULL(pLine->outputRedirect);
    TEST_ASSERT_EQUAL_CHAR_ARRAY(line, pLine->str, strlen(line));
    TEST_ASSERT_NULL(pLine->next);
    freeCmdLine(pLine);
}

void test_output_redirect()
{
    char *line = "echo hello > out.txt";
    ParsedCmdLine *pLine = parseCmdLine(line);
    TEST_ASSERT_NOT_NULL(pLine);
    TEST_ASSERT_NOT_NULL(pLine->args);
    TEST_ASSERT_EQUAL(4, pLine->argCount);
    TEST_ASSERT_EQUAL_CHAR_ARRAY("echo", pLine->args[0] , strlen("echo"));
    TEST_ASSERT_EQUAL_CHAR_ARRAY("hello", pLine->args[1] , strlen("hello"));
    TEST_ASSERT_EQUAL_CHAR_ARRAY(">", pLine->args[2] , strlen(">"));
    TEST_ASSERT_EQUAL_CHAR_ARRAY("out.txt", pLine->args[3] , strlen("out.txt"));
    TEST_ASSERT_NULL(pLine->args[4]);
    TEST_ASSERT_NULL(pLine->inputRedirect);
    TEST_ASSERT_NOT_NULL(pLine->outputRedirect);
    TEST_ASSERT_EQUAL_CHAR_ARRAY("out.txt", pLine->outputRedirect, strlen("out.txt"));;
    TEST_ASSERT_EQUAL_CHAR_ARRAY(line, pLine->str, strlen(line));
    TEST_ASSERT_NULL(pLine->next);
    freeCmdLine(pLine);
}

void test_io_redirect()
{
    char *line = "cat hello < in.txt > out.txt";
    ParsedCmdLine *pLine = parseCmdLine(line);
    TEST_ASSERT_NOT_NULL(pLine);
    TEST_ASSERT_NOT_NULL(pLine->args);
    TEST_ASSERT_EQUAL(6, pLine->argCount);
    TEST_ASSERT_EQUAL_CHAR_ARRAY("cat", pLine->args[0] , strlen("cat"));
    TEST_ASSERT_EQUAL_CHAR_ARRAY("hello", pLine->args[1] , strlen("hello"));
    TEST_ASSERT_EQUAL_CHAR_ARRAY("<", pLine->args[2] , strlen("<"));
    TEST_ASSERT_EQUAL_CHAR_ARRAY("in.txt", pLine->args[3] , strlen("in.txt"));
    TEST_ASSERT_EQUAL_CHAR_ARRAY(">", pLine->args[4] , strlen(">"));
    TEST_ASSERT_EQUAL_CHAR_ARRAY("out.txt", pLine->args[5] , strlen("out.txt"));
    TEST_ASSERT_NULL(pLine->args[6]);
    TEST_ASSERT_NOT_NULL(pLine->inputRedirect);
    TEST_ASSERT_EQUAL_CHAR_ARRAY("in.txt", pLine->inputRedirect, strlen("in.txt"));
    TEST_ASSERT_NOT_NULL(pLine->outputRedirect);
    TEST_ASSERT_EQUAL_CHAR_ARRAY("out.txt", pLine->outputRedirect, strlen("out.txt"));;
    TEST_ASSERT_EQUAL_CHAR_ARRAY(line, pLine->str, strlen(line));
    TEST_ASSERT_NULL(pLine->next);
    freeCmdLine(pLine);
}

void test_non_blocking()
{
    char *line = "echo hello &";
    ParsedCmdLine *pLine = parseCmdLine(line);
    TEST_ASSERT_NOT_NULL(pLine);
    TEST_ASSERT_NOT_NULL(pLine->args);
    TEST_ASSERT_EQUAL(3, pLine->argCount);
    TEST_ASSERT_EQUAL_CHAR_ARRAY("echo", pLine->args[0] , strlen("echo"));
    TEST_ASSERT_EQUAL_CHAR_ARRAY("hello", pLine->args[1], strlen("hello"));
    TEST_ASSERT_EQUAL_CHAR_ARRAY("&", pLine->args[2], strlen("&"));
    TEST_ASSERT_NULL(pLine->args[3]);
    TEST_ASSERT_NULL(pLine->inputRedirect);
    TEST_ASSERT_NULL(pLine->outputRedirect);
    TEST_ASSERT_EQUAL_CHAR_ARRAY(line, pLine->str, strlen(line));
    TEST_ASSERT_NULL(pLine->next);
    freeCmdLine(pLine);
}

void test_good_pipe()
{
    char *line = "echo hello | cat ";
    ParsedCmdLine *pLine = parseCmdLine(line);
    TEST_ASSERT_NOT_NULL(pLine);
    TEST_ASSERT_NOT_NULL(pLine->args);
    TEST_ASSERT_EQUAL(2, pLine->argCount);
    TEST_ASSERT_EQUAL_CHAR_ARRAY("echo", pLine->args[0] , strlen("echo"));
    TEST_ASSERT_EQUAL_CHAR_ARRAY("hello", pLine->args[1] , strlen("hello"));
    TEST_ASSERT_NULL(pLine->args[2]);
    TEST_ASSERT_NULL(pLine->inputRedirect);
    TEST_ASSERT_NULL(pLine->outputRedirect);
    TEST_ASSERT_NOT_NULL(pLine->next);

    TEST_ASSERT_NOT_NULL(pLine->next);
    TEST_ASSERT_NOT_NULL(pLine->next->args);
    TEST_ASSERT_EQUAL(1, pLine->next->argCount);
    TEST_ASSERT_EQUAL_CHAR_ARRAY("cat", pLine->next->args[0] , strlen("cat"));
    TEST_ASSERT_NULL(pLine->next->args[1]);
    TEST_ASSERT_NULL(pLine->next->inputRedirect);
    TEST_ASSERT_NULL(pLine->next->outputRedirect);
    TEST_ASSERT_NULL(pLine->next->next);
    freeCmdLine(pLine);
}

void test_bad_pipe()
{
    char *line = "echo hello |";
    ParsedCmdLine *pLine = parseCmdLine(line);
    TEST_ASSERT_NULL(pLine);
}

void test_all()
{
    char *line = "echo<in.txt|cat>out.txt";
    ParsedCmdLine *pLine = parseCmdLine(line);
    TEST_ASSERT_NOT_NULL(pLine);
    TEST_ASSERT_NOT_NULL(pLine->args);
    TEST_ASSERT_EQUAL(3, pLine->argCount);
    TEST_ASSERT_EQUAL_CHAR_ARRAY("echo", pLine->args[0] , strlen("echo"));
    TEST_ASSERT_EQUAL_CHAR_ARRAY("<", pLine->args[1], strlen("<"));
    TEST_ASSERT_EQUAL_CHAR_ARRAY("in.txt", pLine->args[2] , strlen("in.txt"));
    TEST_ASSERT_NULL(pLine->args[3]);
    TEST_ASSERT_NOT_NULL(pLine->inputRedirect);
    TEST_ASSERT_EQUAL_CHAR_ARRAY("in.txt", pLine->inputRedirect , strlen("in.txt"));
    TEST_ASSERT_NULL(pLine->outputRedirect);
    TEST_ASSERT_NOT_NULL(pLine->next);

    TEST_ASSERT_NOT_NULL(pLine->next);
    TEST_ASSERT_NOT_NULL(pLine->next->args);
    TEST_ASSERT_EQUAL(3, pLine->next->argCount);
    TEST_ASSERT_EQUAL_CHAR_ARRAY("cat", pLine->next->args[0] , strlen("cat"));
    TEST_ASSERT_EQUAL_CHAR_ARRAY(">", pLine->next->args[1], strlen(">"));
    TEST_ASSERT_EQUAL_CHAR_ARRAY("out.txt", pLine->next->args[2], strlen("out.txt"));
    TEST_ASSERT_NULL(pLine->next->args[3]);
    TEST_ASSERT_NULL(pLine->next->inputRedirect);
    TEST_ASSERT_NOT_NULL(pLine->next->outputRedirect);
    TEST_ASSERT_EQUAL_CHAR_ARRAY("out.txt", pLine->next->outputRedirect, strlen("out.txt"));
    TEST_ASSERT_NULL(pLine->next->next);
    freeCmdLine(pLine);
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();
    RUN_TEST(test_basic);
    RUN_TEST(test_sanity_check);
    RUN_TEST(test_input_redirect);
    RUN_TEST(test_output_redirect);
    RUN_TEST(test_io_redirect);
    RUN_TEST(test_non_blocking);
    RUN_TEST(test_good_pipe);
    RUN_TEST(test_bad_pipe);
    RUN_TEST(test_all);
    UNITY_END();
    return 0;
}