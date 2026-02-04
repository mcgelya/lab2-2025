#pragma once

#include <string>

#include "fwd.hpp"
#include "stream.hpp"

enum class AlphabetIndexMode { Words, Chars };

struct TokenPage {
    std::string word;
    int page;
};

class LexerStream : public Stream<std::string> {
public:
    explicit LexerStream(std::string text);
    bool Read(std::string& out) override;

private:
    std::string text_;
    size_t pos_ = 0;
};

class PaginatorStream : public Stream<TokenPage> {
public:
    PaginatorStream(Stream<std::string>& source, size_t page_size, AlphabetIndexMode mode);
    bool Read(TokenPage& out) override;

private:
    size_t PageCapacity(size_t page) const;
    size_t WordSize(const std::string& word, size_t current_size) const;

private:
    Stream<std::string>& source_;
    size_t page_size_;
    AlphabetIndexMode mode_;
    size_t current_page_ = 1;
    size_t current_size_ = 0;
};

template <typename Dict>
IDictionaryPtr<std::string, int> BuildAlphabetIndex(Stream<std::string>& lexer, size_t page_size,
                                                    AlphabetIndexMode mode) {
    auto dict = std::make_shared<Dict>();
    PaginatorStream paginator(lexer, page_size, mode);

    TokenPage tp;
    while (paginator.Read(tp)) {
        if (!dict->ContainsKey(tp.word)) {
            dict->Add(tp.word, tp.page);
        }
    }
    return dict;
}

template <typename Dict>
IDictionaryPtr<std::string, int> BuildAlphabetIndex(const std::string& text, size_t page_size, AlphabetIndexMode mode) {
    LexerStream lexer(text);
    return BuildAlphabetIndex<Dict>(lexer, page_size, mode);
}
