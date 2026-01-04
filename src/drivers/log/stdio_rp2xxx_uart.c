#include <stdio.h>
#include <stdarg.h>

#define PW_LOG_LEN 1024

static char log_buf[PW_LOG_LEN];

void pw_log_vprintf(const char* fmt, va_list args) {
    vsnprintf(log_buf, PW_LOG_LEN, fmt, args);
    printf(log_buf);

}

