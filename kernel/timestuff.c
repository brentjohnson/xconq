/* Xconq is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.  See the file COPYING.  */

/*! \file missing/util/timestuff.c
 * \brief Provide missing time-related structures and functions.
 *
 * This file is used to provide substitute implementations of 
 * time-related functions that may be missing on some systems.
 */

#ifdef HAVE_ACDEFS_H
#include "acdefs.h"
#endif
#include "timestuff.h"

#if (defined (NEED_GETTIMEOFDAY))
int gettimeofday(struct timeval *tv, struct timezone *tz)
{
#if (defined (UNIX) || defined (WIN32))
        clock_t clocks = clock ();
        tv->tv_sec = clocks / CLOCKS_PER_SEC;
        tv->tv_usec = (clocks % CLOCKS_PER_SEC) * (1000000 / CLOCKS_PER_SEC);
        return 0;
#endif
#ifdef MAC
    int ticks = TickCount();

    tv->tv_sec = ticks / 60;
    tv->tv_usec = (ticks % 60) * (1000000 / 60);
    return 0;
#endif
}
#endif
