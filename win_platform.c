
#define  _CRT_SECURE_NO_WARNINGS 1
#include <windows.h>

#include <stdio.h>
#include <stdlib.h>

// Some old MinGW/CYGWIN distributions don't define this:
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
    #define ENABLE_VIRTUAL_TERMINAL_PROCESSING  0x0004
#endif

static HANDLE stdoutHandle, stdinHandle;
static DWORD outModeInit, inModeInit;

void setupConsole(void)
{
    DWORD outMode = 0, inMode = 0;
    stdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    stdinHandle = GetStdHandle(STD_INPUT_HANDLE);

    if(stdoutHandle == INVALID_HANDLE_VALUE || stdinHandle == INVALID_HANDLE_VALUE)
    {
        printf("[%s:%u] err-code= %d\n", __func__, __LINE__, GetLastError());
        return;
    }

    if(!GetConsoleMode(stdoutHandle, &outMode) || !GetConsoleMode(stdinHandle, &inMode))
    {
        printf("[%s:%u] err-code= %d\n", __func__, __LINE__, GetLastError());
        return;
    }

    outModeInit = outMode;
    inModeInit = inMode;

    // Enable ANSI escape codes
    outMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

    // Set stdin as no echo and unbuffered
    inMode &= ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT);

    if( !SetConsoleMode(stdoutHandle, outMode) )
    {
        printf("[%s:%u] err-code= %d\n", __func__, __LINE__, GetLastError());
//        return;
    }

    if( !SetConsoleMode(stdinHandle, inMode) )
    {
        printf("[%s:%u] err-code= %d\n", __func__, __LINE__, GetLastError());
//        return;
    }
}

void restoreConsole(void)
{
    // Reset colors
    printf("\x1b[0m");

    // Reset console mode
    if( !SetConsoleMode(stdoutHandle, outModeInit) ||
        !SetConsoleMode(stdinHandle, inModeInit) )
    {
        printf("[%s:%u] err-code= %d\n", __func__, __LINE__, GetLastError());
        return;
    }
}
