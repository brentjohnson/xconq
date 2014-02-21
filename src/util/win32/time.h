/* Xconq is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.  See the file COPYING.  */

/*! \file 
 * \brief Provide missing time-related structures and functions.
 *
 * This include file is used to provide structure definitions and 
 * function prototypes for time-related things that may be missing 
 * on some systems.
 */

#ifndef _TIMESTUFF_H_
#define _TIMESTUFF_H_

#if (0)
#ifdef __cplusplus
extern "C" {
#endif
#include <time.h>
/* CodeWarrior has sys/time.h only if GUSI is included. */ 
/* All Unix, plus Cygwin and MinGW32 have sys/time.h. */
#if (defined (__CYGWIN__) || defined (__MINGW32__) || defined (UNIX) || defined (GUSI))
#include <sys/time.h>
#endif
#ifdef __cplusplus
}
#endif
#endif

#if (defined (__MWERKS__) || defined (_MSC_VER) || defined (MAC))
#if (defined (_MSC_VER) || (defined (MAC) && !defined (GUSI)))
#ifndef NEED_STRUCT_TIMEVAL
#define NEED_STRUCT_TIMEVAL
#endif
#endif
#ifndef NEED_GETTIMEOFDAY
#define NEED_GETTIMEOFDAY
#endif
#endif

#ifdef NEED_STRUCT_TIMEVAL
struct timeval {
        time_t tv_sec;
        int tv_usec;
};
#endif

#ifdef NEED_STRUCT_TIMEZONE
struct timezone {
        int tz_minuteswest;
        int tz_disttime;
};
#endif

#ifdef NEED_GETTIMEOFDAY
extern int gettimeofday (struct timeval *tv, struct timezone *tz);
#endif

#endif /* ifndef _TIMESTUFF_H_ */
