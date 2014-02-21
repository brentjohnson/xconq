// xConq
// GDL tables.

// $Id: tables.h,v 1.6 2006/06/02 16:58:33 eric_mcdonald Exp $

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

#ifndef XCONQ_GDL_TABLES_H
#define XCONQ_GDL_TABLES_H

#include "gdl/types.h"

// Global Constant Macros

//! Least value for integer in table.
#define TABLO -32768
//! Greatest value for integer in table.
#define TABHI 32767

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Data Structures

//! Game table.
typedef struct tabledefn {
    //! GDL Name
    char *name;
    //! Function to get integer from table.
    int (*getter)(int, int);
    //! Documentation string.
    char *doc;
    //! Pointer to table's integer storage.
    short **table;
    //! Default value of entries.
    short dflt;
    //! Least integer allowed.
    short lo;
    //! Greatest integer allowed.
    short hi;
    //! Type of row indices.
    char index1;
    //! Type of column indices.
    char index2;
    //! Type of data in table.
    char valtype;
} TableDefn;

// Global Variables

// Raw storage and default values.

#ifdef  DEF_UU_TABLE
#undef  DEF_UU_TABLE
#endif
#define DEF_UU_TABLE(name,FNAME,doc,TABLE,CNST,lo,dflt,hi,valtype)  \
  extern short *TABLE, CNST;

#ifdef  DEF_UM_TABLE
#undef  DEF_UM_TABLE
#endif
#define DEF_UM_TABLE(name,FNAME,doc,TABLE,CNST,lo,dflt,hi,valtype)  \
  extern short *TABLE, CNST;

#ifdef  DEF_UT_TABLE
#undef  DEF_UT_TABLE
#endif
#define DEF_UT_TABLE(name,FNAME,doc,TABLE,CNST,lo,dflt,hi,valtype)  \
  extern short *TABLE, CNST;

#ifdef  DEF_TM_TABLE
#undef  DEF_TM_TABLE
#endif
#define DEF_TM_TABLE(name,FNAME,doc,TABLE,CNST,lo,dflt,hi,valtype)  \
  extern short *TABLE, CNST;

#ifdef  DEF_TT_TABLE
#undef  DEF_TT_TABLE
#endif
#define DEF_TT_TABLE(name,FNAME,doc,TABLE,CNST,lo,dflt,hi,valtype)  \
  extern short *TABLE, CNST;

#ifdef  DEF_MM_TABLE
#undef  DEF_MM_TABLE
#endif
#define DEF_MM_TABLE(name,FNAME,doc,TABLE,CNST,lo,dflt,hi,valtype)  \
  extern short *TABLE, CNST;

#ifdef  DEF_UA_TABLE
#undef  DEF_UA_TABLE
#endif
#define DEF_UA_TABLE(name,FNAME,doc,TABLE,CNST,lo,dflt,hi,valtype)  \
  extern short *TABLE, CNST;

#ifdef  DEF_AM_TABLE
#undef  DEF_AM_TABLE
#endif
#define DEF_AM_TABLE(name,FNAME,doc,TABLE,CNST,lo,dflt,hi,valtype)  \
  extern short *TABLE, CNST;

#ifdef  DEF_AA_TABLE
#undef  DEF_AA_TABLE
#endif
#define DEF_AA_TABLE(name,FNAME,doc,TABLE,CNST,lo,dflt,hi,valtype)  \
  extern short *TABLE, CNST;

#include "gdl/table.def"

//! Array of tables.
extern TableDefn tabledefns[];

// Table Accessors

#ifdef  DEF_UU_TABLE
#undef  DEF_UU_TABLE
#endif
#define DEF_UU_TABLE(name,FNAME,doc,TABLE,CNST,lo,dflt,hi,valtype)  \
  extern int FNAME(int u1, int u2);

#ifdef  DEF_UM_TABLE
#undef  DEF_UM_TABLE
#endif
#define DEF_UM_TABLE(name,FNAME,doc,TABLE,CNST,lo,dflt,hi,valtype)  \
  extern int FNAME(int u, int m);

#ifdef  DEF_UT_TABLE
#undef  DEF_UT_TABLE
#endif
#define DEF_UT_TABLE(name,FNAME,doc,TABLE,CNST,lo,dflt,hi,valtype)  \
  extern int FNAME(int u, int t);

#ifdef  DEF_TM_TABLE
#undef  DEF_TM_TABLE
#endif
#define DEF_TM_TABLE(name,FNAME,doc,TABLE,CNST,lo,dflt,hi,valtype)  \
  extern int FNAME(int t, int m);

#ifdef  DEF_TT_TABLE
#undef  DEF_TT_TABLE
#endif
#define DEF_TT_TABLE(name,FNAME,doc,TABLE,CNST,lo,dflt,hi,valtype)  \
  extern int FNAME(int t1, int t2);

#ifdef  DEF_MM_TABLE
#undef  DEF_MM_TABLE
#endif
#define DEF_MM_TABLE(name,FNAME,doc,TABLE,CNST,lo,dflt,hi,valtype)  \
  extern int FNAME(int m1, int m2);

#ifdef  DEF_UA_TABLE
#undef  DEF_UA_TABLE
#endif
#define DEF_UA_TABLE(name,FNAME,doc,TABLE,CNST,lo,dflt,hi,valtype)  \
  extern int FNAME(int u, int a);

#ifdef  DEF_AM_TABLE
#undef  DEF_AM_TABLE
#endif
#define DEF_AM_TABLE(name,FNAME,doc,TABLE,CNST,lo,dflt,hi,valtype)  \
  extern int FNAME(int a, int m);

#ifdef  DEF_AA_TABLE
#undef  DEF_AA_TABLE
#endif
#define DEF_AA_TABLE(name,FNAME,doc,TABLE,CNST,lo,dflt,hi,valtype)  \
  extern int FNAME(int a1, int a2);

#include "gdl/table.def"

//! Default value for table.
extern int table_default(int (*getter)(int, int));

// Game Setup

//! Allocate storage for given GDL table.
extern void allocate_table(int tbl, int reset);

// GDL I/O

//! Print contents of table at given index in table definitions array.
/*!
    \todo Should perhaps use something similar to 'write_table' to get
	  more compact output.
*/
extern void fprinttable(FILE *fp, int n);

//! Fill in a table from GDL form.
extern void interp_table(Obj *form);

#if (0)
//! Serialize stripe of table values to GDL.
extern void write_type_value_list(
    int typ, int *flags, int dim, int (*getter)(int, int), int i);
#endif

//! Serialize table to GDL.
/*!
    Write out a single table.  Only write it if it contains non-default
    values, and try to find runs of constant value, since tables can be
    really large, but often have constant areas within them.
*/
extern void write_table(
    char *name,
    int (*getter)(int, int),
    int dflt,
    int typ1, int typ2, int valtype,
    int compress);

//! Serialize tables to GDL.
extern void write_tables(int compress);

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

#endif // ifndef XCONQ_GDL_TABLES_H
