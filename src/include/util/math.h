// xConq
// Math Functions

// $Id: math.h,v 1.1 2006/04/07 02:13:35 eric_mcdonald Exp $

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
    \brief Math functions and constants.
    \ingroup grp_util
*/

#ifndef XCONQ_MATH_H
#define XCONQ_MATH_H

#include "util/base.h"

#ifndef ABS
//! Absolute value.
#define ABS(x) (((x) < 0) ? (0 - (x)) : (x))
#endif

#ifndef min
//! Minimum value.
#define min(x,y) (((x) < (y)) ? (x) : (y))
#endif

#ifndef max
//! Maximum value.
#define max(x,y) (((x) > (y)) ? (x) : (y))
#endif

//! Is between.
/*! Determines if a number is between two other numbers (inclusive). */
#define between(lo,n,hi) \
        ((lo) <= (n) && (n) <= (hi))

//! Limit value.
/*! Limits the value of the paramter to be between a high and 
    low value (inclusive).
*/
#define limitn(lo,n,hi) ((n) \
    < (lo) ? (lo) : ((n) > (hi) ? (hi) : (n)))

//! Mean of two numbers.
/*! Note that if sum of the numbers is greater than the range of the 
    fundamental type, then the answer will be wrong.
*/
#define avg(a,b) \
        (((a) + (b)) / 2)

//! Normalize a number on a scale between -range and +range.
#define normalize_on_pmscale(n,max,range) \
    ((max) ? (((n) * (range)) / (max)) : 0)

NAMESPACE_XCONQ_BEGIN
NAMESPACE_UTILITY_BEGIN

//! Integer square root.
/*! Improved version from Bruce Fast, via Scott Herod. 
    Good enough, no float trickery or libs needed.
*/
extern int isqrt(int i);

//! Percentage probability, with bounds checking.
extern int probability(int prob);

//! Extract probability portion of a number.
/*! For a number in the range 0 - 10000, divide it by 100 and use the
    remainder as the probability of adding 1.
*/
extern int prob_fraction(int n);

//! Weighted random selection of an index.
extern int select_by_weight(int *arr, int numvals);

NAMESPACE_UTILITY_END
NAMESPACE_XCONQ_END

#endif // ifndef XCONQ_MATH_H
