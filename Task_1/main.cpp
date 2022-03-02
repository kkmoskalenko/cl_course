#include <fstream>
#include <sstream>
#include <filesystem>
#include <codecvt>
#include <clocale>
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

        it++; // Skipping whitespace

        while (it != line.end()) {
            std::wstring grammeme;
            while (*it != ',' && it != line.end()) {
                grammeme += *it;
                it++;
            }

            if (*it == ',') it++;
            word->grammemes.push_back(grammeme);
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

    const auto words = readDictionary("../resources/dict.opcorpora.txt");
    const auto iterator = std::filesystem::directory_iterator("../resources/news");

    for (const auto &entry : iterator) {
        const auto tokens = readTokens(entry.path());
        // TODO: Process tokens
    }

    return 0;
}
