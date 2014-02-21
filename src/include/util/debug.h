// Xconq
// Debug Functions

// $Id: debug.h,v 1.1 2006/04/07 02:13:35 eric_mcdonald Exp $

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
    \brief Debug utilities.
    \ingroup grp_util

    \todo Move 'prealloc_debug' to a proper location.

    Defines constant and function macros for debugging the project.\n
    Declares utility functions for debugging the project.
*/

#ifndef XCONQ_DEBUG_H
#define XCONQ_DEBUG_H

#include "util/base.h"

/* If asserts are off, then an appropriate user diagnostic will be
   generated and/or a flow control statement will be used. If asserts are on,
   then we obviously get the debugging benefits and abortive behavior. */

//! If assertion fails, then issue a run-time warning.
#define assert_warning(cond, msg) \
    { assert(cond); if (!(cond)) run_warning(msg); }
//! If assertion fails, then issue a run-time warning and branch out.
#define assert_warning_break(cond, msg) \
    { assert(cond); if (!(cond)) { run_warning(msg); break; } }
//! If assertion fails, then issue a run-time warning and branch to start.
#define assert_warning_continue(cond, msg) \
    { assert(cond); if (!(cond)) { run_warning(msg); continue; } }
//! If assertion fails, then issue a run-time warning and branch to label.
#define assert_warning_goto(cond, msg, label) \
    { assert(cond); if (!(cond)) { run_warning(msg); goto label; } }
//! If assertion fails, then issue a run-time warning and return.
#define assert_warning_return(cond, msg, retval) \
    { assert(cond); if (!(cond)) { run_warning(msg); return retval; } }
//! If assertion fails, then issue a run-time error, which aborts.
#define assert_error(cond, msg) \
    { assert(cond); if (!(cond)) run_error(msg); }
//! If assertion fails, then branch out.
#define assert_break(cond) \
    { assert(cond); if (!(cond)) break; }
//! If assertion fails, then branch to start.
#define assert_continue(cond) \
    { assert(cond); if (!(cond)) continue; }
//! If assertion fails, then branch to label.
#define assert_goto(cond, label) \
    { assert(cond); if (!(cond)) goto label; }
//! If assertion fails, then return.
#define assert_return(cond, retval) \
    { assert(cond); if (!(cond)) return retval; }

NAMESPACE_XCONQ_BEGIN
NAMESPACE_UTILITY_BEGIN

// Debugging

//! Formatted kernel debug output.
extern void debug_printf(char *str, ...);
//! Formatted AI (machine) debug output.
extern void debugm_printf(char *str, ...);
//! Formatted graphics/GUI debug output.
extern void debugg_printf(char *str, ...);

#if DEBUGGING

/* If the debug flags are not macros, then declare them as globals. */

#ifndef Debug
//! Kernel debugging.
extern int Debug;
#endif
#ifndef DebugM
//! AI (machine) debugging.
extern int DebugM;
#endif
#ifndef DebugG
//! Graphics/GUI debugging.
extern int DebugG;
#endif

#define Dprintf if (Debug && dfp) debug_printf
#define DMprintf if (DebugM && dmfp) debugm_printf
#define DGprintf if (DebugG && dgfp) debugg_printf

#define Dprintlisp(X) if (Debug && dfp) fprintlisp(dfp, (X))
#define DMprintlisp(X) if (DebugM && dmfp) fprintlisp(dmfp, (X))
#define DGprintlisp(X) if (DebugG && dgfp) fprintlisp(dgfp, (X))

extern FILE *dfp;
extern FILE *dmfp;
extern FILE *dgfp;

#else // if !DEBUGGING

/* Make defns and calls vanish if possible. */

#define Dprintf if (0) debug_printf
#define DMprintf if (0) debugm_printf
#define DGprintf if (0) debugg_printf

#define Dprintlisp(X)
#define DMprintlisp(X)
#define DGprintlisp(X)

#define Debug (0)
#define DebugM (0)
#define DebugG (0)

#define dfp stdout
#define dmfp stdout
#define dgfp stdout

#endif // if !DEBUGGING

//! Prime debugging-related data structures.
extern void prealloc_debug(void);

//! Initialize debug output to STDOUT.
extern void init_debug_to_stdout(void);
//! Flush debugging.
extern void update_debugging(void);
//! Toggle debugging on or off.
extern void toggle_debugging(int *flagp);

//! Whether to trace activity or not.
extern Bool activity_trace;

//! Initialize activity record.
extern void record_activity_start(char *type, int detail);
//! Finalize activity record.
extern void record_activity_end(char *type, int detail);
//! Dump activity trace to a file.
extern void dump_activity_trace(void);

NAMESPACE_UTILITY_END
NAMESPACE_XCONQ_END

#endif // ifndef XCONQ_DEBUG_H
