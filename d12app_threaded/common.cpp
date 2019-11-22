#include "common.h"

void log(const char *fmt, ...)
{
    char newLineFmt[512];
    strncpy_s(newLineFmt, sizeof(newLineFmt) - 1, fmt, _TRUNCATE);
    strcat_s(newLineFmt, sizeof(newLineFmt), "\n");

    char msg[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf_s(msg, sizeof(msg), _TRUNCATE, newLineFmt, args);
    va_end(args);

    OutputDebugStringA(msg);
}

void logHr(const char *msg, HRESULT hr)
{
    _com_error err(hr, nullptr);
#ifdef UNICODE
    log("%s: %ls", msg, err.ErrorMessage());
#else
    log("%s: %s", msg, err.ErrorMessage());
#endif
}
