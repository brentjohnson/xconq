// xConq
// Time Functions

// $Id: time.h,v 1.1 2006/04/07 02:13:35 eric_mcdonald Exp $

/*//////////////////////////// COPYRIGHT NOTICE ////////////////////////////////

  Copyright (C) 1987-1989   Stanley T. Shebs
  Copyright (C) 1991-1997   Stanley T. Shebs
  Copyright (C) 1999	    Stanley T. Shebs
  Copyright (C) 2004-2006   Eric A. McDonald

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
    \brief Time functions and constants.
    \ingroup grp_util
*/

#ifndef XCONQ_TIME_H
#define XCONQ_TIME_H

#include "util/base.h"

NAMESPACE_XCONQ_BEGIN
NAMESPACE_UTILITY_BEGIN

// Stopwatch Functions

//! Has certain number of seconds elapsed since stopwatch reset?
extern int n_seconds_elapsed(int n);
//! Has certain number of milliseconds elapsed since stopwatch reset?
extern int n_ms_elapsed(int n);
//! Reset stopwatch.
extern void record_ms(void);

//! Time difference as an long integer number of seconds.
extern long idifftime(time_t t1, time_t t0);

NAMESPACE_UTILITY_END
NAMESPACE_XCONQ_END

#endif // ifndef XCONQ_TIME_H
