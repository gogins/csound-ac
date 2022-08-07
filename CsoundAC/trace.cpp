// trace.cpp -- debugging print function
// Reimplemented by MKG for CsoundAC in cross-platform style.

// (I think this was created to provide a generic print function
// for use in non-command-line Windows applications where printf
// does not work. Currently, it is not used, but kept around for
// possible debugging needs. -RBD)

#include "stdarg.h"
#include "stdio.h"

#pragma GCC diagnostic ignored "-Wformat-security"

void trace(char *format, ...)
{
    char msg[256];
    va_list args;
    va_start(args, format);
#if defined(WIN32)
    _vsnprintf(msg, 256, format, args);
#else
    vsnprintf(msg, 256, format, args);
#endif
    va_end(args);
#if 0
    // Csound doesn't use or need this.
    _CrtDbgReport(_CRT_WARN, NULL, NULL, NULL, msg);
#else
    printf(msg);
#endif
}
