/* Random utilities not classifiable elsewhere.
   Copyright (C) 1987-1989, 1991-1997, 1999 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "config.h"
#include "misc.h"
#include "lisp.h"
#include "dir.h"

using namespace Xconq;

/* Accommodate certain obsolete (pre-ANSI) Unix systems. (?) */

#ifdef TIME_RETURNS_LONG
#ifndef time_t
#define time_t long
#endif
#endif

/* The remote id of this program. */

int my_rid = 0;

/* The remote id of the current master program. */

int master_rid = 0;

/* The list of pathnames to library directories. */

LibraryPath *xconq_libs;

LibraryPath *last_user_xconq_lib;

/* Direction-to-delta conversions. */

int dirx[NUMDIRS] = DIRX;

int diry[NUMDIRS] = DIRY;

/* If true, warnings will not be issued. */

int warnings_suppressed;

/* If true, errors and warnings will be recorded in a file. */

int warnings_logged = TRUE;

/* If true, allocation attempts will garner complaints, but not actually
   fail.  (This is primarily for debugging addon code, such as new AIs
   and user interfaces.) */

int xmalloc_warnings;

/* If true, all allocation will fail, suppresses further attempts. */

int memory_exhausted;

static char *padbuf;

/* Random number handling is important to game but terrible/nonexistent
   in some systems.  Do it ourselves and hope it's better.  Unfortunately,
   it's hard to prevent clueless hackers from calling rand or random
   and thereby confusing everything, so there (should be) is a check in
   the test dir that searches for calls to these. */

/* The random state *must* be at least 32 bits. */

long initrandstate = -1;

long randstate = -1;

/* Seed can come from elsewhere, for repeatability.  Otherwise, it comes
   from the current time, scaled down to where 32-bit arithmetic won't
   overflow. */

void
init_xrandom(int seed)
{
    time_t tm;
    	
    if (seed >= 0) {
	/* If the random state is already set, changes are somewhat
	   suspicious. */
	if (randstate >= 0) {
	    Dprintf("Randstate being changed from %d to %d\n",
		    randstate, seed);
	}
	randstate = seed;
    } else {
    	time(&tm);
    	randstate = (int) tm;
    }
    /* Whatever its source, put the randstate into known range (0 - 99999). */
    randstate = ABS(randstate);
    randstate %= 100000;
    /* This is kept around for the sake of error reporting. */
    initrandstate = randstate;
}

/* Numbers lifted from Numerical Recipes, p. 198. */
/* Arithmetic must be 32-bit. */

#ifdef _MSC_VER
#define INT64 __int64
#else
#define INT64 long long
#endif

int
xrandom(int m)
{
    int rslt;

    INT64 bignum;

    randstate = (8121 * randstate + 28411) % 134456;
    /* Random numbers in too large of a range will overflow 32-bit
       arithmetic, but Xconq rarely needs the large ranges, so only do
       the 64-bit arithmetic if necessary. */
    if (m <= 10000) {
	rslt = (m * randstate) / 134456;
    } else {
	bignum = m * ((INT64) randstate);
	rslt = bignum / 134456;
    }
    return rslt;
}

/* Percentage probability, with bounds checking. */

int
probability(int prob)
{
    if (prob <= 0)
      return FALSE;
    if (prob >= 100)
      return TRUE;
    return (xrandom(100) < prob);
}

namespace Xconq {

Z32
dice1_roll_min(DiceRep dice)
{
    Z16u numdice = 0, die = 0;
    Z16 offset = 0;

    dice1(dice, numdice, die, offset);
    return (Z32)((0 < offset) ? offset : 0) + numdice;
}

Z32
dice2_roll_min(DiceRep dice)
{
    Z16u die = 0;
    Z16 numdice = 0, offset = 0;

    dice2(dice, numdice, die, offset);
    return (Z32)offset + numdice;
}

Z32
dice1_roll_mean(DiceRep dice)
{
    Z16u numdice = 0, die = 0;
    Z16 offset = 0;

    dice1(dice, numdice, die, offset);
    return (Z32)((0 < offset) ? offset : 0) + numdice + ((die - 1)/2)*numdice;
}

Z32
dice2_roll_mean(DiceRep dice)
{
    Z16u die = 0;
    Z16 numdice = 0, offset = 0;

    dice2(dice, numdice, die, offset);
    return (Z32)offset + numdice + ((die - 1)/2)*numdice;
}

Z32
dice1_roll_max(DiceRep dice)
{
    Z16u numdice = 0, die = 0;
    Z16 offset = 0;

    dice1(dice, numdice, die, offset);
    return (Z32)((0 < offset) ? offset : 0) + numdice*die;
}

Z32
dice2_roll_max(DiceRep dice)
{
    Z16u die = 0;
    Z16 numdice = 0, offset = 0;

    dice2(dice, numdice, die, offset);
    return (Z32)offset + numdice*die;
}

Z32
roll_dice1(DiceRep dice)
{
    Z16u numdice = 0, die = 0, i = 0;
    Z16 offset = 0;
    Z32 rslt = 0;

    dice1(dice, numdice, die, offset);
    if (!numdice)
	return offset;
    rslt = offset;
    for (i = 0; i < numdice; ++i)
	rslt += (1 + xrandom(die));
    return rslt;
}

Z32
roll_dice2(DiceRep dice)
{
    Z16u die = 0, i = 0;
    Z16 numdice = 0, offset = 0;
    Z32 rslt = 0;

    dice2(dice, numdice, die, offset);
    if (!numdice)
	return offset;
    rslt = offset;
    for (i = 0; i < numdice; ++i)
	rslt += (1 + xrandom(die));
    return rslt;
}

DiceRep
multiply_dice1(DiceRep dice, int mult)
{
    Z16u numdice = 0, die = 0;
    Z16 offset = 0;

    dice1(dice, numdice, die, offset);
    if (!numdice) {
	offset = (offset * mult) / 100;
	if (0x4000 & offset)
	    run_warning("Scaled fixed dice value, %d, is out-of-range",
			offset);
	offset = max(-16384, offset);
	offset = min(16383, offset);
	return offset;
    }
    die = (die * mult) / 100;
    if (!between(2, die, 17))
	run_warning("Scaled die has %d spots. Must be between 2 and 17", die);
    die = max(2, die);
    die = min(17, die);
    offset = (offset * mult) / 100;
    if (!between(-128, offset, 127))
	run_warning("Scaled dice offset is %d. Must be between -128 and 127",
		    offset);
    offset = max(-128, offset);
    offset = min(127, offset);
    return 
	(DiceRep)(
	    (((0 < offset) ? 0 : 1) << 15) | (1 << 14)
	    | ((numdice - 1) << 11) | ((die - 2) << 7) 
	    | (((0 < offset) ? offset : -offset - 1) & 0x7f));
}

DiceRep
multiply_dice2(DiceRep dice, int mult)
{
    Z16u die = 0;
    Z16 numdice = 0, offset = 0;

    dice2(dice, numdice, die, offset);
    if (!numdice) {
	offset = (offset * mult) / 100;
	if (0x4000 & offset)
	    run_warning("Scaled fixed dice value, %d, is out-of-range",
			offset);
	offset = max(-16384, offset);
	offset = min(16383, offset);
	return offset;
    }
    die = (die * mult) / 100;
    if (!between(2, die, 17))
	run_warning("Scaled die has %d spots. Must be between 2 and 17", die);
    die = max(2, die);
    die = min(17, die);
    offset = (offset * mult) / 100;
    if (0 < numdice) {
	if (!between(0, offset, 127))
	    run_warning("Scaled dice offset is %d. Must be between 0 and 127",
			offset);
	offset = max(0, offset);
	offset = min(127, offset);
    }
    else {
	if (!between(-127, offset, 0))
	    run_warning("Scaled dice offset is %d. Must be between -127 and 0",
			offset);
	offset = max(-127, offset);
	offset = min(0, offset);
    }
    return 
	(DiceRep)(
	    (((0 < offset) ? 0 : 1) << 15) | (1 << 14)
	    | (((0 < offset) ? numdice - 1 : -numdice - 1) << 11) 
	    | ((die - 2) << 7) 
	    | (((0 < offset) ? offset : -offset) & 0x7f));
}

} // namespace Xconq

/* For a number in the range 0 - 10000, divide it by 100 and use the
   remainder as the probability of adding 1. */

int
prob_fraction(int n)
{
  return (n / 100 + (probability(n % 100) ? 1 : 0));
}

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

/* Like sprintf, but appends. */

void
tprintf(char *buf, char *str, ...)
{
    va_list ap;
    char line[TBUFSIZE];

    va_start(ap, str);
    vsnprintf(line, TBUFSIZE, str, ap);
    strcat(buf, line);
    va_end(ap);
}

void
tnprintf(char *buf, int n, char *str, ...)
{
    va_list ap;
    int n1 = n - strlen(buf);
    char line[TBUFSIZE];

    if (n1 > 0) {
	va_start(ap, str);
	vsnprintf(line, TBUFSIZE, str, ap);
	strncat(buf, line, n1);
	va_end(ap);
    }
}

void
vtprintf(char *buf, char *str, va_list ap)
{
    char line[TBUFSIZE];

    vsnprintf(line, TBUFSIZE, str, ap);
    strcat(buf, line);
}

int
select_by_weight(int *arr, int numvals)
{
    int sum = 0, i, n;

    sum = 0;
    for (i = 0; i < numvals; ++i) {
	if (arr[i] <= 0)
	  continue;
	sum += arr[i];
    }
    if (sum == 0)
      return -1;
    /* We now know the range, make a random index into it. */
    n = xrandom(sum);
    /* Go through again to figure out which choice the index refs. */
    sum = 0;
    for (i = 0; i < numvals; ++i) {
	if (arr[i] <= 0)
	  continue;
	sum += arr[i];
	if (sum >= n) {
	    return i;
	}
    }
    run_error("Ooh weird");
    return -1;
}

/* Copy a string to newly-allocated space.  The new space is never freed. */

char *
copy_string(char *str)
{
    int len = strlen(str);
    char *rslt;

    rslt = (char *)xmalloc(len + 1);
    strcpy(rslt, str);
#ifdef DEBUGGING
    copymalloc += len + 1;
#endif
    return rslt;
}

/* Insert the given number of blanks between each char of the string. */

char *
pad_blanks(char *str, int n)
{
    char *pb;
    int i;
  
    if (padbuf == NULL)
      padbuf = (char *)xmalloc(BUFSIZE);
    pb = padbuf;
    while (*str && pb < padbuf + BUFSIZE - 2) {
	*pb++ = *str++;
	if (*str) {
	    for (i = 0; i < n; i++)
	      *pb++ = ' ';
	}
    }
    *pb = '\0';
    return padbuf;
}

/* Get a *numeric* index into a string (more useful than ptr, in Xconq).
   Return -1 on failed search. */

int
iindex(int ch, char *str)
{
    int i;

    if (str == NULL)
      return (-1);
    for (i = 0; str[i] != '\0'; ++i)
      if (ch == str[i])
        return i;
    return (-1);
}

/* Return a time difference as an long integer number of seconds. */

long
idifftime(time_t t1, time_t t0)
{
    return ((long) difftime(t1, t0));
}

/* This little routine goes at the end of all switch statements on internal
   data values. */

void
case_panic(char *str, int var)
{
    run_error("Panic! Unknown %s %d", str, var);
}

/* Integer square root - good enough, no float trickery or libs needed. */

/* Improved version from Bruce Fast, via Scott Herod. */

int
isqrt(int i)
{
    int j, k;

    if (i > 3) {
        for (j = i, k = -1; j >>= 2; k <<= 1)
        	   ;
        k = (~k + i / ~k) / 2;
	k = (k + i / k) / 2;
	k = (1 + k + i / k) / 2;
	return (k + i / k) / 2;
    } else if (i > 0) {
	return 1;
    } else {
	return 0;
    }
}

#ifdef DEBUGGING

/* Debugging flags.
   These are set up so that they can be macros or variables, as desired. */

#ifndef Debug
int Debug = FALSE;
#endif
#ifndef DebugM
int DebugM = FALSE;
#endif
#ifndef DebugG
int DebugG = FALSE;
#endif

/* These are where normal debugging output goes. */

FILE *dfp = NULL;
FILE *dgfp = NULL;
FILE *dmfp = NULL;

/* Map all the debugging outputs to stdout. */
/* You might think stderr would be a better choice, but this way
   the debugging output is sync'ed properly (read: consistently!)
   with "normal" stdout output. */

void
init_debug_to_stdout()
{
    dfp  = stdout;
    dgfp = stdout;
    dmfp = stdout;
}

/* Junk associated with debug output. */

/* This is all fairly elaborate because we need to be able to collect
   detailed logs of AI activity over different periods of time, and just
   dumping to stdout doesn't work in a window system. */

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

/* Debug output goes to a file. */

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

#if 0		/* Unused. */

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

/* The following code records execution traces of game activities.  This
   is generally only useful for high-level code such as run_game. */

struct a_timeline_entry {
  int start;
  int end;
  char *type;
  int detail1;
  int detail2;
};

#define BLOCKSIZE 10000

typedef struct a_timeline_block {
  struct a_timeline_entry entries[BLOCKSIZE];
  int index;
  struct a_timeline_block *next;
} TimelineBlock;

int activity_trace;

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

/* Handling for various kinds of failures. */

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

/* A run error is fatal. */

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

/* Runtime warnings are for when it's important to bug the players,
   usually a problem with Xconq or a game design. */

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

/* When we log a warning, we want to open and close the file each time,
   just in case the warning immediately precedes disaster. */

void
log_warning(char *typ, char *str)
{
    static int first_warning = TRUE;
    static FILE *ewfp;

    if (ewfp == NULL) {
	/* (should make filename platform-specific) */
	ewfp = open_file("Xconq.Warnings", (char *)(first_warning ? "w" : "a"));
	/* (should record date) */
    }
    if (ewfp != NULL) {
	fprintf(ewfp, "%s: %s\n", typ, str);
	fclose(ewfp);
	ewfp = NULL;
	first_warning = FALSE;
    }
}

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
