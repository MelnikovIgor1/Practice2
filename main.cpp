#include "algo.cpp"

int main() {
    Grammar grammar{{{'S', "S(S)"}, {'S', ""}}, 'S'};
    Machine machine{grammar};
    auto x = machine.prepare_nodes();
    WordChecker checker(machine);
    auto z = checker.check_word("())");
    switch (z) {
        case WordChecker::ACCEPTED:
            std::cout << "ACCEPTED\n";
            break;
        case WordChecker::FAILED:
            std::cout << "FAILED\n";
            break;
        case WordChecker::DECLINED:
            std::cout << "DECLINED\n";
            break;
        case WordChecker::CONTINUE:
            throw std::bad_exception();
    }

    return 0;
}
