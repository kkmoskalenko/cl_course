#include <clocale>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

#include "../Parser/Parser.h"
#include "Term.h"

namespace BM25 {
    constexpr double k1 = 2.0;
    constexpr double b = 0.75;
}

Parser::Dictionary dictionary;
std::unordered_map<std::string, Term *> termDictionary;

std::vector<std::string> normalizeRequest(const std::wstring &request) {
    std::vector<std::string> result;

    std::wstringstream ss;
    ss.str(request);

    const auto tokens = Parser::readTokens(ss, true);
    for (const auto &token: tokens) {
        const auto lemmas = Parser::normalizeToken(token, dictionary);
        const auto normalized = (*lemmas.begin())->text;

        result.push_back(normalized);
    }

    return result;
}

int main() {
    std::setlocale(LC_ALL, "ru_RU.UTF-8");

    dictionary = Parser::readDictionary(DICTIONARY_PATH, 6000000);
    const auto iterator = std::filesystem::directory_iterator(NEWS_PATH);

    std::unordered_map<std::string, int> documentSize;
    unsigned long averageDocumentSize = 0;

    for (const auto &entry: iterator) {
        std::wifstream wif(entry.path());
        const auto tokens = Parser::readTokens(wif, true);

        for (int i = 0; i < tokens.size(); i++) {
            const auto token = tokens[i];
            const auto lemmas = Parser::normalizeToken(token, dictionary);
            const auto lemma = *lemmas.begin();

            Term *term;
            if (termDictionary.find(lemma->text) != termDictionary.end()) {
                term = termDictionary.at(lemma->text);
            } else {
                term = new Term();
                termDictionary.emplace(lemma->text, term);
            }

            const auto path = entry.path().string();
            if (term->entries.find(path) == term->entries.end()) {
                term->entries.emplace(path, Term::TextEntry());
            }

            const auto result = term->entries.at(path).positions.insert(i);
            if (result.second) term->frequency++;
        }

        documentSize.emplace(entry.path().string(), tokens.size());
        averageDocumentSize += tokens.size();
    }

    averageDocumentSize /= documentSize.size();

    const auto request = L"";
    const auto words = normalizeRequest(request);
    std::unordered_map<std::string, double> documentScore;

    for (const auto &word: words) {
        if (termDictionary.find(word) == termDictionary.end()) continue;

        const auto entries = termDictionary.at(word)->entries;
        const auto totalDocs = (double) documentSize.size();
        const auto docsWithWord = (double) entries.size();
        const double idf = log((totalDocs - docsWithWord + 0.5) / (docsWithWord + 0.5));

        for (const auto &entry: entries) {
            const auto docSize = documentSize.at(entry.first);
            const auto f = (double) entry.second.positions.size() / docSize;

            if (documentScore.find(entry.first) == documentScore.end()) {
                documentScore.emplace(entry.first, 0.0);
            }

            documentScore.at(entry.first) += idf * (f * (BM25::k1 + 1)) / (
                    f + BM25::k1 * (1 - BM25::b + BM25::b * (
                            docSize / (double) averageDocumentSize
                    ))
            );
        }
    }

    std::map<double, std::string, std::greater<>> results;
    for (const auto &pair: documentScore) {
        results.emplace(pair.second, pair.first);
    }

    for (const auto &item: results) {
        std::cout << item.first << ' ' << item.second << std::endl;
    }

    return 0;
}