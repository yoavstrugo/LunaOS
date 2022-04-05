#pragma once

#include <Renderer.hpp>
#include <stdarg.h>

void loggerInitiallize(Renderer *renderer);

void loggerPrint(const char *msg, ...);

void loggerPrintln(const char *msg, ...);
