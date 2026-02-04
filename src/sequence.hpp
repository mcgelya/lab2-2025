#pragma once

#include <iostream>

#include "fwd.hpp"
#include "iiterator.hpp"

template <typename T>
class Sequence : public IIterable<T> {
public:
    virtual ~Sequence() = default;

    virtual const T& GetFirst() const = 0;
    virtual const T& GetLast() const = 0;
    virtual const T& Get(size_t index) const = 0;
    virtual void Set(const T& item, size_t index) = 0;

    virtual SequencePtr<T> GetSubsequence(size_t startIndex, size_t endIndex) const = 0;
    virtual SequencePtr<T> GetFirst(size_t count) const = 0;
    virtual SequencePtr<T> GetLast(size_t count) const = 0;

    virtual size_t GetLength() const = 0;

    virtual size_t GetCapacity() const {
        return GetLength();
    }

    virtual void Append(const T& item) = 0;
    virtual void Prepend(const T& item) = 0;
    virtual void InsertAt(const T& item, size_t index) = 0;
    virtual void EraseAt(size_t index) = 0;

    virtual void Clear() = 0;
};

template <typename T>
std::ostream& operator<<(std::ostream& os, const Sequence<T>& v) {
    os << "{";
    for (auto it = v.GetIterator(); it->HasNext(); it->Next()) {
        os << it->GetCurrentItem() << ", ";
    }
    os << "}";
    return os;
}
