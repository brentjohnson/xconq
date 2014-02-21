// xConq
// GDL global variables.

// $Id: gvars.h,v 1.4 2006/06/02 16:58:33 eric_mcdonald Exp $

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
    \brief GDL global variables.
    \ingroup grp_gdl
*/

#ifndef XCONQ_GDL_GVARS_H
#define XCONQ_GDL_GVARS_H

#include "gdl/lisp.h"

// Global Constant Macros: GDL Limits

//! Low value for GDL globals.
/*! \todo Make INT_MIN or LONG_MIN. */
#define VARLO -2000000000
//! High value for GDL globals.
/*! \todo Make INT_MAX or LONG_MAX. */
#define VARHI 2000000000

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Data Structures

#ifdef  DEF_VAR_I
#undef  DEF_VAR_I
#endif
//! Define integer GDL global variable.
#define DEF_VAR_I(name,fname,setfname,doc,VAR,lo,dflt,hi)  \
    int VAR;

#ifdef  DEF_VAR_S
#undef  DEF_VAR_S
#endif
//! Define string GDL global variable.
#define DEF_VAR_S(name,fname,setfname,doc,VAR,dflt)  \
    char *VAR;

#ifdef  DEF_VAR_L
#undef  DEF_VAR_L
#endif
//! Define GDL object GDL global variable.
#define DEF_VAR_L(name,fname,setfname,doc,VAR,dflt)  \
    Obj *VAR;

//! GDL Global Variable
/*! The global data can be an int, a string, or a lisp object. */
typedef struct a_globals {

#include "gdl/gvar.def"

} Globals;

//! GDL Global Variable definition.
typedef struct vardefn {
    //! GDL Name
    char *name;
    //! Function to get integer value of GDL global.
    int   (*intgetter)(void);
    //! Function to get string value of GDL global.
    char *(*strgetter)(void);
    //! Function to get GDL object value of GDL global.
    Obj  *(*objgetter)(void);
    //! Function to set integer value of GDL global.
    void (*intsetter)(int);
    //! Function to set string value of GDL global.
    void (*strsetter)(char *);
    //! Function to set GDL object value of GDL global.
    void (*objsetter)(Obj *);
    //! Documentation string.
    char *doc;
    //! Default integer value.
    int dflt;
    //! Default string value.
    char *dfltstr;
    //! Function to return default GDL object value.
    Obj *(*dfltfn)(void);
    //! Least integer allowed,
    int lo;
    //! Greatest integer allowed.
    int hi;
} VarDefn;

// Global Variables: GDL Globals Pools

//! GDL global variables.
extern Globals globals;

//! Table of GDL global var definitions.
extern VarDefn vardefns[];

// GDL Variables Accessors

// GDL global variable getter functions.
#ifdef  DEF_VAR_I
#undef  DEF_VAR_I
#endif
#define DEF_VAR_I(str,FNAME,SETFNAME,doc,var,lo,dflt,hi)  \
  int FNAME(void);
#ifdef  DEF_VAR_S
#undef  DEF_VAR_S
#endif
#define DEF_VAR_S(str,FNAME,SETFNAME,doc,var,dflt)  \
  char *FNAME(void);
#ifdef  DEF_VAR_L
#undef  DEF_VAR_L
#endif
#define DEF_VAR_L(str,FNAME,SETFNAME,doc,var,dflt)  \
  Obj *FNAME(void);

#include "gdl/gvar.def"

// GDL global variable setter functions.
#ifdef  DEF_VAR_I
#undef  DEF_VAR_I
#endif
#define DEF_VAR_I(str,FNAME,SETFNAME,doc,var,lo,dflt,hi)  \
  void SETFNAME(int val);
#ifdef  DEF_VAR_S
#undef  DEF_VAR_S
#endif
#define DEF_VAR_S(str,FNAME,SETFNAME,doc,var,dflt)  \
  void SETFNAME(char *val);
#ifdef  DEF_VAR_L
#undef  DEF_VAR_L
#endif
#define DEF_VAR_L(str,FNAME,SETFNAME,doc,var,dflt)  \
  void SETFNAME(Obj *val);

#include "gdl/gvar.def"

// Queries

//! Default value for integer gvar.
extern int gvar_i_default(int (*intgetter)(void));
//! Default value for string gvar.
extern char *gvar_s_default(char *(*strgetter)(void));
//! Default value for Lisp gvar.
extern Obj *gvar_l_default(Obj *(*objgetter)(void));

// Game Setup

//! Unbind variable.
extern void undefine_variable(Obj *form);

// Lifecycle Management

//! Initialize all globals to default values.
/*!
    Set the globals to their default values.  For Lisp values, the default
    is an initialization function rather than a value.
*/
extern void init_globals(void);

// GDL I/O

//! Set binding of existing known variable.
extern void interp_variable(Obj *form, int isnew);

//! Serialize globals to GDL.
/*!
    Write the globals.  The "complete" flag forces all values out, even
    if they match the compiled-in defaults.
*/
extern void write_globals(void);

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

#endif // ifndef XCONQ_GDL_GVARS_H
