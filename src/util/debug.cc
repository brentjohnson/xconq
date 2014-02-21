// Xconq
// Debug Functions

// $Id: debug.cc,v 1.1 2006/04/07 01:33:24 eric_mcdonald Exp $

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
    \brief Debug functions.
    \ingroup grp_util
*/

#include "util/base.h"
#if TIME_WITH_SYS_TIME
extern "C" {
#   include <sys/time.h>
}
#   include <ctime>
#else
#   if HAVE_SYS_TIME_H
extern "C" {
#       include <sys/time.h>
}
#   else
#       include <ctime>
#   endif
#endif

#include "util/constants.h"
#include "util/memory.h"
#include "util/time.h"
#include "util/fs.h"
#include "util/ui.h"
#include "util/debug.h"

NAMESPACE_XCONQ_BEGIN
NAMESPACE_UTILITY_BEGIN

#ifdef DEBUGGING

#ifndef Debug
int Debug = FALSE;
#endif
#ifndef DebugM
int DebugM = FALSE;
#endif
#ifndef DebugG
int DebugG = FALSE;
#endif

FILE *dfp = NULL;
FILE *dgfp = NULL;
FILE *dmfp = NULL;

void
init_debug_to_stdout()
{
    dfp  = stdout;
    dgfp = stdout;
    dmfp = stdout;
}

FILE *ffp = NULL;

int firstdebug = TRUE;

void
update_debugging()
{
    if (Debug || DebugG || DebugM) {
        /* Always close the file if open, forces to desirable state. */
        if (ffp != NULL) {
            fclose(ffp);
            ffp = NULL;
        }
        /* Reopen the file. */
        if (ffp == NULL) {
            if (my_rid == 0) {
                ffp = open_file("Xconq.DebugOut", "a");
            } else if (my_rid == master_rid) {
                ffp = open_file("Xconq-Master.DebugOut", "a");
            } else {
                ffp = open_file("Xconq-Client.DebugOut", "a");
            }
        }
        if (ffp != NULL) {
            if (Debug)
              dfp = ffp;
            if (DebugG)
              dgfp = ffp;
            if (DebugM)
              dmfp = ffp;
        }
    }
}

void
toggle_debugging(int *flagp)
{
    /* Always close the file if open, forces to desirable state. */
    if (ffp != NULL) {
	fclose(ffp);
	ffp = NULL;
    }
    /* Flip the state of the debugging flag, if supplied. */
    if (flagp != NULL) {
	*flagp = ! *flagp;
    }
    /* (Re-)open the debugging transcript file. */
    if (ffp == NULL) {
	if (my_rid == 0) {
	    ffp = open_file("Xconq.DebugOut", (char *)(firstdebug ? "w" : "a"));
	} else if (my_rid == master_rid) {
	    ffp = open_file("Xconq-Master.DebugOut", (char *)(firstdebug ? "w" : "a"));
	} else {
	    ffp = open_file("Xconq-Client.DebugOut", (char *)(firstdebug ? "w" : "a"));
	}
	firstdebug = FALSE;
    }
    if (ffp != NULL && flagp != NULL) {
	/* Indicate which flags are now on. */
	fprintf(ffp, "\n\n*********** %s %s %s **********\n\n",
		(Debug ? "Debug" : ""), (DebugM ? "DebugM" : ""),
		(DebugG ? "DebugG" : ""));
    }
    /* Set specific debug file pointers to be the same as the
	   pointer to the file. */
    if (ffp != NULL) {
	if (Debug)
	  dfp = ffp;
	if (DebugG)
	  dgfp = ffp;
	if (DebugM)
	  dmfp = ffp;
    }
    /* If all debugging flags have been turned off, close the file too. */
    if (!Debug && !DebugG && !DebugM) {
	if (ffp != NULL) {
	    fclose(ffp);
	    ffp = NULL;
	}
    }
    prealloc_debug();
}

#if 0           /* Unused. */

void
debugging_string(buf)
char *buf;
{
    buf[0] = '\0';
    if (Debug || DebugM || DebugG)
      strcat(buf, "-D");
    if (!Debug)
      strcat(buf, "-");
    if (DebugM)
      strcat(buf, "M");
    if (DebugG)
      strcat(buf, "G");
}

#endif

void
debug_printf(char *str, ...)
{
    va_list ap;

    va_start(ap, str);
    vfprintf(dfp, str, ap);
    va_end(ap);
}

void
debugm_printf(char *str, ...)
{
    va_list ap;

    va_start(ap, str);
    vfprintf(dmfp, str, ap);
    va_end(ap);
}

void
debugg_printf(char *str, ...)
{
    va_list ap;

    va_start(ap, str);
    vfprintf(dgfp, str, ap);
    va_end(ap);
}

#endif /* DEBUGGING */

//! Timeline entry.
/*! Useful for tracing execution.
    Record of activity type, details, and start and end time.
*/
struct a_timeline_entry {
    int start;	    //! Start of activity.
    int end;	    //! End of activity.
    char *type;	    //! Type of activity.
    int detail1;    //! Details on activity.
    int detail2;    //! More details on activity.
};

//! Number of timeline entries to allocate in one block.
#define BLOCKSIZE 10000

//! A block of timeline entries.
typedef struct a_timeline_block {
    struct a_timeline_entry entries[BLOCKSIZE];	    //! Array of timeline entries.
    int index;					    //! Index of block.
    struct a_timeline_block *next;		    //! Pointer to next block.
} TimelineBlock;

Bool activity_trace;

static int starting_trace = TRUE;

static TimelineBlock *current_timeline_block;

static char *current_activity;

static struct timeval initial_time;

void
record_activity_start(char *type, int detail)
{
    TimelineBlock *tlblock;
    int usec;
    struct timeval curtime;

    if (!activity_trace)
      return;
    if (current_activity != NULL)
      run_warning("starting activity %s before %s done\n",
		  type, current_activity);
    /* Allocate a timeline block if needed. */
    if (current_timeline_block == NULL) {
	current_timeline_block =
	  (TimelineBlock *) xmalloc(sizeof(TimelineBlock));
    }
    if (starting_trace) {
	current_timeline_block->index = 0;
	gettimeofday(&initial_time, NULL);
	starting_trace = FALSE;
    }
    tlblock = current_timeline_block;
    if (tlblock->index >= (BLOCKSIZE - 2))
      return;
    gettimeofday(&curtime, NULL);
    usec = (curtime.tv_sec - initial_time.tv_sec) * 1000000
      + curtime.tv_usec - initial_time.tv_usec;
    tlblock->entries[tlblock->index].start = usec;
    tlblock->entries[tlblock->index].type = type;
    tlblock->entries[tlblock->index].detail1 = detail;
    current_activity = type;
}

void
record_activity_end(char *type, int detail)
{
    TimelineBlock *tlblock;
    int usec;
    struct timeval curtime;

    if (!activity_trace)
      return;
    if (type != current_activity)
      run_warning("ending activity %s instead of %s\n",
		  type, (current_activity ? current_activity : "<null>"));
    tlblock = current_timeline_block;
    if (tlblock->index >= (BLOCKSIZE - 2))
      return;
    gettimeofday(&curtime, NULL);
    usec = (curtime.tv_sec - initial_time.tv_sec) * 1000000
      + curtime.tv_usec - initial_time.tv_usec;
    tlblock->entries[tlblock->index].end = usec;
    tlblock->entries[tlblock->index].detail2 = detail;
    ++(tlblock->index);
    current_activity = NULL;
}

void
dump_activity_trace(void)
{
    int i;
    FILE *fp;
    TimelineBlock *tlblock;
    struct a_timeline_entry *entry;

    tlblock = current_timeline_block;
    if (tlblock == NULL)
      return;
    if ((fp = open_file("xconq.trace", "w")) != NULL) {
	for (i = 0; i < tlblock->index; ++i) {
	    entry = &(tlblock->entries[i]);
	    fprintf(fp, "%d %d %s %d %d\n",
		    entry->start, entry->end - entry->start, entry->type,
		    entry->detail1, entry->detail2);
    #if 0  /* doubles size of trace */
	    if (tlblock->entries[i+1].start > entry->end + 5) {
		fprintf(fp, "%d %d %s %d %d\n",
			entry->end + 1,
			tlblock->entries[i+1].start - entry->end,
			"gap", 0, 0);
	    }
    #endif
	}
	fclose(fp);
    }
    /* Once we've dumped a trace, prepare to start over. */
    starting_trace = TRUE;
}


NAMESPACE_UTILITY_END
NAMESPACE_XCONQ_END

