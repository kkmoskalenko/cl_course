#include "Parser.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <system_error>

void throwSystemError(const char *path) {
    throw std::system_error(errno, std::generic_category(), path);
}

void mapFile(const char *path, const std::function<void(const char *, size_t)> &callback) {
    const int fd = open(path, O_RDONLY);
    if (fd == -1) throwSystemError(path);

    struct stat sb{};
    if (fstat(fd, &sb) == -1) throwSystemError(path);
    size_t length = sb.st_size;

    void *ptr = mmap(nullptr, length, PROT_READ, MAP_PRIVATE, fd, 0);
    if (ptr == MAP_FAILED) throwSystemError(path);

    callback((const char *) ptr, length);

    if (munmap(ptr, length) == -1) throwSystemError(path);
    if (close(fd) == -1) throwSystemError(path);
}

Parser::Dictionary Parser::readDictionary(const std::string &path, size_t expectedSize) {
    Parser::Dictionary dictionary;
    dictionary.reserve(expectedSize);

    mapFile(path.c_str(), [&dictionary](const char *ptr, size_t length) {
        Word *lemma;

        for (const char *ch = ptr; ch != ptr + length; ch++) {
            if (*ch == '\n' || std::isdigit(*ch)) {
                lemma = nullptr;
                continue;
            }

            Word *word = new Word{};
            word->lemma = lemma;

            const char *tabPos = strchr(ch, '\t');
            word->text.assign(ch, tabPos - ch);
            ch = tabPos + 1;

            const char *lineEnd = strchr(ch, '\n');
            word->grammemes.assign(ch, lineEnd - ch);
            ch = lineEnd;

            dictionary.emplace(word->text, word);

            if (lemma == nullptr) {
                lemma = word;
            }
        }
    });

    return dictionary;
}

Parser::Tokens Parser::readTokens(std::wistream &stream, bool includePunctuation, const std::locale &locale) {
    stream.imbue(locale);

    Tokens tokens;
    std::wstring token;
    wchar_t ch;

    while (stream.get(ch)) {
        if (std::iswalpha(ch) || ch == '-') {
            token += ch;
        } else {
            if (!token.empty() && token[0] != '-') {
                tokens.push_back(token);
            }

            if (includePunctuation && std::iswpunct(ch)) {
                tokens.emplace_back(1, ch);
            }

            token = L"";
        }
    }

    if (!token.empty() && token[0] != '-') {
        tokens.push_back(token);
    }

    return tokens;
}