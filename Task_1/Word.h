//
// Created by Konstantin Moskalenko on 25.02.2022.
//

#ifndef TASK_1_WORD_H
#define TASK_1_WORD_H

#include <string>
#include <vector>

struct Word {
    std::wstring text;
    std::vector<std::wstring> *grammemes;

    const Word *lemma;
};


#endif //TASK_1_WORD_H
