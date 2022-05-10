#include "GibberishDetector.h"

#include <assert.h>

struct TestCase {
    const char *str;
    bool expected_result;
};

TestCase testCases[] = {
    { "Hello World", false },
    { "Test", false },
    { "Hello", false },
    { "yes", false },
    { "FunctionName", false },
    { "aSasdffffwHGsd", true },
    { "hgfRTFs", true },
    { "bcv3Hgf", true },
};

#define NUM_CASES (sizeof(testCases) / sizeof(testCases[0]))

const char accepted_chars[] = "abcdefghijklmnopqrstuvwxyz ";

int main(int argc, char *argv[])
{
    GibberishDetector detector;
    detector.init(accepted_chars);
    for (int i = 0; i < NUM_CASES; ++i) {
        bool is_gib = detector.is_gibberish(testCases[i].str);
        printf("Testing [%s]: %s\n", testCases[i].str, is_gib ? "GIBBERISH" : "English");
        assert(is_gib == testCases[i].expected_result);
    }

    return 0;
}