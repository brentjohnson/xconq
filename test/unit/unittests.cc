/* Unit tests for pure-kernel logic: the GDL reader/writer, the .def-driven
   type/table machinery, and small utility helpers (dice, pm-scale). These
   run in a single process/session against the kernel's global state (there
   is no teardown), so ordering matters within a section even though the
   sections themselves are independent; see the comment above each one.

   This links kernel/skelconq_stubs.cc for the same reason skelconq does:
   the kernel calls back into a fixed set of functions every interface must
   supply (see the "Declarations of functions that must be supplied by an
   interface" block in conq.h), regardless of whether anything here ever
   opens a display.

   Xconq is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.  See the file COPYING.  */

#include "conq.h"
#include "kernel.h"
#include "skelconq_stubs.h"
#include <stdlib.h>

/* DiceRep and the dice1/dice2 roll helpers live in namespace Xconq (see
   kernel/misc.h); util.cc itself pulls the whole namespace in the same way. */
using namespace Xconq;

static unsigned int checks_run = 0;
static unsigned int checks_failed = 0;

#define CHECK(cond) \
    do { \
	++checks_run; \
	if (!(cond)) { \
	    ++checks_failed; \
	    printf("FAIL: %s:%d: %s\n", __FILE__, __LINE__, #cond); \
	} \
    } while (0)

#define CHECK_EQ_INT(expected, actual) \
    do { \
	long _e = (long) (expected), _a = (long) (actual); \
	++checks_run; \
	if (_e != _a) { \
	    ++checks_failed; \
	    printf("FAIL: %s:%d: expected %ld, got %ld (%s)\n", \
		   __FILE__, __LINE__, _e, _a, #actual); \
	} \
    } while (0)

#define CHECK_EQ_STR(expected, actual) \
    do { \
	const char *_e = (expected), *_a = (actual); \
	++checks_run; \
	if (strcmp(_e, _a) != 0) { \
	    ++checks_failed; \
	    printf("FAIL: %s:%d: expected \"%s\", got \"%s\" (%s)\n", \
		   __FILE__, __LINE__, _e, _a, #actual); \
	} \
    } while (0)

/* Write obj back out the way a save file would: fprintlisp is the
   escape-aware writer (used by write.cc and the interactive "print"
   command); sprintlisp is a separate, simpler printer used for short
   interactive displays and says right in its own source that it does not
   escape special characters, so it is not what a round-trip check wants. */
static const char *
roundtrip(Obj *obj)
{
    static char *buf = NULL;
    static size_t bufsize = 0;
    FILE *fp = open_memstream(&buf, &bufsize);

    fprintlisp(fp, obj);
    fclose(fp);
    return buf;
}

/* ------------------------------------------------------------------ */
/* Section A: the Lisp reader/writer (kernel/lisp.cc). Order-independent
   of everything else -- read_form_from_string/eval/sprintlisp only touch
   the object table and symbol obarray init_lisp() (via
   init_data_structures) already set up. */

static void
test_lisp_roundtrip(void)
{
    Obj *raw;

    printf("Testing GDL reader/writer round trips.\n");

    /* Integers, positive and negative. */
    raw = read_form_from_string("42", NULL, NULL, NULL);
    CHECK(numberp(raw));
    CHECK_EQ_INT(42, c_number(raw));
    CHECK_EQ_STR("42", roundtrip(raw));

    raw = read_form_from_string("-17", NULL, NULL, NULL);
    CHECK(numberp(raw));
    CHECK_EQ_INT(-17, c_number(raw));
    CHECK_EQ_STR("-17", roundtrip(raw));

    /* Empty string. */
    raw = read_form_from_string("\"\"", NULL, NULL, NULL);
    CHECK(stringp(raw));
    CHECK_EQ_STR("", c_string(raw));
    CHECK_EQ_STR("\"\"", roundtrip(raw));

    /* A string with an embedded escaped quote round-trips byte for byte:
       the reader turns \" into a literal ", and the writer re-escapes any
       " it finds when printing back out. */
    raw = read_form_from_string("\"a\\\"b\"", NULL, NULL, NULL);
    CHECK(stringp(raw));
    CHECK_EQ_STR("a\"b", c_string(raw));
    CHECK_EQ_STR("\"a\\\"b\"", roundtrip(raw));

    /* Octal escapes are introduced by a literal leading zero digit (\0101,
       not \101); the reader has no support for a two/three-digit octal
       escape that omits it. \0101 (octal 101 = decimal 65) is 'A'. */
    raw = read_form_from_string("\"\\0101\"", NULL, NULL, NULL);
    CHECK(stringp(raw));
    CHECK_EQ_STR("A", c_string(raw));

    /* Plain symbols. */
    raw = read_form_from_string("foo-bar", NULL, NULL, NULL);
    CHECK(symbolp(raw));
    CHECK_EQ_STR("foo-bar", c_string(raw));
    CHECK_EQ_STR("foo-bar", roundtrip(raw));

    /* |...|-quoted symbols may contain otherwise-special characters
       (here, a space); the writer must re-quote them the same way. */
    raw = read_form_from_string("|foo bar|", NULL, NULL, NULL);
    CHECK(symbolp(raw));
    CHECK_EQ_STR("foo bar", c_string(raw));
    CHECK_EQ_STR("|foo bar|", roundtrip(raw));

    /* Nested lists. */
    raw = read_form_from_string("(a (b c) d)", NULL, NULL, NULL);
    CHECK(consp(raw));
    CHECK_EQ_STR("a", c_string(car(raw)));
    CHECK(consp(cadr(raw)));
    CHECK_EQ_STR("b", c_string(car(cadr(raw))));
    CHECK_EQ_STR("c", c_string(cadr(cadr(raw))));
    CHECK_EQ_STR("d", c_string(caddr(raw)));
    CHECK_EQ_INT(NIL, cdddr(raw)->type);

    /* There are no dotted pairs in this Lisp (see fprintlisp's own comment
       in lisp.cc): a bare "." is read as a self-contained token, and since
       it isn't a digit it falls into the number reader's decimal-point
       branch with no digits on either side, producing the number 0 -- so
       "(a . b)" reads as the 3-element list (a 0 b), not a dotted pair. */
    raw = read_form_from_string("(a . b)", NULL, NULL, NULL);
    CHECK(consp(raw));
    CHECK_EQ_INT(3, length(raw));
    CHECK(numberp(cadr(raw)));
    CHECK_EQ_INT(0, c_number(cadr(raw)));

    /* Dice notation decodes via the DiceRep helpers in Section C; here we
       just confirm the reader recognizes the syntax as a single NUMBER
       token rather than splitting on the 'd' or the sign. */
    raw = read_form_from_string("1d6", NULL, NULL, NULL);
    CHECK(numberp(raw));

    raw = read_form_from_string("(3d6+2)", NULL, NULL, NULL);
    CHECK(consp(raw));
    CHECK_EQ_INT(1, length(raw));
    CHECK(numberp(car(raw)));

    /* A symbol and a number stay distinct tokens whether or not they sit
       next to each other in a list; whitespace/parens are always proper
       delimiters (regression coverage for token-boundary bugs in the
       number/symbol dispatch at the top of read_form_aux). */
    raw = read_form_from_string("(u* 0)", NULL, NULL, NULL);
    CHECK(consp(raw));
    CHECK_EQ_INT(2, length(raw));
    CHECK(symbolp(car(raw)));
    CHECK_EQ_STR("u*", c_string(car(raw)));
    CHECK(numberp(cadr(raw)));
    CHECK_EQ_INT(0, c_number(cadr(raw)));

    printf("\tDone.\n");
}

/* Malformed input must not crash or hang the reader; a couple of these are
   real regressions (see test/fuzz/README.md's "Bugs found and fixed"):
   read_list's EOF-with-no-close-paren case used to loop forever, and the
   BIGBUF token-length cutoff used to write one byte past the buffer.
   Both warn through init_warning, which our low_init_warning stub (in
   skelconq_stubs.cc) normally turns into an immediate process exit; we
   flip tolerate_warnings around these calls to inspect the outcome instead
   of dying mid-test. */
static void
test_lisp_malformed(void)
{
    Obj *raw;
    unsigned int before;
    char oversized[1050];

    printf("Testing malformed GDL input.\n");

    tolerate_warnings = TRUE;

    /* Unterminated list: read_list hits EOF looking for the close paren,
       warns, and returns lispeof instead of hanging or crashing. */
    before = warningcount;
    raw = read_form_from_string("(foo", NULL, NULL, NULL);
    CHECK(raw == lispeof);
    CHECK(warningcount > before);

    /* Unterminated string: read_delimited_text's loop simply stops at EOF
       (only the closing '"'/BIGBUF cases are special-cased), so this is
       *not* a warning case -- the reader just returns whatever text it
       had accumulated, silently. Documented here rather than assumed. */
    before = warningcount;
    raw = read_form_from_string("\"abc", NULL, NULL, NULL);
    CHECK(stringp(raw));
    CHECK_EQ_STR("abc", c_string(raw));
    CHECK_EQ_INT(before, warningcount);

    /* A token at/over the BIGBUF (1000 bytes; kernel/lisp.cc, kept in sync
       here the same way test/fuzz/CMakeLists.txt keeps its reader source
       list in sync) boundary: must truncate to BIGBUF-1 chars plus a NUL,
       not overrun the heap buffer, and it does warn once. */
    memset(oversized, 'x', sizeof oversized - 1);
    oversized[sizeof oversized - 1] = '\0';
    before = warningcount;
    raw = read_form_from_string(oversized, NULL, NULL, NULL);
    CHECK(symbolp(raw));
    CHECK_EQ_INT(999, strlen(c_string(raw)));
    CHECK(warningcount > before);

    tolerate_warnings = FALSE;

    printf("\tDone.\n");
}

/* ------------------------------------------------------------------ */
/* Section B: utility layer (kernel/util.cc, kernel/misc.h). Independent of
   everything else -- dice reps are just packed shorts, constructed here by
   reading the same notation Section A already validated the reader
   accepts. */

static void
test_dice_and_utils(void)
{
    DiceRep d;
    int i, r;

    printf("Testing dice and pm-scale utilities.\n");

    /* "1d6": 1 die, 6 sides, no offset. min 1, mean 1+(6-1)/2 = 3, max 6. */
    d = (DiceRep) c_number(read_form_from_string("1d6", NULL, NULL, NULL));
    CHECK_EQ_INT(1, dice1_roll_min(d));
    CHECK_EQ_INT(3, dice1_roll_mean(d));
    CHECK_EQ_INT(6, dice1_roll_max(d));
    for (i = 0; i < 200; ++i) {
	r = roll_dice1(d);
	CHECK(1 <= r && r <= 6);
    }

    /* "3d6+2": 3 dice, 6 sides, +2 offset.
       min = 2+3 = 5, mean = 2+3+((6-1)/2)*3 = 11, max = 2+3*6 = 20. */
    d = (DiceRep) c_number(read_form_from_string("3d6+2", NULL, NULL, NULL));
    CHECK_EQ_INT(5, dice1_roll_min(d));
    CHECK_EQ_INT(11, dice1_roll_mean(d));
    CHECK_EQ_INT(20, dice1_roll_max(d));
    for (i = 0; i < 200; ++i) {
	r = roll_dice1(d);
	CHECK(5 <= r && r <= 20);
    }

    /* normalize_on_pmscale(n, max, range) scales n from [0, max] onto
       [-range, +range]; also used with negative n and with max == 0 (must
       not divide by zero -- a non-constant zero, so the compiler can't
       just fold the guard away and warn about the unreachable division). */
    {
	int zero_max = 0;

	CHECK_EQ_INT(0, normalize_on_pmscale(0, 100, 1000));
	CHECK_EQ_INT(1000, normalize_on_pmscale(100, 100, 1000));
	CHECK_EQ_INT(500, normalize_on_pmscale(50, 100, 1000));
	CHECK_EQ_INT(-500, normalize_on_pmscale(-50, 100, 1000));
	CHECK_EQ_INT(0, normalize_on_pmscale(50, zero_max, 1000));
    }
}

/* ------------------------------------------------------------------ */
/* Section C: the .def-driven type/table machinery (kernel/types.cc,
   tables.cc, read.cc's interp_form). Unlike the sections above, this one
   is stateful -- interp_form(NULL, ...) accumulates unit/terrain types and
   table contents into kernel-global arrays that are never torn down, so
   the calls below must run in this order (types before the table/gvar
   forms that reference them) and only once per process. */

static void
test_types_and_tables(void)
{
    Obj *form;
    int town, fighter, bomber, plains, swamp;

    printf("Testing type/table/gvar machinery.\n");

    /* Two unit types and two terrain types, defined the same way any GDL
       module does -- interp_form(NULL, ...) is exactly what do_eval calls
       (see skelconq.cc) and what load_game_module ultimately calls per
       top-level form, just without a Module (module-scoped bookkeeping
       like include/variant handling is irrelevant to these forms). */
    form = read_form_from_string("(unit-type town (char \"*\"))",
				  NULL, NULL, NULL);
    interp_form(NULL, form);
    form = read_form_from_string("(unit-type fighter (char \"f\"))",
				  NULL, NULL, NULL);
    interp_form(NULL, form);
    form = read_form_from_string("(unit-type bomber (char \"b\"))",
				  NULL, NULL, NULL);
    interp_form(NULL, form);
    form = read_form_from_string("(terrain-type plains (char \"+\"))",
				  NULL, NULL, NULL);
    interp_form(NULL, form);
    form = read_form_from_string("(terrain-type swamp (char \"~\"))",
				  NULL, NULL, NULL);
    interp_form(NULL, form);

    town = utype_from_name("town");
    fighter = utype_from_name("fighter");
    bomber = utype_from_name("bomber");
    plains = ttype_from_name((char *) "plains");
    swamp = ttype_from_name((char *) "swamp");
    CHECK(town >= 0);
    CHECK(fighter >= 0);
    CHECK(bomber >= 0);
    CHECK(town != fighter && fighter != bomber && town != bomber);
    CHECK(plains >= 0);
    CHECK(swamp >= 0);
    CHECK(plains != swamp);

    /* A u-by-t table: unit-size-in-terrain (kernel/table.def), default 1.
       Set fighter's size explicitly in both terrains to a different value
       per terrain, and bomber's only in plains, to check both orientations
       of the lookup -- holding the unit fixed and varying the terrain, and
       holding the terrain fixed and varying the unit -- against a cell
       that was left at its default. */
    form = read_form_from_string(
	"(table unit-size-in-terrain"
	"  (fighter plains 2)"
	"  (fighter swamp 9)"
	"  (bomber plains 3))",
	NULL, NULL, NULL);
    interp_form(NULL, form);

    CHECK_EQ_INT(1, ut_size(town, plains));   /* untouched cell: default */
    CHECK_EQ_INT(2, ut_size(fighter, plains));
    CHECK_EQ_INT(9, ut_size(fighter, swamp)); /* same unit, other terrain */
    CHECK_EQ_INT(3, ut_size(bomber, plains)); /* other unit, same terrain */

    /* A gvar (kernel/gvar.def): "turn" defaults to 0 and is set the same
       way a module's top-level (set ...) form is. */
    CHECK_EQ_INT(0, g_turn());
    form = read_form_from_string("(set turn 42)", NULL, NULL, NULL);
    interp_form(NULL, form);
    CHECK_EQ_INT(42, g_turn());

    printf("\tDone.\n");
}

int
main(void)
{
    init_data_structures();

    test_lisp_roundtrip();
    test_lisp_malformed();
    test_dice_and_utils();
    test_types_and_tables();

    printf("\n%u checks run, %u failed\n", checks_run, checks_failed);
    return checks_failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
