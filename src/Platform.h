#pragma once

#include <signal.h>
#include <stdint.h>
#include <stdio.h>

/* windows only */
#include <Windows.h>
#include <conio.h> // _kbhit

inline HANDLE hStdin = INVALID_HANDLE_VALUE;
inline DWORD fdwMode, fdwOldMode;

inline void disable_input_buffering() {
  hStdin = GetStdHandle(STD_INPUT_HANDLE);
  GetConsoleMode(hStdin, &fdwOldMode);     /* save old mode */
  fdwMode = fdwOldMode ^ ENABLE_ECHO_INPUT /* no input echo */
            ^ ENABLE_LINE_INPUT;           /* return when one or
                                              more characters are available */
  SetConsoleMode(hStdin, fdwMode);         /* set new mode */
  FlushConsoleInputBuffer(hStdin);         /* clear buffer */
}

inline void restore_input_buffering() { SetConsoleMode(hStdin, fdwOldMode); }

inline uint16_t check_key() {
  return WaitForSingleObject(hStdin, 1000) == WAIT_OBJECT_0 && _kbhit();
}
