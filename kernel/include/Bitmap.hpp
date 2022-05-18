#pragma once

#include <stdint.h>
#include <types.hpp>
#include <constants.hpp>

class Bitmap
{
public:
    /**
     * @brief           Set the buffer of the bitmap
     *
     * @param buffer    The bitmap buffer array
     * @param size      The size of the buffer
     */
    void setBuffer(uint8_t *buffer, uint64_t size);

    /**
     * @brief           Get the size of the buffer
     *
     * @return uint64_t The size of the buffer
     */
    uint64_t getSize();

    /**
     * @brief           Returns whether the bit at the index is set or not
     *
     * @param index     The index of the bit to check
     * @return true     If the bit is set
     * @return false    If the bit is not set
     */
    bool operator[](uint64_t index);

    /**
     * @brief           Change the value of the bit
     *
     * @param index     The index of the bit to change
     * @param value     Set/Unset the bit
     */
    void set(uint64_t index, bool value);

private:
    uint8_t *_buffer;
    uint64_t _size;
};