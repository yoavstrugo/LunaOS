#include <logger.hpp>

static Renderer *loggerRenderer = NULL;

void loggerInitiallize(Renderer *renderer) {
    loggerRenderer = renderer;
}

void loggerPrint(char *msg, ...) {
    va_list valist;
    va_start(valist, msg);
    loggerRenderer->printf(msg, valist);
    va_end(valist);
}

void loggerPrintln(char *msg, ...) {
    va_list valist;
    va_start(valist, msg);
    loggerRenderer->printf(msg, valist);
    loggerRenderer->putchar('\n');
    va_end(valist);
}