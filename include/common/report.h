#ifndef REPORT_H
#define REPORT_H

#include <stddef.h>
#include <string.h>
#include <stdarg.h>

#define __FILENAME__ \
    (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define REPORT(report_stream, format, ...) \
    report_wrapper(report_stream, __FILENAME__, __func__, __LINE__, format, ##__VA_ARGS__)

int report_wrapper(FILE* stream, const char* filename, const char* function, int line, const char* format, ...);

#endif // REPORT_H