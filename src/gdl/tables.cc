// xConq
// GDL tables.

// $Id: tables.cc,v 1.5 2006/06/02 16:57:43 eric_mcdonald Exp $

/*//////////////////////////// COPYRIGHT NOTICE ////////////////////////////////

  Copyright (C) 1987-1989   Stanley T. Shebs
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
    \brief GDL tables.
    \ingroup grp_gdl
*/

#include "gdl/dice.h"
#include "gdl/tables.h"

// Function Macros: Validation

//! Given GDL game entity type, check if given GDL object actual is one.
#define INDEXP(typ, x) \
  ((typ == UTYP) ? utypep(x) : ((typ == MTYP) ? mtypep(x) : ((typ == TTYP) ? ttypep(x) : atypep(x))))

//! Is GDL object not a list?
#define nonlist(x) (!consp(x) && x != lispnil)

//! Is 1st table index of the correct type?
#define CHECK_INDEX_1(tbl, x)  \
  if (!INDEXP(tabledefns[tbl].index1, (x))) {  \
      char errbuf[BUFSIZE]; \
      sprintlisp(errbuf, (x), BUFSIZE); \
      read_warning("table %s index 1 value `%s' has wrong type, skipping clause",  \
		   tabledefns[tbl].name, errbuf);  \
      return;  \
  }

//! Is 2nd table index of the correct type?
#define CHECK_INDEX_2(tbl, x)  \
  if (!INDEXP(tabledefns[tbl].index2, (x))) {  \
      char errbuf[BUFSIZE]; \
      sprintlisp(errbuf, (x), BUFSIZE); \
      read_warning("table %s index 2 value `%s' has wrong type, skipping clause",  \
		   tabledefns[tbl].name, errbuf);  \
      return;  \
  }

//! Is value numberish enough to be table value?
#define CHECK_VALUE(tbl, x)  \
  if (!numberishp(x)) {  \
      read_warning("table %s value is not a number, skipping clause",  \
		   tabledefns[tbl].name);  \
      return;  \
  } \
  { int checknum = c_number(x);  \
    if (!between(tabledefns[tbl].lo, checknum, tabledefns[tbl].hi)) {  \
	read_warning("table %s value %d not within bounds %d to %d, skipping clause",  \
		     tabledefns[tbl].name, checknum, tabledefns[tbl].lo, tabledefns[tbl].hi);  \
	return;  \
    }  \
  }

//! If faced with two lists, do their lengths match?
#define CHECK_LISTS(tablename, lis1, lis2)  \
  if (consp(lis2)  \
      && !list_lengths_match(lis1, lis2, "table", tablename))  {  \
      return;  \
  }

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Local Data Structures

//! Simple histogram struct - count and value, that's all.
struct histo { int count, val; };

// Globals

/*! \showinitializer */
TableDefn tabledefns[] = {

#ifdef  DEF_UU_TABLE
#undef  DEF_UU_TABLE
#endif
#define DEF_UU_TABLE(NAME,FNAME,DOC,TABLE,CNST,LO,DFLT,HI,VALTYPE)  \
    { NAME, FNAME, DOC, &TABLE, DFLT, LO, HI, UTYP, UTYP, VALTYPE },

#ifdef  DEF_UM_TABLE
#undef  DEF_UM_TABLE
#endif
#define DEF_UM_TABLE(NAME,FNAME,DOC,TABLE,CNST,LO,DFLT,HI,VALTYPE)  \
    { NAME, FNAME, DOC, &TABLE, DFLT, LO, HI, UTYP, MTYP, VALTYPE },

#ifdef  DEF_UT_TABLE
#undef  DEF_UT_TABLE
#endif
#define DEF_UT_TABLE(NAME,FNAME,DOC,TABLE,CNST,LO,DFLT,HI,VALTYPE)  \
    { NAME, FNAME, DOC, &TABLE, DFLT, LO, HI, UTYP, TTYP, VALTYPE },

#ifdef  DEF_TM_TABLE
#undef  DEF_TM_TABLE
#endif
#define DEF_TM_TABLE(NAME,FNAME,DOC,TABLE,CNST,LO,DFLT,HI,VALTYPE)  \
    { NAME, FNAME, DOC, &TABLE, DFLT, LO, HI, TTYP, MTYP, VALTYPE },

#ifdef  DEF_TT_TABLE
#undef  DEF_TT_TABLE
#endif
#define DEF_TT_TABLE(NAME,FNAME,DOC,TABLE,CNST,LO,DFLT,HI,VALTYPE)  \
    { NAME, FNAME, DOC, &TABLE, DFLT, LO, HI, TTYP, TTYP, VALTYPE },

#ifdef  DEF_MM_TABLE
#undef  DEF_MM_TABLE
#endif
#define DEF_MM_TABLE(NAME,FNAME,DOC,TABLE,CNST,LO,DFLT,HI,VALTYPE)  \
    { NAME, FNAME, DOC, &TABLE, DFLT, LO, HI, MTYP, MTYP, VALTYPE },

#ifdef  DEF_UA_TABLE
#undef  DEF_UA_TABLE
#endif
#define DEF_UA_TABLE(NAME,FNAME,DOC,TABLE,CNST,LO,DFLT,HI,VALTYPE)  \
    { NAME, FNAME, DOC, &TABLE, DFLT, LO, HI, UTYP, ATYP, VALTYPE },

#ifdef  DEF_AM_TABLE
#undef  DEF_AM_TABLE
#endif
#define DEF_AM_TABLE(NAME,FNAME,DOC,TABLE,CNST,LO,DFLT,HI,VALTYPE)  \
    { NAME, FNAME, DOC, &TABLE, DFLT, LO, HI, ATYP, MTYP, VALTYPE },

#ifdef  DEF_AA_TABLE
#undef  DEF_AA_TABLE
#endif
#define DEF_AA_TABLE(NAME,FNAME,DOC,TABLE,CNST,LO,DFLT,HI,VALTYPE)  \
    { NAME, FNAME, DOC, &TABLE, DFLT, LO, HI, ATYP, ATYP, VALTYPE },

#include "gdl/table.def"

    { NULL }

};

// Local Function Declarations: GDL I/O

//! Read GDL table clause forms into table.
static void add_to_table(Obj *tablename, int tbl, Obj *clauses);

//! Read one GDL table clause form into table.
static void interp_one_clause(
    Obj *tablename, int tbl, int lim1, int lim2,
    Obj *indexes1, Obj *indexes2, Obj *values);

//! Sort predicate - *descending* order by count.
static int histo_compare(const void *x, const void *y);

// Table Accessors

#ifdef  DEF_UU_TABLE
#undef  DEF_UU_TABLE
#endif
#define DEF_UU_TABLE(name,FNAME,doc,TABLE,CNST,lo,DFLT,hi,valtype)  \
  short *TABLE = NULL, CNST = DFLT;  \
  int FNAME(int u1, int u2) \
    { checku(u1);  checku(u2);  \
      return (TABLE ? TABLE[numutypes*(u1)+(u2)] : CNST); }

#ifdef  DEF_UM_TABLE
#undef  DEF_UM_TABLE
#endif
#define DEF_UM_TABLE(name,FNAME,doc,TABLE,CNST,lo,DFLT,hi,valtype)  \
  short *TABLE = NULL, CNST = DFLT;  \
  int FNAME(int u, int m) \
    { checku(u);  checkm(m);  \
      return (TABLE ? TABLE[nummtypes*(u)+(m)] : CNST); }

#ifdef  DEF_UT_TABLE
#undef  DEF_UT_TABLE
#endif
#define DEF_UT_TABLE(name,FNAME,doc,TABLE,CNST,lo,DFLT,hi,valtype)  \
  short *TABLE = NULL, CNST = DFLT;  \
  int FNAME(int u, int t) \
    { checku(u);  checkt(t);  \
      return (TABLE ? TABLE[numttypes*(u)+(t)] : CNST); }

#ifdef  DEF_TM_TABLE
#undef  DEF_TM_TABLE
#endif
#define DEF_TM_TABLE(name,FNAME,doc,TABLE,CNST,lo,DFLT,hi,valtype)  \
  short *TABLE = NULL, CNST = DFLT;  \
  int FNAME(int t, int m) \
    { checkt(t);  checkm(m);  \
      return (TABLE ? TABLE[nummtypes*(t)+(m)] : CNST); }

#ifdef  DEF_TT_TABLE
#undef  DEF_TT_TABLE
#endif
#define DEF_TT_TABLE(name,FNAME,doc,TABLE,CNST,lo,DFLT,hi,valtype)  \
  short *TABLE = NULL, CNST = DFLT;  \
  int FNAME(int t1, int t2) \
    { checkt(t1);  checkt(t2);  \
      return (TABLE ? TABLE[numttypes*(t1)+(t2)] : CNST); }

#ifdef  DEF_MM_TABLE
#undef  DEF_MM_TABLE
#endif
#define DEF_MM_TABLE(name,FNAME,doc,TABLE,CNST,lo,DFLT,hi,valtype)  \
  short *TABLE = NULL, CNST = DFLT;  \
  int FNAME(int m1, int m2) \
    { checkm(m1);  checkm(m2);  \
      return (TABLE ? TABLE[nummtypes*(m1)+(m2)] : CNST); }

#ifdef  DEF_UA_TABLE
#undef  DEF_UA_TABLE
#endif
#define DEF_UA_TABLE(name,FNAME,doc,TABLE,CNST,lo,DFLT,hi,valtype)  \
  short *TABLE = NULL, CNST = DFLT;  \
  int FNAME(int u, int a) \
    { checku(u);  checka(a);  \
      return (TABLE ? TABLE[numatypes*(u)+(a)] : CNST); }

#ifdef  DEF_AM_TABLE
#undef  DEF_AM_TABLE
#endif
#define DEF_AM_TABLE(name,FNAME,doc,TABLE,CNST,lo,DFLT,hi,valtype)  \
  short *TABLE = NULL, CNST = DFLT;  \
  int FNAME(int a, int m) \
    { checka(a);  checkm(m);  \
      return (TABLE ? TABLE[nummtypes*(a)+(m)] : CNST); }

#ifdef  DEF_AA_TABLE
#undef  DEF_AA_TABLE
#endif
#define DEF_AA_TABLE(name,FNAME,doc,TABLE,CNST,lo,DFLT,hi,valtype)  \
  short *TABLE = NULL, CNST = DFLT;  \
  int FNAME(int a1, int a2) \
    { checka(a1);  checka(a2);  \
      return (TABLE ? TABLE[numatypes*(a1)+(a2)] : CNST); }

#include "gdl/table.def"

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

// Game Setup

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

// GDL I/O

void
fprinttable(FILE *fp, int n)
{
    int i = -1, j = -1;
    int (*getter)(int i, int j) = NULL;
    int dim1 = -1, dim2 = -1, valtype = -1;

    getter = tabledefns[n].getter;
    dim1 = numtypes_from_index_type(tabledefns[n].index1);
    dim2 = numtypes_from_index_type(tabledefns[n].index2);
    valtype = tabledefns[n].valtype;
    fprintf(fp, "\n(");
    for (i = 0; i < dim1; ++i) {
	fprintf(fp, "\n\t(");
	for (j = 0; j < dim2; ++j) {
	    fprint_num_or_dice(fp, getter(i, j), valtype);
	}
	fprintf(fp, ")");
    }
    fprintf(fp, ")\n");
}

static void
interp_one_clause(Obj *tablename, int tbl, int lim1, int lim2,
		  Obj *indexes1, Obj *indexes2, Obj *values)
{
    int i, j, num;
    Obj *tmp1, *tmp2, *in1, *in2, *value, *subvalue;

    if (nonlist(indexes1)) {
	CHECK_INDEX_1(tbl, indexes1);
	i = c_number(indexes1);
	if (nonlist(indexes2) ) {
	    CHECK_INDEX_2(tbl, indexes2);
	    j = c_number(indexes2);
	    value = values;
	    CHECK_VALUE(tbl, value);
	    num = c_number(value);
	    (*(tabledefns[tbl].table))[lim2 * i + j] = num;
	} else {
	    CHECK_LISTS(tablename, indexes2, values);
	    for_all_list(indexes2, tmp2) {
		in2 = car(tmp2);
		CHECK_INDEX_2(tbl, in2);
		j = c_number(in2);
		value = (consp(values) ? car(values) : values);
		CHECK_VALUE(tbl, value);
		num = c_number(value);
		(*(tabledefns[tbl].table))[lim2 * i + j] = num;
		if (consp(values))
		  values = cdr(values);
	    }
	}
    } else {
	CHECK_LISTS(tablename, indexes1, values);
	for_all_list(indexes1, tmp1) {
	    in1 = car(tmp1);
	    CHECK_INDEX_1(tbl, in1);
	    i = c_number(in1);
	    value = (consp(values) ? car(values) : values);
	    if (nonlist(indexes2)) {
		CHECK_INDEX_2(tbl, indexes2);
		j = c_number(indexes2);
		CHECK_VALUE(tbl, value);
		num = c_number(value);
		(*(tabledefns[tbl].table))[lim2 * i + j] = num;
	    } else {
		if (nonlist(value)) {
		    subvalue = value;
		    CHECK_VALUE(tbl, subvalue);
		    num = c_number(subvalue);
		    for_all_list(indexes2, tmp2) {
			in2 = car(tmp2);
			CHECK_INDEX_2(tbl, in2);
			j = c_number(in2);
			(*(tabledefns[tbl].table))[lim2 * i + j] = num;
		    }
		} else {
		    CHECK_LISTS(tablename, indexes2, value);
		    for_all_list(indexes2, tmp2) {
			in2 = car(tmp2);
			CHECK_INDEX_2(tbl, in2);
			j = c_number(in2);
			subvalue = car(value);
			CHECK_VALUE(tbl, subvalue);
			num = c_number(subvalue);
			(*(tabledefns[tbl].table))[lim2 * i + j] = num;
			value = cdr(value);
		    }
		}
	    }
	    if (consp(values))
	      values = cdr(values);
	}
    }
}

static void
add_to_table(Obj *tablename, int tbl, Obj *clauses)
{
    int i, num, lim1, lim2;
    Obj *clause, *indexes1, *indexes2, *values;

    lim1 = numtypes_from_index_type(tabledefns[tbl].index1);
    lim2 = numtypes_from_index_type(tabledefns[tbl].index2);
    for ( ; clauses != lispnil; clauses = cdr(clauses)) {
	clause = car(clauses);
	switch (clause->type) {
	  case SYMBOL:
	    clause = eval_symbol(clause);
	    TYPECHECK(numberp, clause, "table clause does not eval to number");
	    /* Now treat it as a number. */
	  case NUMBER:
	    /* A constant value for the table - blast over everything. */
	    CHECK_VALUE(tbl, clause);
	    num = c_number(clause);
	    /* Make sure the table is allocated first. */
	    if (tabledefns[tbl].table == NULL)
	      allocate_table(tbl, TRUE);
	    for (i = 0; i < lim1 * lim2; ++i)
	      (*(tabledefns[tbl].table))[i] = num;
	    break;
	  case CONS:
	    /* Evaluate the three parts of a clause. */
	    indexes1 = eval(car(clause));
	    indexes2 = eval(cadr(clause));
	    values = eval(caddr(clause));
	    if (cdddr(clause) != lispnil) {
		sprintlisp(readerrbuf, clause, BUFSIZE);
		read_warning("In table `%s', extra junk at end of clause `%s', ignoring",
			     c_string(tablename), readerrbuf);
	    }
	    interp_one_clause(tablename, tbl, lim1, lim2,
			      indexes1, indexes2, values);
	    break;
	  case STRING:
	    break; /* error? */
	  default:
	    /* who knows? */
	    break;
	}
    }
}

void
interp_table(Obj *form)
{
    int i, found, reset = TRUE;
    Obj *formsym = cadr(form), *body = cddr(form);
    char *tablename;

    /* It's impossible to know whether all the types indexed by a
       specific table have been defined when the table form is
       evaluated, so don't try any heuristic tricks to load a default
       game here. */
    TYPECHECK(symbolp, formsym, "table name not a symbol");
    tablename = c_string(formsym);
    /* Don't read in tables used internally only. */
    if (strncmp(tablename, "zz-", 3) == 0)
        return;
    found = FALSE;
    /* Search in the list of tables for one with the given name. */
    for (i = 0; tabledefns[i].name != NULL; ++i) {
	if (strcmp(tablename, tabledefns[i].name) == 0) {
	    if (match_keyword(car(body), K_ADD)) {
		body = cdr(body);
		reset = FALSE;
	    }
	    allocate_table(i, reset);
	    add_to_table(formsym, i, body);
	    found = TRUE;
	    break;
	}
    }
    if (!found)
      read_warning( "Undefined table `%s'", tablename);
}

static
int
histo_compare(const void *x, const void *y)
{
    return ((struct histo *) y)->count - ((struct histo *) x)->count;
}

#if (0)
void
write_type_value_list(
    int typ, int *flags, int dim, int (*getter)(int, int), int i)
{
    int j, first = TRUE, listlen = 0;

    for (j = 0; j < dim; ++j)
      if (flags == NULL || flags[j])
        ++listlen;
    if (listlen > 1)
      start_form("");
    for (j = 0; j < dim; ++j) {
	if (flags == NULL || flags[j]) {
	    if (first)
	      first = FALSE;
	    else
	      space_form();
	    add_num_to_form_no_space((*getter)(i, j));
	}
    }
    if (listlen > 1)
      end_form();
}
#endif

void
write_table(char *name, int (*getter)(int, int), int dflt, int typ1, int typ2,
	    int valtype, int compress)
{
    int i, j, k, colvalue, constcol, next;
    int numrandoms, nextrowdiffers, writeconst;
    int sawfirst, constrands, constval;
    int dim1 = numtypes_from_index_type(typ1);
    int dim2 = numtypes_from_index_type(typ2);
    struct histo mostcommon[500]; /* more than max of num[utma]types */
    int indexes1[500], randoms[500];

    start_form(key(K_TABLE));
    add_to_form(name);
    add_to_form(" ;");
    add_num_or_dice_to_form(dflt, valtype);
    if (!compress) {
	/* Write every value separately. */
	for (i = 0; i < dim1; ++i) {
	    newline_form();
	    space_form();
	    space_form();
	    start_form(escaped_symbol(name_from_typ(typ1, i)));
	    add_to_form((char *)star_from_typ(typ2));
	    space_form();
	    start_form("");
	    for (j = 0; j < dim2; ++j) {
		add_num_or_dice_to_form((*getter)(i, j), valtype);
	    }
	    end_form();
	    end_form();
	}
    } else if (dim1 <= dim2) {
        /* Analyze the table by rows. */
	for (k = 0; k < dim1; ++k)
	  indexes1[k] = FALSE;
	for (i = 0; i < dim1; ++i) {
	    /* First see if this row has all the same values as the next. */
	    indexes1[i] = TRUE;
	    nextrowdiffers = FALSE;
	    if (i < dim1 - 1) {
	    	for (j = 0; j < dim2; ++j) {
	    	    if ((*getter)(i, j) != (*getter)(i + 1, j)) {
	    	    	nextrowdiffers = TRUE;
	    	    	break;
	    	    }
	    	}
	    } else {
	    	/* The last row is *always* "different". */
	    	nextrowdiffers = TRUE;
	    }
	    /* (should look at *all* rows to find matching rows before
	       dumping one) */
	    if (nextrowdiffers) {
		/* Make a histogram of all the values in this row. */
		mostcommon[0].count = 1;
		mostcommon[0].val = (*getter)(i, 0);
		next = 1;
		for (j = 0; j < dim2; ++j) {
		    for (k = 0; k < next; ++k) {
			if (mostcommon[k].val == (*getter)(i, j)) {
			    ++(mostcommon[k].count);
			    break;
			}
		    }
		    if (k == next) {
			mostcommon[next].count = 1;
			mostcommon[next].val = (*getter)(i, j);
			++next;
		    }
		}
		if (next == 1 && mostcommon[0].val == dflt) {
		    /* Entire row(s) is/are just the default table value. */
		} else {
		    writeconst = FALSE;
		    numrandoms = 0;
		    if (next == 1) {
			/* Only one value in the row(s). */
			writeconst = TRUE;
		    } else {
			qsort(mostcommon, next, sizeof(struct histo),
			      histo_compare);
			if (mostcommon[0].count >= (3 * dim2) / 4) {
			    /* The most common value in this row(s) is
			       not the only value, but it is worth
			       writing into a separate clause. */
			    writeconst = TRUE;
			    for (j = 0; j < dim2; ++j) {
				/* Flag the other values as needing to be
				   written separately. */
				randoms[j] =
				  (mostcommon[0].val != (*getter)(i, j));
				if (randoms[j])
				  ++numrandoms;
			    }
			} else {
			    /* Flag all in the row as randoms. */
			    for (j = 0; j < dim2; ++j) {
				randoms[j] = TRUE;
				++numrandoms;
			    }
			}
		    }
		    /* Write out the most common value (if
		       non-default) in the row(s), expressing it with
		       a clause that applies the value to the entire
		       row(s). */
		    if (writeconst && mostcommon[0].val != dflt) {
			newline_form();
			space_form();
			space_form();
			start_form("");
			write_type_name_list(typ1, indexes1, dim1);
			add_to_form((char *)star_from_typ(typ2));
			add_num_or_dice_to_form(mostcommon[0].val, valtype);
			end_form();
		    }
		    /* Now override the most common value with any
		       exceptions. */
		    if (numrandoms > 0) {
			constrands = TRUE;
			sawfirst = FALSE;
			for (j = 0; j < dim2; ++j) {
			    if (randoms[j]) {
			        if (!sawfirst) {
				    constval = (*getter)(i, j);
				    sawfirst = TRUE;
			        }
			        if (sawfirst && constval != (*getter)(i, j)) {
				    constrands = FALSE;
				    break;
			        }
			    }
			}
			if (constrands) {
			    newline_form();
			    space_form();
			    space_form();
			    start_form("");
			    write_type_name_list(typ1, indexes1, dim1);
			    space_form();
			    write_type_name_list(typ2, randoms, dim2);
			    add_num_or_dice_to_form(constval, valtype);
			    end_form();
			} else {
			    /* We have a group of rows with varying data
			       in the columns; write a separate row. */
			    for (j = 0; j < dim2; ++j) {
				if (randoms[j]) {
				    newline_form();
				    space_form();
				    space_form();
				    start_form("");
				    write_type_name_list(typ1, indexes1, dim1);
				    add_to_form(escaped_symbol(name_from_typ(typ2, j)));
				    add_num_or_dice_to_form((*getter)(i, j), valtype);
				    end_form();
				}
			    }
			}
		    }
		}
		/* Reset the row flags in preparation for the next group
		   of rows whose contents match each other. */
		for (k = 0; k < dim1; ++k)
		  indexes1[k] = FALSE;
	    }
	}
    } else {
        /* Analyze the table by columns. */
        /* Don't work as hard to optimize; this case should be uncommon,
	   since there are usually more types of units than
	   materials or terrain. */
	for (j = 0; j < dim2; ++j) {
	    constcol = TRUE;
	    colvalue = (*getter)(0, j);
	    for (i = 0; i < dim1; ++i) {
		if ((*getter)(i, j) != colvalue) {
		    constcol = FALSE;
		    break;
		}
	    }
	    if (!constcol || colvalue != dflt) {
		newline_form();
		space_form();
		space_form();
		start_form((char *)star_from_typ(typ1));
		add_to_form(escaped_symbol(name_from_typ(typ2, j)));
		/* Write out either a single constant value or a list of
		   varying values, as appropriate. */
		if (constcol) {
		    add_num_or_dice_to_form(colvalue, valtype);
		} else {
		    space_form();
		    start_form("");
		    for (i = 0; i < dim1; ++i) {
			add_num_or_dice_to_form((*getter)(i, j), valtype);
		    }
		    end_form();
		}
		end_form();
	    }
	}
    }
    newline_form();
    space_form();
    space_form();
    end_form();
    newline_form();
}

void
write_tables(int compress)
{
    int tbl;

    newline_form();
    for (tbl = 0; tabledefns[tbl].name != 0; ++tbl) {
	/* Don't write out tables used internally only, unless debugging. */
	if ((strncmp(tabledefns[tbl].name, "zz-", 3) == 0) && !Debug)
	  continue;
	if (*(tabledefns[tbl].table) != NULL) {
	    write_table(tabledefns[tbl].name,
			tabledefns[tbl].getter, tabledefns[tbl].dflt,
			tabledefns[tbl].index1, tabledefns[tbl].index2,
			tabledefns[tbl].valtype, compress);
	    newline_form();
	}
    }
}

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END
