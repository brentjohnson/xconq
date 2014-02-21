// Xconq
// Unix-specific Signal-Handling Functions

// $Id: signal.cc,v 1.1 2006/04/07 01:12:23 eric_mcdonald Exp $

/*//////////////////////////// COPYRIGHT NOTICE ////////////////////////////////

  Copyright (C) 1987-1989   Stanley T. Shebs
  Copyright (C) 1991-1997   Stanley T. Shebs
  Copyright (C) 1999        Stanley T. Shebs
  Copyright (C) 2006        Eric A. McDonald

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
    \brief Unix-specific signal-handling functions.
    \ingroup grp_util
*/

/* Unix interface stuff.  Do NOT attempt to use this file in a non-Unix
   system, not even an ANSI one! */

extern "C" {
#include <signal.h>
}
#include "util/constants.h"
#include "util/callbacks.h"
#include "util/fs.h"
#include "util/debug.h"

NAMESPACE_XCONQ_BEGIN
NAMESPACE_UTILITY_BEGIN

#if (1) /* POSIX */
static void stop_handler(int sig);
static void crash_handler(int sig);
static void hup_handler(int sig);
#else /* BSD etc? */
static void stop_handler(int sig, int code, struct sigcontext *scp, char *addr);
static void crash_handler(int sig, int code, struct sigcontext *scp, char *addr);
static void hup_handler(int sig, int code, struct sigcontext *scp, char *addr);
#endif

/* Default behavior on explicit kill. */

static void
#if (1)
stop_handler(int sig)
#else
stop_handler(int sig, int code, struct sigcontext *scp, char *addr)
#endif
{
    /* (should try to tell other programs of departure?) */
    close_displays();
    exit(1);
}

/* This routine attempts to save the state before dying. */

static void
#if 1
crash_handler(int sig)
#else
crash_handler(int sig, int code, struct sigcontext *scp, char *addr)
#endif
{
    static int already_been_here = FALSE;

    if (!already_been_here) {
	already_been_here = TRUE;
	close_displays();  
	printf("Fatal error encountered. Signal %d\n", sig);
#if 0
	printf(" code %d\n", code);
#endif
	printf("\n");
	write_entire_game_state(error_save_filename());
    }
#ifdef __cplusplus
    throw "snafu";
#else
    abort();
#endif
}

/* Accidental disconnection saves state. */

static void
#if 1
hup_handler(int sig)
#else
hup_handler(int sig, int code, struct sigcontext *scp, char *addr)
#endif
{
    static int already_been_here = FALSE;

    if (!already_been_here) {
	already_been_here = TRUE;
	close_displays();
	printf("Somebody was disconnected, saving the game.\n");
	write_entire_game_state(error_save_filename());
    }
#ifdef __cplusplus
    throw "snafu";
#else
    abort();
#endif
}

void
init_signal_handlers(void)
{
    signal(SIGINT, stop_handler);
    if (0 /* don't accidently quit */ && !Debug) {
	signal(SIGINT, SIG_IGN);
    } else {
	signal(SIGINT, SIG_DFL);
/*	signal(SIGINT, crash_handler);  */
    }
    signal(SIGHUP, hup_handler);
    signal(SIGSEGV, crash_handler);
    signal(SIGFPE, crash_handler);
    signal(SIGILL, crash_handler);
    signal(SIGINT, crash_handler);
    signal(SIGQUIT, crash_handler);
    signal(SIGTERM, crash_handler);
    /* The following signals may not be available everywhere. */
#ifdef SIGBUS
    signal(SIGBUS, crash_handler);
#endif /* SIGBUS */
#ifdef SIGSYS
    signal(SIGSYS, crash_handler);
#endif /* SIGSYS */
}

NAMESPACE_UTILITY_END
NAMESPACE_XCONQ_END

