#include <clocale>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <codecvt>

#include "../Parser/Parser.h"
#include "Term.h"
#include "Thesaurus.h"
#include "Requests.h"

const auto &request = requests[8];

namespace BM25 {
    constexpr double k1 = 2.0;
    constexpr double b = 0.75;
}

Parser::Dictionary dictionary;
std::unordered_map<std::string, Term *> termDictionary;

std::unordered_map<std::string, int> documentSize;
unsigned long averageDocumentSize = 0;

using ScoreAndEntries = std::pair<double, std::set<int>>;

std::vector<std::string> normalizeRequest(const std::wstring &requestText) {
    std::vector<std::string> result;

    std::wstringstream ss;
    ss.str(requestText);

    const auto tokens = Parser::readTokens(ss, true);
    for (const auto &token: tokens) {
        const auto lemmas = Parser::normalizeToken(token, dictionary);
        const auto normalized = (*lemmas.begin())->text;

        result.push_back(normalized);
    }

    return result;
}

std::unordered_map<std::string, ScoreAndEntries> getDocumentsScore(const std::string &word) {
    std::unordered_map<std::string, ScoreAndEntries> documentScore;
    if (termDictionary.find(word) == termDictionary.end()) {
        return documentScore;
    }

    const auto entries = termDictionary.at(word)->entries;
    const auto totalDocs = (double) documentSize.size();
    const auto docsWithWord = (double) entries.size();
    const double idf = log((totalDocs - docsWithWord + 0.5) / (docsWithWord + 0.5));

    for (const auto &entry: entries) {
        const auto docSize = documentSize.at(entry.first);
        const auto f = (double) entry.second.positions.size() / docSize;

        if (documentScore.find(entry.first) == documentScore.end()) {
            documentScore.emplace(entry.first, ScoreAndEntries{});
        }

        documentScore[entry.first].first += idf * (f * (BM25::k1 + 1)) / (
                f + BM25::k1 * (1 - BM25::b + BM25::b * (
                        docSize / (double) averageDocumentSize
                ))
        );

        const auto &positions = entry.second.positions;
        documentScore[entry.first].second.insert(positions.begin(), positions.end());
    }

    return documentScore;
}

int main() {
    std::setlocale(LC_ALL, "ru_RU.UTF-8");

    dictionary = Parser::readDictionary(DICTIONARY_PATH, 6000000);
    const auto iterator = std::filesystem::directory_iterator(NEWS_PATH);

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

    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    const auto words = normalizeRequest(converter.from_bytes(request));

    std::unordered_map<std::string, ScoreAndEntries> documentScore;
    std::unordered_map<std::string, const Descriptor *> synsets;

    for (const auto &descriptor: thesaurus) {
        for (const auto &synonym: descriptor.synset) {
            synsets.emplace(synonym, &descriptor);
        }
    }

    for (const auto &word: words) {
        auto scores = getDocumentsScore(word);
        if (synsets.find(word) != synsets.end()) {
            const auto merge = [&scores](const decltype(scores) &pairs, double coeff) {
                for (const auto &pair: pairs) {
                    scores[pair.first].first = std::max(scores[pair.first].first, coeff * pair.second.first);
                    const auto &positions = pair.second.second;
                    scores[pair.first].second.insert(positions.begin(), positions.end());
                }
            };

            for (const auto &syn: synsets[word]->synset) merge(getDocumentsScore(syn), 0.5);
            for (const auto &hyper: synsets[word]->hyperonym) merge(getDocumentsScore(hyper), 0.25);
            for (const auto &hypo: synsets[word]->hyponym) merge(getDocumentsScore(hypo), 0.25);
        }

        for (const auto &score: scores) {
            documentScore[score.first].first += score.second.first;

            const auto &positions = score.second.second;
            documentScore[score.first].second.insert(positions.begin(), positions.end());
        }
    }

    for (const auto &score: documentScore) {
        std::vector<int> positions;
        std::copy(score.second.second.begin(),
                  score.second.second.end(),
                  std::back_inserter(positions));

        for (int i = 0; i < positions.size() - 1; i++) {
            if (positions[i] + 1 == positions[i + 1]) {
                documentScore[score.first].first += 1.0;
            } else if (positions[i] + 2 == positions[i + 1]) {
                documentScore[score.first].first += 0.5;
            }
        }
    }

    std::map<double, std::pair<std::string, std::set<int>>, std::greater<>> results;
    for (const auto &pair: documentScore) {
        results.emplace(pair.second.first, std::pair(pair.first, pair.second.second));
    }

    std::cout << results.size() << " results found for «" << request << "»." << std::endl << std::endl;

    for (const auto &item: results) {
        std::cout << "Score = " << item.first << "; Positions = { ";

        const auto &positions = item.second.second;
        for (const auto &pos: positions) {
            std::cout << pos << ' ';
        }

        std::cout << "}; URL = file://" << item.second.first << std::endl;
    }

    return 0;
}