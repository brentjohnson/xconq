// Xconq
// Memory Management Functions

// $Id: memory.cc,v 1.1 2006/04/07 01:33:24 eric_mcdonald Exp $

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
    \brief Memory management functions.
    \ingroup grp_util
*/

#include "util/constants.h"
#include "util/memory.h"
#include "util/ui.h"

NAMESPACE_XCONQ_BEGIN
NAMESPACE_UTILITY_BEGIN

/* If true, allocation attempts will garner complaints, but not actually
   fail.  (This is primarily for debugging addon code, such as new AIs
   and user interfaces.) */

int xmalloc_warnings;

/* If true, all allocation will fail, suppresses further attempts. */

int memory_exhausted;

#ifdef DEBUGGING

/* This tracks our total space allocation. */

#define NUMMALLOCRECS 200

int overflow;

int numoverflow;

int totmalloc;

int nextmalloc;

int copymalloc;

int grandtotmalloc;

struct a_malloc {
    int size;
    int count;
} *mallocs = NULL;

static void record_malloc(int amt);
static int malloc_record_compare(const void *m1, const void *m2);

/* Given an amount of memory allocated, record it with the others. */

static void
record_malloc(int amt)
{
    int i;

    /* Might need to allocate the record of allocations. */
    if (mallocs == NULL) {
	/* Don't use xmalloc here!! */
	mallocs = (struct a_malloc *)
	  malloc(NUMMALLOCRECS * sizeof(struct a_malloc));
	overflow = 0;
	numoverflow = 0;
    }
    /* Search for already-recorded allocations of same-size blocks. */
    for (i = 0; i < nextmalloc; ++i) {
	if (amt == mallocs[i].size) {
	    ++(mallocs[i].count);
	    return;
	}
    }
    if (nextmalloc < NUMMALLOCRECS) {
	/* Record this allocation as a new size of allocation. */
	mallocs[nextmalloc].size = amt;
	mallocs[nextmalloc].count = 1;
	++nextmalloc;
    } else {
    	/* Add to the overflow bucket, which is used for allocations whose
	   sizes could not be recorded individually.  Yes, we could allocate
	   more buckets, but this seems like too much trouble for an
	   uncommon case. */
    	overflow += amt;
    	++numoverflow;
    }
}

static int
malloc_record_compare(CONST void *m1, CONST void *m2)
{
    return (((struct a_malloc *) m1)->size * ((struct a_malloc *) m1)->count
            - ((struct a_malloc *) m2)->size * ((struct a_malloc *) m2)->count);
}

/* Display memory consumption and reset records.  This does not account
   for freeing, but Xconq usually tries to hang onto and reuse anything
   it allocates, very rarely calls free(). */

void
report_malloc()
{
    extern int lispmalloc, numsymbols;
    int i;

    /* If this is called too soon, just leave silently. */
    if (mallocs == NULL)
      return;
    if (nextmalloc == 0) {
	Dprintf("No mallocs since last report.\n");
	return;
    }
    Dprintf("Mallocs since last report:\n");
    Dprintf("    Amount =  Bytes x  Times\n");
    /* Sort the entries. */
    qsort(mallocs, nextmalloc, sizeof(struct a_malloc), malloc_record_compare);
    /* Write out all the records, formatting nicely. */
    for (i = 0; i < nextmalloc; ++i) {
	Dprintf("%10d = %6d x %6d\n",
		mallocs[i].size * mallocs[i].count,
		mallocs[i].size,  mallocs[i].count);
    }
    if (overflow > 0)
      Dprintf("%10d = ?????? x %6d\n", overflow, numoverflow);
    Dprintf("Total malloced = %d bytes.\n", totmalloc);
    Dprintf("String copies = %d bytes.\n", copymalloc);
    Dprintf("Lisp malloced = %d bytes.\n", lispmalloc);
    Dprintf("Symbols interned = %d.\n", numsymbols);
    Dprintf("Grand total allocation = %d bytes.\n", grandtotmalloc);
    /* Reset all the counters for next time. */
    nextmalloc = 0;
    overflow = numoverflow = 0;
    totmalloc = copymalloc = lispmalloc = 0;
}

#endif /* DEBUGGING */

/* This is our improved and adapted version of malloc, that guarantees
   zeroing of the block, checks for memory exhaustion, and collects
   usage statistics. */

void *
xmalloc(int amt)
{
    char *value;

    if (xmalloc_warnings) {
	run_warning("Should not be calling xmalloc (requested %d bytes)", amt);
	/* Keep going though. */
    }
    if (memory_exhausted)
      return NULL;
    /* Do the actual allocation. */
    value = (char *) malloc(amt);

#if 1
    /* Don't die if the only reason why malloc returned NULL is that it was asked to 
    allocate zero blocks. This hack makes it possible to run games with terrain images 
    that are incompatible with the mac interface (should eventually fix imaging code 
    instead). */ 
    if (value == NULL && amt != 0) {
#else
//    if (value == NULL) {
#endif

#ifdef DEBUGGING
	if (Debug) {
	    Dprintf("Unable to allocate %d bytes.\n", amt);
	    /* Write out the allocation report if possible. */
	    report_malloc();
	}
#endif /* DEBUGGING */
	/* This is pretty serious, no way to recover. */
	memory_exhausted = TRUE;
	run_error("Memory exhausted!!");
	/* In case run_error doesn't exit. */
  	exit(1);
    }
    /* Save callers from having to clear things themselves. */
    memset(value, 0, amt);
#ifdef DEBUGGING
    /* This can't be controlled by `Debug', because much allocation
       happens before any command line or dialog options are interpreted. */
    totmalloc += amt;
    grandtotmalloc += amt;
    record_malloc(amt);
#endif /* DEBUGGING */
    return value;
}

NAMESPACE_UTILITY_END
NAMESPACE_XCONQ_END

