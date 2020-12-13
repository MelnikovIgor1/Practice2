#ifndef MACHINE_TESTS
#define MACHINE_TESTS

#include <gtest/gtest.h>
#include "algo.cpp"


class TestMachine: public ::testing::Test {
protected:
    static Grammar scope_grammar() {
        Grammar grammar({{'S', "SaSb"}, {'S', ""}}, 0, "abST");
        return grammar;
    }
    static Grammar other_grammar() {
        Grammar grammar({{'A', "AaAb"},{'S', "A"}, {'S', ""}, {'A', "S"}}, 0, "abST");
        return grammar;
    }
    static void testGrammar() {
        Grammar grammar = scope_grammar();
        grammar.First("a");
        EXPECT_EQ(grammar.First("a"), std::set<char> {'a'});
        EXPECT_EQ(grammar.First("Sb"), (std::set<char> {'a', 'b'}));
        EXPECT_EQ(grammar.First("SSb"), (std::set<char> {'a', 'b'}));
        grammar = other_grammar();
        EXPECT_EQ(grammar.First("a"), std::set<char> {'a'});
        EXPECT_EQ(grammar.First("SSb"), (std::set<char> {'a', 'b'}));
        EXPECT_EQ(grammar.First("AAA"), (std::set<char> {'a'}));
    }
    static void testSuperRule() {
        Grammar grammar = scope_grammar();
        SuperRule rule{{'T', "S"}, 0, '$'};
        auto new_rules = rule.get_followings(grammar);

        EXPECT_EQ(new_rules.size(), 2);
        auto next = rule.get_next();
        EXPECT_EQ(next.first, true);
        EXPECT_EQ(next.second, 'S');
    }
    static void testNode() {
        Grammar grammar = scope_grammar();
        Node node{{{{'T', "S"}, 0, '$'}}, grammar};
        node.Update();
        EXPECT_EQ(node.rules.size(), 5);
        EXPECT_EQ(node.make_step('S').rules.size(), 3);
    }

    static void testMachine() {
        Grammar grammar = scope_grammar();
        Machine machine(grammar);

        EXPECT_EQ(machine.prepare_nodes().size(), 8);
        EXPECT_EQ(machine.grammar.rules.size(), 3);
    }

    static void testWordChecker() {
        Grammar grammar = scope_grammar();
        Machine machine(grammar);
        WordChecker checker(machine);
        machine.prepare_nodes();

        EXPECT_EQ(checker.check_word("ab"), WordChecker::ACCEPTED);
        EXPECT_EQ(checker.check_word("aabbaaabbb"), WordChecker::ACCEPTED);
        EXPECT_EQ(checker.check_word("ba"), WordChecker::DECLINED);
    }

    static void testAlgo() {
        Grammar grammar = scope_grammar();
        Algo algo;
        algo.fit(grammar);

        EXPECT_EQ(algo.predict("aabb"), true);
        EXPECT_EQ(algo.predict("aababbaaabbb"), true);
        EXPECT_EQ(algo.predict("babda"), false);
    }
};

#endif