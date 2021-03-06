//
// Created by Konstantin Moskalenko on 12.05.2022.
//

#ifndef CL_COURSE_MODEL_H
#define CL_COURSE_MODEL_H

#include <utility>

#include "../Parser/Parser.h"

struct BaseModel {
    using ContextIterator = std::vector<const Parser::WordSet>::const_iterator;

    [[nodiscard]] virtual ContextIterator findMatch(
            ContextIterator begin, ContextIterator end,
            std::set<int> &matchedIndices
    ) const = 0;
};

struct PartOfSpeechModel : BaseModel {
    explicit PartOfSpeechModel(std::string pos) : partOfSpeech(std::move(pos)) {};

    [[nodiscard]] ContextIterator findMatch(
            ContextIterator begin, ContextIterator end,
            std::set<int> &matchedIndices
    ) const override {
        int tokenPos = 0;
        for (auto it = begin; it != end; it++, tokenPos++) {
            for (const auto &lemma: *it) {
                if (lemma->getPartOfSpeech() == this->partOfSpeech) {
                    matchedIndices.insert(tokenPos);
                    return it;
                }
            }
        }
        return end;
    }

private:
    const std::string partOfSpeech;
};

struct VocabularyModel : BaseModel {
    using NormalizedNGram = std::set<const std::vector<const Parser::WordSet>>;

    explicit VocabularyModel(NormalizedNGram lemmas) : matchingNGrams(std::move(lemmas)) {}

    [[nodiscard]] ContextIterator findMatch(
            ContextIterator begin, ContextIterator end,
            std::set<int> &matchedIndices
    ) const override {
        int tokenPos = 0;
        for (auto it = begin; it != end; it++, tokenPos++) {
            auto subIter = it;
            auto subTokenPos = tokenPos;

            for (const auto &nGram: matchingNGrams) {
                bool nGramMatched = true;
                std::set<int> nGramIndices;

                for (const auto &lemma: nGram) {
                    if (!intersect(subIter->begin(), subIter->end(),
                                   lemma.begin(), lemma.end())) {
                        nGramMatched = false;
                        break;
                    }
                    if (subIter != end) {
                        subIter++;
                        nGramIndices.insert(subTokenPos++);
                    } else {
                        nGramMatched = false;
                        break;
                    }
                }
                if (nGramMatched) {
                    matchedIndices.insert(nGramIndices.begin(), nGramIndices.end());
                    return subIter;
                }
            }
        }

        return end;
    }

private:
    const NormalizedNGram matchingNGrams;

    template<class InputIt1, class InputIt2>
    static bool intersect(InputIt1 first1, InputIt1 last1,
                          InputIt2 first2, InputIt2 last2) {
        while (first1 != last1 && first2 != last2) {
            if (*first1 < *first2) {
                ++first1;
                continue;
            }
            if (*first2 < *first1) {
                ++first2;
                continue;
            }
            return true;
        }
        return false;
    }
};

struct CompoundModel : BaseModel {
    explicit CompoundModel(std::string name, std::vector<const BaseModel *> models)
            : name(std::move(name)), models(std::move(models)) {}

    [[nodiscard]] ContextIterator findMatch(
            ContextIterator begin, ContextIterator end,
            std::set<int> &matchedIndices
    ) const override {
        auto iterator = begin;
        for (const auto model: models) {
            std::set<int> indices;
            const auto itOffset = (int) (iterator - begin);
            iterator = model->findMatch(iterator, end, indices);
            if (iterator == end) break;
            for (const auto &idx: indices) {
                matchedIndices.insert(itOffset + idx);
            }
        }

        return iterator;
    }

    const std::string name;
private:
    const std::vector<const BaseModel *> models;
};

#endif //CL_COURSE_MODEL_H
