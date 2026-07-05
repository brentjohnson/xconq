/* libFuzzer harness for Xconq's GDL lisp reader.
   Copyright (C) 2026 Stanley T. Shebs and the Xconq maintainers.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/* This targets the lowest layer of the GDL reader: read_form_from_string()
   in kernel/lisp.cc, the same tokenizer/parser that module loading, save
   restore, and the network transfer protocol all funnel untrusted bytes
   through.  It exercises the interesting parsing surface (strings and their
   octal/backslash escapes, |bar| symbols, dice/decimal/percent numbers,
   nested lists, block comments, the BIGBUF token buffer) WITHOUT building a
   game world, so a crash here is a real reader bug rather than an init-order
   artifact.  Interpretation of the parsed forms (interp_form in read.cc)
   needs full world state and file I/O side effects and is deliberately NOT
   driven here -- see test/fuzz/README.md.

   Global-state notes: the lisp package (symbol table, lispnil/lispeof, the
   shared lispstrbuf token buffer) is process-global with no teardown, so
   init_lisp() runs exactly once in LLVMFuzzerInitialize.  Each iteration then
   parses independently: read_form_aux/read_delimited_text rewrite lispstrbuf
   from scratch every call, so iteration N cannot be corrupted by N-1's
   leftovers.  Interned symbols do accumulate across iterations (unbounded but
   bounded-per-input growth); that is acceptable memory growth, not
   corruption -- run with a generous -rss_limit_mb (see README).  */

#include "config.h"
#include "misc.h"
#include "lisp.h"

#include <cstdint>
#include <cstdlib>
#include <cstring>

/* Defined in kernel/util.cc; defaults TRUE, which makes every init_warning
   append to an "Xconq.Warnings" file in the cwd.  Malformed fuzz input warns
   constantly, so leaving it on writes a giant log and makes the run I/O-bound. */
extern int warnings_logged;

extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv)
{
    (void) argc;
    (void) argv;
    /* One-time setup of lispnil/lispeof, the symbol table, and lispstrbuf. */
    init_lisp();
    /* Suppress the on-disk warning log; reader diagnostics are expected here. */
    warnings_logged = FALSE;
    return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* The string reader stops at a NUL, so give it a NUL-terminated copy.
       (An embedded NUL just ends this input early -- acceptable.) */
    char *buf = (char *) malloc(size + 1);
    if (buf == NULL)
      return 0;
    memcpy(buf, data, size);
    buf[size] = '\0';

    /* Parse every top-level form in the buffer, exactly as a module file is
       read: read_form_from_string hands back where it stopped via endstr, so
       we advance until it makes no progress or reports EOF. */
    int startlineno = 0, endlineno = 0;
    const char *cursor = buf;
    for (;;) {
	const char *endstr = cursor;
	Obj *form = read_form_from_string(cursor, &startlineno, &endlineno,
					  &endstr);
	if (form == lispeof || endstr == NULL || endstr <= cursor)
	  break;
	cursor = endstr;
    }

    free(buf);
    return 0;
}
