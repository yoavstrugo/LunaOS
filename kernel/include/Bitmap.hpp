#pragma once

#include <stdint.h>
#include <types.h>
#include <constants.h>

class Bitmap {
    public:
        /**
         * @brief           Set the buffer of the bitmap
         * 
         * @param buffer    The bitmap buffer array
         */
        void setBuffer(uint8_t *buffer);

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
};