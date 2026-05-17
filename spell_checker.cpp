#include "spell_checker.h"
#include <fstream>
#include <cctype>
#include <algorithm>

spell_checker::spell_checker(const std::string& dictionary_path) {
    std::ifstream file(dictionary_path);
    std::string word;
    while (file >> word) {
        std::string lower_word = word;
        std::transform(lower_word.begin(), lower_word.end(), lower_word.begin(),
            [](unsigned char c){ return std::tolower(c); });
        words.insert(lower_word);
    }
}

bool spell_checker::is_correct(const std::string& word) const {
    std::string clean;
    for (char c : word) {
        if (std::isalpha(c)) clean += static_cast<char>(std::tolower(c));
    }
    if (clean.empty()) return true;
    return words.find(clean) != words.end();
}

std::vector<std::string> spell_checker::suggest(const std::string& word) const {
    std::vector<std::string> suggestions;
    std::string target;
    for (char c : word) {
        if (std::isalpha(c)) target += static_cast<char>(std::tolower(c));
    }

    std::set<std::string> candidates;
    for (size_t i = 0; i <= target.length(); ++i) {
        for (char c = 'a'; c <= 'z'; ++c) {
            std::string temp = target;
            temp.insert(i, 1, c);
            if (words.find(temp) != words.end()) candidates.insert(temp);
        }
    }
    for (size_t i = 0; i < target.length(); ++i) {
        std::string temp = target;
        temp.erase(i, 1);
        if (words.find(temp) != words.end()) candidates.insert(temp);

        for (char c = 'a'; c <= 'z'; ++c) {
            temp = target;
            temp[i] = c;
            if (words.find(temp) != words.end()) candidates.insert(temp);
        }
    }
    for (size_t i = 0; i < target.length() - 1; ++i) {
        std::string temp = target;
        std::swap(temp[i], temp[i+1]);
        if (words.find(temp) != words.end()) candidates.insert(temp);
    }

    for (const auto& c : candidates) {
        if (suggestions.size() >= 5) break;
        suggestions.push_back(c);
    }
    return suggestions;
}