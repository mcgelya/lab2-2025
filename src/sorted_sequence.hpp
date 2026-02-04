#pragma once

#include <cstddef>
#include <functional>
#include <memory>

#include "array_sequence.hpp"
#include "dynamic_array.hpp"
#include "fwd.hpp"
#include "isorted_sequence.hpp"

template <typename T, typename Comparator = std::less<T>>
class SortedSequence : public ISortedSequence<T> {
public:
    SortedSequence(const T* items, size_t count, Comparator comp = Comparator())
        : data_(std::make_shared<ArraySequence<T>>(items, count)), comp_(std::move(comp)) {
        Sort();
    }

    SortedSequence(SequencePtr<T> data, Comparator comp = Comparator())
        : data_(std::make_shared<ArraySequence<T>>(std::move(data))), comp_(std::move(comp)) {
        Sort();
    }

    SortedSequence(const Sequence<T>& data, Comparator comp = Comparator())
        : data_(std::make_shared<ArraySequence<T>>(data)), comp_(std::move(comp)) {
        Sort();
    }

    SortedSequence(Comparator comp = Comparator())
        : data_(std::make_shared<ArraySequence<T>>()), comp_(std::move(comp)) {
    }

    size_t GetLength() const override {
        return data_->GetLength();
    }

    bool GetIsEmpty() const override {
        return GetLength() == 0;
    }

    const T& Get(size_t index) const override {
        return data_->Get(index);
    }

    const T& GetFirst() const override {
        return data_->GetFirst();
    }

    const T& GetLast() const override {
        return data_->GetLast();
    }

    int IndexOf(const T& value) const override {
        auto pos = LowerBound(value);
        if (pos == data_->GetLength() || !IsEqual(data_->Get(pos), value)) {
            return -1;
        }
        return static_cast<int>(pos);
    }

    SortedSequencePtr<T> GetSubsequence(size_t startIndex, size_t endIndex) const override {
        return std::make_shared<SortedSequence<T, Comparator>>(data_->GetSubsequence(startIndex, endIndex), comp_);
    }

    size_t LowerBound(const T& value) const override {
        if (data_->GetLength() == 0 || !comp_(data_->GetFirst(), value)) {
            return 0;
        }
        size_t l = 0;
        size_t r = data_->GetLength();
        while (l + 1 < r) {
            size_t mid = (l + r) / 2;
            if (comp_(data_->Get(mid), value)) {
                l = mid;
            } else {
                r = mid;
            }
        }
        return r;
    }

    bool Contains(const T& value) const {
        return IndexOf(value) != -1;
    }

    void Add(const T& value) override {
        auto pos = LowerBound(value);
        data_->InsertAt(value, pos);
    }

    bool Remove(const T& value) {
        auto idx = IndexOf(value);
        if (idx == -1) {
            return false;
        }
        data_->EraseAt(static_cast<size_t>(idx));
        return true;
    }

    void EraseAt(size_t index) override {
        data_->EraseAt(index);
    }

    void Clear() override {
        data_->Clear();
    }

    IIteratorPtr<T> GetIterator() const override {
        return data_->GetIterator();
    }

private:
    bool IsEqual(const T& a, const T& b) const {
        return !comp_(a, b) && !comp_(b, a);
    }

    void Sort() {
        size_t n = data_->GetLength();
        if (n < 2) {
            return;
        }
        DynamicArray<T> buffer(n);
        MergeSort(0, n, buffer);
    }

    void MergeSort(size_t l, size_t r, DynamicArray<T>& buffer) {
        if (r - l <= 1) {
            return;
        }
        size_t mid = (l + r) / 2;
        MergeSort(l, mid, buffer);
        MergeSort(mid, r, buffer);
        Merge(l, mid, r, buffer);
    }

    void Merge(size_t l, size_t mid, size_t r, DynamicArray<T>& buffer) {
        size_t i = l;
        size_t j = mid;
        size_t k = 0;
        while (i < mid && j < r) {
            if (comp_(data_->Get(j), data_->Get(i))) {
                buffer.Set(data_->Get(j++), k++);
            } else {
                buffer.Set(data_->Get(i++), k++);
            }
        }
        while (i < mid) {
            buffer.Set(data_->Get(i++), k++);
        }
        while (j < r) {
            buffer.Set(data_->Get(j++), k++);
        }
        for (size_t t = 0; t < k; ++t) {
            data_->Set(buffer.Get(t), l + t);
        }
    }

private:
    SequencePtr<T> data_;
    Comparator comp_;
};
