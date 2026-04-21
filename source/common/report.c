#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#include "common/report.h"


#define BRED "\033[1;31m"
#define BMAG "\033[1;35m"
#define BCYN "\033[1;36m"
#define CRESET "\033[0m"


static int report_prefix(FILE* stream, const char* filename, const char* function, int line);


int report_wrapper(FILE* stream, const char* filename, const char* function, int line, const char* format, ...)
{
    assert(stream); assert(filename); assert(function); assert(format);


    int total_printed = 0;
    int result = report_prefix(stream, filename, function, line);
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


int report_prefix(FILE* stream, const char* filename, const char* function, int line)
{
    assert(stream); assert(filename); assert(function);

    int result = 0;
    if (stream == stdout || stream == stderr) {
        result = fprintf(stream,
            BRED "[ERROR] " BMAG "[%s] " BCYN "[%s:%d] " CRESET,
            filename, function, line);
    } else {
        result = fprintf(stream,
            "[ERROR] [%s] [%s:%d] ",
            filename, function, line);
    }

    return result;
}