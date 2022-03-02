#include <fstream>
#include <sstream>
#include "Word.h"

std::vector<Word> readDictionary(const std::string &filepath) {
    std::ifstream dictFile(filepath);
    std::string line;

    std::vector<Word> words;

    while (std::getline(dictFile, line)) {
        if (line.empty() || std::isdigit(line[0])) continue;
        Word word;

        std::string::iterator it = line.begin();
        while (!std::isspace(*it)) {
            word.text += *it;
            it++;
        }

        it++; // Skipping whitespace

        while (it != line.end()) {
            std::string grammeme;
            while (*it != ',' && it != line.end()) {
                grammeme += *it;
                it++;
            }

            if (*it == ',') it++;
            word.grammemes.push_back(grammeme);
        }

        words.push_back(word);
    }

    return words;
}

int main() {
    const auto words = readDictionary("../resources/dict.opcorpora.txt");

    return 0;
}
