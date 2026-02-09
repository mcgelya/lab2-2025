#pragma once

#include <cstddef>

template <typename T>
class Stream {
public:
    virtual ~Stream() = default;

    virtual bool Read(T& out) = 0;

    virtual bool IsEnd() const = 0;

    virtual bool Seek(size_t /*pos*/) {
        return false;
    }
};
