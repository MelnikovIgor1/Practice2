#include <iostream>
#include <utility>
#include <vector>
#include <set>
#include <queue>
#include <map>
#include <optional>
#include <algorithm>

const auto BASIC_ALPHABET = "abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

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
    Grammar(std::vector<Rule> rules, char start, std::string alphabet = BASIC_ALPHABET):
            rules(std::move(rules)),
            start(start),
            alphabet(std::move(alphabet)){

    }
    std::vector<Rule> rules;
    char start;
    std::string alphabet;

    std::map<char, std::set<char>> first_;
    bool prepared_ = false;

    void prepare() {
        if (prepared_) {
            return;
        }

        for (auto& rule: rules) {
            if (rule.to.empty()) {
                first_[rule.from].insert('$');
                continue;
            }
            if (!is_terminal(rule.to[0])) {
                first_[rule.from].insert(rule.to[0]);
            }
        }

        bool updated = false;

        while (true) {
            for (auto& rule: rules) {
                if (rule.to.empty()) {
                    continue;
                }

                int cur = 0;
                while (cur < rule.to.size()) {
                    if (!is_terminal(rule.to[cur]) && first_[rule.from].count(rule.to[cur]) == 0) {
                        updated = true;
                        first_[rule.from].insert(rule.to[cur]);
                    } else
                    for(auto new_letter: first_[rule.to[cur]]) {
                        if (new_letter == '$') {
                            continue;
                        }
                        if (first_[rule.from].count(new_letter) == 0) {
                            updated = true;
                            first_[rule.from].insert(new_letter);
                        }
                    }

                    if (first_[rule.to[cur]].count('$') > 0) {
                        ++cur;
                    } else {
                        break;
                    }
                }
                if (cur == rule.to.size() && first_[rule.from].count('$') == 0) {
                    updated = true;
                    first_[rule.from].insert('$');
                }
            }
            if (!updated) {
                break;
            }
            updated = false;
        }
        prepared_ = true;
    }

    std::set<char> First(const std::string& str) {
        if (str.empty()) {
            return {'$'};
        }
        prepare();
        std::set<char> answer;
        int cur = 0;
        while (cur < str.size()) {
            if (!is_terminal(str[cur])) {
                answer.insert(str[cur]);
                break;
            }

            auto& new_ = first_[str[cur]];
            for (auto letter: new_) {
                if (letter != '$') {
                    answer.insert(letter);
                }
            }


            if (new_.count('$') > 0) {
                ++cur;
            } else {
                break;
            }
        }

        return answer;
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
        if (cur < (int)to.size()) {
            return {true, to[cur]};
        }
        return {false, -1};
    }

    std::vector<SuperRule> get_followings(Grammar& grammar) {
        auto [flag, T] = get_next();
        if (!flag || !is_terminal(T)) {
            return {};
        }

        std::vector<SuperRule> answer;
        for (auto& rule: grammar.rules) {
            if (rule.from == T) {
                if (cur + 1 < (int)to.size()) {
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
    Grammar& grammar;

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
            if (rule.cur < (int)rule.to.size() && rule.to[rule.cur] == step_letter) {
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
    explicit Machine(Grammar grammar_):
            grammar(std::move(grammar_)){
        std::string str;
        str += grammar.start;
        grammar.rules.push_back(Rule{'T', str});
        grammar.start = 'T';
    }

    std::vector<Node> nodes;
    std::vector<std::map<char, int>> edges;

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
};

struct WordChecker {
public:
    enum RESULT {
        ACCEPTED,
        DECLINED,
        FAILED,
        CONTINUE
    };

private:
    const Machine& machine;

    int cur = {};
    std::string path_word = {};
    std::vector<int> path = {};
    char search_letter = {};
    std::string word_to_check = {};

    std::vector<SuperRule> find_rules(char letter) {
        std::vector<SuperRule> answer;
        for(const auto& rule: machine.nodes[cur].rules) {
            if (rule.cur == (int)rule.to.size() && rule.letter == letter) {
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

    static bool is_final(const SuperRule& rule) {
        return rule == SuperRule{{'T', "S"}, 1, '$'};
    }

    bool has_final_rule() {
        if (std::any_of(machine.nodes[cur].rules.begin(),
                        machine.nodes[cur].rules.end(), is_final)) {
            return true;
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

public:
    explicit WordChecker(const Machine& machine):
            machine(machine){

    }

    RESULT check_word(const std::string& word) {
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

struct Algo {
private:
    std::optional<Machine> machine;
public:
    void fit(const Grammar& grammar) {
        machine = Machine(grammar);
        machine->prepare_nodes();
    }

    bool predict(const std::string& word) {
        if (!machine) {
            throw std::bad_exception();
        }

        WordChecker checker(*machine);

        auto answer = checker.check_word(word);

        switch (answer) {
            case WordChecker::ACCEPTED:
                return true;
            case WordChecker::FAILED:
                throw std::bad_exception();
            case WordChecker::DECLINED:
                return false;
            case WordChecker::CONTINUE:
                throw std::bad_exception();
        }
        return false;
    }

    Algo():
            machine() {
    }
};
