#pragma once

#include <stivale2/stivale2.h>
#include <stdarg.h>
#include <logger/logger_macros.hpp>

void loggerInitiallize(stivale2_struct *stivale2Struct);

void loggerPrint(const char *msg, ...);

void loggerPrintln(const char *msg, ...);
