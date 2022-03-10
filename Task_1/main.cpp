#include <fstream>
#include <clocale>
#include <map>
#include <set>
#include <iostream>
#include "Word.h"

std::vector<Word *> readDictionary(const std::string &filepath) {
    std::wifstream dictFile(filepath);
    dictFile.imbue(std::locale("ru_RU.UTF-8"));

    std::vector<Word *> words;
    std::wstring line;

    Word *word;
    Word *lemma;

    while (std::getline(dictFile, line)) {
        if (line.empty() || std::isdigit(line[0])) {
            lemma = nullptr;
            continue;
        }

        word = new Word();
        word->lemma = lemma;

        std::wstring::iterator it = line.begin();
        while (!std::isspace(*it)) {
            word->text += *it;
            it++;
        }

        if (lemma == nullptr) {
            it++; // Skipping whitespace
            word->grammemes = new std::vector<std::wstring>();

            while (it != line.end()) {
                std::wstring grammeme;
                while (*it != ',' && it != line.end()) {
                    grammeme += *it;
                    it++;
                }

                if (*it == ',') it++;
                word->grammemes->push_back(grammeme);
            }
        }

        words.push_back(word);

        if (lemma == nullptr) {
            lemma = word;
        }
    }

    return words;
}

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
    std::wcout.imbue(std::locale("ru_RU.UTF-8"));

    const auto words = readDictionary("../resources/dict.opcorpora.txt");
    const auto iterator = std::filesystem::directory_iterator("../resources/news");

    auto dictionary = std::multimap<std::wstring, Word *>();
    for (const auto &word : words) {
        dictionary.insert({word->text, word});
    }

    auto lemmaCounter = std::map<const Word *, int>();
    auto unrecognizedTokens = std::set<const std::wstring>();

    int tokenCount = 0;
    int ambiguityCount = 0;

    for (const auto &entry : iterator) {
        const auto tokens = readTokens(entry.path());
        for (const auto &token : tokens) {
            std::wstring upperToken = token;
            std::transform(upperToken.begin(), upperToken.end(),
                           upperToken.begin(), toupper);

            const auto dict_itr = dictionary.equal_range(upperToken);
            auto lemmas = std::set<const Word *>();

            if (dictionary.count(upperToken) == 0) {
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
    for (const auto &pair : lemmaCounter) {
        const auto word = pair.first;
        const auto count = pair.second;

        statistics.emplace_back(count, word);
    }

    std::sort(statistics.begin(), statistics.end(), std::greater());

    std::wcout << "Total tokens: " << tokenCount << std::endl;
    std::wcout << "Ambiguity count: " << ambiguityCount << std::endl;
    std::wcout << "Unrecognized count: " << unrecognizedTokens.size() << std::endl << std::endl;

    for (const auto &item : statistics) {
        const auto lemma = item.second->text;
        const auto pos = item.second->grammemes->at(0);
        const auto count = item.first;

        std::wcout << lemma << ", " << pos << ", " << count << std::endl;
    }

    return 0;
}
