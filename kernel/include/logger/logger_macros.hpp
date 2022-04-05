#pragma once

#include <logger/logger_log_level.hpp>


#define K_LOG_LEVEL     K_LOG_LEVEL_DEBUG

#if K_LOG_LEVEL <= K_LOG_LEVEL_INFO
#define logInfo(msg...)			loggerPrint(msg)
#define logInfon(msg...)		loggerPrintln(msg)
#define K_LOGGING_INFO			true
#else
#define logInfo(msg...)			{};
#define logInfon(msg...)		{};
#define K_LOGGING_INFO			false
#endif

#if K_LOG_LEVEL <= K_LOG_LEVEL_WARN
#define logWarn(msg...)			loggerPrint(msg)
#define logWarnn(msg...)		loggerPrintln(msg)
#define K_LOGGING_WARN			true
#else
#define logWarn(msg...)			{};
#define logWarnn(msg...)		{};
#define K_LOGGING_WARN			false
#endif

#if G_LOG_LEVEL <= G_LOG_LEVEL_DEBUG
#define logDebug(msg...)		loggerPrint(msg)
#define logDebugn(msg...)		loggerPrintln(msg)
#define K_LOGGING_DEBUG			true
#else
#define logDebug(msg...)		{};
#define logDebugn(msg...)		{};
#define K_LOGGING_DEBUG			false
#endif

