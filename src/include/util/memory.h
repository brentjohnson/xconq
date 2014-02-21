// xConq
// Memory Functions

// $Id: memory.h,v 1.1 2006/04/07 02:13:35 eric_mcdonald Exp $

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
    \brief Memory handling functions and constants.
    \ingroup grp_util
*/

#ifndef XCONQ_MEMORY_H
#define XCONQ_MEMORY_H

#include "util/base.h"

NAMESPACE_XCONQ_BEGIN
NAMESPACE_UTILITY_BEGIN

//! Attempt memory allocation and abort if allocation fails.
extern void *xmalloc(int amt);
//! Report on memory allocations when debugging.
extern void report_malloc(void);

NAMESPACE_UTILITY_END
NAMESPACE_XCONQ_END

#endif // ifndef XCONQ_MEMORY_H
