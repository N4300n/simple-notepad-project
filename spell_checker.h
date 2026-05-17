#ifndef SPELL_CHECKER_H
#define SPELL_CHECKER_H

#include <string>
#include <vector>
#include <set>

class spell_checker {
public:
    explicit spell_checker(const std::string& dictionary_path);
    [[nodiscard]] bool is_correct(const std::string& word) const;
    [[nodiscard]] std::vector<std::string> suggest(const std::string& word) const;

private:
    std::set<std::string> words;
};

#endif