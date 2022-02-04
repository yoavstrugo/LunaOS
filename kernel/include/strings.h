#pragma once

#include <stdint.h>
#include <stddef.h>

int strcmp(const char *X, const char *Y);
int strncmp (const char *s1, const char *s2, size_t n);

const char *toString(uint8_t     num);
const char *toString(uint16_t    num);
const char *toString(uint32_t    num);
const char *toString(uint64_t    num);

const char *toString(int8_t      num);
const char *toString(int16_t     num);
const char *toString(int32_t     num);
const char *toString(int64_t     num);

const char *toHex(uint64_t num, bool ignoreSize);
const char *toHex(uint8_t     num);
const char *toHex(uint16_t    num);
const char *toHex(uint32_t    num);
const char *toHex(uint64_t    num);