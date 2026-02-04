#pragma once

template <typename T>
class Stream {
public:
    virtual ~Stream() = default;

    virtual bool Read(T& out) = 0;
};
