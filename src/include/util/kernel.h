// xConq
// Forward declarations of kernel vars and functions.

// $Id: kernel.h,v 1.2 2006/05/15 03:45:44 eric_mcdonald Exp $

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
    \brief Forward declarations of kernel vars and functions.
    \ingroup grp_util
*/

#ifndef XCONQ_UTILITY_KERNEL_H
#define XCONQ_UTILITY_KERNEL_H

#include "util/base.h"

NAMESPACE_XCONQ_BEGIN
NAMESPACE_UTILITY_BEGIN

// Global Variables: Network

//! The remote id of this program.
/*! Should be initialized to 0 by kernel. */
extern int my_rid;
//! The remote id of the current master program.
/*! Should be initialized to 0 by kernel. */
extern int master_rid;

// Global Functions: Game State

//! Write out the entire game.
/*! If appropriate signal is caught, then this needs to be invoked. */
extern int write_entire_game_state(char *fname);

NAMESPACE_UTILITY_END
NAMESPACE_XCONQ_END

#endif // ifndef XCONQ_UTILITY_KERNEL_H
