/* Fake definitions of the high-level kernel routines that the low-level
   GDL reader (conqlow: lisp.cc, util.cc, unix.cc) references but that the
   fuzz harness does not need -- mirrors the stub set in kernel/imf2imf.cc,
   which links the same low-level library standalone.

   Difference from imf2imf: the warning/error callbacks are silent and
   non-exiting.  During fuzzing, reader diagnostics ("comment not closed",
   "extra close paren", dice out of range, token buffer overflow) fire
   constantly on malformed input and are EXPECTED; only ASan/UBSan aborts
   should stop the run, so these must not print (flooding output) or exit.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "config.h"
#include "misc.h"
#include "lisp.h"

/* Globals the low-level library expects the interface layer to define. */
char *xconqlib = (char *) "";
char spbuf[BUFSIZE];
char readerrbuf[BUFSIZE];

/* --- UI callbacks: silent and non-fatal, see header comment. --- */

void
low_init_error(const char *str)
{
    (void) str;
}

void
low_init_warning(char *str)
{
    (void) str;
}

void
low_run_error(char *str)
{
    (void) str;
}

void
low_run_warning(char *str)
{
    (void) str;
}

void
close_displays(void)
{
}

void
announce_read_progress(void)
{
}

void
syntax_error(Obj *x, const char *msg)
{
    (void) x;
    (void) msg;
}

/* --- Reader linkage fakes (unreachable from the pure lisp reader). --- */

int
keyword_code(const char *str)
{
    (void) str;
    return 0;
}

/* The keyword table so keyword_name() links; not consulted by the reader. */
struct a_key {
    char *name;
} keywordtable[] = {

#undef  DEF_KWD
#define DEF_KWD(NAME,code)  { (char *) NAME },

#include "keyword.def"

    { NULL }
};

const char *
keyword_name(enum keywords k)
{
    return keywordtable[k].name;
}

void
init_predefined_symbols(void)
{
}

int
lazy_bind(Obj *sym)
{
    (void) sym;
    return FALSE;
}

void
prealloc_debug(void)
{
}

/* Table machinery and game-saving hooks referenced by lisp.cc/unix.cc but
   never used when only parsing forms. */

struct fake_tabledefn {
    const char *name;
    int (*getter)(int, int);
    const char *doc;
    short **table;
    short dflt;
    short lo;
    short hi;
    char index1;
    char index2;
    char valtype;
} tabledefns[] = {
    { NULL }
};

int
numtypes_from_index_type(int x)
{
    (void) x;
    return 0;
}

int
write_entire_game_state(const char *fname)
{
    (void) fname;
    return FALSE;
}
