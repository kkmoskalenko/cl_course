#include <vector>
#include <fstream>
#include <clocale>
#include <map>
#include <set>
#include <iostream>
#include <codecvt>

#include "Word.h"
#include "Parser.h"

std::vector<std::wstring> readTokens(const std::string &filepath) {
    std::wifstream wif(filepath);
    wif.imbue(std::locale("ru_RU.UTF-8"));

    std::vector<std::wstring> tokens;
    std::wstring token;
    wchar_t ch;

    while (wif.get(ch)) {
        if (!std::iswalpha(ch)) {
            if (!token.empty()) {
                tokens.push_back(token);
                token = std::wstring();
            }
        } else {
            token += ch;
        }
    }

    return tokens;
}

int main() {
    std::setlocale(LC_ALL, "ru_RU.UTF-8");

    const auto dictionary = Parser::readDictionary("../resources/dict.opcorpora.txt", 6000000);
    const auto iterator = std::filesystem::directory_iterator("../resources/news");

    auto lemmaCounter = std::map<const Word *, int>();
    auto unrecognizedTokens = std::set<const std::wstring>();

    int tokenCount = 0;
    int ambiguityCount = 0;

    for (const auto &entry: iterator) {
        const auto tokens = readTokens(entry.path());
        for (const auto &token: tokens) {
            std::wstring upperToken = token;
            std::transform(upperToken.begin(), upperToken.end(),
                           upperToken.begin(), std::towupper);

            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            const std::string dictKey = converter.to_bytes(upperToken);

            const auto dict_itr = dictionary.equal_range(dictKey);
            auto lemmas = std::set<const Word *>();

            if (dictionary.count(dictKey) == 0) {
                unrecognizedTokens.insert(upperToken);
            }

            for (auto itr = dict_itr.first; itr != dict_itr.second; itr++) {
                const auto word = itr->second;
                const auto key = word->lemma ?: word;

                if (lemmas.count(key) == 0) {
                    lemmas.insert(key);
                    lemmaCounter[key]++;
                }
            }

            tokenCount++;
            if (lemmas.size() > 1) {
                ambiguityCount++;
            }
        }
    }

    std::vector<std::pair<int, const Word *>> statistics;
    for (const auto &pair: lemmaCounter) {
        const auto word = pair.first;
        const auto count = pair.second;

        statistics.emplace_back(count, word);
    }

    std::sort(statistics.begin(), statistics.end(), std::greater());

    std::wcout << "Total tokens: " << tokenCount << std::endl;
    std::wcout << "Ambiguity count: " << ambiguityCount << std::endl;
    std::wcout << "Unrecognized count: " << unrecognizedTokens.size() << std::endl << std::endl;

    for (const auto &item: statistics) {
        const auto lemma = item.second->text;
        const auto pos = item.second->getPartOfSpeech();
        const auto count = item.first;

        std::cout << lemma << ", " << pos << ", " << count << std::endl;
    }

    return 0;
}
