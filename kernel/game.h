/* Interface between game parameters and the rest of Xconq.
   Copyright (C) 1992-1996, 1998-2000 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/*! \file kernel/game.h
 * \brief Interface between game parameters and the rest of Xconq.
 *
 * This file defines the structures that are filled in with type info,
 * one for each type, plus the declarations for all functions and variables.
 */

/* Numbers guaranteed to be invalid types in each category.  Should be
 * careful that these don't overflow anything.
 */

/*! \brief Non unit.
 *
 * Guaranteed not to be a Unit type. */
#define NONUTYPE (-1)
/*! \brief Non Material.
 *
 * Guaranteed not to be a Material type. */
#define NONMTYPE (-1)

/*! \brief Non terrain.
 *
 * Guaranteed not to be a Terrain type. Used in interfaces for unseen terrain. */
#define NONTTYPE (-1)

/*! \brief Bogus window background terrain.
 *
 * Used in interfaces for bogus terrain outside the map when redrawing the 
window background. */
#define BACKTTYPE (-2)					

/*! \brief non advance.
 *
 * Guaranteed not to be an Advance type. */
#define NONATYPE (-1)

/*! \brief Advance complete.
 *
 * Used in advances code, must not overlap with NONATYPE. */
#define DONE (-2)

/*! \brief No advance.
 *
 * Used in advances code, must not overlap with NONATYPE, and 
 * presumably with DONE, as well. */
#define NOADVANCE (-3)

/*! \brief Type of types.
 *
 * Indices for each category of types. */
typedef enum {
  UTYP = 0, /*!< Unit */
  MTYP = 1, /*!< Material */
  TTYP = 2, /*!< Terrain */
  ATYP = 3  /*!< Advance */
} Typetype;

/*! \brief Terrain subtypes.
 *
 * The four roles for Terrain. */
enum terrain_subtype {
    cellsubtype = 0,       /*!< Cell */
    bordersubtype = 1,     /*!< Border */
    connectionsubtype = 2, /*!< Connection */
    coatingsubtype = 3     /*!< Coating */
};

/* Ultimate limits on values in properties. */

/*! \brief Low property value limit. */
#define PROPLO -32768
/*! \brief High property value limit. */
#define PROPHI 32767

/* Ultimate limits on values in tables. */

/*! \brief Low value in table. */
#define TABLO -32768
/*! \brief High value in table */
#define TABHI 32767

//! Integer table.
#define TABINT	    0
//! Type 1 dice table.
#define TABDICE1    1
//! Type 2 dice table.
#define TABDICE2    2

/* Ultimate limits on values of globals. */

/*! \brief Low value for globals. */
#define VARLO -2000000000
/*! \brief High value for globals. */
#define VARHI 2000000000

#include "lisp.h"

/*! \brief Property definition.
 *
 * This is the structure representing info about a property
 * of a type, such as a unit type's maximum speed.
 * */
typedef struct propertydefn {
    const char *name;                 		/*!< name */
    int (*intgetter)(int);      		/*!< function to get n'th integer property */
    const char *(*strgetter)(int);    	/*!< function to get n'th string property */
    Obj *(*objgetter)(int);		/*!< function to get n'th \Object property */
    short offset;               		/*!< offset (to what ?) */
    const char *doc;                  		/*!< documentation */
    short dflt;                 		/*!< default ? */
    const char *dfltstr;              		/*!< default string ? */
    short lo;                   			/*!< low limit */
    short hi;                   		/*!< high limit */
} PropertyDefn;

/*! \brief Table definition.
 *
 * This is the structure with info about a table.
 */
typedef struct tabledefn {
    const char *name;                 		/*!< name of the table */
    int (*getter)(int, int);    	/*!< accessor function */
    const char *doc;                  		/*!< documentation string */
    short **table;              		/*!< pointer to table itself */
    short dflt;                 		/*!< default value of entries */
    short lo;                   			/*!< lower bound of table values */
    short hi;                   		/*!< upper bound of table values */
    char index1;                		/*!< type of row indices */
    char index2;                		/*!< type of column indices */
    char valtype;               		/*!< type of data in table */
} TableDefn;

/*! \brief Global Variables definition.
 *
 * This is the structure with that contains global access definitions
 * and variables.
 */
typedef struct vardefn {
    const char *name;          		/*!< name of the global */
    int   (*intgetter)(void);   	/*!< accessor if integer type */
    const char *(*strgetter)(void);   	/*!< accessor if string type */
    Obj  *(*objgetter)(void);   	/*!< accessor if \Object type */
    void (*intsetter)(int);     	/*!< setter if integer type */
    void (*strsetter)(const char *);  	/*!< setter if string type */
    void (*objsetter)(Obj *);   	/*!< setter if object type */
    const char *doc;           		/*!< documentation string */
    int dflt;                   			/*!< default value if integer type */
    const char *dfltstr;              		/*!< default value if string type */
    Obj *(*dfltfn)(void);       		/*<! function to return default value if object type */
    int lo;                     			/*!< lower bound of integer value. */
    int hi;                     			/*!< upper bound of integer value */
} VarDefn;

/*! \brief Number of Unit types. */
extern short numutypes;
/*! \brief Number of Material types */
extern short nummtypes;
/*! \brief Number of Terrain types */
extern short numttypes;
/*! \brief Number of Advance types. */
extern short numatypes;

#undef  DEF_UPROP_I
#define DEF_UPROP_I(name,fname,doc,SLOT,lo,dflt,hi)  \
    short SLOT; /*!< doc */
#undef  DEF_UPROP_S
#define DEF_UPROP_S(name,fname,doc,SLOT,dflt)  \
    const char *SLOT; /*!< doc */
#undef  DEF_UPROP_L
#define DEF_UPROP_L(name,fname,doc,SLOT)  \
    Obj *SLOT;  /*!< doc */

/*! \brief Unit Type Property Value.
 *
 * Define Unit type structure.  Each unit type property may be
 * a short, a string, or a lisp object which is repeated
 * for each unit type. It is used to return GDL property
 * values.
 *
 */
typedef struct utype {

#include "utype.def"

} Utype;

#undef  DEF_MPROP_I
#define DEF_MPROP_I(name,fname,doc,SLOT,lo,dflt,hi)  \
    short SLOT;
#undef  DEF_MPROP_S
#define DEF_MPROP_S(name,fname,doc,SLOT,dflt)  \
    const char *SLOT;
#undef  DEF_MPROP_L
#define DEF_MPROP_L(name,fname,doc,SLOT)  \
    Obj *SLOT;

/*! \brief Material type properties.
 *
 * Definition of Material type properties. Each material type
 * property may be a short, a string, or a lisp object.  */
typedef struct mtype {

#include "mtype.def"

} Mtype;


#undef  DEF_TPROP_I
#define DEF_TPROP_I(name,fname,doc,SLOT,lo,dflt,hi)  \
    short SLOT;
#undef  DEF_TPROP_S
#define DEF_TPROP_S(name,fname,doc,SLOT,dflt)  \
    const char *SLOT;
#undef  DEF_TPROP_L
#define DEF_TPROP_L(name,fname,doc,SLOT)  \
    Obj *SLOT;

/*! \brief Terrain type properties.
 *
 * Definition of Terrain type properties. Each terrain type property
 * can be a short, a string, or a lisp object. */
typedef struct ttype {

#include "ttype.def"

} Ttype;

#undef  DEF_APROP_I
#define DEF_APROP_I(name,fname,doc,SLOT,lo,dflt,hi)  \
    short SLOT;
#undef  DEF_APROP_S
#define DEF_APROP_S(name,fname,doc,SLOT,dflt)  \
    const char *SLOT;
#undef  DEF_APROP_L
#define DEF_APROP_L(name,fname,doc,SLOT)  \
    Obj *SLOT;

/*! \brief Advance type properties.
 *
 * Definition of Advance type properties. Each Advance type property
 * may be a short, a string, or a lisp object. */
typedef struct atype {

#include "atype.def"

} Atype;

#undef  DEF_VAR_I
#define DEF_VAR_I(name,fname,setfname,doc,VAR,lo,dflt,hi)  \
    int VAR;
#undef  DEF_VAR_S
#define DEF_VAR_S(name,fname,setfname,doc,VAR,dflt)  \
    const char *VAR;
#undef  DEF_VAR_L
#define DEF_VAR_L(name,fname,setfname,doc,VAR,dflt)  \
    Obj *VAR;

/*! \brief Global data structure.
 *
 * The global data can be an int, a string, or a lisp object. */
typedef struct a_globals {

#include "gvar.def"

} Globals;

/* Declarations of the functions accessing and setting type properties. */

#undef  DEF_UPROP_I
#define DEF_UPROP_I(name,FNAME,doc,slot,lo,dflt,hi)  int FNAME(int u);
#undef  DEF_UPROP_S
#define DEF_UPROP_S(name,FNAME,doc,slot,dflt)  const char *FNAME(int u);
#undef  DEF_UPROP_L
#define DEF_UPROP_L(name,FNAME,doc,slot)  Obj *FNAME(int u);

#include "utype.def"

#undef  DEF_MPROP_I
#define DEF_MPROP_I(name,FNAME,doc,slot,lo,dflt,hi)  int FNAME(int m);
#undef  DEF_MPROP_S
#define DEF_MPROP_S(name,FNAME,doc,slot,dflt)  const char *FNAME(int m);
#undef  DEF_MPROP_L
#define DEF_MPROP_L(name,FNAME,doc,slot)  Obj *FNAME(int m);

#include "mtype.def"

#undef  DEF_TPROP_I
#define DEF_TPROP_I(name,FNAME,doc,slot,lo,dflt,hi)  int FNAME(int t);
#undef  DEF_TPROP_S
#define DEF_TPROP_S(name,FNAME,doc,slot,dflt)  const char *FNAME(int t);
#undef  DEF_TPROP_L
#define DEF_TPROP_L(name,FNAME,doc,slot)  Obj *FNAME(int t);

#include "ttype.def"

#undef  DEF_APROP_I
#define DEF_APROP_I(name,FNAME,doc,slot,lo,dflt,hi)  int FNAME(int a);
#undef  DEF_APROP_S
#define DEF_APROP_S(name,FNAME,doc,slot,dflt)  const char *FNAME(int a);
#undef  DEF_APROP_L
#define DEF_APROP_L(name,FNAME,doc,slot)  Obj *FNAME(int a);

#include "atype.def"

#undef  DEF_VAR_I
#define DEF_VAR_I(str,FNAME,SETFNAME,doc,var,lo,dflt,hi)  \
  int FNAME(void); 
#undef  DEF_VAR_S
#define DEF_VAR_S(str,FNAME,SETFNAME,doc,var,dflt)  \
  const char *FNAME(void);
#undef  DEF_VAR_L
#define DEF_VAR_L(str,FNAME,SETFNAME,doc,var,dflt)  \
  Obj *FNAME(void);

#include "gvar.def"

#undef  DEF_VAR_I
#define DEF_VAR_I(str,FNAME,SETFNAME,doc,var,lo,dflt,hi)  \
  void SETFNAME(int val);
#undef  DEF_VAR_S
#define DEF_VAR_S(str,FNAME,SETFNAME,doc,var,dflt)  \
  void SETFNAME(const char *val);
#undef  DEF_VAR_L
#define DEF_VAR_L(str,FNAME,SETFNAME,doc,var,dflt)  \
  void SETFNAME(Obj *val);

#include "gvar.def"

/* Declarations of table accessor functions and the globals
   for constant and filled-in tables. */

#undef  DEF_UU_TABLE
#define DEF_UU_TABLE(name,FNAME,doc,TABLE,CNST,lo,dflt,hi,valtype)  \
  int FNAME(int u1, int u2);

#undef  DEF_UM_TABLE
#define DEF_UM_TABLE(name,FNAME,doc,TABLE,CNST,lo,dflt,hi,valtype)  \
  int FNAME(int u, int m);

#undef  DEF_UT_TABLE
#define DEF_UT_TABLE(name,FNAME,doc,TABLE,CNST,lo,dflt,hi,valtype)  \
  int FNAME(int u, int t);

#undef  DEF_TM_TABLE
#define DEF_TM_TABLE(name,FNAME,doc,TABLE,CNST,lo,dflt,hi,valtype)  \
  int FNAME(int t, int m);

#undef  DEF_TT_TABLE
#define DEF_TT_TABLE(name,FNAME,doc,TABLE,CNST,lo,dflt,hi,valtype)  \
  int FNAME(int t1, int t2);

#undef  DEF_MM_TABLE
#define DEF_MM_TABLE(name,FNAME,doc,TABLE,CNST,lo,dflt,hi,valtype)  \
  int FNAME(int m1, int m2);

#undef  DEF_UA_TABLE
#define DEF_UA_TABLE(name,FNAME,doc,TABLE,CNST,lo,dflt,hi,valtype)  \
  int FNAME(int u, int a);  

#undef  DEF_AM_TABLE
#define DEF_AM_TABLE(name,FNAME,doc,TABLE,CNST,lo,dflt,hi,valtype)  \
  int FNAME(int a, int m);  

#undef  DEF_AA_TABLE
#define DEF_AA_TABLE(name,FNAME,doc,TABLE,CNST,lo,dflt,hi,valtype)  \
  int FNAME(int a1, int a2);  

#include "table.def"

#undef  DEF_UU_TABLE
#define DEF_UU_TABLE(name,FNAME,doc,TABLE,CNST,lo,dflt,hi,valtype)  \
  extern short *TABLE, CNST;

#undef  DEF_UM_TABLE
#define DEF_UM_TABLE(name,FNAME,doc,TABLE,CNST,lo,dflt,hi,valtype)  \
  extern short *TABLE, CNST;

#undef  DEF_UT_TABLE
#define DEF_UT_TABLE(name,FNAME,doc,TABLE,CNST,lo,dflt,hi,valtype)  \
  extern short *TABLE, CNST;

#undef  DEF_TM_TABLE
#define DEF_TM_TABLE(name,FNAME,doc,TABLE,CNST,lo,dflt,hi,valtype)  \
  extern short *TABLE, CNST;

#undef  DEF_TT_TABLE
#define DEF_TT_TABLE(name,FNAME,doc,TABLE,CNST,lo,dflt,hi,valtype)  \
  extern short *TABLE, CNST;

#undef  DEF_MM_TABLE
#define DEF_MM_TABLE(name,FNAME,doc,TABLE,CNST,lo,dflt,hi,valtype)  \
  extern short *TABLE, CNST;

#undef  DEF_UA_TABLE
#define DEF_UA_TABLE(name,FNAME,doc,TABLE,CNST,lo,dflt,hi,valtype)  \
  extern short *TABLE, CNST;

#undef  DEF_AM_TABLE
#define DEF_AM_TABLE(name,FNAME,doc,TABLE,CNST,lo,dflt,hi,valtype)  \
  extern short *TABLE, CNST;

#undef  DEF_AA_TABLE
#define DEF_AA_TABLE(name,FNAME,doc,TABLE,CNST,lo,dflt,hi,valtype)  \
  extern short *TABLE, CNST;

#include "table.def"

/* Declarations of the globals description structures. */

/*! \brief \ref a_globals "Global" data variable. */
extern Globals globals;

/*! \brief Unit type properties. */
extern Utype *utypes;

/*! \brief Material type properties. */
extern Mtype *mtypes;

/*! \brief Terrain type properties. */
extern Ttype *ttypes;

/*! \brief Advance type properties. */
extern Atype *atypes;

/*! \brief Array of Unit type property definitions. */
extern PropertyDefn utypedefns[];

/*! \brief Array of Material type property definitions. */
extern PropertyDefn mtypedefns[];

/*! \brief Array of Terrain type property definitions. */
extern PropertyDefn ttypedefns[];

/*! \brief Array of Advance type property definitions. */
extern PropertyDefn atypedefns[];

/*! \brief Array of table definitions. */
extern TableDefn tabledefns[];

/*! \brief Array of variable definitions. */
extern VarDefn vardefns[];

/* Macros for iterating over types. */

/*! \brief Iterate all Unit types.
 *
 * For header to iterate through all Unit types defined.
 * \param v is the Unit type iteration variable.
 */
#define for_all_unit_types(v)      for (v = 0; v < numutypes; ++v)

/*! \brief Iterate all Material types.
 *
 * For header to iterate through all Material types.
 * \param v is the Material type iteration variable.
 */
#define for_all_material_types(v)  for (v = 0; v < nummtypes; ++v)

/*! \brief Iterate through all Terrain types.
 *
 * For header to iterate through all Terrain types.
 * \param v is the Terran iteration variable.
 */
#define for_all_terrain_types(v)   for (v = 0; v < numttypes; ++v)

/*! \brief Iterate through all Advance types.
 *
 * For header to iterate through all Advance types.
 * \param v is the Advance iteration variable.
 */
#define for_all_advance_types(v)   for (v = 0; v < numatypes; ++v)

/*! \brief Iterate through aux terrain types.
 *
 * Iterate over aux terrain types by using cached array
 * linking each type to the next.
 * \param t is the Terrain iteration variable.
 */
#define for_all_aux_terrain_types(t)    \
  for ((t) = first_auxt_type; (t) >= 0; (t) = next_auxt_type[(t)])

/*! \brief Iterate through borders.
 *
 * Iterate over border types by using a cached array
 * linking each type to the next.
 * \param t is the Terrain iteration variable.
 */
#define for_all_border_types(t)  \
  for ((t) = first_bord_type; (t) >= 0; (t) = next_bord_type[(t)])

/*! \brief Iterate through connections.
 *
 * Iterate over connection types by using a cached array
 * linking each type to the next.
 * \param t is the Terrain iteration variable.
 */
#define for_all_connection_types(t)  \
  for ((t) = first_conn_type; (t) >= 0; (t) = next_conn_type[(t)])

/* Macros encapsulating things about units. */

/*! \brief Check Unit type range.
 *
 * If the Unit is out of range, call utype_error().
 * \see numutypes, utype_error.
 * \param x is the Unit type.
 */
#define checku(x) if ((x) < 0 || (x) >= numutypes) utype_error(x);

/*! \brief Check Material type range.
 *
 * If the Material type is out of range, call mtype_error().
 * \see nummtypes, mtype_error.
 * \param x is the Terrain type.
 */
#define checkm(x) if ((x) < 0 || (x) >= nummtypes) mtype_error(x);

/*! \brief Check Terrain type range.
 *
 * If the Terrain type is out of range, call ttype_error().
 * \see numttypes, ttype_error.
 * \param x is the Terrain type.
 */
#define checkt(x) if ((x) < 0 || (x) >= numttypes) ttype_error(x);

/*! \brief Check Advance type range.
 *
 * If the Advance type is out of range, call atype_error().
 * \see numatypes, atype_error.
 * \param x is the Advance type.
 */
#define checka(x) if ((x) < 0 || (x) >= numatypes) atype_error(x);

/* Fix eventually. */

/* (should say u_... or ..._type ?) */

/*! \brief Is actor?
 *
 * Is the Unit ACP greater than 0?
 * \see u_acp.
 * \todo reanme to u_actor or actor_type?
 * \param u is a Unit type.
 * \return 
 *    - FALSE if the ACP of the \Unit is less than 1,
 *    - TRUE otherwise.
 */
#define actor(u) (u_acp(u) > 0)

/*! \brief Is mobile?
 *
 * Is the Unit capable of moving?
 * \see u_speed.
 * \todo rename to u_mobile or moble_type?
 * \param u is a Unit type.
 * \return 
 *    - TRUE if the Unit type's speed is greater than 0;
 *    - FALSE otherwise.
 */
#define mobile(u) (u_speed(u) > 0)

//! Is utype mobile?
/*!
    \todo Consider ACP-less or ACP-indep movement. Materials are then the 
	    limiting factor and so this macro will no longer suffice.
*/
#define u_mobile(u) (0 < mp_per_turn_max(u))

/*! \brief Unit hit points.
 *
 * Get the Unit type's maximum hit points.
 * \see u_hp_max.
 * \param u is unit type.
 * \return the maximum hit points for the unit.
 */
#define u_hp(u) (u_hp_max(u))

//! Could u be on t?
extern int could_be_on(int u, int t);

/*! \brief Can Unit survive on Terrain?
 *
 * Check to see if a Unit type can live on a Terrain type.
 * \see cold_be_on, ut_vanishes_on, ut_wrecks_on.
 * \param u is the Unit type.
 * \param t is the Terrain type.
 * \return
 *   - TRUE if
 *     - the Unit type can be on the Terrain type, and
 *     - the Unit type won't vanish on the Terrain type, and
 *     - the Unit type won't wreck on the Terrain type;
 *   - FALsE otherwise.
 */
#define could_live_on(u,t)  \
   (could_be_on(u, t) && !ut_vanishes_on(u, t) && !ut_wrecks_on(u, t))

/*! \brief Unit could carry Unit?
 *
 * Check to see if a Unit type may carry another Unit type.
 * \see uu_capacity_x, uu_size, u_capacity.
 * \param u1 is the transport Unit type.
 * \param u2 is the passenger Unit type.
 * \returns 
 *   - TRUE if
 *     - the transport can explictly carry the passenger, or
 *     - the size of the passenger on the transport is less than
 *       the transport's capacity;
 *   - FALSE otherwise.
 */
#define could_carry(u1,u2)  \
  (uu_capacity_x(u1, u2) > 0 || uu_size(u2, u1) <= u_capacity(u1))

/*! \brief Unit could develop Unit?
 *
 * Check to see a Unit type can develop a Unit type.
 * \see uu_acp_to_develop.
 * \param u1 is the developing Unit type.
 * \param u2 is the developed Unit type.
 * \return 
 *   - TRUE if possible;
 *   - FALSE otherwise.
 */
#define could_develop(u1,u2) (uu_acp_to_develop(u1, u2) > 0)

/* These need actual units rather than types. */

/*! \brief Impassable?
 *
 * Can \ref a_unit "unit" pass the Terrain at the cell?
 * \see could_be_on, terrain_at.
 * \param u is a pointer to a Unit.
 * \param x is the x-coordinate of the cell.
 * \param y is the y-coordinate of the cell.
 * \return
 *   - TRUE if the Unit type could not be on the Terrain type at (x,y);
 *   - FALSE otherwise.
 */
#define impassable(u, x, y) (!could_be_on((u)->type, terrain_at((x), (y))))

/*! \brief Is the Unit a base?
 *
 * Check to see the the type of the Unit is a base.
 * \see u_is_base.
 * \param u is a pointer to a Unit.
 * \return
 *   - TRUE if the type of the \Unit is a base type;
 *   - FALSE otherwise.
 */
#define isbase(u) (u_is_base((u)->type))

/*! \brief Is the Unit a base builder?
 *
 * Check to see the the type of the Unit is a base builder.
 * \see u_is_base_builder.
 * \param u is a pointer to a Unit.
 * \return
 *   - TRUE if the type of the \Unit is a base builder type;
 *   - FALSE otherwise.
 */
#define base_builder(u) (u_is_base_builder((u)->type))

/*! \brief Is the Unit a transport?
 *
 * Check to see the the type of the Unit is a transport.
 * \see u_is_transport.
 * \param u is a pointer to a Unit.
 * \return
 *   - TRUE if the type of the \Unit is a transport type;
 *   - FALSE otherwise.
 */
#define istransport(u) (u_is_transport((u)->type))

/*! \brief Is the Terrain a cell?
 *
 * Check to see the Terrain subtype is a cell.
 * \see t_subtype, cellsubtype.
 * \param t is a Terrain type.
 * \return
 *   - TRUE if the Terrain's subtype is cell.
 *   - FALSE otherwise.
 */
#define t_is_cell(t) (t_subtype(t) == cellsubtype)

/*! \brief Is the Terrain a border?
 *
 * Check to see the Terrain subtype is a border.
 * \see t_subtype, bordersubtype.
 * \param t is a Terrain type.
 * \return
 *   - TRUE if the Terrain's subtype is border.
 *   - FALSE otherwise.
 */
#define t_is_border(t) (t_subtype(t) == bordersubtype)

/*! \brief Is the Terrain a connection?
 *
 * Check to see the Terrain subtype is a connection.
 * \see t_subtype, connectionsubtype.
 * \param t is a Terrain type.
 * \return
 *   - TRUE if the Terrain's subtype is connection.
 *   - FALSE otherwise.
 */
#define t_is_connection(t) (t_subtype(t) == connectionsubtype)

/*! \brief Is the Terrain a coating?
 *
 * Check to see the Terrain subtype is a coating.
 * \see t_subtype, coatingsubtype.
 * \param t is a Terrain type.
 * \return
 *   - TRUE if the Terrain's subtype is coating.
 *   - FALSE otherwise.
 */
#define t_is_coating(t) (t_subtype(t) == coatingsubtype)

/*! \brief Is the Unit type valid?
 *
 * Check to see if the Unit type is within the range of
 * Unit types.
 * \see numutypes.
 * \param u is the Unit type.
 * \return
 *   - TRUE if the Unit type is withing range;
 *   - FALSE, otherwise.
 */
#define is_unit_type(u) ((u) >= 0 && (u) < numutypes)

/*! \brief Is the Material type valid?
 *
 * Check to see if the Material type is within the range of
 * Material types.
 * \see nummtypes.
 * \param m is the Material type.
 * \return
 *   - TRUE if the Material type is within range;
 *   - FALSE, otherwise.
 */
#define is_material_type(m) ((m) >= 0 && (m) < nummtypes)

/*! \brief Is the Terrain type valid?
 *
 * Check to see if the Terrain type is within the range of
 * Terrain types.
 * \see numttypes.
 * \param t is the Terrain type.
 * \return
 *   - TRUE if the Terrain type is within range;
 *   - FALSE, otherwise.
 */
#define is_terrain_type(t) ((t) >= 0 && (t) < numttypes)

/*! \brief Is the Advance type valid?
 *
 * Check to see if the Advance type is within the range of
 * Advance types.
 * \see numatypes.
 * \param a is the Advance type.
 * \return
 *   - TRUE if the Advance type is within range;
 *   - FALSE, otherwise.
 */
#define is_advance_type(a) ((a) >= 0 && (a) < numatypes)

/*! \brief Can add Unit type flag. */
extern short canaddutype;
/*! \brief Can add Material type flag. */
extern short canaddmtype;
/*! \brief Can add Terrain type flag. */
extern short canaddttype;
/*! \brief Can add Advance type flag. */
extern short canaddatype;

/*! \brief Temporary Unit type. */
extern short tmputype;
/*! \brief Temporary Material type. */
extern short tmpmtype;
/*! \brief Temporary Terrain type. */
extern short tmpttype;
/*! \brief Temporary Advance type. */
extern short tmpatype;

/*! \brief Number of cell types. */
extern int numcelltypes;
/*! \brief Number of border types. */
extern int numbordtypes;
/*! \brief Number of connection types. */
extern int numconntypes;
/*! \brief Number of coating types. */
extern int numcoattypes;

/*! \brief Head of aux terrain type list. */
extern short first_auxt_type;
/*! \brief Next aux terrain type in list. */
extern short *next_auxt_type;
/*! \brief Head of border type list. */
extern short first_bord_type;
/*! \brief Next border type in list. */
extern short *next_bord_type;
/*! \brief Head of connection type list. */
extern short first_conn_type;
/*! \brief Next connection type in list. */
extern short *next_conn_type;

/*! \brief Tempoary Unit type array. */
extern int *tmp_u_array;
//! More utype scratch space.
extern int *tmp_u2_array;
/*! \brief Tempoary Terrain type array. */
extern int *tmp_t_array;
/*! \brief Tempoary Material type array. */
extern int *tmp_m_array;
/*! \brief Tempoary Advance type array. */
extern int *tmp_a_array;

//! Scratch space in the likeness of a TableUU.
extern int **tmp_uu_array;

extern void utype_error(int u);
extern void mtype_error(int m);
extern void ttype_error(int t);
extern void atype_error(int s);

extern void init_types(void);
extern void init_globals(void);
extern int create_unit_type(void);
extern int create_material_type(void);
extern int create_terrain_type(void);
extern int create_advance_type(void);
extern void default_unit_type(int x);
extern void default_material_type(int x);
extern void default_terrain_type(int x);
extern void default_advance_type(int x);

extern void allocate_table(int tbl, int reset);
extern int numtypes_from_index_type(int x);
extern char *index_type_name(int x);

extern void disallow_more_unit_types(void);
extern void disallow_more_terrain_types(void);
extern void disallow_more_material_types(void);
extern void disallow_more_advance_types(void);
extern void count_terrain_subtypes(void);

/* Accessors for table, property, and global defaults. */

//! Default value for table.
extern int table_default(int (*getter)(int, int));
//! Default value for integer uprop.
extern short uprop_i_default(int (*intgetter)(int));
//! Default value for string uprop.
extern const char *uprop_s_default(const char *(*strgetter)(int));
//! Default value for Lisp uprop.
extern Obj *uprop_l_default(Obj *(*objgetter)(int));
//! Default value for integer tprop.
extern short tprop_i_default(int (*intgetter)(int));
//! Default value for string tprop.
extern const char *tprop_s_default(const char *(*strgetter)(int));
//! Default value for Lisp tprop.
extern Obj *tprop_l_default(Obj *(*objgetter)(int));
//! Default value for integer mprop.
extern short mprop_i_default(int (*intgetter)(int));
//! Default value for string mprop.
extern const char *mprop_s_default(const char *(*strgetter)(int));
//! Default value for Lisp mprop.
extern Obj *mprop_l_default(Obj *(*objgetter)(int));
//! Default value for integer aprop.
extern short aprop_i_default(int (*intgetter)(int));
//! Default value for string aprop.
extern const char *aprop_s_default(const char *(*strgetter)(int));
//! Default value for Lisp aprop.
extern Obj *aprop_l_default(Obj *(*objgetter)(int));
//! Default value for integer gvar.
extern int gvar_i_default(int (*intgetter)(void));
//! Default value for string gvar.
extern char *gvar_s_default(char *(*strgetter)(void));
//! Default value for Lisp gvar.
extern Obj *gvar_l_default(Obj *(*objgetter)(void));

extern Obj * g_synth_methods_default(void);
