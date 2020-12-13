#include <gtest/gtest.h>
#include "Tests.h"

TEST_F(TestMachine, testGrammar) {
    testGrammar();
}
TEST_F(TestMachine, testSuperRule) {
    testSuperRule();
}

TEST_F(TestMachine, testNode) {
    testNode();
}

TEST_F(TestMachine, testMachine) {
    testMachine();
}

TEST_F(TestMachine, testWordChecker) {
    testWordChecker();
}

TEST_F(TestMachine, testAlgo) {
    testAlgo();
}