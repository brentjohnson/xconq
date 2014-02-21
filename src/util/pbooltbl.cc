// Xconq
// Packed Boolean Table

// $Id: pbooltbl.cc,v 1.1 2006/04/07 01:33:24 eric_mcdonald Exp $

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
    \brief Functions for packed boolean table.
    \ingroup grp_util
*/

#include "util/memory.h"
#include "util/pbooltbl.h"
#include "util/ui.h"

NAMESPACE_XCONQ_BEGIN
NAMESPACE_UTILITY_BEGIN

PackedBoolTable*
create_packed_bool_table(int m, int n)
{
    PackedBoolTable *tbl = NULL;

    if ((m < 1) || (n < 1))
	run_error("Attempted to create a table with a dimension size < 1!");
    tbl = (PackedBoolTable *)xmalloc(sizeof(PackedBoolTable));
    tbl->whichway = (n >= m);
    tbl->sizeofint = sizeof(int) * 8;
    tbl->rdim1size = ((n >= m) ? n : m) / tbl->sizeofint;
    if (((n >= m) ? n : m) % tbl->sizeofint)
	++(tbl->rdim1size);
    tbl->dim2size = ((n >= m) ? m : n);
    tbl->pbools = NULL; /* Initialization of array is done separately. */
    return tbl;
}

void
init_packed_bool_table(PackedBoolTable *tbl)
{
    if (valid_packed_bool_table(tbl))
	run_warning("Attempted to re-initialize a valid, initialized table!");
    if (!tbl || (tbl->sizeofint <= 0) || (tbl->rdim1size < 0)
	|| (tbl->dim2size < 1))
	run_error("Attempted to initialize an invalid table!");
    tbl->pbools = (int *)xmalloc(sizeof(int) * (tbl->rdim1size + 1) *
				 tbl->dim2size);
    /* Most of uses of the packed bool table will want a clean slate,
       so provide one. */
    memset(tbl->pbools, 0, sizeof(int) * (tbl->rdim1size + 1) * tbl->dim2size);
}

void
destroy_packed_bool_table(PackedBoolTable *tbl)
{
    if (tbl->pbools)
	free(tbl->pbools);
    free(tbl);
}

NAMESPACE_UTILITY_END
NAMESPACE_XCONQ_END

