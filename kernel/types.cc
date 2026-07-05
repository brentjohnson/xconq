/* Data structures defining the properties of types.
   Copyright (C) 1991-1996, 1998, 1999 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "config.h"
#include "misc.h"
#include "lisp.h"
#include "game.h"

/* (Now implemented in 'config.h'.) */
#if (0)
#ifdef offsetof
#define OFFS(TYPE,FLD) (offsetof(TYPE, FLD))
#else
#define OFFS(TYPE,FLD)  \
  ((unsigned int) (((char *) (&(((TYPE *) NULL)->FLD))) - ((char *) NULL)))
#endif
#endif
#define OFFS(TYPE,FLD)	(offsetof(TYPE, FLD))

PropertyDefn utypedefns[] = {

#undef  DEF_UPROP_I
#define DEF_UPROP_I(NAME,FNAME,DOC,SLOT,LO,DFLT,HI)  \
    { NAME, FNAME, NULL, NULL, OFFS(Utype, SLOT), DOC, DFLT, NULL, LO, HI },
#undef  DEF_UPROP_S
#define DEF_UPROP_S(NAME,FNAME,DOC,SLOT,DFLT)  \
    { NAME, NULL, FNAME, NULL, OFFS(Utype, SLOT), DOC,    0, DFLT,  0,  0 },
#undef  DEF_UPROP_L
#define DEF_UPROP_L(NAME,FNAME,DOC,SLOT)  \
    { NAME, NULL, NULL, FNAME, OFFS(Utype, SLOT), DOC,    0, NULL,  0,  0 },

#include "utype.def"

    { NULL }
};

PropertyDefn mtypedefns[] = {

#undef  DEF_MPROP_I
#define DEF_MPROP_I(NAME,FNAME,DOC,SLOT,LO,DFLT,HI)  \
    { NAME, FNAME, NULL, NULL, OFFS(Mtype, SLOT), DOC, DFLT, NULL, LO, HI },
#undef  DEF_MPROP_S
#define DEF_MPROP_S(NAME,FNAME,DOC,SLOT,DFLT)  \
    { NAME, NULL, FNAME, NULL, OFFS(Mtype, SLOT), DOC,    0, DFLT,  0,  0 },
#undef  DEF_MPROP_L
#define DEF_MPROP_L(NAME,FNAME,DOC,SLOT)  \
    { NAME, NULL, NULL, FNAME, OFFS(Mtype, SLOT), DOC,    0, NULL,  0,  0 },

#include "mtype.def"

    { NULL }
};

PropertyDefn ttypedefns[] = {

#undef  DEF_TPROP_I
#define DEF_TPROP_I(NAME,FNAME,DOC,SLOT,LO,DFLT,HI)  \
    { NAME, FNAME, NULL, NULL, OFFS(Ttype, SLOT), DOC, DFLT, NULL, LO, HI },
#undef  DEF_TPROP_S
#define DEF_TPROP_S(NAME,FNAME,DOC,SLOT,DFLT)  \
    { NAME, NULL, FNAME, NULL, OFFS(Ttype, SLOT), DOC,    0, DFLT,  0,  0 },
#undef  DEF_TPROP_L
#define DEF_TPROP_L(NAME,FNAME,DOC,SLOT)  \
    { NAME, NULL, NULL, FNAME, OFFS(Ttype, SLOT), DOC,    0, NULL,  0,  0 },

#include "ttype.def"

    { NULL }
};

PropertyDefn atypedefns[] = {

#undef  DEF_APROP_I
#define DEF_APROP_I(NAME,FNAME,DOC,SLOT,LO,DFLT,HI)  \
    { NAME, FNAME, NULL, NULL, OFFS(Atype, SLOT), DOC, DFLT, NULL, LO, HI },
#undef  DEF_APROP_S
#define DEF_APROP_S(NAME,FNAME,DOC,SLOT,DFLT)  \
    { NAME, NULL, FNAME, NULL, OFFS(Atype, SLOT), DOC,    0, DFLT,  0,  0 },
#undef  DEF_APROP_L
#define DEF_APROP_L(NAME,FNAME,DOC,SLOT)  \
    { NAME, NULL, NULL, FNAME, OFFS(Atype, SLOT), DOC,    0, NULL,  0,  0 },

#include "atype.def"

    { NULL }
};

/* Define functions for all unit type properties. */

#undef  DEF_UPROP_I
#define DEF_UPROP_I(name,FNAME,doc,SLOT,lo,dflt,hi)  \
  int FNAME(int u) { checku(u); return utypes[u].SLOT; }
#undef  DEF_UPROP_S
#define DEF_UPROP_S(name,FNAME,doc,SLOT,dflt)  \
  const char *FNAME(int u) { checku(u); return utypes[u].SLOT; }
#undef  DEF_UPROP_L
#define DEF_UPROP_L(name,FNAME,doc,SLOT)  \
  Obj *FNAME(int u) { checku(u); return utypes[u].SLOT; }

#include "utype.def"

/* Define functions for all material type properties. */

#undef  DEF_MPROP_I
#define DEF_MPROP_I(name,FNAME,doc,SLOT,lo,dflt,hi)  \
  int FNAME(int m) { checkm(m); return mtypes[m].SLOT; }
#undef  DEF_MPROP_S
#define DEF_MPROP_S(name,FNAME,doc,SLOT,dflt)  \
  const char *FNAME(int m) { checkm(m);  return mtypes[m].SLOT; }
#undef  DEF_MPROP_L
#define DEF_MPROP_L(name,FNAME,doc,SLOT)  \
  Obj *FNAME(int m) { checkm(m);  return mtypes[m].SLOT; }

#include "mtype.def"

/* Define functions for all terrain type properties. */

#undef  DEF_TPROP_I
#define DEF_TPROP_I(name,FNAME,doc,SLOT,lo,dflt,hi)  \
  int FNAME(int t) { checkt(t); return ttypes[t].SLOT; }
#undef  DEF_TPROP_S
#define DEF_TPROP_S(name,FNAME,doc,SLOT,dflt)  \
  const char *FNAME(int t) { checkt(t); return ttypes[t].SLOT; }
#undef  DEF_TPROP_L
#define DEF_TPROP_L(name,FNAME,doc,SLOT)  \
  Obj *FNAME(int t) { checkt(t); return ttypes[t].SLOT; }

#include "ttype.def"

/* Define functions for all advance type properties. */

#undef  DEF_APROP_I
#define DEF_APROP_I(name,FNAME,doc,SLOT,lo,dflt,hi)  \
  int FNAME(int a) { checka(a); return atypes[a].SLOT; }
#undef  DEF_APROP_S
#define DEF_APROP_S(name,FNAME,doc,SLOT,dflt)  \
  const char *FNAME(int a) { checka(a); return atypes[a].SLOT; }
#undef  DEF_APROP_L
#define DEF_APROP_L(name,FNAME,doc,SLOT)  \
  Obj *FNAME(int a) { checka(a); return atypes[a].SLOT; }

#include "atype.def"
