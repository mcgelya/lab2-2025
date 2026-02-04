#pragma once

#include "linked_list.hpp"
#include "sequence.hpp"

template <typename T>
class ListSequenceIterator : public IIterator<T> {
public:
    ListSequenceIterator(ListNodePtr<T> it) : it_(std::move(it)) {
    }

    bool HasNext() const override {
        return it_ != nullptr;
    }

    bool Next() override {
        if (!HasNext()) {
            return false;
        }
        it_ = it_->next;
        return true;
    }

    const T& GetCurrentItem() const override {
        if (!HasNext()) {
            throw std::out_of_range("No next element");
        }
        return it_->value;
    }

    bool TryGetCurrentItem(T& element) const override {
        if (!HasNext()) {
            return false;
        }
        element = it_->value;
        return true;
    }

private:
    ListNodePtr<T> it_;
};

template <typename T>
class ListSequence : public Sequence<T> {
public:
    ListSequence(const T* items, size_t count) : data_(items, count) {
    }

    ListSequence(LinkedList<T> l) : data_(std::move(l)) {
    }

    ListSequence(const Sequence<T>& a) {
        for (IIteratorPtr<T> it = a.GetIterator(); it->HasNext(); it->Next()) {
            Append(it->GetCurrentItem());
        }
    }

    ListSequence(SequencePtr<T> a) : ListSequence(*a) {
    }

    ListSequence() {
    }

    const T& GetFirst() const override {
        return data_.GetFirst();
    }

    const T& GetLast() const override {
        return data_.GetLast();
    }

    const T& Get(size_t index) const override {
        return data_.Get(index);
    }

    void Set(const T& item, size_t index) override {
        data_.Set(item, index);
    }

    SequencePtr<T> GetSubsequence(size_t startIndex, size_t endIndex) const override {
        return std::make_shared<ListSequence<T>>(data_.GetSubList(startIndex, endIndex));
    }

    SequencePtr<T> GetFirst(size_t count) const override {
        if (count == 0) {
            return std::make_shared<ListSequence<T>>();
        }
        return GetSubsequence(0, count - 1);
    }

    SequencePtr<T> GetLast(size_t count) const override {
        if (count == 0) {
            return std::make_shared<ListSequence<T>>();
        }
        if (count > data_.GetLength()) {
            throw std::out_of_range("Requested elements count is greater than size");
        }
        return GetSubsequence(data_.GetLength() - count, data_.GetLength() - 1);
    }

    size_t GetLength() const override {
        return data_.GetLength();
    }

    void Append(const T& item) override {
        data_.Append(item);
    }

    void Prepend(const T& item) override {
        data_.Prepend(item);
    }

    void InsertAt(const T& item, size_t index) override {
        data_.InsertAt(item, index);
    }

    void EraseAt(size_t index) override {
        data_.EraseAt(index);
    }

    void Clear() override {
        data_.Clear();
    }

    IIteratorPtr<T> GetIterator() const override {
        return std::make_shared<ListSequenceIterator<T>>(data_.GetBegin());
    }

private:
    LinkedList<T> data_;
};
