#include "alphabet_index.hpp"

#include <cctype>
#include <fstream>
#include <iostream>

#include "flat_table.hpp"
#include "hash_table.hpp"
#include "list_sequence.hpp"

StringCharStream::StringCharStream(std::string t) : text_(std::move(t)) {
}

bool StringCharStream::Read(char& out) {
    if (pos_ >= text_.size())
        return false;
    out = text_[pos_++];
    return true;
}

bool StringCharStream::IsEnd() const {
    return pos_ >= text_.size();
}

bool StringCharStream::Seek(size_t p) {
    if (p > text_.size())
        return false;
    pos_ = p;
    return true;
}

LexerStream::LexerStream(Stream<char>& source) : source_(source) {
}

bool LexerStream::Read(std::string& out) {
    out.clear();
    char ch;
    while (source_.Read(ch)) {
        if (!std::isspace(ch)) {
            out.push_back(ch);
            break;
        }
    }
    if (out.empty()) {
        return false;
    }
    while (source_.Read(ch)) {
        if (std::isspace(ch)) {
            break;
        }
        out.push_back(ch);
    }
    return true;
}

bool LexerStream::IsEnd() const {
    return source_.IsEnd();
}

bool LexerStream::Seek(size_t pos) {
    return source_.Seek(pos);
}

LineRenderer::LineRenderer(Stream<std::string>& source, size_t line_limit, AlphabetIndexMode mode)
    : source_(source), line_limit_(line_limit == 0 ? 1 : line_limit), mode_(mode) {}

size_t WordWeight(const std::string& word, AlphabetIndexMode mode, size_t current) {
    if (mode == AlphabetIndexMode::Words)
        return 1;
    return word.size() + (current == 0 ? 0 : 1);
}

bool LineRenderer::Read(Line& out) {
    auto words = std::make_shared<ListSequence<std::string>>();
    size_t used = 0;

    auto add_word = [&](const std::string& w) -> bool {
        size_t wsize = WordWeight(w, mode_, used);
        if (used > 0 && used + wsize > line_limit_) {
            pending_ = w;
            has_pending_ = true;
            return false;
        }
        used += wsize;
        words->Append(w);
        return true;
    };

    std::string token;
    if (has_pending_) {
        token = std::move(pending_);
        has_pending_ = false;
    } else if (!source_.Read(token)) {
        return false;
    }

    while (true) {
        if (!add_word(token)) {
            break;
        }
        if (!source_.Read(token)) {
            break;
        }
    }

    if (words->GetLength() == 0) {
        return false;
    }

    out.words = words;
    return true;
}

bool LineRenderer::IsEnd() const {
    return !has_pending_ && source_.IsEnd();
}

PaginatorStream::PaginatorStream(Stream<Line>& source, size_t page_size, AlphabetIndexMode mode)
    : source_(source), page_size_(page_size), mode_(mode) {
}

size_t PaginatorStream::PageCapacity(size_t page) const {
    size_t cap = page_size_;
    if (page == 1) {
        cap = page_size_ / 2;
    } else if (page % 10 == 0) {
        cap = (page_size_ * 3) / 4;
    }
    return cap == 0 ? 1 : cap;
}

size_t PaginatorStream::LineWeight(const Line& line) const {
    size_t total = 0;
    bool first = true;
    for (auto wit = line.words->GetIterator(); wit->HasNext(); wit->Next()) {
        const auto& word = wit->GetCurrentItem();
        if (mode_ == AlphabetIndexMode::Words) {
            ++total;
        } else {
            total += word.size();
            if (!first) {
                ++total;
            }
        }
        first = false;
    }
    return total;
}

bool PaginatorStream::Read(Page& out) {
    auto lines = std::make_shared<ListSequence<Line>>();
    size_t cap = PageCapacity(current_page_);
    size_t used = 0;

    auto take_line = [&](const Line& line) -> bool {
        size_t w = LineWeight(line);
        if (used == 0 && w > cap) {
            used += w;
            lines->Append(line);
            return true;
        }
        if (used + w > cap) {
            pending_ = line;
            has_pending_ = true;
            return false;
        }
        used += w;
        lines->Append(line);
        return true;
    };

    if (has_pending_) {
        if (!take_line(pending_)) {
            return false;
        }
        has_pending_ = false;
    }

    Line line;
    while (source_.Read(line)) {
        if (!take_line(line)) {
            break;
        }
    }

    if (lines->GetLength() == 0) {
        return false;
    }

    out.number = current_page_;
    out.lines = lines;
    ++current_page_;
    return true;
}

bool PaginatorStream::IsEnd() const {
    return !has_pending_ && source_.IsEnd();
}

void WriteBook(const Book& book, std::ostream& out) {
    out << "Pages:\n";
    if (book.pages == nullptr || book.pages->GetLength() == 0) {
        out << "(empty)\n";
    } else {
        for (auto pit = book.pages->GetIterator(); pit->HasNext(); pit->Next()) {
            const auto& page = pit->GetCurrentItem();
            out << "Page " << page.number << ":\n";
            size_t line_no = 1;
            if (page.lines != nullptr) {
                for (auto lit = page.lines->GetIterator(); lit->HasNext(); lit->Next()) {
                    const auto& line = lit->GetCurrentItem();
                    out << "  [" << line_no++ << "] ";
                    bool first = true;
                    if (line.words != nullptr) {
                        for (auto wit = line.words->GetIterator(); wit->HasNext(); wit->Next()) {
                            if (!first) {
                                out << ' ';
                            }
                            out << wit->GetCurrentItem();
                            first = false;
                        }
                    }
                    out << '\n';
                }
            }
        }
    }

    out << "Index:\n";
    if (book.index == nullptr || book.index->GetCount() == 0) {
        out << "(empty)\n";
        return;
    }
    for (auto it = book.index->GetIterator(); it->HasNext(); it->Next()) {
        const auto& kv = it->GetCurrentItem();
        out << kv.key << " -> " << kv.value << '\n';
    }
}

bool SaveBook(const Book& book, const std::string& path) {
    if (path.empty()) {
        return false;
    }
    if (path == "-") {
        WriteBook(book, std::cout);
        return true;
    }
    std::ofstream out(path);
    if (!out.is_open()) {
        return false;
    }
    WriteBook(book, out);
    return out.good();
}
