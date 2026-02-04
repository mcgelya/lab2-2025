#pragma once

#include <memory>
#include <stdexcept>
#include <string>

template <typename T>
struct ListNode;

template <typename T>
using ListNodePtr = std::shared_ptr<ListNode<T>>;

template <typename T>
struct ListNode : public std::enable_shared_from_this<ListNode<T>> {
    T value;
    ListNodePtr<T> next;

    ListNode(T value) : value(value) {
    }

    ListNodePtr<T> NextNth(size_t n) {
        ListNodePtr<T> cur = this->shared_from_this();
        for (size_t i = 0; i < n; ++i) {
            cur = cur->next;
        }
        return cur;
    }
};

template <typename T>
class LinkedList {
public:
    LinkedList(const T* items, size_t count) {
        for (size_t i = 0; i < count; ++i) {
            Append(items[i]);
        }
    }

    LinkedList() {
    }

    LinkedList(const LinkedList<T>& l) {
        ListNodePtr<T> cur = l.first_;
        for (size_t i = 0; i < l.size_; ++i) {
            Append(cur->value);
            cur = cur->next;
        }
    }

    const T& GetFirst() const {
        if (size_ == 0) {
            throw std::out_of_range("List is empty");
        }
        return first_->value;
    }

    const T& GetLast() const {
        if (size_ == 0) {
            throw std::out_of_range("List is empty");
        }
        return last_->value;
    }

    const T& Get(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Index is out of range: " + std::to_string(index) + " " + std::to_string(size_));
        }
        return first_->NextNth(index)->value;
    }

    void Set(const T& item, size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Index is out of range: " + std::to_string(index) + " " + std::to_string(size_));
        }
        first_->NextNth(index)->value = item;
    }

    LinkedList<T> GetSubList(size_t startIndex, size_t endIndex) const {
        if (startIndex >= size_) {
            throw std::out_of_range("Index is out of range: " + std::to_string(startIndex) + " " +
                                    std::to_string(size_));
        }
        if (endIndex >= size_) {
            throw std::out_of_range("Index is out of range: " + std::to_string(endIndex) + " " + std::to_string(size_));
        }
        if (startIndex > endIndex) {
            throw std::out_of_range("startIndex is greater than endIndex");
        }
        LinkedList<T> res;
        ListNodePtr<T> cur = first_->NextNth(startIndex);
        for (size_t i = startIndex; i <= endIndex; ++i) {
            res.Append(cur->value);
            cur = cur->next;
        }
        return res;
    }

    size_t GetLength() const {
        return size_;
    }

    void Append(const T& item) {
        ListNodePtr<T> cur = std::make_shared<ListNode<T>>(item);
        if (size_ == 0) {
            first_ = cur;
            last_ = cur;
            ++size_;
            return;
        }
        last_->next = cur;
        last_ = cur;
        ++size_;
    }

    void Prepend(const T& item) {
        ListNodePtr<T> cur = std::make_shared<ListNode<T>>(item);
        if (size_ == 0) {
            first_ = cur;
            last_ = cur;
            ++size_;
            return;
        }
        cur->next = first_;
        first_ = cur;
        ++size_;
    }

    // Insert before index
    void InsertAt(const T& item, size_t index) {
        if (index > size_) {
            throw std::out_of_range("Index is out of range: " + std::to_string(index) + " " + std::to_string(size_));
        }
        if (index == size_) {
            Append(item);
            return;
        }
        if (index == 0) {
            Prepend(item);
            return;
        }
        ListNodePtr<T> prev = first_->NextNth(index - 1);
        ListNodePtr<T> next = prev->next;
        ListNodePtr<T> cur = std::make_shared<ListNode<T>>(item);
        prev->next = cur;
        cur->next = next;
        ++size_;
    }

    void Concat(const LinkedList<T>& l) {
        if (size_ == 0) {
            first_ = l.first_;
            last_ = l.last_;
            size_ = l.size_;
            return;
        }
        last_->next = l.first_;
        last_ = l.last_;
        size_ += l.size_;
    }

    ListNodePtr<T> GetBegin() const {
        return first_;
    }

    void EraseAt(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Index is out of range: " + std::to_string(index) + " " + std::to_string(size_));
        }
        if (index == 0) {
            first_ = first_->next;
            if (size_ == 1) {
                last_.reset();
            }
            --size_;
            return;
        }
        ListNodePtr<T> prev = first_->NextNth(index - 1);
        ListNodePtr<T> target = prev->next;
        prev->next = target->next;
        if (index == size_ - 1) {
            last_ = prev;
        }
        --size_;
    }

    void Clear() {
        first_.reset();
        last_.reset();
        size_ = 0;
    }

private:
    ListNodePtr<T> first_;
    ListNodePtr<T> last_;
    size_t size_ = 0;
};
