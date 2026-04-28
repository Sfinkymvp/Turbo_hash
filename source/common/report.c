#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#include "common/report.h"

#define _C_RESET "\033[0m"
#define _C_RED   "\033[1;31m"
#define _C_GRN   "\033[1;32m"
#define _C_YLW   "\033[1;33m"
#define _C_BLU   "\033[1;34m"
#define _C_MAG   "\033[1;35m"
#define _C_CYN   "\033[1;36m"
#define _C_GRAY  "\033[0;37m"

static int _msg_prefix(LogLevel level, FILE *stream, const char *file_path, const char *function, int line);

int _msg_wrapper(LogLevel level, FILE *stream, const char *file_path, const char *function, int line, const char *format, ...)
{
    assert(stream);
    assert(file_path);
    assert(function);
    assert(format);

    int total_printed = 0;
    int result = _msg_prefix(level, stream, file_path, function, line);
    if (result < 0) {
        return result;
    }
    total_printed += result;

    va_list args;
    va_start(args, format);
    result = vfprintf(stream, format, args);
    va_end(args);

    if (result < 0) {
        return result;
    }
    total_printed += result;

    result = fprintf(stream, "\n");
    if (result < 0) {
        return result;
    }
    total_printed += result;

    return total_printed;
}

static int _msg_prefix(LogLevel level, FILE *stream, const char *file_path, const char *function, int line)
{
    assert(stream); assert(file_path); assert(function);

    const char *level_str = "LOG  ";
    const char* level_color = _C_RESET;

    switch (level) {
        case LOG_INFO: {
            level_str = "[INFO] ";
            level_color = _C_GRN;
            break;
        }
        case LOG_DEBUG: {
            level_str = "[DEBUG]";
            level_color = _C_GRAY;
            break;
        }
        case LOG_WARN: {
            level_str = "[WARN] ";
            level_color = _C_YLW;
            break;
        }
        case LOG_ERROR: {
            level_str = "[ERROR]";
            level_color = _C_RED;
            break;
        }
        default: {
            level_str = "[NONE] ";
            level_color = _C_RESET;
        }
    }

    int result = 0;
    if (stream == stdout || stream == stderr) {
        result = fprintf(stream,
            "%s%s " _C_MAG "[%s] " _C_CYN "[%s:%d] " _C_RESET,
            level_color, level_str, file_path, function, line);
    } else {
        result = fprintf(stream,
            "[%s] [%s] [%s:%d] ",
            level_str, file_path, function, line);
    }

    return result;
}
