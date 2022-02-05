#include <Bitmap.hpp>

void Bitmap::setBuffer(uint8_t *buffer) {
    this->_buffer = buffer;
}

bool Bitmap::operator[](uint64_t index) {
    // We will consider the buffer as a byte array, therefore we want
    // to get the byte that 'holds' the bit.
    uint64_t byteIndex = index / BYTE_SIZE;

    // The remainder is the index of the bit within the byte.
    uint8_t bitIndex = index % BYTE_SIZE;

    // Create a mask to read the n-th bit
    uint8_t mask = 1 << bitIndex;

    // Check the bit with mask
    return ((uint8_t *)this->_buffer)[byteIndex] & mask;
}

void Bitmap::set(uint64_t index, bool value) {
    // We will consider the buffer as a byte array, therefore we want
    // to get the byte that 'holds' the bit.
    uint64_t byteIndex = index / BYTE_SIZE;

    // The remainder is the index of the bit within the byte.
    uint8_t bitIndex = index % BYTE_SIZE;

    // Create a mask for the n-th bit
    uint8_t mask = 1 << bitIndex;

    // Unset the bit
    this->_buffer[byteIndex] &= ~mask;

    // Set/Unset the bit
    if (value)
        this->_buffer[byteIndex] |= mask;
}