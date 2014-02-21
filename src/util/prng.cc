// Xconq
// Pseudo-random Number Generation Functions

// $Id: prng.cc,v 1.1 2006/04/07 01:33:24 eric_mcdonald Exp $

/*//////////////////////////// COPYRIGHT NOTICE ////////////////////////////////

  Copyright (C) 1987-1989   Stanley T. Shebs
  Copyright (C) 1991-1997   Stanley T. Shebs
  Copyright (C) 1999        Stanley T. Shebs
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
    \brief Pseudo-random number generation functions.
    \ingroup grp_util

    \todo Although probably not necessary, it would be fun to include a 
	  Mersenne Twister implementation, instead of just a LCRNG.

    \todo Handle 64-bit signed integer type in 'basetypes/primitives.h',
	  not here.
*/

#include <ctime>
#include "util/math.h"
#include "util/debug.h"
#include "util/prng.h"

NAMESPACE_XCONQ_BEGIN
NAMESPACE_UTILITY_BEGIN

// State vars for pseudo-random number generator (PRNG).

//! Initial state of PRNG.
/*! The random state *must* be at least 32 bits. */
Z32 initrandstate = -1;
//! Current state of PRNG.
/*! The random state *must* be at least 32 bits. */
Z32 randstate = -1;

// PRNG Functions

//! Initialize PRNG.
/*! Seed can come from elsewhere, for repeatability. Otherwise, it comes
    from the current time, scaled down to where 32-bit arithmetic won't
    overflow.
*/
void
init_xrandom(Z32 seed)
{
    time_t tm;
    	
    if (seed >= 0) {
	/* If the random state is already set, changes are somewhat
	   suspicious. */
	if (randstate >= 0) {
	    Dprintf("Randstate being changed from %d to %d\n",
		    randstate, seed);
	}
	randstate = seed;
    } else {
    	time(&tm);
    	randstate = (int) tm;
    }
    /* Whatever its source, put the randstate into known range (0 - 99999). */
    randstate = ABS(randstate);
    randstate %= 100000;
    /* This is kept around for the sake of error reporting. */
    initrandstate = randstate;
}

#ifdef _MSC_VER
//! Microsoft Visual C 64-bit signed integer type for PRNG.
#define INT64 __int64
#else
//! 64-bit signed integer type for PRNG.
#define INT64 long long
#endif

int
xrandom(int m)
{
    int rslt;

    INT64 bignum;

    randstate = (8121 * randstate + 28411) % 134456;
    /* Random numbers in too large of a range will overflow 32-bit
       arithmetic, but Xconq rarely needs the large ranges, so only do
       the 64-bit arithmetic if necessary. */
    if (m <= 10000) {
	rslt = (m * randstate) / 134456;
    } else {
	bignum = m * ((INT64) randstate);
	rslt = bignum / 134456;
    }
    return rslt;
}

NAMESPACE_UTILITY_END
NAMESPACE_XCONQ_END

