//
// Created by Konstantin Moskalenko on 12.05.2022.
//

#ifndef CL_COURSE_MODEL_H
#define CL_COURSE_MODEL_H

#include <utility>

#include "../Parser/Parser.h"

struct BaseModel {
    using ContextIterator = std::vector<const Parser::WordSet>::const_iterator;

    [[nodiscard]] virtual ContextIterator findMatch(ContextIterator begin, ContextIterator end) const = 0;
};

struct PartOfSpeechModel : BaseModel {
    explicit PartOfSpeechModel(std::string pos) : partOfSpeech(std::move(pos)) {};

    [[nodiscard]] ContextIterator findMatch(ContextIterator begin, ContextIterator end) const override {
        for (auto it = begin; it != end; it++) {
            for (const auto &lemma: *it) {
                if (lemma->getPartOfSpeech() == this->partOfSpeech) {
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
    explicit VocabularyModel(Parser::WordSet lemmas) : matchingLemmas(std::move(lemmas)) {}

    [[nodiscard]] ContextIterator findMatch(ContextIterator begin, ContextIterator end) const override {
        for (auto it = begin; it != end; it++) {
            const auto lemmas = *it;
            auto first1 = lemmas.begin();
            auto last1 = lemmas.end();
            auto first2 = matchingLemmas.begin();
            auto last2 = matchingLemmas.end();

            while (first1 != last1 && first2 != last2) {
                if (*first1 < *first2) { ++first1; }
                else if (*first2 < *first1) { ++first2; }
                else { return it; }
            }
        }

        return end;
    }

private:
    const Parser::WordSet matchingLemmas;
};

struct CompoundModel : BaseModel {
    explicit CompoundModel(std::string name, std::vector<const BaseModel *> models)
            : name(std::move(name)), models(std::move(models)) {}

    [[nodiscard]] ContextIterator findMatch(ContextIterator begin, ContextIterator end) const override {
        auto iterator = begin;
        for (const auto model: models) {
            iterator = model->findMatch(iterator, end);
            if (iterator == end) break;
        }

        return iterator;
    }

    const std::string name;
private:
    const std::vector<const BaseModel *> models;
};

#endif //CL_COURSE_MODEL_H
