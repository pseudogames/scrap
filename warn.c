#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

void log_print(char* type, char* file, int line, char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "%s [%s:%d] ", type, file, line);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fflush(stderr);
}

void log_info(char* file, int line, char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    log_print("INFO", file, line, fmt, args);
    va_end(args);
}

void log_fatal(char* file, int line, int exit_status, char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    log_print("FATAL", file, line, fmt, args);
    va_end(args);
    
    exit(exit_status);
}

