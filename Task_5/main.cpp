#include <clocale>
#include <iostream>
#include <fstream>

#include "../Parser/Parser.h"
#include "Term.h"

std::unordered_map<const Word *, Term *> termDictionary;

int main() {
    std::setlocale(LC_ALL, "ru_RU.UTF-8");

    auto dictionary = Parser::readDictionary(DICTIONARY_PATH, 6000000);
    const auto iterator = std::filesystem::directory_iterator(NEWS_PATH);

    for (const auto &entry: iterator) {
        std::wifstream wif(entry.path());
        const auto tokens = Parser::readTokens(wif, true);

        for (int i = 0; i < tokens.size(); i++) {
            const auto token = tokens[i];
            const auto lemmas = Parser::normalizeToken(token, dictionary);

            for (const auto lemma: lemmas) {
                Term *term;
                if (termDictionary.find(lemma) != termDictionary.end()) {
                    term = termDictionary.at(lemma);
                } else {
                    term = new Term();
                    termDictionary.emplace(lemma, term);
                }

                const auto path = entry.path().string();
                if (term->entries.find(path) == term->entries.end()) {
                    term->entries.emplace(path, Term::TextEntry());
                }

                const auto result = term->entries.at(path).positions.insert(i);
                if (result.second) term->frequency++;
            }
        }
    }

    return 0;
}