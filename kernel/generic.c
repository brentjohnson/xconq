/* Low-level support for variables and properties in Xconq GDL.
   Copyright (C) 1991-1996, 1998-2000 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "config.h"
#include "misc.h"
#include "lisp.h"
#include "game.h"

/* The total number of unit/material/terrain/advance types. */

short numutypes;
short nummtypes;
short numttypes;
short numatypes;

/* These variables indicate whether new types can still be defined.
   Once a table or list of types is manipulated, these are turned off. */

short canaddutype = TRUE;
short canaddmtype = TRUE;
short canaddttype = TRUE;
short canaddatype = TRUE;

/* This is the number of terrain types that can fill a cell. */

int numcelltypes = 0;

/* This is the number of terrain types that can be aux terrain. */

int numauxttypes = 0;

/* This is the number of terrain types that can be border terrain. */

int numbordtypes = 0;

/* This is the number of types that can be connections. */

int numconntypes = 0;

/* This is the number of types that can be coatings. */

int numcoattypes = 0;

/* This is an array listing all the aux types in order, typically used
   to construct menus in interfaces. */

short *aux_terrain_type_index;

/* This is true if any terrain type has nonzero thickness. */

int any_thickness;

short first_auxt_type;
short *next_auxt_type = NULL;
short first_bord_type;
short *next_bord_type = NULL;
short first_conn_type;
short *next_conn_type = NULL;

int *tmp_u_array;
int *tmp_u2_array;
int *tmp_t_array;
int *tmp_m_array;
int *tmp_a_array;

int **tmp_uu_array;

/* It would be useful for these to mention the slot or table access that
   resulted in the error, but we don't want to have to pass a bunch of
   parameters to these routines, since they're referenced a lot. */

void
utype_error(int u)
{
    run_warning("Bad utype %d", u);
}

void
mtype_error(int m)
{
    run_warning("Bad mtype %d", m);
}

void
ttype_error(int t)
{
    run_warning("Bad ttype %d", t);
}

void
atype_error(int a)
{
    run_warning("Bad atype %d", a);
}

/* Declarations of the type definitions themselves. */

Utype *utypes;

Mtype *mtypes;

Ttype *ttypes;

Atype *atypes;

Globals globals;

/* These variables contain the current size of the type property
   arrays.  The values are arbitrary, since create_unit_type and
   friend will increase these if necessary; so we pick values that
   are large enough for most game designs. */

int curmaxutypes = 100;

int curmaxmtypes = 20;

int curmaxttypes = 40;

int curmaxatypes = 200;

/* This prepares the type definitions to be filled in, doing initial
   allocations, etc. */

void
init_types(void)
{
    numutypes = nummtypes = numttypes = numatypes = 0;

    utypes = (Utype *) xmalloc(sizeof(Utype) * curmaxutypes);
    mtypes = (Mtype *) xmalloc(sizeof(Mtype) * curmaxmtypes);
    ttypes = (Ttype *) xmalloc(sizeof(Ttype) * curmaxttypes);
    atypes = (Atype *) xmalloc(sizeof(Atype) * curmaxatypes);
}

VarDefn vardefns[] = {

#undef  DEF_VAR_I
#define DEF_VAR_I(NAME,FNAME,SETFNAME,DOC,var,LO,DFLT,HI)  \
    { NAME, FNAME, NULL, NULL, SETFNAME, NULL, NULL, DOC, DFLT, NULL, NULL, LO, HI },
#undef  DEF_VAR_S
#define DEF_VAR_S(NAME,FNAME,SETFNAME,DOC,var,DFLT)  \
    { NAME, NULL, FNAME, NULL, NULL, SETFNAME, NULL, DOC,    0, DFLT, NULL,  0,  0 },
#undef  DEF_VAR_L
#define DEF_VAR_L(NAME,FNAME,SETFNAME,DOC,var,DFLT)  \
    { NAME, NULL, NULL, FNAME, NULL, NULL, SETFNAME, DOC,    0, NULL, DFLT,  0,  0 },

#include "gvar.def"

    { NULL }
};

/* Define all the global-getting and -setting functions. */

#undef  DEF_VAR_I
#define DEF_VAR_I(str,FNAME,SETFNAME,doc,VAR,lo,dflt,hi)  \
  int FNAME(void) { return globals.VAR; }  \
  void SETFNAME(int v) { globals.VAR = v; }
#undef  DEF_VAR_S
#define DEF_VAR_S(str,FNAME,SETFNAME,doc,VAR,dflt)  \
  char *FNAME(void) { return globals.VAR; }  \
  void SETFNAME(char *v) { globals.VAR = v; }
#undef  DEF_VAR_L
#define DEF_VAR_L(str,FNAME,SETFNAME,doc,VAR,DFLT)  \
  Obj *FNAME(void) { return globals.VAR; }  \
  void SETFNAME(Obj *v) { globals.VAR = v; }

#include "gvar.def"

/* Set the globals to their default values.  For Lisp values, the default
   is an initialization function rather than a value. */

void
init_globals(void)
{
#undef  DEF_VAR_I
#define DEF_VAR_I(str,fname,SETFNAME,doc,var,lo,DFLT,hi)  \
    SETFNAME(DFLT);
#undef  DEF_VAR_S
#define DEF_VAR_S(str,fname,SETFNAME,doc,var,DFLT)  \
    SETFNAME(DFLT);
  /* The 0 had been NULL but this was changed along with calls in gvar.def
  after compile warnings from g++ */
#undef  DEF_VAR_L
#define DEF_VAR_L(str,fname,SETFNAME,doc,var,DFLT)  \
    if ((DFLT) != 0) {  \
	Obj *(*fn)(void) = (DFLT);  \
	if (fn != NULL) SETFNAME((*fn)());  \
    } else { \
	SETFNAME(lispnil);  \
    }

#include "gvar.def"

}

/* Return the number of a new unit type, being careful to allocate
   more space for unit properties if needed. */

int
create_unit_type(void)
{
    int u, newmax;
    Utype *newutypes;

    if (numutypes >= curmaxutypes) {
	/* Double in size.  This is reasonable because the type
	   structures are smallish, and don't need to grow very
	   often. */
	newmax = curmaxutypes * 2;
	/* This is similar to realloc, except that we need to be sure
	   that the new space is all zeros. */
	newutypes = (Utype *) xmalloc(newmax * sizeof(Utype));
	memcpy(newutypes, utypes, curmaxutypes * sizeof(Utype));
	free(utypes);
	utypes = newutypes;
	curmaxutypes = newmax;
    }
    u = numutypes++;
    return u;
}

/* Return the number of a new material type. */
 
int
create_material_type(void)
{
    int m, newmax;
    Mtype *newmtypes;

    if (nummtypes >= curmaxmtypes) {
	newmax = curmaxmtypes * 2;
	newmtypes = (Mtype *) xmalloc(newmax * sizeof(Mtype));
	memcpy(newmtypes, mtypes, curmaxmtypes * sizeof(Mtype));
	free(mtypes);
	mtypes = newmtypes;
	curmaxmtypes = newmax;
    }
    m = nummtypes++;
    return m;
}

/* Return the number of a new terrain type. */
 
int
create_terrain_type(void)
{
    int t, newmax;
    Ttype *newttypes;

    if (numttypes >= curmaxttypes) {
	newmax = curmaxttypes * 2;
	newttypes = (Ttype *) xmalloc(newmax * sizeof(Ttype));
	memcpy(newttypes, ttypes, curmaxttypes * sizeof(Ttype));
	free(ttypes);
	ttypes = newttypes;
	curmaxttypes = newmax;
    }
    t = numttypes++;
    return t;
}

/* Return the number of a new advance type. */

int
create_advance_type(void)
{
    int a, newmax;
    Atype *newatypes;

    if (numatypes >= curmaxatypes) {
	newmax = curmaxatypes * 2;
	newatypes = (Atype *) xmalloc(newmax * sizeof(Atype));
	memcpy(newatypes, atypes, curmaxatypes * sizeof(Atype));
	free(atypes);
	atypes = newatypes;
	curmaxatypes = newmax;
    }
    a = numatypes++;
    return a;
}

#define TYPEPROP(TYPES, N, DEFNS, I, TYPE)  \
  ((TYPE *) &(((char *) (&(TYPES[N])))[DEFNS[I].offset]))[0]

/* This sets all the defaults in a unit type definition.  Note that all
   type structures get blasted with zeros initially, so we really only
   need to do default settings of nonzero values, thus the test.  (It
   helps if the compiler is smart enough to remove dead code.) */

void
default_unit_type(int u)
{
    int i;

    for (i = 0; utypedefns[i].name != NULL; ++i) {
	if (utypedefns[i].intgetter) {
	    if (utypedefns[i].dflt != 0)
	      TYPEPROP(utypes, u, utypedefns, i, short) = utypedefns[i].dflt;
	} else if (utypedefns[i].strgetter) {
	    if (utypedefns[i].dfltstr != NULL)
	      TYPEPROP(utypes, u, utypedefns, i, char *) =
		(char *) utypedefns[i].dfltstr;
	} else {
	    TYPEPROP(utypes, u, utypedefns, i, Obj *) = lispnil;
	}
    }
}

/* This sets all the defaults in a material type definition. */

void
default_material_type(int m)
{
    int i;
	
    for (i = 0; mtypedefns[i].name != NULL; ++i) {
	if (mtypedefns[i].intgetter) {
	    if (mtypedefns[i].dflt != 0)
	      TYPEPROP(mtypes, m, mtypedefns, i, short) = mtypedefns[i].dflt;
	} else if (mtypedefns[i].strgetter) {
	    if (mtypedefns[i].dfltstr != NULL)
	      TYPEPROP(mtypes, m, mtypedefns, i, char *) = (char *) mtypedefns[i].dfltstr;
	} else {
	    TYPEPROP(mtypes, m, mtypedefns, i, Obj *) = lispnil;
	}
    }
}

/* This sets all the defaults in a terrain type definition. */

void
default_terrain_type(int t)
{
    int i;
	
    for (i = 0; ttypedefns[i].name != NULL; ++i) {
	if (ttypedefns[i].intgetter) {
	    if (ttypedefns[i].dflt != 0)
	      TYPEPROP(ttypes, t, ttypedefns, i, short) = ttypedefns[i].dflt;
	} else if (ttypedefns[i].strgetter) {
	    if (ttypedefns[i].dfltstr != 0)
	      TYPEPROP(ttypes, t, ttypedefns, i, char *) = (char *) ttypedefns[i].dfltstr;
	} else {
	    TYPEPROP(ttypes, t, ttypedefns, i, Obj *) = lispnil;
	}
    }
}

/* This sets all the defaults in an advance type definition. */

void
default_advance_type(int a)
{
    int i;
	
    for (i = 0; atypedefns[i].name != NULL; ++i) {
	if (atypedefns[i].intgetter) {
	    if (atypedefns[i].dflt != 0)
	      TYPEPROP(atypes, a, atypedefns, i, short) = atypedefns[i].dflt;
	} else if (atypedefns[i].strgetter) {
	    if (atypedefns[i].dfltstr != 0)
	      TYPEPROP(atypes, a, atypedefns, i, char *) = (char *) atypedefns[i].dfltstr;
	} else {
	    TYPEPROP(atypes, a, atypedefns, i, Obj *) = lispnil;
	}
    }
}

char *
index_type_name(int x)
{
    return (char *)((x) == UTYP ? "unit"
	    : ((x) == MTYP ? "material"
	       : ((x) == TTYP ? "terrain"
		  : "advance")));
}

/* This function allocates a parameter table and fills it with a default. */

void
allocate_table(int tbl, int reset)
{
    int i, lim1, lim2, dflt = tabledefns[tbl].dflt;
    short *rslt;

    if (reset) *(tabledefns[tbl].table) = NULL;
    if (*tabledefns[tbl].table == NULL) {
	lim1 = numtypes_from_index_type(tabledefns[tbl].index1);
	lim2 = numtypes_from_index_type(tabledefns[tbl].index2);
	if (lim1 == 0) {
	    run_warning("Can't allocate the %s table, no %s types defined",
			tabledefns[tbl].name,
			index_type_name(tabledefns[tbl].index1));
	    return;
	}
	if (lim2 == 0) {
	    run_warning("Can't allocate the %s table, no %s types defined",
			tabledefns[tbl].name,
			index_type_name(tabledefns[tbl].index2));
	    return;
	}
	/* Allocate the table itself. */
	rslt = (short *) xmalloc(lim1 * lim2 * sizeof(short));
	/* Put the table's default everywhere in the table. */
	for (i = 0; i < lim1 * lim2; ++i) rslt[i] = dflt;
	*(tabledefns[tbl].table) = rslt;
	/* For each index, flag that no more types of that sort allowed. */
	switch (tabledefns[tbl].index1) {
	  case UTYP: disallow_more_unit_types();  break;
	  case MTYP: disallow_more_material_types();  break;
	  case TTYP: disallow_more_terrain_types();  break;
	  case ATYP: disallow_more_advance_types();  break;
	}
	switch (tabledefns[tbl].index2) {
	  case UTYP: disallow_more_unit_types();  break;
	  case MTYP: disallow_more_material_types();  break;
	  case TTYP: disallow_more_terrain_types();  break;
	  case ATYP: disallow_more_advance_types();  break;
	}
    }
}

int
numtypes_from_index_type(int x)
{
    return ((x) == UTYP ? numutypes
	    : ((x) == MTYP ? nummtypes
	       : ((x) == TTYP ? numttypes
		  : numatypes)));
}

void
disallow_more_unit_types(void)
{
    int u = NONUTYPE;

    canaddutype = FALSE;
    if (tmp_u_array == NULL)
      tmp_u_array = (int *) xmalloc(numutypes * sizeof(int));
    if (!tmp_u2_array)
      tmp_u2_array = (int *) xmalloc(numutypes * sizeof(int));
    if (!tmp_uu_array) {
	tmp_uu_array = (int **) xmalloc(numutypes * sizeof(int *));
	for_all_unit_types(u)
	  tmp_uu_array[u] = (int *) xmalloc(numutypes * sizeof(int));
    }
}

void
disallow_more_terrain_types(void)
{
    canaddttype = FALSE;
    if (tmp_t_array == NULL)
      tmp_t_array = (int *) xmalloc(numttypes * sizeof(int));
    if (next_auxt_type == NULL)
      next_auxt_type = (short *) xmalloc(numttypes * sizeof(short));
    if (next_bord_type == NULL)
      next_bord_type = (short *) xmalloc(numttypes * sizeof(short));
    if (next_conn_type == NULL)
      next_conn_type = (short *) xmalloc(numttypes * sizeof(short));
    if (aux_terrain_type_index == NULL)
      aux_terrain_type_index = (short *) xmalloc(numttypes * sizeof(short));
    count_terrain_subtypes();
}

/* Prevent more material types from being added, and prepare some integer 
   scratch space for doing math with materials types. */

void
disallow_more_material_types(void)
{
    canaddmtype = FALSE;
    if (tmp_m_array == NULL)
      tmp_m_array = (int *) xmalloc(nummtypes * sizeof(int));
}

/* Prevent more advance types from being added, and prepare some integer 
   scratch space for doing math with advances types. */

void
disallow_more_advance_types(void)
{
    canaddatype = FALSE;
    if (tmp_a_array == NULL)
      tmp_a_array = (int *) xmalloc(numatypes * sizeof(int));
}

/* Calculate and cache some info about terrain types. */

void
count_terrain_subtypes(void)
{
    int t, i = 0, preva = -1, prevb = -1, prevc = -1;

    /* Note that this function may be called several times. */
    first_auxt_type = first_bord_type = first_conn_type = -1;

    numcelltypes = numbordtypes = numconntypes = numcoattypes = 0;
    numauxttypes = 0;
    any_thickness = FALSE;
    for_all_terrain_types(t) {
        switch (t_subtype(t)) {
          case bordersubtype: case connectionsubtype: case coatingsubtype:
            ++numauxttypes;
            if (preva < 0) {
                first_auxt_type = t;
            } else {
                next_auxt_type[preva] = t;
            }
            preva = t;
	    aux_terrain_type_index[i++] = t;
            break;
        }
	switch (t_subtype(t)) {
	  case cellsubtype:
	    ++numcelltypes;
	    break;
	  case bordersubtype:
	    ++numbordtypes;
	    if (prevb < 0) {
		first_bord_type = t;
	    } else {
		next_bord_type[prevb] = t;
	    }
	    prevb = t;
	    break;
	  case connectionsubtype:
	    ++numconntypes;
	    if (prevc < 0) {
		first_conn_type = t;
	    } else {
		next_conn_type[prevc] = t;
	    }
	    prevc = t;
	    break;
	  case coatingsubtype:
	    ++numcoattypes;
	    break;
	}
	if (t_thickness(t) > 0)
	  any_thickness = TRUE;
    }
    /* Mark the end of the links. */
    if (preva >= 0)
      next_auxt_type[preva] = -1;
    if (prevb >= 0)
      next_bord_type[prevb] = -1;
    if (prevc >= 0)
      next_conn_type[prevc] = -1;
}

//! Default value for table.

int
table_default(int (*getter)(int, int))
{
    int i = 0;

    for (i = 0; tabledefns[i].name != NULL; ++i) {
        if (tabledefns[i].getter == getter)
          return tabledefns[i].dflt;
    }
    assert_error((tabledefns[i].name != NULL),
                 "Invalid table lookup attempted!");
    return 0;
}

//! Default value for integer uprop.

short
uprop_i_default(int (*intgetter)(int))
{
    int i = 0;

    for (i = 0; utypedefns[i].name != NULL; ++i) {
        if (utypedefns[i].intgetter == intgetter)
          return utypedefns[i].dflt;
    }
    assert_error((utypedefns[i].name != NULL),
                 "Invalid unit property lookup attempted!");
    return 0;
}

//! Default value for string uprop.

char *
uprop_s_default(char *(*strgetter)(int))
{
    int i = 0;

    for (i = 0; utypedefns[i].name != NULL; ++i) {
        if (utypedefns[i].strgetter == strgetter)
          return utypedefns[i].dfltstr;
    }
    assert_error((utypedefns[i].name != NULL),
                 "Invalid unit property lookup attempted!");
    return NULL;
}

//! Default value for Lisp uprop.

Obj *
uprop_l_default(Obj *(*objgetter)(int))
{
    return lispnil;
}

//! Default value for integer tprop.

short
tprop_i_default(int (*intgetter)(int))
{
    int i = 0;

    for (i = 0; ttypedefns[i].name != NULL; ++i) {
        if (ttypedefns[i].intgetter == intgetter)
          return ttypedefns[i].dflt;
    }
    assert_error((ttypedefns[i].name != NULL),
                 "Invalid terrain property lookup attempted!");
    return 0;
}

//! Default value for string tprop.

char *
tprop_s_default(char *(*strgetter)(int))
{
    int i = 0;

    for (i = 0; ttypedefns[i].name != NULL; ++i) {
        if (ttypedefns[i].strgetter == strgetter)
          return ttypedefns[i].dfltstr;
    }
    assert_error((ttypedefns[i].name != NULL),
                 "Invalid terrain property lookup attempted!");
    return 0;
}

//! Default value for Lisp tprop.

Obj *
tprop_l_default(Obj *(*objgetter)(int))
{
    return lispnil;
}

//! Default value for integer mprop.

short
mprop_i_default(int (*intgetter)(int))
{
    int i = 0;

    for (i = 0; mtypedefns[i].name != NULL; ++i) {
        if (mtypedefns[i].intgetter == intgetter)
          return mtypedefns[i].dflt;
    }
    assert_error((mtypedefns[i].name != NULL),
                 "Invalid material property lookup attempted!");
    return 0;
}

//! Default value for string mprop.

char *
mprop_s_default(char *(*strgetter)(int))
{
    int i = 0;

    for (i = 0; mtypedefns[i].name != NULL; ++i) {
        if (mtypedefns[i].strgetter == strgetter)
          return mtypedefns[i].dfltstr;
    }
    assert_error((mtypedefns[i].name != NULL),
                 "Invalid material property lookup attempted!");
    return 0;
}

//! Default value for Lisp mprop.

Obj *
mprop_l_default(Obj *(*objgetter)(int))
{
    return lispnil;
}

//! Default value for integer aprop.

short
aprop_i_default(int (*intgetter)(int))
{
    int i = 0;

    for (i = 0; atypedefns[i].name != NULL; ++i) {
        if (atypedefns[i].intgetter == intgetter)
          return atypedefns[i].dflt;
    }
    assert_error((atypedefns[i].name != NULL),
                 "Invalid advance property lookup attempted!");
    return 0;
}

//! Default value for string aprop.

char *
aprop_s_default(char *(*strgetter)(int))
{
    int i = 0;

    for (i = 0; atypedefns[i].name != NULL; ++i) {
        if (atypedefns[i].strgetter == strgetter)
          return atypedefns[i].dfltstr;
    }
    assert_error((atypedefns[i].name != NULL),
                 "Invalid advance property lookup attempted!");
    return 0;
}

//! Default value for Lisp aprop.

Obj *
aprop_l_default(Obj *(*objgetter)(int))
{
    return lispnil;
}

//! Default value for integer gvar.

int
gvar_i_default(int (*intgetter)(void))
{
    int i = 0;

    for (i = 0; vardefns[i].name != NULL; ++i) {
        if (vardefns[i].intgetter == intgetter)
          return vardefns[i].dflt;
    }
    assert_error((vardefns[i].name != NULL),
                 "Invalid global variable lookup attempted!");
    return 0;
}

//! Default value for string gvar.

char *
gvar_s_default(char *(*strgetter)(void))
{
    int i = 0;

    for (i = 0; vardefns[i].name != NULL; ++i) {
        if (vardefns[i].strgetter == strgetter)
          return vardefns[i].dfltstr;
    }
    assert_error((vardefns[i].name != NULL),
                 "Invalid global variable lookup attempted!");
    return NULL;
}

//! Default value for Lisp gvar.

Obj *
gvar_l_default(Obj *(*objgetter)(void))
{
    int i = 0;

    for (i = 0; vardefns[i].name != NULL; ++i) {
        if (vardefns[i].objgetter == objgetter)
          return (*(vardefns[i].dfltfn))();
    }
    assert_error((vardefns[i].name != NULL),
                 "Invalid global variable lookup attempted!");
    return NULL;
}
