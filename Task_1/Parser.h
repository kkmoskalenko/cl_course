#ifndef TASK_1_PARSER_H
#define TASK_1_PARSER_H

#include "Word.h"
#include <unordered_map>

namespace Parser {
    using Dictionary = std::unordered_multimap<std::string, const Word *>;

    Dictionary readDictionary(const char *path, size_t expectedSize = 0);
}

#endif //TASK_1_PARSER_H
