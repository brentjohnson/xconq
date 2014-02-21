// xConq
// Game entity types.

// $Id: types.cc,v 1.6 2006/06/02 16:57:43 eric_mcdonald Exp $

/*//////////////////////////// COPYRIGHT NOTICE ////////////////////////////////

  Copyright (C) 1986-1989   Stanley T. Shebs
  Copyright (C) 1991-2000   Stanley T. Shebs
  Copyright (C) 2005-2006   Eric A. McDonald

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
    \brief Game entity types.
    \ingroup grp_gdl
*/

#include "gdl/tables.h"
#include "gdl/module.h"

// Function Macros: Game Entity Property Accessors

#ifdef offsetof
//! Get offset of field within given data structure.
#define OFFS(TYPE,FLD)  (offsetof(TYPE, FLD))
#else
//! Get offset of field within given data structure.
#define OFFS(TYPE,FLD)  \
  ((unsigned int) (((char *) (&(((TYPE *) NULL)->FLD))) - ((char *) NULL)))
#endif

#ifdef TYPEPROP
#undef TYPEPROP
#endif
//! Access property for given type.
#define TYPEPROP(TYPES, N, DEFNS, I, TYPE)  \
  ((TYPE *) &(((char *) (&(TYPES[N])))[DEFNS[I].offset]))[0]

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Global Variables

/*! \showinitializer */
PropertyDefn utypedefns[] = {

#ifdef  DEF_UPROP_I
#undef  DEF_UPROP_I
#endif
#define DEF_UPROP_I(NAME,FNAME,DOC,SLOT,LO,DFLT,HI)  \
    { NAME, FNAME, NULL, NULL, OFFS(Utype, SLOT), DOC, DFLT, NULL, LO, HI },

#ifdef  DEF_UPROP_S
#undef  DEF_UPROP_S
#endif
#define DEF_UPROP_S(NAME,FNAME,DOC,SLOT,DFLT)  \
    { NAME, NULL, FNAME, NULL, OFFS(Utype, SLOT), DOC,    0, DFLT,  0,  0 },

#ifdef  DEF_UPROP_L
#undef  DEF_UPROP_L
#endif
#define DEF_UPROP_L(NAME,FNAME,DOC,SLOT)  \
    { NAME, NULL, NULL, FNAME, OFFS(Utype, SLOT), DOC,    0, NULL,  0,  0 },

#include "gdl/utype.def"

    { NULL }

};

/*! \showinitializer */
PropertyDefn mtypedefns[] = {

#ifdef  DEF_MPROP_I
#undef  DEF_MPROP_I
#endif
#define DEF_MPROP_I(NAME,FNAME,DOC,SLOT,LO,DFLT,HI)  \
    { NAME, FNAME, NULL, NULL, OFFS(Mtype, SLOT), DOC, DFLT, NULL, LO, HI },

#ifdef  DEF_MPROP_S
#undef  DEF_MPROP_S
#endif
#define DEF_MPROP_S(NAME,FNAME,DOC,SLOT,DFLT)  \
    { NAME, NULL, FNAME, NULL, OFFS(Mtype, SLOT), DOC,    0, DFLT,  0,  0 },

#ifdef  DEF_MPROP_L
#undef  DEF_MPROP_L
#endif
#define DEF_MPROP_L(NAME,FNAME,DOC,SLOT)  \
    { NAME, NULL, NULL, FNAME, OFFS(Mtype, SLOT), DOC,    0, NULL,  0,  0 },

#include "gdl/mtype.def"

    { NULL }

};

/*! \showinitializer */
PropertyDefn ttypedefns[] = {

#ifdef  DEF_TPROP_I
#undef  DEF_TPROP_I
#endif
#define DEF_TPROP_I(NAME,FNAME,DOC,SLOT,LO,DFLT,HI)  \
    { NAME, FNAME, NULL, NULL, OFFS(Ttype, SLOT), DOC, DFLT, NULL, LO, HI },

#ifdef  DEF_TPROP_S
#undef  DEF_TPROP_S
#endif
#define DEF_TPROP_S(NAME,FNAME,DOC,SLOT,DFLT)  \
    { NAME, NULL, FNAME, NULL, OFFS(Ttype, SLOT), DOC,    0, DFLT,  0,  0 },

#ifdef  DEF_TPROP_L
#undef  DEF_TPROP_L
#endif
#define DEF_TPROP_L(NAME,FNAME,DOC,SLOT)  \
    { NAME, NULL, NULL, FNAME, OFFS(Ttype, SLOT), DOC,    0, NULL,  0,  0 },

#include "gdl/ttype.def"

    { NULL }

};

/*! \showinitializer */
PropertyDefn atypedefns[] = {

#ifdef  DEF_APROP_I
#undef  DEF_APROP_I
#endif
#define DEF_APROP_I(NAME,FNAME,DOC,SLOT,LO,DFLT,HI)  \
    { NAME, FNAME, NULL, NULL, OFFS(Atype, SLOT), DOC, DFLT, NULL, LO, HI },

#ifdef  DEF_APROP_S
#undef  DEF_APROP_S
#endif
#define DEF_APROP_S(NAME,FNAME,DOC,SLOT,DFLT)  \
    { NAME, NULL, FNAME, NULL, OFFS(Atype, SLOT), DOC,    0, DFLT,  0,  0 },

#ifdef  DEF_APROP_L
#undef  DEF_APROP_L
#endif
#define DEF_APROP_L(NAME,FNAME,DOC,SLOT)  \
    { NAME, NULL, NULL, FNAME, OFFS(Atype, SLOT), DOC,    0, NULL,  0,  0 },

#include "gdl/atype.def"

    { NULL }

};

Utype *utypes;
short numutypes;

Mtype *mtypes;
short nummtypes;

Ttype *ttypes;
short numttypes;

Atype *atypes;
short numatypes;

int numcelltypes = 0;
int numauxttypes = 0;
int numbordtypes = 0;
int numconntypes = 0;
int numcoattypes = 0;

short first_auxt_type;
short *next_auxt_type = NULL;
short *aux_terrain_type_index;

short first_bord_type;
short *next_bord_type = NULL;
short first_conn_type;
short *next_conn_type = NULL;

// Global Variables: Behavior Options

short canaddutype = TRUE;
short canaddmtype = TRUE;
short canaddttype = TRUE;
short canaddatype = TRUE;

// Global Variables: Scratch

short tmputype;
short tmpmtype;
short tmpttype;
short tmpatype;

int *tmp_u_array;
int *tmp_u2_array;
int *tmp_m_array;
int *tmp_t_array;
int *tmp_a_array;

int **tmp_uu_array;

// Global Variables: Buffers

short *rd_u_arr;

// Local Variables

//! Array of shortest unit type names.
static char **shortestnames = NULL;
//! Longest of the shortest unit type names.
static int longest_shortest;

//! True, if any terrain type has nonzero thickness.
static int any_thickness;

// Local Variables: Counters

//! Currently defined number of unit types.
static int curmaxutypes = 100;
//! Currently defined number of material types.
static int curmaxmtypes = 20;
//! Currently defined number of terrain types.
static int curmaxttypes = 40;
//! Currently defined number of advance types.
static int curmaxatypes = 200;

// Local Function Declarations: Game Setup

//! Fill in unit type from GDL list.
static void fill_in_utype(int u, Obj *list);
//! Fill in material type from GDL list.
static void fill_in_mtype(int m, Obj *list);
//! Fill in terrain type from GDL list.
static void fill_in_ttype(int t, Obj *list);
//! Fill in advance type from GDL list.
static void fill_in_atype(int s, Obj *list);

//! Set individual unit type property from GDL object.
static int set_utype_property(int u, char *propname, Obj *val);
//! Set unit type internal name.
static void set_u_internal_name(int u, char *s) { utypes[u].iname = s; }

//! Set unit type GDL name.
static void set_u_type_name(int u, char *s) { utypes[u].name = s; }
//! Set material type GDL name.
static void set_m_type_name(int m, char *s) { mtypes[m].name = s; }
//! Set terrain type GDL name.
static void set_t_type_name(int t, char *s) { ttypes[t].name = s; }
//! Set advance type GDL name.
static void set_a_type_name(int a, char *s) { atypes[a].name = s; }

//! Add properties to unit type.
static void add_to_utypes(Obj *types, Obj *prop, Obj *values);
//! Add properties to material type.
static void add_to_mtypes(Obj *types, Obj *prop, Obj *values);
//! Add properties to terrain type.
static void add_to_ttypes(Obj *types, Obj *prop, Obj *values);
//! Add properties to advance type.
static void add_to_atypes(Obj *types, Obj *prop, Obj *values);

// Notifications

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

void
type_error(Obj *x, char *msg)
{
    sprintlisp(readerrbuf, x, BUFSIZE);
    read_warning("type error in `%s' - %s", readerrbuf, msg);
}

// Queries

int
numtypes_from_index_type(int x)
{
    return ((x) == UTYP ? numutypes
	    : ((x) == MTYP ? nummtypes
	       : ((x) == TTYP ? numttypes
		  : numatypes)));
}

char *
index_type_name(int x)
{
    return (char *)((x) == UTYP ? "unit"
	    : ((x) == MTYP ? "material"
	       : ((x) == TTYP ? "terrain"
		  : "advance")));
}

int
utype_from_name(char *str)
{
    char *tmpstr;
    int u;
    Obj *sym;

    if (empty_string(str))
      return NONUTYPE;
    for_all_unit_types(u) {
	if (strcmp(str, u_type_name(u)) == 0)
	  return u;
    }
    /* Try evaluating the symbol. */
    sym = intern_symbol(str);
    if (boundp(sym) && utypep(symbol_value(sym)))
      return c_number(symbol_value(sym));
    /* Try some less common possibilities. */
    for_all_unit_types(u) {
	tmpstr = u_short_name(u);
	if (tmpstr && strcmp(str, tmpstr) == 0)
	  return u;
    }
    for_all_unit_types(u) {
	tmpstr = u_long_name(u);
	if (tmpstr && strcmp(str, tmpstr) == 0)
	  return u;
    }
    return NONUTYPE;
}

int
mtype_from_name(char *str)
{
    int m;
    Obj *sym;

    for_all_material_types(m) {
	if (strcmp(str, m_type_name(m)) == 0)
	  return m;
    }
    /* Try evaluating the symbol. */
    sym = intern_symbol(str);
    if (boundp(sym) && mtypep(symbol_value(sym)))
      return c_number(symbol_value(sym));
    return NONMTYPE;
}

int
ttype_from_name(char *str)
{
    int t;
    Obj *sym;

    for_all_terrain_types(t) {
	if (strcmp(str, t_type_name(t)) == 0)
	  return t;
    }
    /* Try evaluating the symbol. */
    sym = intern_symbol(str);
    if (boundp(sym) && ttypep(symbol_value(sym)))
      return c_number(symbol_value(sym));
    return NONTTYPE;
}

int
atype_from_name(char *str)
{
    int a;
    Obj *sym;

    for_all_advance_types(a) {
	if (strcmp(str, a_type_name(a)) == 0)
	  return a;
    }
    /* Try evaluating the symbol. */
    sym = intern_symbol(str);
    if (boundp(sym) && atypep(symbol_value(sym)))
      return c_number(symbol_value(sym));
    return NONATYPE;
}

int
utype_from_symbol(Obj *sym)
{
    char *str, *tmpstr;
    int u;

    if (boundp(sym) && utypep(symbol_value(sym)))
      return c_number(symbol_value(sym));
    str = c_string(sym);
    for_all_unit_types(u) {
	if (strcmp(str, u_type_name(u)) == 0)
	  return u;
    }
    /* Try some less common possibilities. */
    for_all_unit_types(u) {
	tmpstr = u_short_name(u);
	if (tmpstr && strcmp(str, tmpstr) == 0)
	  return u;
    }
    for_all_unit_types(u) {
	tmpstr = u_long_name(u);
	if (tmpstr && strcmp(str, tmpstr) == 0)
	  return u;
    }
    return NONUTYPE;
}

char *
shortest_unique_name(int u)
{
    char namebuf[BUFSIZE], *name1;
    int u1, u2, i, len, allhavechars, *firstuniq, firstuniq1;

    /* Don't try to allocate shortestnames before numutypes has been
       defined. This will cause crashes later on as the program
       mistakenly believes that all shortestnames[u] have been
       allocated just because shortestnames != NULL. */
    if (numutypes == 0)
      return NULL;
    if (shortestnames == NULL) {
	shortestnames = (char **) xmalloc(numutypes * sizeof(char *));
	firstuniq = (int *) xmalloc(numutypes * sizeof(int));
	/* First use game definition's single chars if possible. */
	allhavechars = TRUE;
	for_all_unit_types(u1) {
	    firstuniq[u1] = -1;
	    if (!empty_string(u_uchar(u1))) {
		namebuf[0] = (u_uchar(u1))[0];
		namebuf[1] = '\0';
		shortestnames[u1] = copy_string(namebuf);
		firstuniq[u1] = 0;
	    } else {
		allhavechars = FALSE;
	    }
	}
	if (!allhavechars) {
	    /* Start with copies of full name for all types not
               already named. */
	    for_all_unit_types(u1) {
		if (shortestnames[u1] == NULL) {
		    shortestnames[u1] = copy_string(u_type_name(u1));
		    firstuniq[u1] = 0;
		}
	    }
	    for_all_unit_types(u1) {
		if (firstuniq[u1] < 0) {
		    name1 = shortestnames[u1];
		    firstuniq1 = firstuniq[u1];
		    for_all_unit_types(u2) {
			if (u1 != u2) {
			    /* Look through the supposedly minimal
			       unique substring and see if it is the
			       same. */
			    for (i = 0; i < firstuniq1; ++i ) {
				if (name1[i] != (shortestnames[u2])[i]) {
				    break;
				}
			    }
			    /* If so, must extend the unique substring. */
			    if (i == firstuniq1) {
				/* Look for the first nonmatching char. */
				while (name1[firstuniq1] == (shortestnames[u2])[firstuniq1]) {
				    ++firstuniq1;
				}
			    }
			}
		    }
		    firstuniq[u1] = firstuniq1;
		}
	    }
	    /* For any types where the unique short name is shorter
	       than the seed name, truncate appropriately. */
	    longest_shortest = 0;
	    for_all_unit_types(u1) {
		if (size_t(firstuniq[u1] + 1) < strlen(shortestnames[u1])) {
		    (shortestnames[u1])[firstuniq[u1] + 1] = '\0';
		}
		len = strlen(shortestnames[u1]);
		if (len > longest_shortest)
		  longest_shortest = len;
	    }
	} else {
		longest_shortest = 1;
	}
	if (Debug) {
	    for_all_unit_types(u1) {
		Dprintf("Shortest type name: %s for %s\n",
			shortestnames[u1], u_type_name(u1));
	    }
	}
    }
    return shortestnames[u];
}

char *
shortest_escaped_name(int u)
{
    char *internalname = u_internal_name(u);

    sprintf(shortestbuf, "%s", escaped_symbol(internalname));
    return shortestbuf;
}

int
could_be_on(int u, int t)
{
    assert_error(is_unit_type(u), "Attempted to manipulate an invalid utype");
    assert_error(is_terrain_type(t),
		 "Attempted to manipulate an invalid ttype");
    if ((t_capacity(t) < ut_size(u, t)) && (0 >= ut_capacity_x(u, t)))
      return FALSE;
    return TRUE;
}

// Game Entity Property Accessors

#ifdef  DEF_UPROP_I
#undef  DEF_UPROP_I
#endif
#define DEF_UPROP_I(name,FNAME,doc,SLOT,lo,dflt,hi)  \
  int FNAME(int u) { checku(u); return utypes[u].SLOT; }

#ifdef  DEF_UPROP_S
#undef  DEF_UPROP_S
#endif
#define DEF_UPROP_S(name,FNAME,doc,SLOT,dflt)  \
  char *FNAME(int u) { checku(u); return utypes[u].SLOT; }

#ifdef  DEF_UPROP_L
#undef  DEF_UPROP_L
#endif
#define DEF_UPROP_L(name,FNAME,doc,SLOT)  \
  Obj *FNAME(int u) { checku(u); return utypes[u].SLOT; }

#include "gdl/utype.def"

#ifdef  DEF_MPROP_I
#undef  DEF_MPROP_I
#endif
#define DEF_MPROP_I(name,FNAME,doc,SLOT,lo,dflt,hi)  \
  int FNAME(int m) { checkm(m); return mtypes[m].SLOT; }

#ifdef  DEF_MPROP_S
#undef  DEF_MPROP_S
#endif
#define DEF_MPROP_S(name,FNAME,doc,SLOT,dflt)  \
  char *FNAME(int m) { checkm(m);  return mtypes[m].SLOT; }

#ifdef  DEF_MPROP_L
#undef  DEF_MPROP_L
#endif
#define DEF_MPROP_L(name,FNAME,doc,SLOT)  \
  Obj *FNAME(int m) { checkm(m);  return mtypes[m].SLOT; }

#include "gdl/mtype.def"

#ifdef  DEF_TPROP_I
#undef  DEF_TPROP_I
#endif
#define DEF_TPROP_I(name,FNAME,doc,SLOT,lo,dflt,hi)  \
  int FNAME(int t) { checkt(t); return ttypes[t].SLOT; }

#ifdef  DEF_TPROP_S
#undef  DEF_TPROP_S
#endif
#define DEF_TPROP_S(name,FNAME,doc,SLOT,dflt)  \
  char *FNAME(int t) { checkt(t); return ttypes[t].SLOT; }

#ifdef  DEF_TPROP_L
#undef  DEF_TPROP_L
#endif
#define DEF_TPROP_L(name,FNAME,doc,SLOT)  \
  Obj *FNAME(int t) { checkt(t); return ttypes[t].SLOT; }

#include "gdl/ttype.def"

#ifdef  DEF_APROP_I
#undef  DEF_APROP_I
#endif
#define DEF_APROP_I(name,FNAME,doc,SLOT,lo,dflt,hi)  \
  int FNAME(int a) { checka(a); return atypes[a].SLOT; }

#ifdef  DEF_APROP_S
#undef  DEF_APROP_S
#endif
#define DEF_APROP_S(name,FNAME,doc,SLOT,dflt)  \
  char *FNAME(int a) { checka(a); return atypes[a].SLOT; }

#ifdef  DEF_APROP_L
#undef  DEF_APROP_L
#endif
#define DEF_APROP_L(name,FNAME,doc,SLOT)  \
  Obj *FNAME(int a) { checka(a); return atypes[a].SLOT; }

#include "gdl/atype.def"

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

Obj *
uprop_l_default(Obj *(*objgetter)(int))
{
    return lispnil;
}

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

Obj *
mprop_l_default(Obj *(*objgetter)(int))
{
    return lispnil;
}

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

Obj *
tprop_l_default(Obj *(*objgetter)(int))
{
    return lispnil;
}

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

Obj *
aprop_l_default(Obj *(*objgetter)(int))
{
    return lispnil;
}

// Game Setup

void
init_types(void)
{
    numutypes = nummtypes = numttypes = numatypes = 0;

    utypes = (Utype *) xmalloc(sizeof(Utype) * curmaxutypes);
    mtypes = (Mtype *) xmalloc(sizeof(Mtype) * curmaxmtypes);
    ttypes = (Ttype *) xmalloc(sizeof(Ttype) * curmaxttypes);
    atypes = (Atype *) xmalloc(sizeof(Atype) * curmaxatypes);
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
disallow_more_material_types(void)
{
    canaddmtype = FALSE;
    if (tmp_m_array == NULL)
      tmp_m_array = (int *) xmalloc(nummtypes * sizeof(int));
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

void
disallow_more_advance_types(void)
{
    canaddatype = FALSE;
    if (tmp_a_array == NULL)
      tmp_a_array = (int *) xmalloc(numatypes * sizeof(int));
}

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

// Lifecycle Management

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

// GDL I/O

static
int
set_utype_property(int u, char *propname, Obj *val)
{
    int i, found = FALSE, numval;
    char *strval;

    for (i = 0; utypedefns[i].name != NULL; ++i) {
	if (strcmp(propname, utypedefns[i].name) == 0) {
	    if (utypedefns[i].intgetter) {
		val = eval(val);
		if (!numberishp(val)) {
		    read_warning("unit type %s property %s value not a number",
				 u_type_name(u), utypedefns[i].name);
		    return TRUE;
		}
		numval = c_number(val);
		if (!between(utypedefns[i].lo, numval, utypedefns[i].hi)) {
		    read_warning("unit type %s property %s value %d not between %d and %d",
				 u_type_name(u), utypedefns[i].name, numval,
				 utypedefns[i].lo, utypedefns[i].hi);
		    /* Let it pass through, at least for now. */
		}
		TYPEPROP(utypes, u, utypedefns, i, short) = numval;
	    } else if (utypedefns[i].strgetter) {
		val = eval(val);
		if (!stringp(val)) {
		    read_warning("unit type %s property %s value not a string",
				 u_type_name(u), utypedefns[i].name);
		    return TRUE;
		}
		strval = c_string(val);
		TYPEPROP(utypes, u, utypedefns, i, char *) = strval;
	    } else {
		TYPEPROP(utypes, u, utypedefns, i, Obj *) = val;
	    }
	    found = TRUE;
	    break;
	}
    }
    if (!found)
      unknown_property("unit type", u_type_name(u), propname);
    return found;
}

static
void
fill_in_utype(int u, Obj *list)
{
    char *propname;
    Obj *bdg, *val;

    for ( ; list != lispnil; list = cdr(list)) {
	bdg = car(list);
	PARSE_PROPERTY(bdg, propname, val);
	/* Don't read in props used internally only. */
	if (strncmp(propname, "zz-", 3) == 0)
	      continue;
	set_utype_property(u, propname, val);
    }
}

static
void
fill_in_mtype(int m, Obj *list)
{
    int i, found, numval;
    char *strval;
    Obj *bdg, *val;
    char *propname;

    for ( ; list != lispnil; list = cdr(list)) {
	bdg = car(list);
	PARSE_PROPERTY(bdg, propname, val);
	/* Don't read in props used internally only. */
	if (strncmp(propname, "zz-", 3) == 0)
	    continue;
	found = FALSE;
	for (i = 0; mtypedefns[i].name != NULL; ++i) {
	    if (strcmp(propname, mtypedefns[i].name) == 0) {
		if (mtypedefns[i].intgetter) {
		    val = eval(val);
		    if (!numberishp(val)) {
			read_warning("material type %s property %s value not a number",
				     m_type_name(m), mtypedefns[i].name);
			return;
		    }
		    numval = c_number(val);
		    if (!between(mtypedefns[i].lo, numval, mtypedefns[i].hi)) {
			read_warning("material type %s property %s value %d not between %d and %d",
				     m_type_name(m), mtypedefns[i].name, numval,
				     mtypedefns[i].lo, mtypedefns[i].hi);
			/* Let it pass through, at least for now. */
		    }
		    TYPEPROP(mtypes, m, mtypedefns, i, short) = numval;
		} else if (mtypedefns[i].strgetter) {
		    val = eval(val);
		    if (!stringp(val)) {
			read_warning("material type %s property %s value not a string",
				     m_type_name(m), mtypedefns[i].name);
			return;
		    }
		    strval = c_string(val);
		    TYPEPROP(mtypes, m, mtypedefns, i, char *) = strval;
		} else {
		    TYPEPROP(mtypes, m, mtypedefns, i, Obj *) = val;
		}
		found = TRUE;
		break;
	    }
	}
	if (!found)
	  unknown_property("material type", m_type_name(m), propname);
    }
}

static
void
fill_in_ttype(int t, Obj *list)
{
    int i, found, numval;
    char *strval;
    char *propname;
    Obj *bdg, *val;

    for ( ; list != lispnil; list = cdr(list)) {
	bdg = car(list);
	PARSE_PROPERTY(bdg, propname, val);
	/* Don't read in props used internally only. */
	if (strncmp(propname, "zz-", 3) == 0)
	    continue;
	found = FALSE;
	for (i = 0; ttypedefns[i].name != NULL; ++i) {
	    if (strcmp(propname, ttypedefns[i].name) == 0) {
		if (ttypedefns[i].intgetter) {
		    val = eval(val);
		    if (!numberishp(val)) {
			read_warning("terrain type %s property %s value not a number",
				     t_type_name(t), ttypedefns[i].name);
			return;
		    }
		    numval = c_number(val);
		    if (!between(ttypedefns[i].lo, numval, ttypedefns[i].hi)) {
			read_warning("terrain type %s property %s value %d not between %d and %d",
				     t_type_name(t), ttypedefns[i].name, numval,
				     ttypedefns[i].lo, ttypedefns[i].hi);
			/* Let it pass through, at least for now. */
		    }
		    TYPEPROP(ttypes, t, ttypedefns, i, short) = numval;
		} else if (ttypedefns[i].strgetter) {
		    val = eval(val);
		    if (!stringp(val)) {
			read_warning("terrain type %s property %s value not a string",
				     t_type_name(t), ttypedefns[i].name);
			return;
		    }
		    strval = c_string(val);
		    TYPEPROP(ttypes, t, ttypedefns, i, char *) = strval;
		} else {
		    TYPEPROP(ttypes, t, ttypedefns, i, Obj *) = val;
		}
		found = TRUE;
		break;
	    }
	}
	if (!found)
	  unknown_property("terrain type", t_type_name(t), propname);
    }
}

static
void
fill_in_atype(int a, Obj *list)
{
    int i, found, numval;
    char *strval;
    Obj *bdg, *val;
    char *propname;

    for ( ; list != lispnil; list = cdr(list)) {
	bdg = car(list);
	PARSE_PROPERTY(bdg, propname, val);
	/* Don't read in props used internally only. */
	if (strncmp(propname, "zz-", 3) == 0)
	    continue;
	found = FALSE;
	for (i = 0; atypedefns[i].name != NULL; ++i) {
	    if (strcmp(propname, atypedefns[i].name) == 0) {
		if (atypedefns[i].intgetter) {
		    val = eval(val);
		    if (!numberishp(val)) {
			read_warning("advance type %s property %s value not a number",
				     a_type_name(a), atypedefns[i].name);
			return;
		    }
		    numval = c_number(val);
		    if (!between(atypedefns[i].lo, numval, atypedefns[i].hi)) {
			read_warning("advance type %s property %s value %d not between %d and %d",
				     a_type_name(a), atypedefns[i].name, numval,
				     atypedefns[i].lo, atypedefns[i].hi);
			/* Let it pass through, at least for now. */
		    }
		    TYPEPROP(atypes, a, atypedefns, i, short) = numval;
		} else if (atypedefns[i].strgetter) {
		    val = eval(val);
		    if (!stringp(val)) {
			read_warning("advance type %s property %s value not a string",
				     a_type_name(a), atypedefns[i].name);
			return;
		    }
		    strval = c_string(val);
		    TYPEPROP(atypes, a, atypedefns, i, char *) = strval;
		} else {
		    TYPEPROP(atypes, a, atypedefns, i, Obj *) = val;
		}
		found = TRUE;
		break;
	    }
	}
	if (!found)
	  unknown_property("advance type", a_type_name(a), propname);
    }
}

void
interp_utype(Obj *form)
{
    int u;
    Obj *name = cadr(form), *utype;

    TYPECHECK(symbolp, name, "unit-type name not a symbol");
    if (!canaddutype)
      read_warning("Should not be defining more unit types");
    u = create_unit_type();
    utype = new_utype(u);
    /* Set default values for the unit type's props first. */
    /* Any default type name shouldn't confuse the code below. */
    default_unit_type(u);
    setq(name, utype);
    /* Set the values of random props. */
    fill_in_utype(u, cddr(form));
    /* If no internal type name string given, use the defined name. */
    if (empty_string(u_internal_name(u)))
      set_u_internal_name(u, c_string(name));
    if (empty_string(u_type_name(u)))
      set_u_type_name(u, u_internal_name(u));
    /* If the official type name is different from the internal name,
       make it a variable bound to the type. */
    if (strcmp(u_type_name(u), u_internal_name(u)) != 0)
      setq(intern_symbol(u_type_name(u)), utype);
    /* Blast any cached list of types. */
    makunbound(intern_symbol(keyword_name(K_USTAR)));
    eval_symbol(intern_symbol(keyword_name(K_USTAR)));
}

void
interp_mtype(Obj *form)
{
    int m;
    Obj *name = cadr(form), *mtype;

    TYPECHECK(symbolp, name, "material-type name not a symbol");
    if (!canaddmtype)
      read_warning("Should not be defining more material types");
    m = create_material_type();
    mtype = new_mtype(m);
    /* Set default values for the material type's properties first. */
    default_material_type(m);
    setq(name, mtype);
    /* Set the values of random props. */
    fill_in_mtype(m, cddr(form));
    /* If no type name string given, use the defined name. */
    if (empty_string(m_type_name(m)))
      set_m_type_name(m, c_string(name));
    /* Blast and remake any cached list of types. */
    makunbound(intern_symbol(keyword_name(K_MSTAR)));
    eval_symbol(intern_symbol(keyword_name(K_MSTAR)));
}

void
interp_ttype(Obj *form)
{
    int t;
    Obj *name = cadr(form), *ttype;

    TYPECHECK(symbolp, name, "terrain-type name not a symbol");
    if (!canaddttype)
      read_warning("Should not be defining more terrain types");
    t = create_terrain_type();
    ttype = new_ttype(t);
    /* Set default values for the terrain type's props first. */
    default_terrain_type(t);
    setq(name, ttype);
    /* Set the values of random properties. */
    fill_in_ttype(t, cddr(form));
    /* If no type name string given, use the defined name. */
    if (empty_string(t_type_name(t)))
      set_t_type_name(t, c_string(name));
    /* Blast and remake any cached list of all types. */
    makunbound(intern_symbol(keyword_name(K_TSTAR)));
    eval_symbol(intern_symbol(keyword_name(K_TSTAR)));
}

void
interp_atype(Obj *form)
{
    int a;
    Obj *name = cadr(form), *atype;

    TYPECHECK(symbolp, name, "advance type name not a symbol");
    if (!canaddatype)
      read_warning("Should not be defining more advance types");
    a = create_advance_type();
    atype = new_atype(a);
    /* Set default values for the material type's properties first. */
    default_advance_type(a);
    setq(name, atype);
    /* Set the values of random props. */
    fill_in_atype(a, cddr(form));
    /* If no type name string given, use the defined name. */
    if (empty_string(a_type_name(a)))
      set_a_type_name(a, c_string(name));
    /* Blast and remake any cached list of types. */
    makunbound(intern_symbol(keyword_name(K_ASTAR)));
    eval_symbol(intern_symbol(keyword_name(K_ASTAR)));
}

void
interp_utype_list(short *arr, Obj *lis)
{
    int u = 0;
    Obj *rest, *head;

    /* Assume that if the destination array does not exist, there is
       probably a reason, and it's not our concern. */
    if (arr == NULL)
      return;
    lis = eval(lis);
    if (!consp(lis))
      lis = cons(lis, lispnil);
    if (numberp(car(lis)) && length(lis) == numutypes) {
	u = 0;
	for_all_list(lis, rest)
	  arr[u++] = c_number(car(rest));
    } else {
	for_all_list(lis, rest) {
	    head = car(rest);
	    if (utypep(head)) {
		arr[c_number(head)] = TRUE;
	    } else {
		/* syntax error */
	    }
	}
    }
}

void
interp_utype_value_list(short *arr, Obj *lis)
{
    int u = 0;
    Obj *rest, *head, *types, *values, *subrest, *subrest2, *uval;

    /* Assume that if the destination array does not exist, there is
       probably a reason, and it's not our concern. */
    if (arr == NULL)
      return;
    for_all_list(lis, rest) {
    	head = car(rest);
    	if (numberp(head)) {
	    if (u < numutypes) {
	    	arr[u++] = c_number(head);
	    } else {
		init_warning("too many numbers in list");
	    }
	} else if (consp(head)) {
	    types = eval(car(head));
	    values = eval(cadr(head));
	    if (utypep(types)) {
		u = types->v.num;
	    	arr[u++] = c_number(values);
	    } else if (consp(values)) {
		for_both_lists(types, values, subrest, subrest2) {
		    uval = car(subrest);
		    TYPECHECK(utypep, uval, "not a unit type");
		    u = uval->v.num;
		    arr[u++] = c_number(car(subrest2));
		}
	    } else {
		for_all_list(types, subrest) {
		    uval = car(subrest);
		    TYPECHECK(utypep, uval, "not a unit type");
		    u = uval->v.num;
		    arr[u++] = c_number(values);
		}
	    }
	} else {
	    /* syntax error */
	}
    }
}

void
interp_mtype_value_list(short *arr, Obj *lis)
{
    int m = 0;
    Obj *rest, *head, *types, *values, *subrest, *subrest2, *mval;

    /* Assume that if the destination array does not exist, there is
       probably a reason, and it's not our concern. */
    if (arr == NULL)
      return;
    for_all_list(lis, rest) {
    	head = car(rest);
    	if (numberp(head)) {
	    if (m < nummtypes) {
	    	arr[m++] = c_number(head);
	    } else {
		init_warning("too many numbers in list");
	    }
	} else if (consp(head)) {
	    types = eval(car(head));
	    values = eval(cadr(head));
	    if (mtypep(types)) {
		m = types->v.num;
	    	arr[m++] = c_number(values);
	    } else if (consp(values)) {
		for_both_lists(types, values, subrest, subrest2) {
		    mval = car(subrest);
		    TYPECHECK(mtypep, mval, "not a material type");
		    m = mval->v.num;
		    arr[m++] = c_number(car(subrest2));
		}
	    } else {
		for_all_list(types, subrest) {
		    mval = car(subrest);
		    TYPECHECK(mtypep, mval, "not a material type");
		    m = mval->v.num;
		    arr[m++] = c_number(values);
		}
	    }
	} else {
	    /* syntax error */
	}
    }
}

void
interp_treasury_list(long *arr, Obj *lis)
{
    int m = 0;
    Obj *rest, *head, *types, *values, *subrest, *subrest2, *mval;

    /* Assume that if the destination array does not exist, there is
       probably a reason, and it's not our concern. */
    if (arr == NULL)
      return;
    for_all_list(lis, rest) {
    	head = car(rest);
    	if (numberp(head)) {
	    if (m < nummtypes) {
	    	arr[m++] = c_number(head);
	    } else {
		init_warning("too many numbers in list");
	    }
	} else if (consp(head)) {
	    types = eval(car(head));
	    values = eval(cadr(head));
	    if (mtypep(types)) {
		m = types->v.num;
	    	arr[m++] = c_number(values);
	    } else if (consp(values)) {
		for_both_lists(types, values, subrest, subrest2) {
		    mval = car(subrest);
		    TYPECHECK(mtypep, mval, "not a material type");
		    m = mval->v.num;
		    arr[m++] = c_number(car(subrest2));
		}
	    } else {
		for_all_list(types, subrest) {
		    mval = car(subrest);
		    TYPECHECK(mtypep, mval, "not a material type");
		    m = mval->v.num;
		    arr[m++] = c_number(values);
		}
	    }
	} else {
	    /* syntax error */
	}
    }
}

void
interp_atype_value_list(short *arr, Obj *lis)
{
    int a = 0;
    Obj *rest, *head, *types, *values, *subrest, *subrest2, *aval;

    /* Assume that if the destination array does not exist, there is
       probably a reason, and it's not our concern. */
    if (arr == NULL)
      return;
    for_all_list(lis, rest) {
    	head = car(rest);
    	if (numberp(head)) {
	    if (a < numatypes) {
	    	arr[a++] = c_number(head);
	    } else {
		init_warning("too many numbers in list");
	    }
	} else if (consp(head)) {
	    types = eval(car(head));
	    values = eval(cadr(head));
	    if (atypep(types)) {
		a = types->v.num;
	    	arr[a++] = c_number(values);
	    } else if (consp(values)) {
		for_both_lists(types, values, subrest, subrest2) {
		    aval = car(subrest);
		    TYPECHECK(atypep, aval, "not an advance");
		    a = aval->v.num;
		    arr[a++] = c_number(car(subrest2));
		}
	    } else {
		for_all_list(types, subrest) {
		    aval = car(subrest);
		    TYPECHECK(atypep, aval, "not an advance");
		    a = aval->v.num;
		    arr[a++] = c_number(values);
		}
	    }
	} else {
	    /* syntax error */
	}
    }
}

static
void
add_to_utypes(Obj *types, Obj *prop, Obj *values)
{
    char *propname = c_string(prop);
    Obj *lis1, *lis2, *carlis1;

    if (utypep(types)) {
	set_utype_property(types->v.num, propname, values);
    } else if (consp(types)) {
	if (consp(values) && !is_quoted_lisp(values)) {
	    if (!list_lengths_match(types, values, "utype property", prop))
	      return;
	    for_both_lists(types, values, lis1, lis2) {
		carlis1 = car(lis1);
		TYPECHECK(utypep, carlis1, "not a unit type");
		if (!set_utype_property(carlis1->v.num, propname, car(lis2)))
		  break;
	    }
	} else {
	    if (is_quoted_lisp(values))
	      values = eval(values);
	    for_all_list(types, lis1) {
		carlis1 = car(lis1);
		TYPECHECK(utypep, carlis1, "not a unit type");
		if (!set_utype_property(carlis1->v.num, propname, values))
		  break;
	    }
	}
    }
}

static
void
add_to_mtypes(Obj *types, Obj *prop, Obj *values)
{
    Obj *lis1, *lis2, *carlis1;

    if (mtypep(types)) {
	fill_in_mtype(types->v.num,
		      cons(cons(prop, cons(values, lispnil)), lispnil));
    } else if (consp(types)) {
	if (consp(values) && !is_quoted_lisp(values)) {
	    if (!list_lengths_match(types, values, "mtype property", prop))
	      return;
	    for_both_lists(types, values, lis1, lis2) {
		carlis1 = car(lis1);
		TYPECHECK(mtypep, carlis1, "not a unit type");
		fill_in_mtype(carlis1->v.num,
			      cons(cons(prop, cons(car(lis2), lispnil)),
				   lispnil));
	    }
	} else {
	    if (is_quoted_lisp(values))
	      values = eval(values);
	    for_all_list(types, lis1) {
		carlis1 = car(lis1);
		TYPECHECK(mtypep, carlis1, "not a material type");
		fill_in_mtype(carlis1->v.num,
			      cons(cons(prop, cons(values, lispnil)),
				   lispnil));
	    }
	}
    }
}

static
void
add_to_ttypes(Obj *types, Obj *prop, Obj *values)
{
    Obj *lis1, *lis2, *carlis1;

    if (ttypep(types)) {
	fill_in_ttype(types->v.num,
		      cons(cons(prop, cons(values, lispnil)), lispnil));
    } else if (consp(types)) {
	if (consp(values) && !is_quoted_lisp(values)) {
	    if (!list_lengths_match(types, values, "ttype property", prop))
	      return;
	    for_both_lists(types, values, lis1, lis2) {
		carlis1 = car(lis1);
		TYPECHECK(ttypep, carlis1, "not a terrain type");
		fill_in_ttype(carlis1->v.num,
			      cons(cons(prop, cons(car(lis2), lispnil)),
				   lispnil));
	    }
	} else {
	    if (is_quoted_lisp(values))
	      values = eval(values);
	    for_all_list(types, lis1) {
		carlis1 = car(lis1);
		TYPECHECK(ttypep, carlis1, "not a terrain type");
		fill_in_ttype(carlis1->v.num,
			      cons(cons(prop, cons(values, lispnil)),
				   lispnil));
	    }
	}
    }
}

static
void
add_to_atypes(Obj *types, Obj *prop, Obj *values)
{
    Obj *lis1, *lis2, *carlis1;

    if (atypep(types)) {
	fill_in_atype(types->v.num,
		      cons(cons(prop, cons(values, lispnil)), lispnil));
    } else if (consp(types)) {
	if (consp(values) && !is_quoted_lisp(values)) {
	    if (!list_lengths_match(types, values, "atype property", prop))
	      return;
	    for_both_lists(types, values, lis1, lis2) {
		carlis1 = car(lis1);
		TYPECHECK(atypep, carlis1, "not an advance type");
		fill_in_atype(carlis1->v.num,
			      cons(cons(prop, cons(car(lis2), lispnil)),
				   lispnil));
	    }
	} else {
	    if (is_quoted_lisp(values))
	      values = eval(values);
	    for_all_list(types, lis1) {
		carlis1 = car(lis1);
		TYPECHECK(atypep, carlis1, "not an advance type");
		fill_in_atype(carlis1->v.num,
			      cons(cons(prop, cons(values, lispnil)),
				   lispnil));
	    }
	}
    }
}

void
add_properties(Obj *form)
{
    Obj *rest, *types, *prop, *values;

    rest = cdr(form);
    types = eval(car(rest));
    rest = cdr(rest);
    prop = car(rest);
    SYNTAX(form, symbolp(prop), "not a property name in third position");
    rest = cdr(rest);
    SYNTAX(form, rest != lispnil, "no property values supplied");
    if (!is_quoted_lisp(car(rest)))
      values = eval(car(rest));
    else
      values = car(rest);
    /* Complain about, but ignore, extra things. */
    if (cdr(rest) != lispnil) {
	sprintlisp(readerrbuf, form, BUFSIZE);
	read_warning("Extra junk at the end of `%s', ignoring", readerrbuf);
    }
    if (utypep(types) || (consp(types) && utypep(car(types)))) {
	add_to_utypes(types, prop, values);
    } else if (mtypep(types) || (consp(types) && mtypep(car(types)))) {
	add_to_mtypes(types, prop, values);
    } else if (ttypep(types) || (consp(types) && ttypep(car(types)))) {
	add_to_ttypes(types, prop, values);
    } else if (atypep(types) || (consp(types) && atypep(car(types)))) {
	add_to_atypes(types, prop, values);
    } else {
	sprintlisp(readerrbuf, form, BUFSIZE);
	read_warning("No types to add to in `%s'", readerrbuf);
    }
}

void
write_utype_value_list(char *name, short *arr, int dflt, int addnewline)
{
    int u, writeany;

    if (arr == NULL)
      return;
    writeany = FALSE;
    for_all_unit_types(u) {
	if (arr[u] != dflt) {
	    writeany = TRUE;
	    break;
	}
    }
    if (!writeany)
      return;
    space_form();
    start_form(name);
    for_all_unit_types(u) {
	add_num_to_form(arr[u]);
    }
    end_form();
    if (addnewline) {
	newline_form();
	space_form();
    }
}

void
write_mtype_value_list(char *name, short *arr, int dflt, int addnewline)
{
    int m, writeany;

    if (nummtypes == 0 || arr == NULL)
      return;
    writeany = FALSE;
    for_all_material_types(m) {
	if (arr[m] != dflt) {
	    writeany = TRUE;
	    break;
	}
    }
    if (!writeany)
      return;
    space_form();
    start_form(name);
    for_all_material_types(m) {
	add_num_to_form(arr[m]);
    }
    end_form();
    if (addnewline) {
	newline_form();
	space_form();
    }
}

void
write_treasury_list(char *name, long *arr, int dflt, int addnewline)
{
    int m, writeany;

    if (nummtypes == 0 || arr == NULL)
      return;
    writeany = FALSE;
    for_all_material_types(m) {
	if (arr[m] != dflt) {
	    writeany = TRUE;
	    break;
	}
    }
    if (!writeany)
      return;
    space_form();
    start_form(name);
    for_all_material_types(m) {
	add_num_to_form(arr[m]);
    }
    end_form();
    if (addnewline) {
	newline_form();
	space_form();
    }
}

void
write_atype_value_list(char *name, short *arr, int dflt, int addnewline)
{
    int a, writeany;

    if (numatypes == 0 || arr == NULL)
      return;
    writeany = FALSE;
    for_all_advance_types(a) {
	if (arr[a] != dflt) {
	    writeany = TRUE;
	    break;
	}
    }
    if (!writeany)
      return;
    space_form();
    start_form(name);
    for_all_advance_types(a) {
	add_num_to_form(arr[a]);
    }
    end_form();
    if (addnewline) {
	newline_form();
	space_form();
    }
}

void
write_utype_string_list(char *name, char **arr, char *dflt, int addnewline)
{
    int u, writeany;

    if (arr == NULL)
      return;
    writeany = FALSE;
    for_all_unit_types(u) {
	if (arr[u] != dflt /* bogus, should use strcmp */) {
	    writeany = TRUE;
	    break;
	}
    }
    if (!writeany)
      return;
    space_form();
    start_form(name);
    for_all_unit_types(u) {
	add_to_form(escaped_string(arr[u]));
    }
    end_form();
    if (addnewline) {
	newline_form();
	space_form();
    }
}

void
write_type_name_list(int typ, int *flags, int dim)
{
    int j, first = TRUE, listlen = 0;

    if (flags == NULL)
      return;
    for (j = 0; j < dim; ++j)
      if (flags[j])
        ++listlen;
    if (listlen > 1)
      start_form("");
    for (j = 0; j < dim; ++j) {
	if (flags[j]) {
	    if (first)
	      first = FALSE;
	    else
	      space_form();
	    add_to_form_no_space(escaped_symbol(name_from_typ(typ, j)));
	}
    }
    if (listlen > 1)
      end_form();
}

void
write_types(void)
{
    int u, m, t, i, ival;
    char *name, *sval;
    Obj *obj;

    /* (or write out all the default values first for doc, then
       only write changed values) */

    for_all_unit_types(u) {
	start_form(key(K_UNIT_TYPE));
	name = shortest_escaped_name(u);
	add_to_form(name);
	newline_form();
	space_form();
	for (i = 0; utypedefns[i].name != NULL; ++i) {
	    /* Don't write out props used internally only, unless debugging. */
	    if ((strncmp(utypedefns[i].name, "zz-", 3) == 0) && !Debug)
	      continue;
	    if (utypedefns[i].intgetter) {
		ival = (*(utypedefns[i].intgetter))(u);
		write_num_prop(utypedefns[i].name, ival,
			       utypedefns[i].dflt, FALSE, TRUE);
	    } else if (utypedefns[i].strgetter) {
		sval = (*(utypedefns[i].strgetter))(u);
		/* Special-case a couple possibly-redundant slots. */
		if (utypedefns[i].strgetter == u_type_name
		    && strcmp(name, sval) == 0)
		  continue;
		if (utypedefns[i].strgetter == u_internal_name
		    && strcmp(name, sval) == 0)
		  continue;
		write_str_prop(utypedefns[i].name, sval,
			       utypedefns[i].dfltstr, FALSE, TRUE);
	    } else {
		obj = (*(utypedefns[i].objgetter))(u);
		write_lisp_prop(utypedefns[i].name, obj,
				lispnil, FALSE, FALSE, TRUE);
	    }
	}
	space_form();
	end_form();
	newline_form();
    }
    newline_form();
    for_all_material_types(m) {
	start_form(key(K_MATERIAL_TYPE));
	name = escaped_symbol(m_type_name(m));
	add_to_form(name);
	newline_form();
	space_form();
	for (i = 0; mtypedefns[i].name != NULL; ++i) {
	    /* Don't write out props used internally only, unless debugging. */
	    if ((strncmp(mtypedefns[i].name, "zz-", 3) == 0) && !Debug)
	      continue;
	    if (mtypedefns[i].intgetter) {
		ival = (*(mtypedefns[i].intgetter))(m);
		write_num_prop(mtypedefns[i].name, ival,
			       mtypedefns[i].dflt, FALSE, TRUE);
	    } else if (mtypedefns[i].strgetter) {
		sval = (*(mtypedefns[i].strgetter))(m);
		/* Special-case a a possibly-redundant slot. */
		if (mtypedefns[i].strgetter == m_type_name
		    && strcmp(name, sval) == 0)
		  continue;
		write_str_prop(mtypedefns[i].name, sval,
			       mtypedefns[i].dfltstr, FALSE, TRUE);
	    } else {
		obj = (*(mtypedefns[i].objgetter))(m);
		write_lisp_prop(mtypedefns[i].name, obj,
				lispnil, FALSE, FALSE, TRUE);
	    }
	}
	space_form();
	end_form();
	newline_form();
    }
    newline_form();
    for_all_terrain_types(t) {
	start_form(key(K_TERRAIN_TYPE));
	name = escaped_symbol(t_type_name(t));
	add_to_form(name);
	newline_form();
	space_form();
	for (i = 0; ttypedefns[i].name != NULL; ++i) {
	    /* Don't write out props used internally only, unless debugging. */
	    if ((strncmp(ttypedefns[i].name, "zz-", 3) == 0) && !Debug)
	      continue;
	    if (ttypedefns[i].intgetter) {
		ival = (*(ttypedefns[i].intgetter))(t);
		write_num_prop(ttypedefns[i].name, ival,
			       ttypedefns[i].dflt, FALSE, TRUE);
	    } else if (ttypedefns[i].strgetter) {
		sval = (*(ttypedefns[i].strgetter))(t);
		/* Special-case a a possibly-redundant slot. */
		if (ttypedefns[i].strgetter == t_type_name
		    && strcmp(name, sval) == 0)
		  continue;
		write_str_prop(ttypedefns[i].name, sval,
			       ttypedefns[i].dfltstr, FALSE, TRUE);
	    } else {
		obj = (*(ttypedefns[i].objgetter))(t);
		write_lisp_prop(ttypedefns[i].name, obj,
				lispnil, FALSE, FALSE, TRUE);
	    }
	}
	space_form();
	end_form();
	newline_form();
    }
    for_all_advance_types(t) {
	start_form(key(K_ADVANCE_TYPE));
	add_to_form(escaped_symbol(a_type_name(t)));
	newline_form();
	space_form();
	for (i = 0; atypedefns[i].name != NULL; ++i) {
	    /* Don't write out props used internally only, unless debugging. */
	    if ((strncmp(atypedefns[i].name, "zz-", 3) == 0) && !Debug)
	      continue;
	    if (atypedefns[i].intgetter) {
		ival = (*(atypedefns[i].intgetter))(t);
		write_num_prop(atypedefns[i].name, ival,
			       atypedefns[i].dflt, FALSE, TRUE);
	    } else if (atypedefns[i].strgetter) {
		sval = (*(atypedefns[i].strgetter))(t);
		/* Special-case a a possibly-redundant slot. */
		if (atypedefns[i].strgetter == a_type_name
		    && strcmp(name, sval) == 0)
		  continue;
		write_str_prop(atypedefns[i].name, sval,
			       atypedefns[i].dfltstr, FALSE, TRUE);
	    } else {
		obj = (*(atypedefns[i].objgetter))(t);
		write_lisp_prop(atypedefns[i].name, obj,
				lispnil, FALSE, FALSE, TRUE);
	    }
	}
	space_form();
	end_form();
	newline_form();
    }
    newline_form();
}

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END
