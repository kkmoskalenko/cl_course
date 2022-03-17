//
// Created by Konstantin Moskalenko on 25.02.2022.
//

#ifndef TASK_1_WORD_H
#define TASK_1_WORD_H

#include <string>

struct Word {
    std::string text;
    std::string grammemes;

    const Word *lemma;

    [[nodiscard]] std::string getPartOfSpeech() const {
        return grammemes.substr(0, grammemes.find(','));
    }
};


#endif //TASK_1_WORD_H
