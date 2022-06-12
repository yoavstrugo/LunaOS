#pragma once

#include <memory/heap.hpp>
#include <stddef.h>

template <class T>
struct List
{

    List(int initSize)
    {
        buffer = NULL; // or buffer(0) if not in C++11
        cnt = 0;
        totalSize = 0;
        if (initSize == 0)
            initSize = 1;
        resize(initSize);
    }

    ~List() { delete[] buffer; }

    void add(T t)
    {
        if (cnt == totalSize)
            resize(totalSize * 2);

        buffer[cnt++] = t;
    }

    int size() { return cnt; }

    T get(int index) { return buffer[index]; }

private:
    T *buffer;
    int cnt;
    int totalSize;

    void resize(int newSize)
    {
        T *temp = new T[newSize];

        if (buffer)
        {
            for (int i = 0; i < cnt; i++)
                temp[i] = buffer[i];

            delete[] buffer;
        }

        buffer = temp;
        totalSize = newSize;
    }
};