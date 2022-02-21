//
// Created by sebastian on 21.02.22.
//

#ifndef RAYTRACEENGINE_ARRAY_H
#define RAYTRACEENGINE_ARRAY_H

#include <cstdint>
#include <stdexcept>

template<class T>
class Array {
public:
    T *array;
    uint64_t size;
    bool moved_from;

public:
    explicit Array(uint64_t size) : size(size), moved_from(false) {
        array = new T[size];
    }

    Array(const Array &) = delete;

    Array(Array &&other) noexcept {
        other.moved_from = true;
        array = other.array;
        size = other.size;
        moved_from = false;
    }

    Array &operator=(const Array &) = delete;

    Array &operator=(Array &&other) noexcept {
        other.moved_from = true;
        array = other.array;
        size = other.size;
        moved_from = false;
        return *this;
    }

    ~Array() {
        if (!moved_from)
            delete[] array;
    }

    T &at(uint64_t key) {
        if (key >= size)
            throw std::out_of_range("key is out of range of array");
        return array[key];
    }
};

#endif //RAYTRACEENGINE_ARRAY_H
