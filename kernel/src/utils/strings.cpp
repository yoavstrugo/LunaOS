#include "strings.h"

// Function to implement strcmp function
int strcmp(const char *X, const char *Y)
{
    while (*X)
    {
        // if characters differ, or end of the second string is reached
        if (*X != *Y) {
            break;
        }
 
        // move to the next pair of characters
        X++;
        Y++;
    }
 
    // return the ASCII difference after converting `char*` to `unsigned char*`
    return *(const unsigned char*)X - *(const unsigned char*)Y;
}

int strncmp (const char *s1, const char *s2, size_t n)
{
  unsigned char c1 = '\0';
  unsigned char c2 = '\0';

  if (n >= 4)
    {
      size_t n4 = n >> 2;
      do
	{
	  c1 = (unsigned char) *s1++;
	  c2 = (unsigned char) *s2++;
	  if (c1 == '\0' || c1 != c2)
	    return c1 - c2;
	  c1 = (unsigned char) *s1++;
	  c2 = (unsigned char) *s2++;
	  if (c1 == '\0' || c1 != c2)
	    return c1 - c2;
	  c1 = (unsigned char) *s1++;
	  c2 = (unsigned char) *s2++;
	  if (c1 == '\0' || c1 != c2)
	    return c1 - c2;
	  c1 = (unsigned char) *s1++;
	  c2 = (unsigned char) *s2++;
	  if (c1 == '\0' || c1 != c2)
	    return c1 - c2;
	} while (--n4 > 0);
      n &= 3;
    }

  while (n > 0)
    {
      c1 = (unsigned char) *s1++;
      c2 = (unsigned char) *s2++;
      if (c1 == '\0' || c1 != c2)
	return c1 - c2;
      n--;
    }

  return c1 - c2;
}

// toString methods
#define MAX_STRING_LENGTH 128
char _toStringOut[MAX_STRING_LENGTH];

void _toString(uint64_t num, size_t size, char *array) {
    // Calculate the length of the number
    uint8_t length = 0;
    {
        uint64_t tempNum = num;
        while (tempNum > 0) {
            length++;
            tempNum /= 10;
        }
    }
    
    // Make a string of the number
    uint8_t idx = 1;
    while (num > 0) {
        uint8_t digit = num % 10;
        array[length - idx] = '0' + digit;
        num /= 10;
        idx++;
    }

    // Terminate the string
    array[length] = NULL;
}

const char *toString(uint8_t num) {
    _toString((uint64_t)num, sizeof(uint8_t), _toStringOut);
    return _toStringOut;
}

const char *toString(uint16_t num) {
    _toString((uint64_t)num, sizeof(uint16_t), _toStringOut);
    return _toStringOut;
}

const char *toString(uint32_t num) {
    _toString((uint64_t)num, sizeof(uint32_t), _toStringOut);
    return _toStringOut;
}

const char *toString(uint64_t num) {
    _toString(num, sizeof(uint64_t), _toStringOut);
    return _toStringOut;
}

void _toString(int64_t num, size_t size) {
    if (num < 0) {
        _toStringOut[0] = '-';
        _toString((uint64_t)(num * -1), size, _toStringOut + 1);
    } else {
        _toString((uint64_t)num, size, _toStringOut);
    }
}

const char *toString(int8_t num) {
    _toString((int64_t)num, sizeof(num));
    return _toStringOut;
}

const char *toString(int16_t num) {
    _toString((int64_t)num, sizeof(num));
    return _toStringOut;
}

const char *toString(int32_t num) {
    _toString((int64_t)num, sizeof(num));
    return _toStringOut;
}

const char *toString(int64_t num) {
    _toString((int64_t)num, sizeof(num));
    return _toStringOut;
}

const char *toHex(uint64_t num, bool ignoreSize) {

    int length = 0; 
    {
        uint64_t tempNum = num;
        while (tempNum > 0) {
            tempNum /= 10;
            length++;
        }
    }

    for (int i = length - 1; i >= 0; i--) {
        uint8_t dig = num % 16;
        char hex_chr = dig < 10 ? ('0' + dig) : ('A' + (dig - 10));
        _toStringOut[i] = hex_chr;
        num /= 16;
    }

    _toStringOut[length] = NULL;

    return _toStringOut;
}

const char *_toHex(uint64_t num, size_t n) {
    uint8_t length = n * 2;

    for (int i = length - 1; i >= 0; i--) {
        uint8_t dig = num % 16;
        char hex_chr = dig < 10 ? ('0' + dig) : ('A' + (dig - 10));
        _toStringOut[i] = hex_chr;
        num /= 16;
    }

    _toStringOut[length] = NULL;

    return _toStringOut;
}

const char *toHex(uint8_t num) {
    return _toHex((uint64_t)num, sizeof(num));
}

const char *toHex(uint16_t num) {
    return _toHex((uint64_t)num, sizeof(num));
}

const char *toHex(uint32_t num) {
    return _toHex((uint64_t)num, sizeof(num));
}

const char *toHex(uint64_t num) {
    return _toHex((uint64_t)num, sizeof(num));
}