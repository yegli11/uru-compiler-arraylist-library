#pragma once
#ifndef LINKEDLIST_H
#define LINKEDLIST_H
#include <stdexcept>

template <typename T>
struct Node {
    T data; 
	Node<T>* next; 
    Node<T>* prev;
};
template <typename T>
class LinkedList {
private:
	Node<T>* cursor; //direccion de memoria del valor actual
    Node<T>* head;
    Node<T>* tail;
    int length;

public:
    LinkedList()
        : cursor(nullptr), head(nullptr), tail(nullptr), length(0)
    {
    }
    ~LinkedList() { 
        Node<T>* cursor = head;
        while (cursor != nullptr)
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
            newNode->next = nullptr;
            newNode->prev = nullptr;
            
            head = newNode;
            tail = newNode;
            cursor = newNode;

            length++;
        } else {
            newNode->data = value;
            newNode->next = nullptr;
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
        if (cursor == nullptr) {
            throw std::out_of_range("List is empty");
        }
        Node<T>* temp = cursor; 
        if (cursor == head) {
            head = head->next;
        }
        if (cursor == tail) {
            tail = tail->prev;
        }
        if (cursor->prev != nullptr) {
            cursor->prev->next = cursor->next;
        }
        if (cursor->next != nullptr) {
            cursor->next->prev = cursor->prev;
        }
        if (cursor->next != nullptr) {
            cursor = cursor->next;
        }
        else if (cursor->prev != nullptr) {
            cursor = cursor->prev;
        }
        else {
            cursor = nullptr;
        }
        delete temp;

        length--;
    };
    T current() {
        if (cursor == nullptr) {
            throw std::out_of_range("List is empty");
        }
        return cursor->data;
    };
    void next() {
        if (cursor == nullptr) {
            throw std::out_of_range("List is empty");
        }
        if (cursor->next == nullptr) {
            throw std::out_of_range("Next is null");
        }
        cursor = cursor->next;
    };
    void prior() {
        if (cursor == nullptr) {
            throw std::out_of_range("List is empty");
        }
        if (cursor->prev == nullptr) {
            throw std::out_of_range("Prev is null");
        }
        cursor = cursor->prev;
    };
    T first() {
        if (head == nullptr) {
            throw std::out_of_range("List is empty");
        }
        return head->data;
    };
    T last() {
        if (tail == nullptr) {
            throw std::out_of_range("List is empty");
        }
        return tail->data;
    };
    int size() {
        return length;
    };

    T peek() {
        if (cursor == nullptr) {
            throw std::out_of_range("List is empty");
        }
        if (cursor->next == nullptr) {
            throw std::out_of_range("Next is null");
        }
        return cursor->next->data;
	};
};

#endif
#pragma once
