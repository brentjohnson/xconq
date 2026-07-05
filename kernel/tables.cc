/* Definitions and functions for Xconq's tables.
   Copyright (C) 1991, 1992, 1993, 1994, 1996, 1998 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "config.h"
#include "misc.h"
#include "lisp.h"
#include "game.h"

/* Fill in the array that defines the characteristics of each table. */

TableDefn tabledefns[] = {

#undef  DEF_UU_TABLE
#define DEF_UU_TABLE(NAME,FNAME,DOC,TABLE,CNST,LO,DFLT,HI,VALTYPE)  \
    { NAME, FNAME, DOC, &TABLE, DFLT, LO, HI, UTYP, UTYP, VALTYPE },
#undef  DEF_UM_TABLE
#define DEF_UM_TABLE(NAME,FNAME,DOC,TABLE,CNST,LO,DFLT,HI,VALTYPE)  \
    { NAME, FNAME, DOC, &TABLE, DFLT, LO, HI, UTYP, MTYP, VALTYPE },
#undef  DEF_UT_TABLE
#define DEF_UT_TABLE(NAME,FNAME,DOC,TABLE,CNST,LO,DFLT,HI,VALTYPE)  \
    { NAME, FNAME, DOC, &TABLE, DFLT, LO, HI, UTYP, TTYP, VALTYPE },
#undef  DEF_TM_TABLE
#define DEF_TM_TABLE(NAME,FNAME,DOC,TABLE,CNST,LO,DFLT,HI,VALTYPE)  \
    { NAME, FNAME, DOC, &TABLE, DFLT, LO, HI, TTYP, MTYP, VALTYPE },
#undef  DEF_TT_TABLE
#define DEF_TT_TABLE(NAME,FNAME,DOC,TABLE,CNST,LO,DFLT,HI,VALTYPE)  \
    { NAME, FNAME, DOC, &TABLE, DFLT, LO, HI, TTYP, TTYP, VALTYPE },
#undef  DEF_MM_TABLE
#define DEF_MM_TABLE(NAME,FNAME,DOC,TABLE,CNST,LO,DFLT,HI,VALTYPE)  \
    { NAME, FNAME, DOC, &TABLE, DFLT, LO, HI, MTYP, MTYP, VALTYPE },
#undef  DEF_UA_TABLE
#define DEF_UA_TABLE(NAME,FNAME,DOC,TABLE,CNST,LO,DFLT,HI,VALTYPE)  \
    { NAME, FNAME, DOC, &TABLE, DFLT, LO, HI, UTYP, ATYP, VALTYPE },
#undef  DEF_AM_TABLE
#define DEF_AM_TABLE(NAME,FNAME,DOC,TABLE,CNST,LO,DFLT,HI,VALTYPE)  \
    { NAME, FNAME, DOC, &TABLE, DFLT, LO, HI, ATYP, MTYP, VALTYPE },
#undef  DEF_AA_TABLE
#define DEF_AA_TABLE(NAME,FNAME,DOC,TABLE,CNST,LO,DFLT,HI,VALTYPE)  \
    { NAME, FNAME, DOC, &TABLE, DFLT, LO, HI, ATYP, ATYP, VALTYPE },

#include "table.def"

    { NULL }
};

/* Define the accessor functions themselves. */

#undef  DEF_UU_TABLE
#define DEF_UU_TABLE(name,FNAME,doc,TABLE,CNST,lo,DFLT,hi,valtype)  \
  short *TABLE = NULL, CNST = DFLT;  \
  int FNAME(int u1, int u2) \
    { checku(u1);  checku(u2);  \
      return (TABLE ? TABLE[numutypes*(u1)+(u2)] : CNST); }

#undef  DEF_UM_TABLE
#define DEF_UM_TABLE(name,FNAME,doc,TABLE,CNST,lo,DFLT,hi,valtype)  \
  short *TABLE = NULL, CNST = DFLT;  \
  int FNAME(int u, int m) \
    { checku(u);  checkm(m);  \
      return (TABLE ? TABLE[nummtypes*(u)+(m)] : CNST); }

#undef  DEF_UT_TABLE
#define DEF_UT_TABLE(name,FNAME,doc,TABLE,CNST,lo,DFLT,hi,valtype)  \
  short *TABLE = NULL, CNST = DFLT;  \
  int FNAME(int u, int t) \
    { checku(u);  checkt(t);  \
      return (TABLE ? TABLE[numttypes*(u)+(t)] : CNST); }

#undef  DEF_TM_TABLE
#define DEF_TM_TABLE(name,FNAME,doc,TABLE,CNST,lo,DFLT,hi,valtype)  \
  short *TABLE = NULL, CNST = DFLT;  \
  int FNAME(int t, int m) \
    { checkt(t);  checkm(m);  \
      return (TABLE ? TABLE[nummtypes*(t)+(m)] : CNST); }

#undef  DEF_TT_TABLE
#define DEF_TT_TABLE(name,FNAME,doc,TABLE,CNST,lo,DFLT,hi,valtype)  \
  short *TABLE = NULL, CNST = DFLT;  \
  int FNAME(int t1, int t2) \
    { checkt(t1);  checkt(t2);  \
      return (TABLE ? TABLE[numttypes*(t1)+(t2)] : CNST); }

#undef  DEF_MM_TABLE
#define DEF_MM_TABLE(name,FNAME,doc,TABLE,CNST,lo,DFLT,hi,valtype)  \
  short *TABLE = NULL, CNST = DFLT;  \
  int FNAME(int m1, int m2) \
    { checkm(m1);  checkm(m2);  \
      return (TABLE ? TABLE[nummtypes*(m1)+(m2)] : CNST); }

#undef  DEF_UA_TABLE
#define DEF_UA_TABLE(name,FNAME,doc,TABLE,CNST,lo,DFLT,hi,valtype)  \
  short *TABLE = NULL, CNST = DFLT;  \
  int FNAME(int u, int a) \
    { checku(u);  checka(a);  \
      return (TABLE ? TABLE[numatypes*(u)+(a)] : CNST); }

#undef  DEF_AM_TABLE
#define DEF_AM_TABLE(name,FNAME,doc,TABLE,CNST,lo,DFLT,hi,valtype)  \
  short *TABLE = NULL, CNST = DFLT;  \
  int FNAME(int a, int m) \
    { checka(a);  checkm(m);  \
      return (TABLE ? TABLE[nummtypes*(a)+(m)] : CNST); }

#undef  DEF_AA_TABLE
#define DEF_AA_TABLE(name,FNAME,doc,TABLE,CNST,lo,DFLT,hi,valtype)  \
  short *TABLE = NULL, CNST = DFLT;  \
  int FNAME(int a1, int a2) \
    { checka(a1);  checka(a2);  \
      return (TABLE ? TABLE[numatypes*(a1)+(a2)] : CNST); }

#include "table.def"

