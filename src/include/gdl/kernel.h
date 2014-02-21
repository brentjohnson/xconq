// xConq
// Forward declarations of kernel vars and functions.

// $Id: kernel.h,v 1.3 2006/06/02 16:58:33 eric_mcdonald Exp $

/*//////////////////////////// COPYRIGHT NOTICE ////////////////////////////////

  Copyright (C) 2006     Eric A. McDonald

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
    \ingroup grp_gdl
*/

#ifndef XCONQ_GDL_KERNEL_H
#define XCONQ_GDL_KERNEL_H

#include "gdl/lisp.h"

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Global Variables: Network

//! Number of remote peers.
extern int numremotes;

//! Temporary remote id.
extern int tmprid;

// Global Variables: Game State

//! True, if before start of game.
extern int beforestart;

//! True, if at end of game.
extern int endofgame;

//! True, if synchronizing debugging state.
extern int debugging_state_sync;

//! Real time at which game was started.
extern time_t game_start_in_real_time;

//! True, if post event scoring is needed.
extern int need_post_event_scores;

// Global Variables: Notification

//! Flag: warnings suppressed.
extern int warnings_suppressed;

// Network

//! Add string to network packet.
extern void add_to_packet(char *str);

//! Finalize packet write.
extern void flush_write(void);

// Game Setup

//! Return GDL list of default world synthesis methods.
extern Obj * g_synth_methods_default(void);

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

#endif // ifndef XCONQ_GDL_KERNEL_H
