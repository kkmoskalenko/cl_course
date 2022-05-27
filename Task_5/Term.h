//
// Created by Konstantin Moskalenko on 27.05.2022.
//

#ifndef CL_COURSE_TERM_H
#define CL_COURSE_TERM_H

struct Term {
    struct TextEntry {
        std::set<const int> positions;
    };

    int frequency = 0;
    std::unordered_map<std::string, TextEntry> entries;
};

#endif //CL_COURSE_TERM_H
