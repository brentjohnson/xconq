// Xconq
// Math Functions

// $Id: math.cc,v 1.1 2006/04/07 01:33:24 eric_mcdonald Exp $

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
    \brief Math functions.
    \ingroup grp_util
*/

#include "util/constants.h"
#include "util/math.h"
#include "util/prng.h"
#include "util/ui.h"

NAMESPACE_XCONQ_BEGIN
NAMESPACE_UTILITY_BEGIN

int
isqrt(int i)
{
    int j, k;

    if (i > 3) {
	for (j = i, k = -1; j >>= 2; k <<= 1)
		   ;
	k = (~k + i / ~k) / 2;
	k = (k + i / k) / 2;
	k = (1 + k + i / k) / 2;
	return (k + i / k) / 2;
    } else if (i > 0) {
	return 1;
    } else {
	return 0;
    }
}

int
probability(int prob)
{
    if (prob <= 0)
      return FALSE;
    if (prob >= 100)
      return TRUE;
    return (xrandom(100) < prob);
}

int
prob_fraction(int n)
{
    return (n / 100 + (probability(n % 100) ? 1 : 0));
}

int
select_by_weight(int *arr, int numvals)
{
    int sum = 0, i, n;

    sum = 0;
    for (i = 0; i < numvals; ++i) {
	if (arr[i] <= 0)
	  continue;
	sum += arr[i];
    }
    if (sum == 0)
      return -1;
    /* We now know the range, make a random index into it. */
    n = xrandom(sum);
    /* Go through again to figure out which choice the index refs. */
    sum = 0;
    for (i = 0; i < numvals; ++i) {
	if (arr[i] <= 0)
	  continue;
	sum += arr[i];
	if (sum >= n) {
	    return i;
	}
    }
    run_error("Ooh weird");
    return -1;
}

NAMESPACE_UTILITY_END
NAMESPACE_XCONQ_END

