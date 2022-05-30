#pragma once

#include <stdint.h>
#include <stddef.h>

int strcmp(const char *X, const char *Y);
int strncmp(const char *s1, const char *s2, size_t n);

char *strchr(const char *p, int ch);

int memcmp(const void *str1, const void *str2, size_t count);

void memcpy(void *dstpp, void *srcpp, size_t len);
void memcpy(void *dstpp, const void *srcpp, size_t len);

void memset(unsigned char *addr, char value, size_t len);
void memset(char *addr, char value, size_t len);
void memset(void *addr, char value, size_t len);

size_t strlen(const char *str);

const char *toString(uint8_t num);
const char *toString(uint16_t num);
const char *toString(uint32_t num);
const char *toString(uint64_t num);

const char *toString(int8_t num);
const char *toString(int16_t num);
const char *toString(int32_t num);
const char *toString(int64_t num);

const char *toHex(uint64_t num, bool ignoreSize);
const char *toHex(uint8_t num);
const char *toHex(uint16_t num);
const char *toHex(uint32_t num);
const char *toHex(uint64_t num);