#pragma once

#include <logger_log_level.hpp>


#define K_LOG_LEVEL     K_LOG_LEVEL_DEBUG

#if K_LOG_LEVEL <= K_LOG_LEVEL_INFO
#define logInfo(msg...)			loggerPrintln(msg)
#define logInfon(msg...)		loggerPrint(msg)
#define K_LOGGING_INFO			true
#else
#define logInfo(msg...)			{};
#define logInfon(msg...)		{};
#define K_LOGGING_INFO			false
#endif

#if K_LOG_LEVEL <= K_LOG_LEVEL_WARN
#define logWarn(msg...)			loggerPrintln(msg)
#define logWarnn(msg...)		loggerPrint(msg)
#define K_LOGGING_WARN			true
#else
#define logWarn(msg...)			{};
#define logWarnn(msg...)		{};
#define K_LOGGING_WARN			false
#endif

#if G_LOG_LEVEL <= G_LOG_LEVEL_DEBUG
#define logDebug(msg...)		loggerPrintln(msg)
#define logDebugn(msg...)		loggerPrint(msg)
#define K_LOGGING_DEBUG			true
#else
#define logDebug(msg...)		{};
#define logDebugn(msg...)		{};
#define K_LOGGING_DEBUG			false
#endif

