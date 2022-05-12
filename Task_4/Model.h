//
// Created by Konstantin Moskalenko on 12.05.2022.
//

#ifndef CL_COURSE_MODEL_H
#define CL_COURSE_MODEL_H

#include <utility>

#include "../Parser/Parser.h"

struct BaseModel {
    using ContextIterator = std::vector<const Parser::WordSet>::const_iterator;

    [[nodiscard]] virtual bool matchesContext(ContextIterator begin, ContextIterator end) const = 0;

    [[nodiscard]] virtual int size() const = 0;
};

struct PartOfSpeechModel : BaseModel {
    explicit PartOfSpeechModel(std::string pos) : partOfSpeech(std::move(pos)) {};

    [[nodiscard]] bool matchesContext(ContextIterator begin, ContextIterator end) const override {
        if (std::distance(begin, end) < size()) {
            return false;
        }

        const auto lemmas = *begin;
        return std::any_of(lemmas.begin(), lemmas.end(), [this](const Word *lemma) {
            return lemma->getPartOfSpeech() == this->partOfSpeech;
        });
    }

    [[nodiscard]] int size() const override {
        return 1;
    }

private:
    const std::string partOfSpeech;
};

struct VocabularyModel : BaseModel {
    explicit VocabularyModel(Parser::WordSet lemmas) : matchingLemmas(std::move(lemmas)) {}

    [[nodiscard]] bool matchesContext(ContextIterator begin, ContextIterator end) const override {
        if (std::distance(begin, end) < size()) {
            return false;
        }

        const auto lemmas = *begin;
        auto first1 = lemmas.begin();
        auto last1 = lemmas.end();
        auto first2 = matchingLemmas.begin();
        auto last2 = matchingLemmas.end();

        while (first1 != last1 && first2 != last2) {
            if (*first1 < *first2) { ++first1; }
            else if (*first2 < *first1) { ++first2; }
            else { return true; }
        }

        return false;
    }

    [[nodiscard]] int size() const override {
        return 1;
    }

private:
    const Parser::WordSet matchingLemmas;
};

struct CompoundModel : BaseModel {
    explicit CompoundModel(std::string name, std::vector<const BaseModel *> models)
            : name(std::move(name)), models(std::move(models)) {}

    [[nodiscard]] bool matchesContext(ContextIterator begin, ContextIterator end) const override {
        if (std::distance(begin, end) < size()) {
            return false;
        }

        auto iterator = begin;
        for (const auto model: models) {
            if (!model->matchesContext(iterator, end)) {
                return false;
            }

            iterator += model->size();
        }

        return true;
    }

    [[nodiscard]] int size() const override {
        int result = 0;
        for (const auto model: models) {
            result += model->size();
        }
        return result;
    }

    const std::string name;
private:
    const std::vector<const BaseModel *> models;
};

#endif //CL_COURSE_MODEL_H
