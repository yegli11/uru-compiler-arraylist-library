#pragma once
#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <stdexcept>
#if __cplusplus >= 201103L
#define MY_NULLPTR nullptr
#else
#define MY_NULLPTR NULL
#endif

template <typename T>
struct Node {
    T data;
    Node<T>* next;
    Node<T>* prev;
};
template <typename T>
class LinkedList {
private:
    Node<T>* cursor;
    Node<T>* head;
    Node<T>* tail;
    int length;

public:
    LinkedList()
        : cursor(MY_NULLPTR), head(MY_NULLPTR), tail(MY_NULLPTR), length(0)
    {
    }
    ~LinkedList() {
        Node<T>* cursor = head;
        while (cursor != MY_NULLPTR)
        {
            Node<T>* next = cursor->next;
            delete cursor;
            cursor = next;
        }
    };

    void add(T value) {
        Node<T>* newNode = new Node<T>;
        if (length == 0) {
            newNode->data = value;
            newNode->next = MY_NULLPTR;
            newNode->prev = MY_NULLPTR;
            
            head = newNode;
            tail = newNode;
            cursor = newNode;

            length++;
        } else {
            newNode->data = value;
            newNode->next = MY_NULLPTR;
            newNode->prev = tail;

            tail->next = newNode;
            tail = newNode;
            cursor = newNode;

            length++;
        }
    };
    T get(int index) {
        if (index < 0 || index >= length) {
            throw std::out_of_range("Index out of range");
        }
        Node<T>* cursor = head;
        for (int i = 1; i <= index; i++)
        {
            cursor = cursor->next;
        }
        return cursor->data;
    };
    void remove() {
        if (cursor == MY_NULLPTR) {
            throw std::out_of_range("List is empty");
        }
        Node<T>* temp = cursor;
        if (cursor == head) {
            head = head->next;
        }
        if (cursor == tail) {
            tail = tail->prev;
        }
        if (cursor->prev != MY_NULLPTR) {
            cursor->prev->next = cursor->next;
        }
        if (cursor->next != MY_NULLPTR) {
            cursor->next->prev = cursor->prev;
        }
        if (cursor->next != MY_NULLPTR) {
            cursor = cursor->next;
        }
        else if (cursor->prev != MY_NULLPTR) {
            cursor = cursor->prev;
        }
        else {
            cursor = MY_NULLPTR;
        }
        delete temp;

        length--;
    };
    void current() {
        if (cursor == MY_NULLPTR) {
            throw std::out_of_range("List is empty");
        }
        return cursor->data;
    };
    void next() {
        if (cursor == MY_NULLPTR) {
            throw std::out_of_range("List is empty");
        }
        if (cursor->next == MY_NULLPTR) {
            throw std::out_of_range("Next is null");
        }
        cursor = cursor->next;
    };
    void prior() {
        if (cursor == MY_NULLPTR) {
            throw std::out_of_range("List is empty");
        }
        if (cursor->prev == MY_NULLPTR) {
            throw std::out_of_range("Prev is null");
        }
        cursor = cursor->prev;
    };
    T first() {
        if (head == MY_NULLPTR) {
            throw std::out_of_range("List is empty");
        }
        return head->data;
    };
    T last() {
        if (tail == MY_NULLPTR) {
            throw std::out_of_range("List is empty");
        }
        return tail->data;
    };
    int size() {
        return length;
    };
};

#endif
#pragma once
