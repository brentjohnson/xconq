// xConq
// Dice manipulation.

// $Id: dice.h,v 1.3 2006/05/23 03:17:38 eric_mcdonald Exp $

/*//////////////////////////// COPYRIGHT NOTICE ////////////////////////////////

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
    \brief Dice manipulation.
    \ingroup grp_gdl
*/

#ifndef XCONQ_GDL_DICE_H
#define XCONQ_GDL_DICE_H

#include "gdl/lisp.h"

// Function Macros: Dice Decomposition

//! Decompose dice representation into Type 1 dice parts.
#define dice1(N,NUMDICE,SPOTS,OFFSET)   \
    (((0 == ((N) >> 14)) || (3 == ((N) >> 14))) ?   \
     (NUMDICE = 0, SPOTS = 0, OFFSET = (N)) :   \
     (NUMDICE = (((N) >> 11) & 0x07) + 1,  \
      SPOTS = (((N) >> 7) & 0x0f) + 2,  \
      OFFSET = ((N) & 0x8000) ? (-((N) & 0x7f) - 1) : ((N) & 0x7f)))

//! Decompose dice representation into Type 2 dice parts.
#define dice2(N,NUMDICE,SPOTS,OFFSET)   \
    (((0 == ((N) >> 14)) || (3 == ((N) >> 14))) ?   \
     (NUMDICE = 0, SPOTS = 0, OFFSET = (N)) :   \
     (NUMDICE = ((N) & 0x8000) ? -((((N) >> 11) & 0x07) + 1) \
				  : (((N) >> 11) & 0x07) + 1,  \
      SPOTS = (((N) >> 7) & 0x0f) + 2,  \
      OFFSET = ((N) & 0x8000) ? -((N) & 0x7f) : ((N) & 0x7f)))

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Aliases

//! Internal dice representation. Two bytes.
typedef short   DiceRep;

// Queries

//! Get Type 1 dice spec from internal dice representation.
extern void dice1_desc(char *buf, DiceRep dice);
//! Get Type 2 dice spec from internal dice representation.
extern void dice2_desc(char *buf, DiceRep dice);

//! Print dice spec or integer, according to type.
extern void fprint_num_or_dice(FILE *fp, int x, int valtype);

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

#endif // ifndef XCONQ_GDL_DICE_H
