#include "alphabet_index.hpp"

#include <cctype>
#include <utility>

LexerStream::LexerStream(std::string text) : text_(std::move(text)) {
}

bool LexerStream::Read(std::string& out) {
    while (pos_ < text_.size() && std::isspace(static_cast<unsigned char>(text_[pos_]))) {
        ++pos_;
    }
    if (pos_ >= text_.size()) {
        return false;
    }
    size_t start = pos_;
    while (pos_ < text_.size() && !std::isspace(static_cast<unsigned char>(text_[pos_]))) {
        ++pos_;
    }
    out = text_.substr(start, pos_ - start);
    return true;
}

PaginatorStream::PaginatorStream(Stream<std::string>& source, size_t page_size, AlphabetIndexMode mode)
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

size_t PaginatorStream::WordSize(const std::string& word, size_t current_size) const {
    if (mode_ == AlphabetIndexMode::Words) {
        return 1;
    }
    return word.size() + (current_size == 0 ? 0 : 1);
}

bool PaginatorStream::Read(TokenPage& out) {
    std::string token;
    if (!source_.Read(token)) {
        return false;
    }

    size_t cap = PageCapacity(current_page_);
    size_t wsize = WordSize(token, current_size_);
    if (current_size_ + wsize > cap && current_size_ > 0) {
        ++current_page_;
        current_size_ = 0;
        cap = PageCapacity(current_page_);
        wsize = WordSize(token, current_size_);
    }

    current_size_ += wsize;
    out.word = std::move(token);
    out.page = static_cast<int>(current_page_);
    return true;
}
