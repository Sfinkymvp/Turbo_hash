#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#include "common/report.h"

#define _BRED "\033[1;31m"
#define _BMAG "\033[1;35m"
#define _BCYN "\033[1;36m"
#define _CRESET "\033[0m"

static int _msg_prefix(FILE *stream, const char *file_path, const char *function, int line);

int _msg_wrapper(FILE *stream, const char *file_path, const char *function, int line, const char *format, ...)
{
    assert(stream);
    assert(file_path);
    assert(function);
    assert(format);

    int total_printed = 0;
    int result = _msg_prefix(stream, file_path, function, line);
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

int _msg_prefix(FILE *stream, const char *file_path, const char *function, int line)
{
    assert(stream); assert(file_path); assert(function);

    int result = 0;
    if (stream == stdout || stream == stderr) {
        result = fprintf(stream,
            _BRED "[ERROR] " _BMAG "[%s] " _BCYN "[%s:%d] " _CRESET,
            file_path, function, line);
    } else {
        result = fprintf(stream,
            "[ERROR] [%s] [%s:%d] ",
            file_path, function, line);
    }

    return result;
}

