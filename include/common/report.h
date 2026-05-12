#ifndef REPORT_H
#define REPORT_H

#include <stddef.h>
#include <string.h>
#include <stdarg.h>

#define __FILENAME__ \
    (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

typedef enum {
    LOG_INFO,
    LOG_DEBUG,
    LOG_WARN,
    LOG_ERROR
} LogLevel;

#if !defined(DISABLE_LOGS)
    #define _LOG_ALL(level, stream, format, ...) \
        _msg_wrapper(level, stream, __FILENAME__, __func__, __LINE__, format, ##__VA_ARGS__)

    #define INFO_F(stream,  format, ...) _LOG_ALL(LOG_INFO,  stream, format, ##__VA_ARGS__)
    #define DEBUG_F(stream, format, ...) _LOG_ALL(LOG_DEBUG, stream, format, ##__VA_ARGS__)
    #define WARN_F(stream,  format, ...) _LOG_ALL(LOG_WARN,  stream, format, ##__VA_ARGS__)
    #define ERROR_F(stream, format, ...) _LOG_ALL(LOG_ERROR, stream, format, ##__VA_ARGS__)

    #define INFO(format,  ...) INFO_F(stderr,  format, ##__VA_ARGS__)
    #define DEBUG(format, ...) DEBUG_F(stderr, format, ##__VA_ARGS__)
    #define WARN(format,  ...) WARN_F(stderr , format, ##__VA_ARGS__)
    #define ERROR(format, ...) ERROR_F(stderr, format, ##__VA_ARGS__)

#else
    #define INFO_F(stream,  format, ...) ((void)0)
    #define DEBUG_F(stream, format, ...) ((void)0)
    #define WARN_F(stream,  format, ...) ((void)0)
    #define ERROR_F(stream, format, ...) ((void)0)

    #define INFO(format,  ...) ((void)0)
    #define DEBUG(format, ...) ((void)0)
    #define WARN(format,  ...) ((void)0)
    #define ERROR(format, ...) ((void)0)

#endif // DISABLE_LOGS
    
int _msg_wrapper(LogLevel level, FILE *stream, const char *filename, const char *function, int line, const char *format, ...);

#endif // REPORT_H