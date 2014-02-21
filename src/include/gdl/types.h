// xConq
// Game entity types.

// $Id: types.h,v 1.7 2006/06/02 16:58:33 eric_mcdonald Exp $

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
    \brief Game entity types.
    \ingroup grp_gdl
*/

#ifndef XCONQ_GDL_TYPES_H
#define XCONQ_GDL_TYPES_H

#include "gdl/lisp.h"

// Global Constant Macros

//! Least value for integer property.
#define PROPLO -32768
//! Greatest value for integer property.
#define PROPHI 32767

//! Non unit type.
/*! Guaranteed not to be a Unit type. */
#define NONUTYPE (-1)

//! Non material type.
/*! Guaranteed not to be a Material type. */
#define NONMTYPE (-1)

//! Non terrain type.
/*! Guaranteed not to be a Terrain type. Used in interfaces for unseen terrain. */
#define NONTTYPE (-1)
//! Background terrain outside the map.
/*!
    Used in interfaces for bogus terrain outside the map when redrawing the
    window background.
*/
#define BACKTTYPE (-2)

//! Non advance type.
/*! Guaranteed not to be an Advance type. */
#define NONATYPE (-1)
//! Advance complete.
/*! Used in advances code, must not overlap with NONATYPE. */
#define DONE (-2)
/*! No advance.
/*! Used in advances code, must not overlap with NONATYPE, and
    presumably with DONE, as well. */
#define NOADVANCE (-3)

// Function Macros: Validation

//! Test validity of type, and error if invalid.
#define TYPECHECK(PRED,X,MSG)  \
    if (!PRED(X)) {  \
      type_error((X), (MSG));  \
      return;  \
    }

//! Test validity of type, and error if invalid.
/*! Return given return code, if invalid. */
#define TYPECHECK_RETURN(PRED,X,MSG,RET)  \
    if (!PRED(X)) {  \
      type_error((X), (MSG));  \
      return (RET);  \
    }

// Function Macros: Queries

//! Check unit type range.
/*! If the Unit is out of range, call utype_error(). */
#define checku(x) if ((x) < 0 || (x) >= numutypes) utype_error(x);

//! Check material type range.
/*! If the Material type is out of range, call mtype_error(). */
#define checkm(x) if ((x) < 0 || (x) >= nummtypes) mtype_error(x);

//! Check terrain type range.
/*! If the Terrain type is out of range, call ttype_error(). */
#define checkt(x) if ((x) < 0 || (x) >= numttypes) ttype_error(x);

//! Check advance type range.
/*! If the Advance type is out of range, call atype_error(). */
#define checka(x) if ((x) < 0 || (x) >= numatypes) atype_error(x);

//! Is unit type valid?
/*! Check to see if unit type is within the range of unit types. */
#define is_unit_type(u) ((u) >= 0 && (u) < numutypes)

//! Is material type valid?
/*! Check to see if material type is within the range of material types. */
#define is_material_type(m) ((m) >= 0 && (m) < nummtypes)

//! Is the terrain type valid?
/*! Check to see if terrain type is within the range of terrain types. */
#define is_terrain_type(t) ((t) >= 0 && (t) < numttypes)

//! Is advance type valid?
/*! Check to see if advance type is within the range of advance types. */
#define is_advance_type(a) ((a) >= 0 && (a) < numatypes)

//! Get wildcard symbol from game entity type.
#define star_from_typ(typ)  \
  ((typ) == UTYP ? \
    "u*" : \
    ((typ) == MTYP ? \
        "m*" : \
        ((typ) == TTYP ? "t*" : "a*")))

//! Get name of type from game entity type and index.
#define name_from_typ(typ, i)  \
  ((typ) == UTYP ? \
    shortest_escaped_name(i) : \
    ((typ) == MTYP ? \
        m_type_name(i) : \
        ((typ) == TTYP ? \
            t_type_name(i) : a_type_name(i))))

//! Is terrain type cell-filling terrain?
#define t_is_cell(t) (t_subtype(t) == cellsubtype)

//! Is terrain type a cell border?
#define t_is_border(t) (t_subtype(t) == bordersubtype)

//! Is terrain type a connection between cells?
#define t_is_connection(t) (t_subtype(t) == connectionsubtype)

//! Is terrain type cell-coating terrain?
#define t_is_coating(t) (t_subtype(t) == coatingsubtype)

//! Is unit type able to act (based on ACP)?
/*!
    Is the Unit ACP greater than 0?

    \todo Rename to 'u_actor' or 'actor_type'?
*/
#define actor(u) (u_acp(u) > 0)

//! Is utype mobile?
/*!
    \todo Consider ACP-less or ACP-indep movement. Materials are then the
          limiting factor and so this macro will no longer suffice.
*/
#define u_mobile(u) (0 < mp_per_turn_max(u))

//! Is unit type able to move (based on speed)?
/*! \todo Rename to 'u_mobile' or 'moble_type'? */
#define mobile(u) (u_speed(u) > 0)

//! Maximum hitpoints for unit type.
#define u_hp(u) (u_hp_max(u))

//! Could unit type survive on terrain type?
#define could_live_on(u,t)  \
   (could_be_on(u, t) && !ut_vanishes_on(u, t) && !ut_wrecks_on(u, t))

//! Could 1st given unit type carry 2nd given unit type?
#define could_carry(u1,u2)  \
  (uu_capacity_x(u1, u2) > 0 || uu_size(u2, u1) <= u_capacity(u1))

//! Could 1st given unit type develop 2nd given unit type?
#define could_develop(u1,u2) (uu_acp_to_develop(u1, u2) > 0)

// Iterator Macros

//! Iterate all unit types.
#define for_all_unit_types(v)      for (v = 0; v < numutypes; ++v)

//! Iterate all material types.
#define for_all_material_types(v)  for (v = 0; v < nummtypes; ++v)

//! Iterate through all terrain types.
#define for_all_terrain_types(v)   for (v = 0; v < numttypes; ++v)

//! Iterate through aux terrain types.
/*!
    Iterate over aux terrain types by using cached array
    linking each type to the next.
*/
#define for_all_aux_terrain_types(t)    \
  for ((t) = first_auxt_type; (t) >= 0; (t) = next_auxt_type[(t)])

//! Iterate through connection terrain types.
/*!
    Iterate over connection types by using a cached array
    linking each type to the next.
*/
#define for_all_connection_types(t)  \
  for ((t) = first_conn_type; (t) >= 0; (t) = next_conn_type[(t)])

//! Iterate through borders.
/*!
    Iterate over border types by using a cached array
    linking each type to the next.
*/
#define for_all_border_types(t)  \
  for ((t) = first_bord_type; (t) >= 0; (t) = next_bord_type[(t)])

//! Iterate through all advance types.
#define for_all_advance_types(v)   for (v = 0; v < numatypes; ++v)

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Data Structures

//! Types of GDL game entities.
typedef enum {
    //! Unit
    UTYP = 0,
    //! Material
    MTYP = 1,
    //! Terrain
    TTYP = 2,
    //! Advance
    ATYP = 3
} Typetype;

//! Property definition.
/*!
    This is the structure representing info about a property
    of a type, such as a unit type's maximum speed.
*/
typedef struct propertydefn {
    //! GDL Name
    char *name;
    //! Function to get integer property for given type.
    int (*intgetter)(int);
    //! Function to get string property for given type.
    char *(*strgetter)(int);
    //! Function to get GDL object property for given type.
    Obj *(*objgetter)(int);
    //! Offset to location in array.
    short offset;
    //! Documentation string.
    char *doc;
    //! Default integer value.
    short dflt;
    //! Default string value.
    char *dfltstr;
    //! Least integer allowed.
    short lo;
    //! Greatest integer allowed.
    short hi;
} PropertyDefn;

//! Unit Type
/*!
    Define unit type structure. Each unit type property may be
    a short, a string, or a lisp object which is repeated
    for each unit type. It is used to return GDL property
    values.
*/
typedef struct utype {

#ifdef  DEF_UPROP_I
#undef  DEF_UPROP_I
#endif
//! Define integer property for unit type.
#define DEF_UPROP_I(name,fname,doc,SLOT,lo,dflt,hi)  \
    short SLOT;

#ifdef  DEF_UPROP_S
#undef  DEF_UPROP_S
#endif
//! Define string property for unit type.
#define DEF_UPROP_S(name,fname,doc,SLOT,dflt)  \
    char *SLOT;

#ifdef  DEF_UPROP_L
#undef  DEF_UPROP_L
#endif
//! Define GDL object property for unit type.
#define DEF_UPROP_L(name,fname,doc,SLOT)  \
    Obj *SLOT;

#include "gdl/utype.def"

} Utype;

//! Material Type
/*!
    Definition of material type properties. Each material type
    property may be a short, a string, or a GDL object.
*/
typedef struct mtype {

#ifdef  DEF_MPROP_I
#undef  DEF_MPROP_I
#endif
//! Define integer property for material type.
#define DEF_MPROP_I(name,fname,doc,SLOT,lo,dflt,hi)  \
    short SLOT;

#ifdef  DEF_MPROP_S
#undef  DEF_MPROP_S
#endif
//! Define string property for material type.
#define DEF_MPROP_S(name,fname,doc,SLOT,dflt)  \
    char *SLOT;

#ifdef  DEF_MPROP_L
#undef  DEF_MPROP_L
#endif
//! Define GDL object property for material type.
#define DEF_MPROP_L(name,fname,doc,SLOT)  \
    Obj *SLOT;

#include "gdl/mtype.def"

} Mtype;

//! Terrain Type
/*!
    Definition of terrain type properties. Each terrain type property
    can be a short, a string, or a lisp object.
*/
typedef struct ttype {

#ifdef  DEF_TPROP_I
#undef  DEF_TPROP_I
#endif
//! Define integer property for terrain type.
#define DEF_TPROP_I(name,fname,doc,SLOT,lo,dflt,hi)  \
    short SLOT;

#ifdef  DEF_TPROP_S
#undef  DEF_TPROP_S
#endif
//! Define string property for terrain type.
#define DEF_TPROP_S(name,fname,doc,SLOT,dflt)  \
    char *SLOT;

#ifdef  DEF_TPROP_L
#undef  DEF_TPROP_L
#endif
//! Define GDL object property for terrain type.
#define DEF_TPROP_L(name,fname,doc,SLOT)  \
    Obj *SLOT;

#include "gdl/ttype.def"

} Ttype;

//! Advance Type
/*!
    Definition of advance type properties. Each advance type property
    may be a short, a string, or a lisp object.
*/
typedef struct atype {

#ifdef  DEF_APROP_I
#undef  DEF_APROP_I
#endif
//! Define integer property for advance type.
#define DEF_APROP_I(name,fname,doc,SLOT,lo,dflt,hi)  \
    short SLOT;

#ifdef  DEF_APROP_S
#undef  DEF_APROP_S
#endif
//! Define string property for advance type.
#define DEF_APROP_S(name,fname,doc,SLOT,dflt)  \
    char *SLOT;

#ifdef  DEF_APROP_L
#undef  DEF_APROP_L
#endif
//! Define GDL object property for advance type.
#define DEF_APROP_L(name,fname,doc,SLOT)  \
    Obj *SLOT;

#include "gdl/atype.def"

} Atype;

//! Terrain subtypes.
enum terrain_subtype {
    // Cell.
    cellsubtype = 0,
    // Border.
    bordersubtype = 1,
    // Connection.
    connectionsubtype = 2,
    // Coating.
    coatingsubtype = 3
};

// Global Variables

//! Array of Unit type property definitions.
extern PropertyDefn utypedefns[];

//! Array of Material type property definitions.
extern PropertyDefn mtypedefns[];

//! Array of Terrain type property definitions.
extern PropertyDefn ttypedefns[];

//! Array of Advance type property definitions.
extern PropertyDefn atypedefns[];

//! Unit types.
extern Utype *utypes;
//! Number of unit types.
extern short numutypes;

//! Material types.
extern Mtype *mtypes;
//! Number of material types.
extern short nummtypes;

//! Terrain types.
extern Ttype *ttypes;
//! Number of terrain types.
extern short numttypes;

//! Advance types.
extern Atype *atypes;
//! Number of advance types.
extern short numatypes;

//! Number of cell terrain types.
extern int numcelltypes;
//! Number of aux terrain types.
extern int numauxtypes;
//! Number of border terrain types.
extern int numbordtypes;
//! Number of connection terrain types.
extern int numconntypes;
//! Number of coating terrain types.
extern int numcoattypes;

//! Head of aux terrain type list.
extern short first_auxt_type;
//! Next aux terrain type in list.
extern short *next_auxt_type;
//! List of all aux terrain types.
extern short *aux_terrain_type_index;

//! Head of border type list.
extern short first_bord_type;
//! Next border type in list.
extern short *next_bord_type;

//! Head of connection type list.
extern short first_conn_type;
//! Next connection type in list.
extern short *next_conn_type;

// Global Variables: Behavior Options

//! Can add unit types?
extern short canaddutype;
//! Can add material types?
extern short canaddmtype;
//! Can add terrain types?
extern short canaddttype;
//! Can add advance types?
extern short canaddatype;

// Global Variables: Scratch

//! Temporary unit type.
extern short tmputype;
//! Temporary material type.
extern short tmpmtype;
//! Temporary terrain type.
extern short tmpttype;
//! Temporary advance type.
extern short tmpatype;

//! Unit type scratch space.
extern int *tmp_u_array;
//! More unit type scratch space.
extern int *tmp_u2_array;
//! Material type scratch space.
extern int *tmp_m_array;
//! Terrain type scratch space.
extern int *tmp_t_array;
//! Advance type scratch space.
extern int *tmp_a_array;

//! Scratch space in the likeness of a TableUU.
extern int **tmp_uu_array;

// Global Variables: Buffers

//! Unit types to apply doctrine to.
extern short *rd_u_arr;

// Notifications

//! Report invalid unit type ID.
extern void utype_error(int u);
//! Report invalid material type ID.
extern void mtype_error(int m);
//! Report invalid terrain type ID.
extern void ttype_error(int t);
//! Report invalid advance type ID.
extern void atype_error(int s);

//! Report type error for given GDL object.
extern void type_error(Obj *x, char *msg);

// Queries

//! Get number entities of a particular GDL game entity type.
extern int numtypes_from_index_type(int x);
//! Get name of a particular GDL game entity type.
extern char *index_type_name(int x);

//! Try to find unit type named by string.
extern int utype_from_name(char *str);
//! Try to find material type named by string.
extern int mtype_from_name(char *str);
//! Try to find terrain type named by string.
extern int ttype_from_name(char *str);
//! Try to find advance type named by string.
extern int atype_from_name(char *str);

//! Retrieve unit type ID from GDL symbol.
extern int utype_from_symbol(Obj *sym);

//! Get shortest distinct name for unit type.
extern char *shortest_unique_name(int u);
//! Get shortest escaped name for unit type.
/*!
    Return the shortest properly escaped name that can be used to identify
    unit type.
*/
extern char *shortest_escaped_name(int u);

//! Could given unit type be on given terrain type?
extern int could_be_on(int u, int t);

// Game Entity Property Accessors

#ifdef  DEF_UPROP_I
#undef  DEF_UPROP_I
#endif
#define DEF_UPROP_I(name,FNAME,doc,slot,lo,dflt,hi)  \
    extern int FNAME(int u);

#ifdef  DEF_UPROP_S
#undef  DEF_UPROP_S
#endif
#define DEF_UPROP_S(name,FNAME,doc,slot,dflt)  \
    extern char *FNAME(int u);

#ifdef  DEF_UPROP_L
#undef  DEF_UPROP_L
#endif
#define DEF_UPROP_L(name,FNAME,doc,slot)  \
    extern Obj *FNAME(int u);

#include "gdl/utype.def"

#ifdef  DEF_MPROP_I
#undef  DEF_MPROP_I
#endif
#define DEF_MPROP_I(name,FNAME,doc,slot,lo,dflt,hi)  \
    extern int FNAME(int m);

#ifdef  DEF_MPROP_S
#undef  DEF_MPROP_S
#endif
#define DEF_MPROP_S(name,FNAME,doc,slot,dflt)  \
    extern char *FNAME(int m);

#ifdef  DEF_MPROP_L
#undef  DEF_MPROP_L
#endif
#define DEF_MPROP_L(name,FNAME,doc,slot)  \
    extern Obj *FNAME(int m);

#include "gdl/mtype.def"

#ifdef  DEF_TPROP_I
#undef  DEF_TPROP_I
#endif
#define DEF_TPROP_I(name,FNAME,doc,slot,lo,dflt,hi)  \
    extern int FNAME(int t);

#ifdef  DEF_TPROP_S
#undef  DEF_TPROP_S
#endif
#define DEF_TPROP_S(name,FNAME,doc,slot,dflt)  \
    extern char *FNAME(int t);

#ifdef  DEF_TPROP_L
#undef  DEF_TPROP_L
#endif
#define DEF_TPROP_L(name,FNAME,doc,slot)  \
    extern Obj *FNAME(int t);

#include "gdl/ttype.def"

#ifdef  DEF_APROP_I
#undef  DEF_APROP_I
#endif
#define DEF_APROP_I(name,FNAME,doc,slot,lo,dflt,hi)  \
    extern int FNAME(int a);

#ifdef  DEF_APROP_S
#undef  DEF_APROP_S
#endif
#define DEF_APROP_S(name,FNAME,doc,slot,dflt)  \
    extern char *FNAME(int a);

#ifdef  DEF_APROP_L
#undef  DEF_APROP_L
#endif
#define DEF_APROP_L(name,FNAME,doc,slot)  \
    extern Obj *FNAME(int a);

#include "gdl/atype.def"

//! Default value for integer uprop.
extern short uprop_i_default(int (*intgetter)(int));
//! Default value for string uprop.
extern char *uprop_s_default(char *(*strgetter)(int));
//! Default value for Lisp uprop.
extern Obj *uprop_l_default(Obj *(*objgetter)(int));

//! Default value for integer mprop.
extern short mprop_i_default(int (*intgetter)(int));
//! Default value for string mprop.
extern char *mprop_s_default(char *(*strgetter)(int));
//! Default value for Lisp mprop.
extern Obj *mprop_l_default(Obj *(*objgetter)(int));

//! Default value for integer tprop.
extern short tprop_i_default(int (*intgetter)(int));
//! Default value for string tprop.
extern char *tprop_s_default(char *(*strgetter)(int));
//! Default value for Lisp tprop.
extern Obj *tprop_l_default(Obj *(*objgetter)(int));

//! Default value for integer aprop.
extern short aprop_i_default(int (*intgetter)(int));
//! Default value for string aprop.
extern char *aprop_s_default(char *(*strgetter)(int));
//! Default value for Lisp aprop.
extern Obj *aprop_l_default(Obj *(*objgetter)(int));

// Game Setup

//! Allocate arrays for GDL game entity types.
extern void init_types(void);

//! Do not allow any more unit types to be added.
/*! Allocate some scratch space for working with unit types. */
extern void disallow_more_unit_types(void);
//! Do not allow any more material types to be added.
/*! Allocate some scratch space for working with material types. */
extern void disallow_more_material_types(void);
//! Do not allow any more terrain types to be added.
/*! Allocate some scratch space for working with terrain types. */
extern void disallow_more_terrain_types(void);
//! Do not allow any more advance types to be added.
/*! Allocate some scratch space for working with advance types. */
extern void disallow_more_advance_types(void);

//! Set all properties of given unit type to their defaults.
extern void default_unit_type(int x);
//! Set all properties of given material type to their defaults.
extern void default_material_type(int x);
//! Set all properties of given terrain type to their defaults.
extern void default_terrain_type(int x);
//! Set all properties of given advance type to their defaults.
extern void default_advance_type(int x);

//! Cache counts of the various terrain subtypes.
extern void count_terrain_subtypes(void);

// Lifecycle Management

//! Return ID of newly created unit type.
/*! Extend unit types array, if necessary. */
extern int create_unit_type(void);
//! Return ID of newly created material type.
/*! Extend material types array, if necessary. */
extern int create_material_type(void);
//! Return ID of newly created terrain type.
/*! Extend terrain types array, if necessary. */
extern int create_terrain_type(void);
//! Return ID of newly created advance type.
/*! Extend advance types array, if necessary. */
extern int create_advance_type(void);

// GDL I/O

//! Read unit type from GDL form.
extern void interp_utype(Obj *form);
//! Read material type from GDL form.
extern void interp_mtype(Obj *form);
//! Read terrain type from GDL form.
extern void interp_ttype(Obj *form);
//! Read advance type from GDL form.
extern void interp_atype(Obj *form);

//! Read list of unit types from GDL list.
extern void interp_utype_list(short *arr, Obj *lis);

//! Read array of unit type and value pairs from GDL list.
/*! Note that array is not initialized; allows for multiple calls. */
extern void interp_utype_value_list(short *arr, Obj *lis);
//! Read array of material type and value pairs from GDL list.
extern void interp_mtype_value_list(short *arr, Obj *lis);
//! Similar to 'interp_mtype_value_list', but with wider ints.
extern void interp_treasury_list(long *arr, Obj *lis);
//! Read array of advance type and value pairs from GDL list.
extern void interp_atype_value_list(short *arr, Obj *lis);

//! Read material treasuries from GDL list.
extern void interp_treasury_list(long *arr, Obj *lis);

//! Read type properties from GDL form.
extern void add_properties(Obj *form);

//! Serialize list of unit type-value pairs to GDL.
extern void write_utype_value_list(
    char *name, short *arr, int dflt, int addnewline);
//! Serialize list of material type-value pairs to GDL.
extern void write_mtype_value_list(
    char *name, short *arr, int dflt, int addnewline);
//! Serialize treasury list to GDL.
/*! Same as material type-value list but with wider values. */
extern void write_treasury_list(
    char *name, long *arr, int dflt, int addnewline);
//! Serialize list of advance type-value pairs to GDL.
extern void write_atype_value_list(
    char *name, short *arr, int dflt, int addnewline);

//! Serialize list of unit type-string pairs to GDL.
extern void write_utype_string_list(
    char *name, char **arr, char *dflt, int addnewline);

//! Serialize game entity type-name list to GDL.
extern void write_type_name_list(int typ, int *flags, int dim);
//! Serialize game entity types to GDL.
extern void write_types(void);

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

#endif // ifndef XCONQ_GDL_TYPES_H
