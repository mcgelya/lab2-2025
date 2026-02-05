#pragma once

#include "fwd.hpp"
#include "iiterator.hpp"

template <typename T>
class ISortedSequence : public IIterable<T> {
public:
    virtual ~ISortedSequence() = default;

    virtual size_t GetLength() const = 0;
    virtual bool GetIsEmpty() const = 0;

    virtual const T& Get(size_t index) const = 0;
    virtual const T& GetFirst() const = 0;
    virtual const T& GetLast() const = 0;

    virtual int IndexOf(const T& value) const = 0;
    virtual SortedSequencePtr<T> GetSubsequence(size_t startIndex, size_t endIndex) const = 0;

    virtual size_t LowerBound(const T& value) const = 0;

    virtual void Add(const T& value) = 0;
    virtual void EraseAt(size_t index) = 0;
    virtual void Clear() = 0;
};
