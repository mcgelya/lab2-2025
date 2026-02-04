#pragma once

#include <stdexcept>
#include <string>

template <typename T>
class DynamicArray {
public:
    DynamicArray(const T* items, size_t count) : size_(count) {
        data_ = new T[size_]();
        for (size_t i = 0; i < size_; ++i) {
            data_[i] = items[i];
        }
    }

    DynamicArray() {
    }

    DynamicArray(size_t size) : size_(size) {
        if (size != 0) {
            data_ = new T[size]();
        }
    }

    DynamicArray(const DynamicArray<T>& v) : size_(v.size_) {
        if (size_ != 0) {
            data_ = new T[size_]();
            for (size_t i = 0; i < size_; ++i) {
                data_[i] = v.data_[i];
            }
        }
    }

    DynamicArray<T>& operator=(const DynamicArray<T>& v) {
        if (data_ != nullptr) {
            delete[] data_;
        }
        size_ = v.size_;
        if (size_ != 0) {
            data_ = new T[size_]();
            for (size_t i = 0; i < size_; ++i) {
                data_[i] = v.data_[i];
            }
        }
        return *this;
    }

    DynamicArray<T>& operator=(DynamicArray<T>&& v) {
        if (data_ != nullptr) {
            delete[] data_;
        }
        size_ = v.size_;
        data_ = v.data_;
        v.size_ = 0;
        v.data_ = nullptr;
        return *this;
    }

    DynamicArray(DynamicArray<T>&& v) : size_(v.size_), data_(v.data_) {
        v.size_ = 0;
        v.data_ = nullptr;
    }

    ~DynamicArray() {
        if (data_ != nullptr) {
            delete[] data_;
        }
    }

    const T& Get(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Index is out of range: " + std::to_string(index) + " " + std::to_string(size_));
        }
        return data_[index];
    }

    size_t GetSize() const {
        return size_;
    }

    void Set(const T& item, size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Index is out of range: " + std::to_string(index) + " " + std::to_string(size_));
        }
        data_[index] = item;
    }

    void Resize(size_t newSize) {
        if (newSize < size_) {
            size_ = newSize;
            return;
        }
        T* newData = new T[newSize]();
        for (size_t i = 0; i < size_; ++i) {
            newData[i] = data_[i];
        }
        delete[] data_;
        data_ = newData;
        size_ = newSize;
    }

    const T* GetBegin() const {
        return data_;
    }

private:
    size_t size_ = 0;
    T* data_ = nullptr;
};
