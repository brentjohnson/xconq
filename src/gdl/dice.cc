// Xconq
// Dice manipulation.

// $Id: dice.cc,v 1.3 2006/05/23 03:17:38 eric_mcdonald Exp $

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

#include "gdl/dice.h"

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Queries

void
dice1_desc(char *buf, DiceRep dice)
{
    Z16u numdice = 0, die = 0;
    Z16 offset = 0;

    dice1(dice, numdice, die, offset);
    if (!numdice)
        sprintf(buf, "%d", dice);
    else if (!offset)
        sprintf(buf, "%dd%d", numdice, die);
    else
        sprintf(buf, "%dd%d%+d", numdice, die, offset);
}

void
dice2_desc(char *buf, DiceRep dice)
{
    Z16u die = 0;
    Z16 numdice = 0, offset = 0;

    dice2(dice, numdice, die, offset);
    if (!numdice)
        sprintf(buf, "%d", dice);
    else if (!offset)
        sprintf(buf, "%dd%d", numdice, die);
    else
        sprintf(buf, "%dd%d%+d", numdice, die, offset);
}

void
fprint_num_or_dice(FILE *fp, int x, int valtype)
{
    char valbuf [BUFSIZE];

    if (GDLDICE1 == valtype)
	dice1_desc(valbuf, (DiceRep)x);
    else if (GDLDICE2 == valtype)
	dice2_desc(valbuf, (DiceRep)x);
    else
	snprintf(valbuf, BUFSIZE, "%d", x);
    fprintf(fp, " %s", valbuf);
}

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

