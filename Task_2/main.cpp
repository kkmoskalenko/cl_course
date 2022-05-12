#include <clocale>
#include <filesystem>
#include <codecvt>
#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <map>
#include <list>
#include <iterator>

#include "../Parser/Parser.h"

using Context = std::vector<std::string>;

std::wstring_convert<std::codecvt_utf8<wchar_t>> converter; // NOLINT(cert-err58-cpp)

std::set<Context> getAllContexts(const std::list<Parser::WordSet> &lemmasList) {
    std::set<Context> allContexts;
    allContexts.emplace();

    for (const auto &lemmas: lemmasList) {
        std::set<Context> contexts;
        for (const auto &context: allContexts) {
            for (const auto &lemma: lemmas) {
                Context contextCopy(context);
                contextCopy.push_back(lemma->text);
                contexts.insert(contextCopy);
            }
        }

        allContexts = contexts;
    }

    return allContexts;
}

void printStatistics(const std::map<Context, int> &contextMap, const std::string &prefix, int minOccurrences = 2) {
    std::vector<std::pair<int, Context>> statistics;
    for (const auto &pair: contextMap) {
        const auto context = pair.first;
        const auto count = pair.second;

        statistics.emplace_back(count, context);
    }

    std::sort(statistics.begin(), statistics.end(), std::greater());
    for (const auto &item: statistics) {
        if (item.first < minOccurrences) break;

        std::cout << prefix << ',';
        for (const auto &lemma: item.second) {
            std::cout << " " << lemma;
        }

        std::cout << ", " << item.first << std::endl;
    }
}

int main() {
    std::setlocale(LC_ALL, "ru_RU.UTF-8");

    std::string term;
    int windowSize;

    std::cout << "Enter the search term: ";
    std::getline(std::cin, term);
    std::cout << "Enter the window size: ";
    std::cin >> windowSize;

    auto dictionary = Parser::readDictionary(DICTIONARY_PATH, 6000000);
    const auto iterator = std::filesystem::directory_iterator(NEWS_PATH);

    std::wstringstream wss(converter.from_bytes(term));
    const auto termTokens = Parser::readTokens(wss, true);

    std::vector<Parser::WordSet> normalizedTerm;
    normalizedTerm.reserve(termTokens.size());
    for (const auto &token: termTokens) {
        normalizedTerm.push_back(Parser::normalizeToken(token, dictionary));
    }

    auto leftContexts = std::map<Context, int>();
    auto rightContexts = std::map<Context, int>();

    for (const auto &entry: iterator) {
        std::wifstream wif(entry.path());
        const auto tokens = Parser::readTokens(wif, true);

        const auto maxListSize = windowSize + termTokens.size();
        std::list<Parser::WordSet> lemmasList;

        for (auto tokenIt = tokens.begin(); tokenIt != tokens.end(); tokenIt++) {
            lemmasList.push_back(Parser::normalizeToken(*tokenIt, dictionary));
            if (lemmasList.size() > maxListSize) lemmasList.pop_front();

            bool contextMatches = lemmasList.size() > termTokens.size();
            if (!contextMatches) continue;

            auto termIt = normalizedTerm.begin();
            auto listIt = lemmasList.begin();

            for (; termIt != normalizedTerm.end() && listIt != lemmasList.end(); termIt++, listIt++) {
                std::vector<const Word *> intersection;
                std::set_intersection(termIt->begin(), termIt->end(),
                                      listIt->begin(), listIt->end(),
                                      std::back_inserter(intersection));

                if (intersection.empty()) {
                    contextMatches = false;
                    break;
                }
            }

            if (contextMatches) {
                // Right Context
                auto allContexts = getAllContexts(lemmasList);
                for (const auto &context: allContexts) {
                    rightContexts[context]++;
                }

                // Left Context
                std::list<Parser::WordSet> leftLemmas;

                const auto leftContextEnd = tokenIt - (int) (lemmasList.size() - termTokens.size());
                const auto leftContextBegin = leftContextEnd - (int) maxListSize;

                for (auto it = leftContextEnd; it != leftContextBegin && it >= tokens.begin(); it--) {
                    leftLemmas.push_front(Parser::normalizeToken(*it, dictionary));
                }

                if (leftLemmas.size() > 1) {
                    allContexts = getAllContexts(leftLemmas);
                    for (const auto &context: allContexts) {
                        leftContexts[context]++;
                    }
                }
            }
        }
    }

    std::cout << std::endl;
    printStatistics(leftContexts, "L");
    printStatistics(rightContexts, "R");

    return 0;
}