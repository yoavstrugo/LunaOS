#pragma once

#include <fatfs/ff.h>

struct FD {
    FIL *file;
    FD *next;
};

