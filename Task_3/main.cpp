#include <iostream>
#include <clocale>
#include <filesystem>
#include <fstream>
#include <codecvt>
#include <set>

#include "../Parser/Parser.h"

constexpr int MIN_COUNT = 2;
constexpr float MAX_STABILITY = 0.1;

std::unordered_map<std::string, std::vector<const Word *>> corpus;

struct NGram {
    std::string normalizedForm;
    size_t contextLength;

    int totalEntryCount = 0;
    double stability = 0;
    std::set<std::string> textEntry;

    NGram(const std::string &normalizedForm, const std::vector<const Word *> &words) {
        this->normalizedForm = normalizedForm;
        this->contextLength = words.size();
    }
};

std::unordered_map<std::string, std::vector<const NGram *>> leftExtensions;
std::unordered_map<std::string, std::vector<const NGram *>> rightExtensions;
std::unordered_map<std::string, std::vector<int>> currentPositions;
std::unordered_map<std::string, std::vector<int>> previousPositions;

NGram *handleContext(const std::vector<const Word *> &wordVector, int windowSize,
                     const std::unordered_map<std::string, NGram *> &NGrams) {
    std::string normalizedForm;
    normalizedForm.reserve(windowSize * 8);

    std::string leftNormalizedExtension;
    leftNormalizedExtension.reserve(windowSize * 8);

    std::string rightNormalizedExtension;
    rightNormalizedExtension.reserve(windowSize * 8);

    std::vector<const Word *> contextVector;
    for (int i = 0; i < windowSize; i++) {
        const Word *word = wordVector[i];
        contextVector.push_back(word);

        if (i == 0) {
            rightNormalizedExtension += word->text + ' ';
        } else if (i == wordVector.size() - 1) {
            leftNormalizedExtension += word->text + ' ';
        } else {
            leftNormalizedExtension += word->text + ' ';
            rightNormalizedExtension += word->text + ' ';
        }

        normalizedForm += word->text + ' ';
    }

    leftNormalizedExtension.pop_back();
    rightNormalizedExtension.pop_back();
    normalizedForm.pop_back();

    if (windowSize == 2 ||
        NGrams.find(leftNormalizedExtension) != NGrams.end() ||
        NGrams.find(rightNormalizedExtension) != NGrams.end()) {

        auto *wc = new NGram(normalizedForm, contextVector);
        if (leftExtensions.find(leftNormalizedExtension) == leftExtensions.end()) {
            leftExtensions.emplace(leftNormalizedExtension, std::vector<const NGram *>{});
        }
        if (rightExtensions.find(rightNormalizedExtension) == rightExtensions.end()) {
            rightExtensions.emplace(rightNormalizedExtension, std::vector<const NGram *>{});
        }

        std::vector<const NGram *> &wordContextsLeft = leftExtensions.at(leftNormalizedExtension);
        std::vector<const NGram *> &wordContextsRight = rightExtensions.at(rightNormalizedExtension);

        wordContextsLeft.push_back(wc);
        wordContextsRight.push_back(wc);

        return wc;
    } else {
        return nullptr;
    }
}

void handleWord(const Word *word,
                std::vector<const Word *> &leftWordContext,
                std::unordered_map<std::string, NGram *> &nGrams,
                int windowSize, const std::string &filePath,
                const int wordPosition) {
    for (int i = windowSize; i <= windowSize + 1; i++) {
        if (i <= leftWordContext.size()) {
            bool isCloseToNGram = false;
            if (windowSize > 2) {
                if (previousPositions.find(filePath) == currentPositions.end())
                    continue;
                std::vector<int> positions = previousPositions.at(filePath);
                for (int position: positions)
                    if (wordPosition == position) {
                        isCloseToNGram = true;
                        break;
                    }
            } else {
                isCloseToNGram = true;
            }

            if (!isCloseToNGram) continue;

            NGram *phraseContext = handleContext(leftWordContext, i, nGrams);
            if (phraseContext != nullptr) {
                if (nGrams.find(phraseContext->normalizedForm) == nGrams.end()) {
                    nGrams.emplace(phraseContext->normalizedForm, phraseContext);
                }

                auto &ngram = nGrams.at(phraseContext->normalizedForm);
                ngram->totalEntryCount++;
                ngram->textEntry.insert(filePath);

                if (currentPositions.find(filePath) == currentPositions.end()) {
                    currentPositions.emplace(filePath, std::vector<int>{});
                }
                currentPositions.at(filePath).push_back(wordPosition);
            }
        }
    }

    if (leftWordContext.size() == (windowSize + 1)) {
        leftWordContext.erase(leftWordContext.begin());
    }

    leftWordContext.push_back(word);
}

std::vector<const NGram *> extractNGrams() {
    std::unordered_map<std::string, NGram *> nGrams;
    std::vector<const NGram *> stableNGrams;

    for (int windowSize = 2;; windowSize++) {
        for (const auto &pair: corpus) {
            const auto filepath = pair.first;
            const auto fileContent = pair.second;

            std::vector<const Word *> leftWordContext;

            int wordPosition = 0;
            for (const Word *word: fileContent) {
                handleWord(word, leftWordContext, nGrams, windowSize, filepath, wordPosition);
                wordPosition++;
            }
        }

        std::unordered_map<std::string, NGram *> thresholdNGrams;

        for (auto &pair: nGrams) {
            if (pair.second->totalEntryCount >= MIN_COUNT) {
                try {
                    if (pair.second->contextLength != windowSize)
                        continue;

                    int extensionMax = 0;

                    std::vector<const NGram *> leftWordContexts = leftExtensions.at(pair.second->normalizedForm);
                    std::vector<const NGram *> rightWordContexts = rightExtensions.at(
                            pair.second->normalizedForm);

                    for (const NGram *wc: leftWordContexts) {
                        if ((wc->contextLength == pair.second->contextLength + 1) && wc->totalEntryCount > extensionMax)
                            extensionMax = wc->totalEntryCount;
                    }

                    for (const NGram *wc: rightWordContexts) {
                        if ((wc->contextLength == pair.second->contextLength + 1) && wc->totalEntryCount > extensionMax)
                            extensionMax = wc->totalEntryCount;
                    }

                    pair.second->stability = ((float) extensionMax / (float) pair.second->totalEntryCount);
                    if (pair.second->stability <= MAX_STABILITY) {
                        stableNGrams.push_back(pair.second);
                    }

                    thresholdNGrams.emplace(pair);
                } catch (const std::out_of_range &) {}
            }
        }

        nGrams = thresholdNGrams;

        previousPositions = currentPositions;
        currentPositions.clear();

        if (nGrams.empty()) break;
    }

    return stableNGrams;
}

const Word *normalizeToken(const std::wstring &token, Parser::Dictionary &dictionary) {
    static std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

    std::wstring upperToken = token;
    std::transform(upperToken.begin(), upperToken.end(),
                   upperToken.begin(), std::towupper);

    const std::string dictKey = converter.to_bytes(upperToken);
    const auto dict_itr = dictionary.equal_range(dictKey);

    if (dictionary.count(dictKey) == 0) {
        const auto lemma = new Word{dictKey, "UNKNOWN"};
        dictionary.emplace(lemma->text, lemma);
        return lemma;
    } else {
        auto word = dict_itr.first->second;
        return word->lemma ?: word;
    }
}

int main() {
    std::setlocale(LC_ALL, "ru_RU.UTF-8");

    auto dictionary = Parser::readDictionary(DICTIONARY_PATH, 6000000);
    const auto iterator = std::filesystem::directory_iterator(NEWS_PATH);

    for (const auto &entry: iterator) {
        std::wifstream wif(entry.path());
        const auto tokens = Parser::readTokens(wif, true);

        std::vector<const Word *> lemmas;
        lemmas.reserve(tokens.size());

        for (const auto &token: tokens) {
            lemmas.push_back(normalizeToken(token, dictionary));
        }
        corpus.emplace(entry.path().string(), lemmas);
    }

    auto stableNGrams = extractNGrams();
    sort(stableNGrams.begin(), stableNGrams.end(), [](const NGram *a, const NGram *b) {
        return a->stability < b->stability;
    });

    for (const NGram *context: stableNGrams) {
        std::cout << context->normalizedForm
                  << ", Frequency: " << context->totalEntryCount
                  << ", Text frequency: " << context->textEntry.size()
                  << ", IDF: " << log((double) corpus.size() / (double) context->textEntry.size())
                  << std::endl;
    }

    return 0;
}
