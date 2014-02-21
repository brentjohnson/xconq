// Xconq
// Unix-specific Time Functions

// $Id: time.cc,v 1.2 2006/04/07 01:33:25 eric_mcdonald Exp $

/*//////////////////////////// COPYRIGHT NOTICE ////////////////////////////////

  Copyright (C) 1987-1989   Stanley T. Shebs
  Copyright (C) 1991-1997   Stanley T. Shebs
  Copyright (C) 1999        Stanley T. Shebs
  Copyright (C) 2006        Eric A. McDonald

//////////////////////////////////// LICENSE ///////////////////////////////////

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

//////////////////////////////////////////////////////////////////////////////*/

/*! \file
    \brief Unix-specific time functions.
    \ingroup grp_util
*/

#include "util/base.h"
#if TIME_WITH_SYS_TIME
extern "C" {
#   include <sys/time.h>
}
#   include <ctime>
#else
#   if HAVE_SYS_TIME_H
extern "C" {
#	include <sys/time.h>
}
#   else
#	include <ctime>
#   endif
#endif

#include "util/constants.h"
#include "util/time.h"

#ifdef TIME_RETURNS_LONG
#ifndef time_t
//! Define 'time_t' as a 32-bit signed integer type.
/*! Accommodate certain obsolete (pre-ANSI) Unix systems. (?) */
#define time_t long
#endif
#endif

NAMESPACE_XCONQ_BEGIN
NAMESPACE_UTILITY_BEGIN

#if NEED_GETTIMEOFDAY
int gettimeofday(struct timeval *tv, struct timezone *tz)
{
    clock_t clocks = clock ();
    tv->tv_sec = clocks / CLOCKS_PER_SEC;
    tv->tv_usec = (clocks % CLOCKS_PER_SEC) * (1000000 / CLOCKS_PER_SEC);
    return 0;
}
#endif

struct timeval reallasttime = { 0, 0 };

struct timeval realcurtime;

int
n_seconds_elapsed(int n)
{
    gettimeofday(&realcurtime, NULL);
    if (realcurtime.tv_sec > (reallasttime.tv_sec + (n - 1))) {
	reallasttime = realcurtime;
	return TRUE;
    } else {
	return FALSE;
    }
}

/* Returns true if n milliseconds have passed since the time was recorded
   via record_ms(). */

struct timeval reallastmstime = { 0, 0 };

int
n_ms_elapsed(int n)
{
    int interval;
    struct timeval tmprealtime;

    gettimeofday(&tmprealtime, NULL);
    interval =
      (tmprealtime.tv_sec - reallastmstime.tv_sec) * 1000
	+ (tmprealtime.tv_usec - reallastmstime.tv_usec) / 1000;
    return (interval > n);
}

/* Record the current time of day. */

void
record_ms(void)
{
    gettimeofday(&reallastmstime, NULL);
}

NAMESPACE_UTILITY_END
NAMESPACE_XCONQ_END

