// xConq
// Low-Level to UI Callbacks

// $Id: ui.cc,v 1.1 2006/04/07 01:33:24 eric_mcdonald Exp $

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
    \brief Low-level to UI callbacks.
    \ingroup grp_util
*/

#include <cstdarg>

#include "util/constants.h"
#include "util/callbacks.h"
#include "util/log.h"
#include "util/ui.h"
#include "util/debug.h"

NAMESPACE_XCONQ_BEGIN
NAMESPACE_UTILITY_BEGIN

//! If true, warnings will not be issued.
int warnings_suppressed;

void
init_warning(char *str, ...)
{
    char buf[BUFSIZE];
    va_list ap;

    va_start(ap, str);
    vsnprintf(buf, BUFSIZE, str, ap);
    va_end(ap);

    Dprintf("INIT WARNING %s INIT WARNING\n", buf);
    if (warnings_logged)
      log_warning("INIT", buf);
    if (warnings_suppressed)
      return;
    low_init_warning(buf);
}

void
init_error(char *str, ...)
{
    char buf[BUFSIZE];
    va_list ap;

    va_start(ap, str);
    vsnprintf(buf, BUFSIZE, str, ap);
    va_end(ap);

    Dprintf("INIT ERROR %s INIT ERROR\n", buf);
    if (warnings_logged)
      log_warning("INIT ERROR", buf);
    low_init_error(buf);
}

void
run_warning(char *str, ...)
{
    char buf[BUFSIZE];
    va_list ap;

    va_start(ap, str);
    vsnprintf(buf, BUFSIZE, str, ap);
    va_end(ap);

    Dprintf("RUN WARNING %s RUN WARNING\n", buf);
    if (warnings_logged)
      log_warning("RUN", buf);
    if (warnings_suppressed)
      return;
    low_run_warning(buf);
}

void
run_error(char *str, ...)
{
    char buf[BUFSIZE];
    va_list ap;

    va_start(ap, str);
    vsnprintf(buf, BUFSIZE, str, ap);
    va_end(ap);

    Dprintf("RUN ERROR %s RUN ERROR\n", buf);
    if (warnings_logged)
	log_warning("RUN ERROR", buf);
    low_run_error(buf);
}

void
case_panic(char *str, int var)
{
    run_error("Panic! Unknown %s %d", str, var);
}

NAMESPACE_UTILITY_END
NAMESPACE_XCONQ_END

