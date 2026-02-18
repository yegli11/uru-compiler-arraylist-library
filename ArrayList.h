#pragma once
#ifndef ARRAYLIST_H
#define ARRAYLIST_H

#include <stdexcept>
#if __cplusplus >= 201103L
#define MY_NULLPTR nullptr
#else
#define MY_NULLPTR NULL
#endif

template <typename T>
class ArrayList {
private:
    T* items;
    int length;
    int capacity;
    int cursor;  

public:
    ArrayList()
        : items(MY_NULLPTR),
        length(0),
        capacity(8),
        cursor(0)
    {
        items = new T[capacity];
    }

    ~ArrayList() {
        delete[] items;
    }

    void add(T value) {
        if (length == capacity) {
            int newCapacity = capacity * 2;
            T* newItems = new T[newCapacity];

            for (int i = 0; i < length; i++) {
                newItems[i] = items[i];
            }

            delete[] items;
            items = newItems;
            capacity = newCapacity;
        }

        items[length] = value;
        length++;
    }

    T get(int index) {
        if (index < 0 || index >= length) {
            throw std::out_of_range("Index out of range");
        }
        return items[index];
    }

    void remove() {
        if (length == 0) {
            throw std::out_of_range("The array list is empty");
        }
        if (cursor < 0 || cursor >= length) {
            throw std::out_of_range("cursor index out of range");
        }

        for (int i = cursor; i < length - 1; i++) {
            items[i] = items[i + 1];
        }

        length--;

        if (length == 0) {
            cursor = 0;
        }
        else if (cursor >= length) {
            cursor = length - 1;
        }
    }

    void next() {
        if (length == 0) {
            throw std::out_of_range("The array list is empty");
        }
        if (cursor >= length - 1) {
            throw std::out_of_range("Next is out of range");
        }
        cursor++;
    }

    void prior() {
        if (length == 0) {
            throw std::out_of_range("The array list is empty");
        }
        if (cursor <= 0) {
            throw std::out_of_range("Previous is out of range");
        }
        cursor--;
    }

    T first() {
        if (length == 0) {
            throw std::out_of_range("The array list is empty");
        }
        return items[0];
    }

    T last() {
        if (length == 0) {
            throw std::out_of_range("The array list is empty");
        }
        return items[length - 1];
    }

    T current() {
        if (length == 0) {
            throw std::out_of_range("The array list is empty");
        }
        return items[cursor];
    }

    int size() {
        return length;
    }
};

#endif
