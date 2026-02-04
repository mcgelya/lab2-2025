#pragma once

#include <memory>

template <typename T>
class IIterator {
public:
    virtual ~IIterator() = default;

    virtual bool HasNext() const = 0;

    virtual bool Next() = 0;

    virtual const T& GetCurrentItem() const = 0;

    virtual bool TryGetCurrentItem(T& element) const = 0;
};

template <typename T>
using IIteratorPtr = std::shared_ptr<IIterator<T>>;

template <typename T>
class IIterable {
public:
    virtual IIteratorPtr<T> GetIterator() const = 0;
};
