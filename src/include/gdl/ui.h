// xConq
// Forward declarations for UI vars and functions.

// $Id: ui.h,v 1.3 2006/06/02 16:58:33 eric_mcdonald Exp $

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
    \brief Forward declarations for UI vars and functions.
    \ingroup grp_gdl
*/

#ifndef XCONQ_GDL_UI_H
#define XCONQ_GDL_UI_H

#include "gdl/lisp.h"
#include "gdl/unit/unit_FWD.h"
#include "gdl/side/side_FWD.h"

// Global Constants: Display Updates

//! Display Update: always
#define UPDATE_ALWAYS 1
//! Display Update: adjacent
#define UPDATE_ADJ 2
//! Display Update: cover
#define UPDATE_COVER 4
//! Display Update: termperature
#define UPDATE_TEMP 8
//! Display Update: clouds
#define UPDATE_CLOUDS 16
//! Display Update: winds
#define UPDATE_WINDS 32

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Notifications

//! Notify given side of something.
extern void notify(Side *side, char *str, ...);

//! Notify all sides of something.
extern void notify_all(char *str, ...);

//! Announce amount of progress in reading a GDL form.
extern void announce_read_progress(void);

// Queries

//! Does the side have an active display associated with it?
extern int active_display(Side *side);

// Debugging/Tracing

//! Print GDL form via UI.
extern void print_form(Obj *form);

//! End printing forms via UI.
extern void end_printing_forms(void);

// Status Displays


//! Ask UI to update display of given unit.
extern void update_unit_display(Side *side, Unit *unit, int rightnow);
//! Ask UI to update display of given cell.
extern void update_cell_display(Side *side, int x, int y, int flags);
//! Ask UI to update display of given event.
extern void update_event_display(Side *side, struct a_histevent *hevt, int rightnow);
//! Flush display buffers for given side.
extern void flush_display_buffers(Side *side);

//! Play any movies associated with event.
extern void play_event_movies(Side *side, struct a_histevent *hevt);

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

#endif // ifndef XCONQ_GDL_UI_H
