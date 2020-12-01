#include <iostream>
#include <utility>
#include <vector>
#include <set>
#include <queue>
#include <map>


bool is_terminal(char letter) {
    return letter >= 'A' && letter <= 'Z';
}

struct Rule {
    char from;
    std::string to;

    bool operator < (const Rule& rule) const {
        if (from < rule.from || ((from == rule.from) && to < rule.to)) {
            return true;
        }

        return false;
    }
};

struct Grammar {
    std::vector<Rule> rules;
    char start;
    std::string alphabet = "abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ()";

    std::set<char> First(const std::string& str) const {
        if (str.empty()) {
            return {'$'};
        }
        if (str.size() == 1) {
            return {str[0]};
        }
        auto answ1 = First(str.substr(0, 1));
        auto answ2 = First(str.substr(1));

        if (answ1.count('$') == 0) {
            return answ1;
        } else {
            answ1.insert(answ2.begin(), answ2.end());

            return answ1;
        }
    }
};

struct SuperRule : Rule {
    SuperRule(Rule r, int cur, char letter):
        Rule(std::move(r)),
        cur(cur),
        letter(letter){

    }
    int cur{};
    char letter{};

    std::pair<bool, char> get_next() {
        if (cur < to.size()) {
            return {true, to[cur]};
        }
        return {false, -1};
    }

    std::vector<SuperRule> get_followings(const Grammar& grammar) {
        auto [flag, T] = get_next();
        if (!flag || !is_terminal(T)) {
            return {};
        }

        std::vector<SuperRule> answer;
        for (auto& rule: grammar.rules) {
            if (rule.from == T) {
                if (cur + 1 < to.size()) {
                    for (auto symbol: grammar.First(to.substr(cur + 1))) {
                        answer.emplace_back(rule, 0, symbol);
                    }

                } else {
                    answer.emplace_back(rule, 0, letter);
                }
            }
        }

        return answer;
    }

    bool operator < (const SuperRule& rule) const {
        const Rule& this_ = *this;
        const Rule& rule_ = rule;
        if (this_ < rule_) {
            return true;
        }
        if (rule_ < this_) {
            return false;
        }

        if (cur < rule.cur || (cur == rule.cur && letter < rule.letter)) {
            return true;
        }
        return false;
    }

    bool operator == (const SuperRule& rule) const {
        return !(*this < rule) && !(rule < *this);
    }
};

struct Node {
    std::set<SuperRule> rules;
    const Grammar& grammar;

    void Update() {
        std::queue<SuperRule> queue;
        for (auto& rule: rules) {
            queue.push(rule);
        }

        while (!queue.empty()) {
            auto s_rule = queue.front();
            queue.pop();

            auto vec = s_rule.get_followings(grammar);

            for (const auto& new_rule: vec) {
                if (rules.count(new_rule) == 0) {
                    queue.push(new_rule);
                    rules.insert(new_rule);
                }
            }
        }
    }
    Node make_step(char step_letter) {
        Node answer{{}, grammar};
        for (const auto& rule: rules) {
            if (rule.cur < rule.to.size() && rule.to[rule.cur] == step_letter) {
                auto new_rule = rule;
                ++new_rule.cur;
                answer.rules.insert(new_rule);
            }
        }
        answer.Update();
        return answer;
    }

    bool operator < (const Node& node) const {
        if (rules < node.rules) {
            return true;
        }

        return false;
    }

    bool operator == (const Node& node) const {
        return !(*this < node) && !(node < *this);
    }
};


struct Machine {
    Grammar grammar;
    Machine(Grammar grammar_):
        grammar(std::move(grammar_)){
        std::string str;
        str += grammar.start;
        grammar.rules.push_back(Rule{'T', str});
        grammar.start = 'T';
    }

    std::vector<Node> nodes;
    std::vector<std::map<char, int>> edges;
    int cur = 0;
    std::string path_word = "";
    std::vector<int> path;
    char search_letter;
    std::string word_to_check;

    enum RESULT {
        ACCEPTED,
        DECLINED,
        FAILED,
        CONTINUE
    };

    auto prepare_nodes() {
        std::set<Node> nodes_set;
        Node first_node{{{{'T', "S"}, 0, '$'}}, grammar};
        first_node.Update();
        std::queue<std::pair<Node, int>> queue;
        nodes.push_back(first_node);
        edges.emplace_back();
        queue.push({first_node, 0});
        nodes_set.insert(first_node);

        while (!queue.empty()) {
            auto [cur_node, index] = queue.front();
            queue.pop();

            for (char terminal_letter: grammar.alphabet) {
                auto new_node = cur_node.make_step(terminal_letter);
                if (new_node.rules.empty()) {
                    continue;
                }

                new_node.Update();
                if (nodes_set.count(new_node) == 0) {
                    nodes_set.insert(new_node);
                    nodes.push_back(new_node);
                    edges.emplace_back();
                    edges[index].insert({terminal_letter, nodes.size() - 1});
                    queue.push({new_node, nodes.size() - 1});
                } else {
                    auto iter = std::find(nodes.begin(), nodes.end(), new_node);
                    edges[index].insert({terminal_letter, iter - nodes.begin()});
                }
            }
        }

        return nodes_set;
    }

    std::vector<SuperRule> find_rules(char letter) {
        std::vector<SuperRule> answer;
        for(const auto& rule: nodes[cur].rules) {
            if (rule.cur == rule.to.size() && rule.letter == letter) {
                answer.push_back(rule);
            }
        }

        return answer;
    }

    int go_back(const std::string& str) {
        for (int i = (int)str.size() - 1; i >= 0; --i) {
            if (path.empty()) {
                return DECLINED;
            }
            cur = path.back();
            path.pop_back();
            if (str[i] != path_word.back()) {
                return DECLINED;
            }
            path_word.pop_back();
        }

        return CONTINUE;
    }

    bool has_final_rule() {
        for (const auto& rule: nodes[cur].rules) {
            if (rule == SuperRule{{'T', "S"}, 1, '$'}) {
                return true;
            }
        }
        return false;
    }

    RESULT step() {
        if (edges[cur].count(search_letter) > 0) {
            path_word += search_letter;
            path.push_back(cur);
            cur = edges[cur][search_letter];
            search_letter = word_to_check.back();
            if (word_to_check.empty()) {
                return DECLINED;
            }
            word_to_check.pop_back();

            return CONTINUE;
        } else {
            if (word_to_check.empty() && search_letter == '$') {
                if (has_final_rule()) {
                    return ACCEPTED;
                }
            }
            auto rules = find_rules(search_letter);
            if (rules.size() >= 2) {
                return FAILED;
            }
            if (rules.empty()) {
                return DECLINED;
            }
            auto& rule = rules[0];
            word_to_check += search_letter;
            if (go_back(rule.to) == DECLINED) {
                return DECLINED;
            }
            search_letter = rule.from;
            return CONTINUE;
        }
    }

    RESULT check_word(const std::string& word) {
        if (word.empty()) {

        }
        word_to_check = word + "$";
        std::reverse(word_to_check.begin(), word_to_check.end());
        cur = 0;
        path_word = "";
        path = {};
        search_letter = word_to_check.back();
        word_to_check.pop_back();

        while (true) {
            auto state = step();
            switch (state) {
                case DECLINED:
                    return DECLINED;
                case CONTINUE:
                    continue;
                case FAILED:
                    return FAILED;
                case ACCEPTED:
                    return ACCEPTED;
            }
        }
    }
};

struct WordChecker {
private:
    const Machine& machine;

    int cur = {};
    std::string path_word = {};
    std::vector<int> path = {};
    char search_letter = {};
    std::string word_to_check = {};

    enum RESULT {
        ACCEPTED,
        DECLINED,
        FAILED,
        CONTINUE
    };

    std::vector<SuperRule> find_rules(char letter) {
        std::vector<SuperRule> answer;
        for(const auto& rule: machine.nodes[cur].rules) {
            if (rule.cur == rule.to.size() && rule.letter == letter) {
                answer.push_back(rule);
            }
        }

        return answer;
    }

    int go_back(const std::string& str) {
        for (int i = (int)str.size() - 1; i >= 0; --i) {
            if (path.empty()) {
                return DECLINED;
            }
            cur = path.back();
            path.pop_back();
            if (str[i] != path_word.back()) {
                return DECLINED;
            }
            path_word.pop_back();
        }

        return CONTINUE;
    }

    bool has_final_rule() {
        for (const auto& rule: machine.nodes[cur].rules) {
            if (rule == SuperRule{{'T', "S"}, 1, '$'}) {
                return true;
            }
        }
        return false;
    }

    RESULT step() {
        if (machine.edges[cur].count(search_letter) > 0) {
            path_word += search_letter;
            path.push_back(cur);
            cur = machine.edges[cur].at(search_letter);
            search_letter = word_to_check.back();
            if (word_to_check.empty()) {
                return DECLINED;
            }
            word_to_check.pop_back();

            return CONTINUE;
        } else {
            if (word_to_check.empty() && search_letter == '$') {
                if (has_final_rule()) {
                    return ACCEPTED;
                }
            }
            auto rules = find_rules(search_letter);
            if (rules.size() >= 2) {
                return FAILED;
            }
            if (rules.empty()) {
                return DECLINED;
            }
            auto& rule = rules[0];
            word_to_check += search_letter;
            if (go_back(rule.to) == DECLINED) {
                return DECLINED;
            }
            search_letter = rule.from;
            return CONTINUE;
        }
    }

    RESULT check_word(const std::string& word) {
        if (word.empty()) {

        }
        word_to_check = word + "$";
        std::reverse(word_to_check.begin(), word_to_check.end());
        cur = 0;
        path_word = "";
        path = {};
        search_letter = word_to_check.back();
        word_to_check.pop_back();

        while (true) {
            auto state = step();
            switch (state) {
                case DECLINED:
                    return DECLINED;
                case CONTINUE:
                    continue;
                case FAILED:
                    return FAILED;
                case ACCEPTED:
                    return ACCEPTED;
            }
        }
    }
public:
    WordChecker(const Machine& machine):
        machine(machine){

    }
};

struct Algo {
    void fit(const Grammar& grammar) {

    }
};

int main() {
    Grammar grammar{{{'S', "S(S)"}, {'S', ""}}, 'S'};
    Machine machine{grammar};
    auto x = machine.prepare_nodes();
    auto z = machine.check_word("(((()))(()()))");
    switch (z) {
        case Machine::ACCEPTED:
            std::cout << "ACCEPTED\n";
            break;
        case Machine::FAILED:
            std::cout << "FAILED\n";
            break;
        case Machine::DECLINED:
            std::cout << "DECLINED\n";
            break;
        case Machine::CONTINUE:
            throw std::bad_exception();
    }

    return 0;
}
