/* Shared "minimal interface" callback stubs, factored out of skelconq.cc so
   that both skelconq (the headless game-driving interface) and the
   test/unit unittests binary can link the same set of functions the kernel
   requires every interface to supply (see the "Declarations of functions
   that must be supplied by an interface" block in conq.h). skelconq.cc keeps
   everything specific to its interactive command loop (main, get_input,
   interpret_command, the cmdtable, etc.); this file has no interactive
   behavior at all, so it is equally usable by a program that never reads a
   command from stdin.

   Copyright (C) 1991-1997, 1999-2000 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#ifndef SKELCONQ_STUBS_H
#define SKELCONQ_STUBS_H

/* Like the rest of the kernel's headers, this one has no include guard
   dependency on conq.h and assumes the includer already pulled in conq.h
   first (skelconq.cc and skelconq_stubs.cc both do). */

/* This structure maintains state that is local to a side's display.
   At the very least, it must track when the display is open and closed. */
typedef struct a_ui {
    int active;
} UI;

extern Side *defaultside;
extern time_t skelturnstart;
extern int freerunturns;
extern int numcellupdatesperturn;
extern int numusefulcellupdatesperturn;
extern int linemiddle;

/* skelconq's original policy (unchanged here): any init/run warning is a
   sign the game/module is broken, so it aborts the process rather than
   limping on (this is what lets check-lib/actions/save/test fail on a
   logged warning; see test/common.sh). The unit-test binary wants to
   provoke a handful of reader warnings on purpose (malformed-input checks)
   and inspect them instead of dying, so it flips tolerate_warnings around
   just those calls and checks warningcount; skelconq itself never touches
   either variable, so its behavior is exactly as before. */
extern int tolerate_warnings;
extern unsigned int warningcount;

#endif /* SKELCONQ_STUBS_H */
