// xConq
// Forward declarations of UI vars and functions.

// $Id: ui.h,v 1.2 2006/05/14 18:51:49 eric_mcdonald Exp $

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
    \brief Forward declarations of UI vars and functions.
    \ingroup grp_util
*/

#ifndef XCONQ_UTILITY_UI_H
#define XCONQ_UTILITY_UI_H

#include "util/base.h"

NAMESPACE_XCONQ_BEGIN
NAMESPACE_UTILITY_BEGIN

// Global Functions: Display

//! Close all open UI displays.
/*! Needed by Unix and Windows. */
extern void close_displays(void);

//! Print init-time warning message via UI.
extern void low_init_warning(char *str);
//! Print init-time error message via UI.
extern void low_init_error(char *str);
//! Print run-time warning message via UI.
extern void low_run_warning(char *str);
//! Print run-time error message via UI.
extern void low_run_error(char *str);

//! Formatted print init-time warning message via UI.
extern void init_warning(char *str, ...);
//! Formatted print init-time error message via UI.
extern void init_error(char *str, ...);
//! Formatted print run-time warning message via UI.
extern void run_warning(char *str, ...);
//! Formatted print run-time error message via UI.
/*! An error is fatal, and causes an abort. */
extern void run_error(char *str, ...);

//! Report, if flow of execution falls off of end of switch.
/*! This little routine goes at the end of all switch statements on
    internal data values.
*/
extern void case_panic(char *str, int var);

NAMESPACE_UTILITY_END
NAMESPACE_XCONQ_END

#endif // ifndef XCONQ_UTILITY_UI_H
