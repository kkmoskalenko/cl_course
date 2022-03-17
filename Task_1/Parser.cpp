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

Parser::Dictionary Parser::readDictionary(const char *path, size_t expectedSize) {
    Parser::Dictionary dictionary;
    dictionary.reserve(expectedSize);

    mapFile(path, [&dictionary](const char *ptr, size_t length) {
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