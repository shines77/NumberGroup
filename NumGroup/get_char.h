
#ifndef JM_GET_CHAR_H
#define JM_GET_CHAR_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* For Keyboard virtual code */

enum MM_VT_KEYBOARDS {
    //
    MM_VT_KEY_FIRST = 0,
    MM_VT_KEY_TAB = 8,
    MM_VT_KEY_RETURN = 13,
    MM_VT_KEY_LAST = 255
};

#ifndef _USE_GETCHAR
  #ifdef __linux__
    #define _USE_GETCHAR        1
  #else
    #define _USE_GETCHAR        0
  #endif
#endif  /* _USE_GETCHAR */

#define GETCH_DEFUALT_VALUE     (-1)
#define GETCH_EXIT_PROGRAM      (0)

/// <comment>
///
/// What is equivalent to getch() & getche() in Linux?
///
/// From: http://stackoverflow.com/questions/7469139/what-is-equivalent-to-getch-getche-in-linux
///
/// </comment>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__linux__)

void init_terminal_os(int echo);
void reset_terminal_os(void);

/* Read 1 character - echo defines echo mode */
char jm_getch_(int echo);

#endif  /* __linux__ */

/* Read 1 character without echo. */
char jm_getch(void);

/* Read 1 character with echo. */
char jm_getche(void);

/* Read 1 character without echo and new line. */
char jm_readkey(void);

/* Read 1 character with echo and new line. */
char jm_readkey_echo(void);

/* Sleep() in Windows, usleep() in Linux. */
void jm_sleep(int millisec);

#ifdef __cplusplus
}
#endif

#endif  /* JM_GET_CHAR_H */