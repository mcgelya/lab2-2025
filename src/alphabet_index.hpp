#pragma once

#include <string>

#include "fwd.hpp"
#include "list_sequence.hpp"
#include "sequence.hpp"
#include "stream.hpp"

enum class AlphabetIndexMode { Words, Chars };

class LexerStream : public Stream<std::string> {
public:
    explicit LexerStream(Stream<char>& source);
    bool Read(std::string& out) override;
    bool IsEnd() const override;
    bool Seek(size_t pos) override;

private:
    Stream<char>& source_;
};

struct Line {
    SequencePtr<std::string> words;
};

struct Page {
    int number = 0;
    SequencePtr<Line> lines;
};

class LineRenderer : public Stream<Line> {
public:
    LineRenderer(Stream<std::string>& source, size_t line_limit, AlphabetIndexMode mode);
    bool Read(Line& out) override;
    bool IsEnd() const override;

private:
    Stream<std::string>& source_;
    size_t line_limit_;
    AlphabetIndexMode mode_;
    bool has_pending_ = false;
    std::string pending_;
};

class PaginatorStream : public Stream<Page> {
public:
    PaginatorStream(Stream<Line>& source, size_t page_size, AlphabetIndexMode mode);
    bool Read(Page& out) override;
    bool IsEnd() const override;

private:
    size_t PageCapacity(size_t page) const;
    size_t LineWeight(const Line& line, size_t current_size) const;

private:
    Stream<Line>& source_;
    size_t page_size_;
    AlphabetIndexMode mode_;
    size_t current_page_ = 1;
    bool has_pending_ = false;
    Line pending_;
};

class StringCharStream : public Stream<char> {
public:
    explicit StringCharStream(std::string t);
    bool Read(char& out) override;
    bool IsEnd() const override;
    bool Seek(size_t p) override;

private:
    std::string text_;
    size_t pos_ = 0;
};

struct Book {
    SequencePtr<Page> pages;
    IDictionaryPtr<std::string, int> index;
};

template <typename Dict>
Book BuildBook(const std::string& text, size_t page_size, AlphabetIndexMode mode) {
    StringCharStream char_stream(text);
    LexerStream lexer(char_stream);
    LineRenderer lines(lexer, page_size, mode);
    PaginatorStream paginator(lines, page_size, mode);
    auto pages = std::make_shared<ListSequence<Page>>();
    auto index = std::make_shared<Dict>();
    Page page;
    while (paginator.Read(page)) {
        pages->Append(page);
        for (auto lit = page.lines->GetIterator(); lit->HasNext(); lit->Next()) {
            const auto& line = lit->GetCurrentItem();
            for (auto wit = line.words->GetIterator(); wit->HasNext(); wit->Next()) {
                const auto& word = wit->GetCurrentItem();
                if (!index->ContainsKey(word)) {
                    index->Add(word, page.number);
                }
            }
        }
    }
    return Book{std::move(pages), std::move(index)};
}
