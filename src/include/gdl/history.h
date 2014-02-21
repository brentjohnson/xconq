// xConq
// Lifecycle management and ser/deser of game history.

// $Id: history.h,v 1.2 2006/06/02 16:58:33 eric_mcdonald Exp $

/*//////////////////////////// COPYRIGHT NOTICE ////////////////////////////////

  Copyright (C) 1987-1989   Stanley T. Shebs
  Copyright (C) 1991-2000   Stanley T. Shebs
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
    \brief Lifecycle management and ser/deser of game history.
    \ingroup grp_gdl
*/

#ifndef XCONQ_GDL_HISTORY_H
#define XCONQ_GDL_HISTORY_H

#include "gdl/side/side.h"

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Data Structures

#ifdef  DEF_HEVT
#undef  DEF_HEVT
#endif
#define DEF_HEVT(name, CODE, datadescs) CODE,

//! History event type.
typedef enum {

#include "gdl/history.def"

    NUMHEVTTYPES

} HistEventType;

//! Historical event definition.
/*! This is the form of the description of a event type.
    It contains the name of the event, and the description
    of the event.
*/
typedef struct a_hevt_defn {
    //! Name of event.
    char *name;
    //! Description of event.
    char *datadescs;
} HevtDefn;

//! History event record.
typedef struct a_histevent {
    //! Type of history event.
    HistEventType type;
    //! Turn when event started.
    short startdate;
    //! Turn when event ended.
    short enddate;
    //! Bitvector of sides which observed event.
    SideMask observers;
    //! Next history event.
    struct a_histevent *next;
    //! Previous history event.
    struct a_histevent *prev;
    //! Data descriing event.
    int data[4];
} HistEvent;

//! Reasons for gain.
enum gain_reasons {
    //! Initially on side.
    initial_gain = 0,
    //! Gained through construction.
    build_gain = 1,
    //! Gained by capture.
    capture_gain = 2,
    //! Some other reason for gain.
    other_gain = 3,
    //! Total number of gain reasons.
    num_gain_reasons = 4
};

//! Reasons for loss.
enum loss_reasons {
    //! Lost to combat attrition.
    combat_loss = 0,
    //! Lost by capture.
    capture_loss = 1,
    //! Lost by starvation.
    starvation_loss = 2,
    //! Lost in terrain accident.
    accident_loss = 3,
    //! Lost to terrain attrition.
    attrition_loss = 4,
    //! Lost by disbanding.
    disband_loss = 5,
    //! Some other reason for loss.
    other_loss = 6,
    //! Total number of loss reasons.
    num_loss_reasons = 7
};

// Global Variables

//! Array of history event definitions.
extern HevtDefn hevtdefns[];

//! List of history event records.
extern HistEvent *history;

// Queries

//! Find event type from GDL container.
extern int find_event_type(Obj *sym);

//! Does pattern match event?
extern int pattern_matches_event(Obj *pattern, HistEvent *hevt);

//! Describe event from GDL list.
extern void event_desc_from_list(
    Side *side, Obj *lis, HistEvent *hevt, char *buf);

// History Management

//! Record event into game history.
extern HistEvent *record_event(HistEventType type, SideMask observers, ...);

// Lifecycle Management

//! Create new history event record.
extern HistEvent *create_historical_event(HistEventType type);

// GDL I/O

//! Read history record from GDL form.
extern void interp_history(Obj *form);

//! Serialize historical event to GDL.
extern void write_historical_event(HistEvent *hevt);
//! Serialize history to GDL.
extern void write_history(void);

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

#endif // ifndef XCONQ_GDL_HISTORY_H
