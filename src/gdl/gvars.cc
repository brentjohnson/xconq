// xConq
// GDL global variables.

// $Id: gvars.cc,v 1.5 2006/06/02 16:57:43 eric_mcdonald Exp $

/*//////////////////////////// COPYRIGHT NOTICE ////////////////////////////////

  Copyright (C) 1987-1989   Stanley T. Shebs
  Copyright (C) 1991-2000   Stanley T. Shebs
  Copyright (C) 2004-2006   Eric A. McDonald

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

#include "gdl/kernel.h"
#include "gdl/types.h"
#include "gdl/gvars.h"

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Global Variables

Globals globals;

/*! \showinitializer */
VarDefn vardefns[] = {

#ifdef  DEF_VAR_I
#undef  DEF_VAR_I
#endif
#define DEF_VAR_I(NAME,FNAME,SETFNAME,DOC,var,LO,DFLT,HI)  \
    { NAME, FNAME, NULL, NULL, SETFNAME, NULL, NULL, DOC, DFLT, NULL, NULL, LO, HI },
#ifdef  DEF_VAR_S
#undef  DEF_VAR_S
#endif
#define DEF_VAR_S(NAME,FNAME,SETFNAME,DOC,var,DFLT)  \
    { NAME, NULL, FNAME, NULL, NULL, SETFNAME, NULL, DOC,    0, DFLT, NULL,  0,  0 },
#ifdef  DEF_VAR_L
#undef  DEF_VAR_L
#endif
#define DEF_VAR_L(NAME,FNAME,SETFNAME,DOC,var,DFLT)  \
    { NAME, NULL, NULL, FNAME, NULL, NULL, SETFNAME, DOC,    0, NULL, DFLT,  0,  0 },

#include "gdl/gvar.def"

    { NULL }
};

// GDL Globals Accessors

#ifdef  DEF_VAR_I
#undef  DEF_VAR_I
#endif
#define DEF_VAR_I(str,FNAME,SETFNAME,doc,VAR,lo,dflt,hi)  \
  int FNAME(void) { return globals.VAR; }  \
  void SETFNAME(int v) { globals.VAR = v; }

#ifdef  DEF_VAR_S
#undef  DEF_VAR_S
#endif
#define DEF_VAR_S(str,FNAME,SETFNAME,doc,VAR,dflt)  \
  char *FNAME(void) { return globals.VAR; }  \
  void SETFNAME(char *v) { globals.VAR = v; }

#ifdef  DEF_VAR_L
#undef  DEF_VAR_L
#endif
#define DEF_VAR_L(str,FNAME,SETFNAME,doc,VAR,DFLT)  \
  Obj *FNAME(void) { return globals.VAR; }  \
  void SETFNAME(Obj *v) { globals.VAR = v; }

#include "gdl/gvar.def"

// Queries

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

// Game Setup

void
undefine_variable(Obj *form)
{
    Obj *var = cadr(form);

    if (!symbolp(var)) {
	read_warning("Can't undefine a non-symbol!");
	return;
    }
    makunbound(var);
}

// Lifecycle Management

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

#include "gdl/gvar.def"

}

// GDL I/O

void
interp_variable(Obj *form, int isnew)
{
    int i, numval;
    Obj *var = cadr(form);
    Obj *val = eval(caddr(form));
    char *name;

    SYNTAX(form, symbolp(var), "variable is not a symbol");
    name = c_string(var);

    if (isnew) {
	if (boundp(var))
	  read_warning("Symbol `%s' has been bound already, overwriting", name);
	setq(var, val);
    } else {
	/* Look through the set of defined globals. */
	for (i = 0; vardefns[i].name != 0; ++i) {
	    if (strcmp(name, vardefns[i].name) == 0) {
		if (vardefns[i].intgetter != NULL) {
		    TYPECHECK(numberishp, val, "is not a number or type");
		    numval = c_number(val);
		    if (!between(vardefns[i].lo, numval, vardefns[i].hi)) {
			read_warning("global %s value %d not between %d and %d",
				     vardefns[i].name, numval,
				     vardefns[i].lo, vardefns[i].hi);
			/* Let it pass through, at least for now. */
		    }
		    (*(vardefns[i].intsetter))(numval);
		} else if (vardefns[i].strgetter != NULL) {
		    TYPECHECK(stringp, val, "is not a string");
		    (*(vardefns[i].strsetter))(c_string(val));
		} else if (vardefns[i].objgetter != NULL) {
		    (*(vardefns[i].objsetter))(val);
		} else {
#ifdef __cplusplus
		    throw "snafu";
#else
		    abort();
#endif
		}
		return;
	    }
	}
	/* Try as a random symbol. */
	if (boundp(var)) {
	    setq(var, val);
	    return;
	}
	/* Out of luck. */
	read_warning("Can't set unknown global named `%s'", name);
    }
}

void
write_globals(void)
{
    int i, complete = FALSE;
    Obj *val;
    time_t now;

    /* Snapshot realtime values, but only after they've been set up. */
    if (g_elapsed_time() >= 0) {
	time(&now);
	set_g_elapsed_time(idifftime(now, game_start_in_real_time));
    }

    newline_form();

    for (i = 0; vardefns[i].name != 0; ++i) {
	if (debugging_state_sync
	    && strcmp(vardefns[i].name, "elapsed-real-time") == 0)
	  continue;
	if (vardefns[i].intgetter != NULL) {
	    if (complete || (*(vardefns[i].intgetter))() != vardefns[i].dflt) {
		start_form(key(K_SET));
		add_to_form(vardefns[i].name);
		add_num_to_form((*(vardefns[i].intgetter))());
		end_form();
		newline_form();
	    }
	} else if (vardefns[i].strgetter != NULL) {
	    if (complete
		|| string_not_default((*(vardefns[i].strgetter))(),
				      vardefns[i].dfltstr)) {
		start_form(key(K_SET));
		add_to_form(vardefns[i].name);
		add_to_form(escaped_string((*(vardefns[i].strgetter))()));
		end_form();
		newline_form();
	    }
	} else if (vardefns[i].objgetter != NULL) {
	    val = (*(vardefns[i].objgetter))();
	    if (complete
		|| val != lispnil
		/* Must write out explicit nil setting if the default
		   function is defined, so it won't re-run on startup. */
		|| (val == lispnil && vardefns[i].dfltfn != NULL)) {
		start_form(key(K_SET));
		add_to_form(vardefns[i].name);
		space_form();
		/* Suppress evaluation upon readin. */
		add_to_form_no_space("'");
		add_form_to_form((*(vardefns[i].objgetter))());
		end_form();
		newline_form();
	    }
	} else {
#ifdef __cplusplus
	    throw "snafu";
#else
	    abort();
#endif
	}
    }
}

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END
