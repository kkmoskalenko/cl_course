#ifndef TASK_1_PARSER_H
#define TASK_1_PARSER_H

#include "Word.h"

#include <unordered_map>
#include <vector>
#include <set>
#include <istream>
#include <locale>

namespace Parser {
    using Dictionary = std::unordered_multimap<std::string, const Word *>;
    using Tokens = std::vector<std::wstring>;
    using WordSet = std::set<const Word *>;

    Dictionary readDictionary(const std::string &path, size_t expectedSize = 0);

    Tokens readTokens(std::wistream &stream, bool includePunctuation = false,
                      const std::locale &locale = std::locale("ru_RU.UTF-8"));

    WordSet normalizeToken(const std::wstring &token, Parser::Dictionary &dictionary);
}

#endif //TASK_1_PARSER_H
