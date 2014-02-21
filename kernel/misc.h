/* Random definitions used everywhere in Xconq.
   Copyright (C) 1987-1989, 1991-1997, 1999 Stanley T. Shebs.
   Copyright (C) 2004-2005 Eric A. McDonald.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/*! \file kernel/misc.h
 * \brief Random definitions used everywhere in Xconq.
 *
 * This file contains miscellaneous macro and constant definitions.
 */

#ifndef TRUE
#define TRUE (1)
#endif
#ifndef FALSE
#define FALSE (0)
#endif

#ifndef CONST
/*! \brief const definiton.
 *
 * This is how we do optional const decls. */
#define CONST const
#endif /* CONST */

#ifndef ABS
/*! \brief Absolute value.
 *
 * If the system does not define an absolute value
 * macro, use this definition.  The absolute value of a 
 * number is the number stripped of it's sign and expressed
 * as a positive number.  I.e. ABS(-3) == 3, ABS(3) == 3.
 * \param x is the number of which to take the absolute value.
 * \return the absolute value of x.
 */
#define ABS(x) (((x) < 0) ? (0 - (x)) : (x))
#endif

#ifndef min
/*! \brief Minimum value.
 *
 * If the system doesn't define a minimum value macro or
 * function, use this one.  Find the smaller of two numbers.
 * \param x is a number.
 * \param y is a number.
 * \return the smaller of the two numbers.
 */
#define min(x,y) (((x) < (y)) ? (x) : (y))
#endif

#ifndef max
/*! \brief Maximum value.
 *
 * If the system doesn't define a maximum value macro or
 * function, use this one.  Find the larger of two numbers.
 * \param x is a number.
 * \param y is a number.
 * \return the larger of the two numbers.
 */
#define max(x,y) (((x) > (y)) ? (x) : (y))
#endif

/*! \brief Is between.
 *
 *  Determines if a number is between two other numbers (inclusive).
 *  \param lo is the low value to check
 *  \param n is the number to be checked
 *  \param hi is the high value to check
 *  \return 
 *     - true if n is between lo and hi (inclusive);
 *     - false otherwise.
 */
#define between(lo,n,hi) \
	((lo) <= (n) && (n) <= (hi))

/*! \brief Limit value.
 *
 * Limits the value of the paramter to be between a high and low 
 * value (inclusive).
 * \param lo is the low value
 * \param n is the number to limit
 * \param hi is the high value
 * \return
 *    - lo if n is less than lo;
 *    - n if n is between lo and hi;
 *    - hi if n is greater than hi.
 */
#define limitn(lo,n,hi) ((n) \
    < (lo) ? (lo) : ((n) > (hi) ? (hi) : (n)))

/*! \brief Random true/false.
 *
 *  \return true or false randomly
 */
#define flip_coin() \
	(xrandom(257) % 2)

/*! \brief Average.
 *
 * Calculates the average value.  Note that if sum of
 * the numbers is greater than the range of the fundamental
 * type,the answer will be wrong.
 * \param a is a number
 * \param b is a number
 * \return the average of a and b
 */
#define avg(a,b) \
	(((a) + (b)) / 2)

//! Decompose dice representation into Type 1 dice parts.

#define dice1(N,NUMDICE,SPOTS,OFFSET)	\
    (((0 == ((N) >> 14)) || (3 == ((N) >> 14))) ?   \
     (NUMDICE = 0, SPOTS = 0, OFFSET = (N)) :	\
     (NUMDICE = (((N) >> 11) & 0x07) + 1,  \
      SPOTS = (((N) >> 7) & 0x0f) + 2,	\
      OFFSET = ((N) & 0x8000) ? (-((N) & 0x7f) - 1) : ((N) & 0x7f)))

//! Decompose dice representation into Type 2 dice parts.

#define dice2(N,NUMDICE,SPOTS,OFFSET)	\
    (((0 == ((N) >> 14)) || (3 == ((N) >> 14))) ?   \
     (NUMDICE = 0, SPOTS = 0, OFFSET = (N)) :	\
     (NUMDICE = ((N) & 0x8000) ? -((((N) >> 11) & 0x07) + 1) \
				  : (((N) >> 11) & 0x07) + 1,  \
      SPOTS = (((N) >> 7) & 0x0f) + 2,	\
      OFFSET = ((N) & 0x8000) ? -((N) & 0x7f) : ((N) & 0x7f)))

//! Normalize a number on a scale between -range and +range.

#define normalize_on_pmscale(n,max,range) \
    ((max) ? (((n) * (range)) / (max)) : 0)

#ifndef isspace
/*! \brief Is space.
 *
 * If the system doesn't provide a definition of isspace, this
 * will define it.  If a charact si a space, newline, tab or
 * carraige return, this is a space.
 * \param c is character to test.
 * \return
 *    - true if c is a space, tab, newline, or carriage return;
 *    - false otherwise.
 */
#define isspace(c) \
	((c) == ' ' || (c) == '\n' || (c) == '\t' || (c) == '\r')
#endif

#define lowercase(c) (isupper(c) ? tolower(c) : (c))

#define uppercase(c) (islower(c) ? toupper(c) : (c))

/*! \brief Is string empty.
 *
 * This tests a string to see if it has anything in it.
 * \param s is a char* to a null terminated string, or
 *        a null.
 * \return 
 *    - true if an empty string;
 *    - false otherwise.
 */
#define empty_string(s) \
	((s) == NULL || s[0] == '\0')

/*! \brief ??? */
extern char spbuf[];
/*! \brief ??? */
extern char tmpbuf[];

/*! \brief Table of Packed Booleans.
 *
 * This structure contains an array of packed booleans.
 *
 */

typedef struct a_packed_bool_table {
    int rdim1size;      /* Reduced size of the first table dimension. */
    int dim2size;       /* Normal size of the second dimension. */
    short sizeofint;    /* Precalculated sizeof(int) in bits. */
    short whichway;     /* Which packing direction is more efficient? */
    int *pbools;        /* Pointer to the packed booleans. */
} PackedBoolTable;

/*! \brief Get a boolean value from a table of packed booleans.
 */

#ifdef get_packed_bool
#undef get_packed_bool
#endif
#define get_packed_bool(tbl,m,n)    \
    (((tbl)->pbools[((tbl)->whichway ? (m) : (n)) * (tbl)->rdim1size + ((tbl)->whichway ? (n) : (m)) / (tbl)->sizeofint] & (1 << ((tbl)->whichway ? (n) : (m)) % (tbl)->sizeofint)) ? TRUE : FALSE)

/*! \brief Set a boolean value in a table of packed booleans.
 */

#ifdef set_packed_bool
#undef set_packed_bool
#endif
#define set_packed_bool(tbl,m,n,boolval)    \
    ((boolval) ? ((tbl)->pbools[((tbl)->whichway ? (m) : (n)) * (tbl)->rdim1size + ((tbl)->whichway ? (n) : (m)) / (tbl)->sizeofint] |= (1 << ((tbl)->whichway ? (n) : (m)) % (tbl)->sizeofint)) : ((tbl)->pbools[((tbl)->whichway ? (m) : (n)) * (tbl)->rdim1size + ((tbl)->whichway ? (n) : (m)) / (tbl)->sizeofint] &= ~(1 << ((tbl)->whichway ? (n) : (m)) % (tbl)->sizeofint)))

/*! \brief Validate a supposed table of packed booleans.
 */

#ifdef valid_packed_bool_table
#undef valid_packed_bool_table
#endif
#define valid_packed_bool_table(tbl) \
    (((tbl) != NULL) && ((tbl)->pbools != NULL) && ((tbl)->sizeofint > 0) && ((tbl)->rdim1size >= 0) && ((tbl)->dim2size > 0))

/*! \brief Prototypes of functions used in the packed boolean implementation.
 */

extern PackedBoolTable* create_packed_bool_table(int m, int n);
extern void init_packed_bool_table(PackedBoolTable* tbl);
extern void destroy_packed_bool_table(PackedBoolTable* tbl);

/* Debugging definitions. */
/* If asserts are off, then an appropriate user diagnostic will be 
   generated and/or a flow control statement will be used. If asserts are on, 
   then we obviously get the debugging benefits and abortive behavior. */

#define assert_warning(cond, msg) \
    { assert(cond); if (!(cond)) run_warning(msg); }
#define assert_warning_break(cond, msg) \
    { assert(cond); if (!(cond)) { run_warning(msg); break; } }
#define assert_warning_continue(cond, msg) \
    { assert(cond); if (!(cond)) { run_warning(msg); continue; } }
#define assert_warning_goto(cond, msg, label) \
    { assert(cond); if (!(cond)) { run_warning(msg); goto label; } }
#define assert_warning_return(cond, msg, retval) \
    { assert(cond); if (!(cond)) { run_warning(msg); return retval; } }
#define assert_error(cond, msg) \
    { assert(cond); if (!(cond)) run_error(msg); }
#define assert_break(cond) \
    { assert(cond); if (!(cond)) break; }
#define assert_continue(cond) \
    { assert(cond); if (!(cond)) continue; }
#define assert_goto(cond, label) \
    { assert(cond); if (!(cond)) goto label; }
#define assert_return(cond, retval) \
    { assert(cond); if (!(cond)) return retval; }

#ifdef DEBUGGING

/* Debugging definitions. */

#define Dprintf if (Debug && dfp) debug_printf
#define DMprintf if (DebugM && dmfp) debugm_printf
#define DGprintf if (DebugG && dgfp) debugg_printf

#define Dprintlisp(X) if (Debug && dfp) fprintlisp(dfp, (X))
#define DMprintlisp(X) if (DebugM && dmfp) fprintlisp(dmfp, (X))
#define DGprintlisp(X) if (DebugG && dgfp) fprintlisp(dgfp, (X))

/* If the debug flags are not macros, then declare them as globals. */

#ifndef Debug
extern int Debug;
#endif
#ifndef DebugM
extern int DebugM;
#endif
#ifndef DebugG
extern int DebugG;
#endif

extern FILE *dfp;
extern FILE *dmfp;
extern FILE *dgfp;

#else /* DEBUGGING */

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

#endif /* DEBUGGING */

/*! \brief Library path.
 *
 * List of paths for the library.
 */
typedef struct a_library_path {
    char *path;                     /*!< Path */
    struct a_library_path *next;    /*!< Pointer to next path */
} LibraryPath;

/*! \brief Library path list.
 *
 * List of \ref a_library_path "LibraryPath's" to Xconq libraries (games)
 */
extern LibraryPath *xconq_libs;

/*! \brief Last user library.
 *
 * Pointer to last \ref a_library_path "LibraryPath" loaded by user.
 */
extern LibraryPath *last_user_xconq_lib;

/*! \brief Iterate library paths.
 *
 * This defines a for loop header to walk through
 * all library paths in a library path list.
 * \param p is a \ref a_library_path "LibraryPath" iteration variable..
 */
#define for_all_library_paths(p)  \
  for (p = xconq_libs; p != NULL; p = p->next)

extern void init_xrandom(int seed);
extern int xrandom(int m);
extern int probability(int prob);
extern int prob_fraction(int n);

/* Dice Information and Manipulation */

namespace Xconq {

//! Internal dice representation. Two bytes.
typedef short   DiceRep;

//! Minimum value from Type 1 dice.
extern Z32 dice1_roll_min(DiceRep dice);
//! Minimum value from Type 2 dice.
extern Z32 dice2_roll_min(DiceRep dice);
//! Mean value from Type 1 dice.
extern Z32 dice1_roll_mean(DiceRep dice);
//! Mean value from Type 2 dice.
extern Z32 dice2_roll_mean(DiceRep dice);
//! Maximum value from Type 1 dice.
extern Z32 dice1_roll_max(DiceRep dice);
//! Maximum value from Type 2 dice.
extern Z32 dice2_roll_max(DiceRep dice);
//! Roll Type 1 dice.
extern Z32 roll_dice1(DiceRep dice);
//! Roll Type 2 dice.
extern Z32 roll_dice2(DiceRep dice);
//! Scale Type 1 dice by a percentage.
extern DiceRep multiply_dice1(DiceRep dice, int mult);
//! Scale Type 2 dice by a percentage.
extern DiceRep multiply_dice2(DiceRep dice, int mult);

}

extern void *xmalloc(int amt);
extern void report_malloc(void);
extern void tprintf(char *buf, char *str, ...);
extern void tnprintf(char *buf, int n, char *str, ...);
extern int select_by_weight(int *arr, int numvals);
extern char *copy_string(char *str);
extern char *pad_blanks(char *str, int n);
extern int iindex(int ch, char *str);
extern long idifftime(time_t t1, time_t t0);
extern void case_panic(char *str, int var);
extern int isqrt(int i);
extern void init_debug_to_stdout(void);
extern void update_debugging(void);
extern void toggle_debugging(int *flagp);
extern void debug_printf(char *str, ...);
extern void debugm_printf(char *str, ...);
extern void debugg_printf(char *str, ...);

extern void prealloc_debug(void);

extern void record_activity_start(char *type, int detail);
extern void record_activity_end(char *type, int detail);
extern void dump_activity_trace(void);

extern void vtprintf(char *buf, char *str, va_list ap);

extern void log_warning(char *typ, char *str);

/* Needed for emergency game savers. */
extern int write_entire_game_state(char *fname);

/* Called from various places. */
extern char *find_name(char *fname);

/* New wrapper for fopen defined in mac.c and unix.c. */
extern FILE *open_file(char *filename, char *mode);

/* For coordinated saving of network games. */
extern void save_game(char *fname);

/* Needed by Unix and Windows. */
extern void close_displays(void);
