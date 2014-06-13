#ifndef __WARN_H
#define __WARN_H

#define LOG __FILE__, __LINE__

void log_info(char* file, int line, char* fmt, ...);
void log_fatal(char* file, int line, int exit_status, char* fmt, ...);

#endif // __WARN_H