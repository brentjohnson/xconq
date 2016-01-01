/* Name generation for Xconq.
   Copyright (C) 1991-2000 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/* Naming is a special class of init method, since it may run both during
   init and throughout a game.  Name generation has a very strong influence
   on the flavor of a game, so it has some extra flexibility, including
   a capability to use a simple context-free grammar to generate names. */

#include "conq.h"

Obj *namerlist;

Obj *lastnamer;

static int total_side_weights;
static int total_color_weights;

static char *name_from_grammar(Obj *grammar);
static void gen_name(Obj *nonterm, Obj *rules, int depth, char *rslt);
static void gen_from_rule(Obj *rule, Obj *rules, int depth, char *rslt);

void
init_namers(void)
{
    namerlist = lastnamer = lispnil;
    total_side_weights = total_color_weights = 0;
}

/* Pick a side name that is not already being used. */

void
make_up_side_name(Side *side)
{
    int uniq, tries, n, found = FALSE;
    const char *colstr, *method, *sidename;
    Obj *sidelib, *subobj, *subelts, *head, *filler, *colorlib, *col;

    sidelib = g_side_lib();
    filler = lispnil;
    if (sidelib != lispnil
    	&& side != indepside) {
	tries = 0;
	while (tries++ < 100 * numsides) {
	    subobj = choose_from_weighted_list(sidelib, &total_side_weights,
					       FALSE);
	    /* Scan the properties of the chosen side library element,
	       looking for names already in use. */
	    uniq = TRUE;
	    for_all_list(subobj, subelts) {
		head = car(subelts);
		if (stringp(cadr(head))) {
		    found = TRUE;
		    if (name_in_use(side, c_string(cadr(head)))) {
			uniq = FALSE;
			found = FALSE;
			break;
		    }
		}
	    }
	    if (uniq) {
		filler = subobj;
		break;
	    }
	}
    }
    /* Don't rename indepside to '@'. */
    if (!found && side != indepside) {
	Module *module;

    	/* Try to use a side namer. */
    	method = g_side_namer();
	/* Load the default side namer module. */    	
	module = add_game_module("ng-sides", mainmodule);
	if (!module->loaded) {    
		load_game_module(module, FALSE);
	}
    	if (boundp(intern_symbol(method))) {
		tries = 0;
		while (tries++ < 100 * numsides) {
			sidename = run_namer(symbol_value(intern_symbol(method)));
			if (!name_in_use(side, sidename)) {
				sprintf(spbuf, "%s", sidename);
				sprintf(tmpbuf, "%sian", sidename);
				filler = cons(cons(intern_symbol(keyword_name(K_NAME)),
						   cons(new_string(copy_string(spbuf)),
							lispnil)),
					      filler);
				filler = cons(cons(intern_symbol(keyword_name(K_NOUN)),
						   cons(new_string(copy_string(tmpbuf)),
							lispnil)),
					      filler);
				break;
			}
		}
	} else {
		/* The fallback is to label the side 'A', 'B', etc.  Note that
		   we don't test for these "names" being already in use, but this
		   should be OK, because real games should rarely need these. */
		n = side_number(side) - 1;
		sprintf(spbuf, "%c", 'A' + n);
		sprintf(tmpbuf, "%cian", 'A' + n); /* should be in nlang.c? */
		filler = cons(cons(intern_symbol(keyword_name(K_NAME)),
				   cons(new_string(copy_string(spbuf)),
					lispnil)),
			      filler);
		filler = cons(cons(intern_symbol(keyword_name(K_NOUN)),
				   cons(new_string(copy_string(tmpbuf)),
					lispnil)),
			      filler);
	}
    }
    /* Now fill the side from the chosen obj - no effect if it is nil. */
    fill_in_side(side, filler, FALSE);
    /* Add a color scheme if necessary and possible. */
    colorlib = g_side_color_lib();
    if (colorlib != lispnil
	&& (empty_string(side->colorscheme)
	    /* check for the default from side.c */
	    || strcmp(side->colorscheme, "black,black,white") == 0)) {
	tries = 0;
	while (tries++ < 100 * numsides) {
	    col = choose_from_weighted_list(colorlib, &total_color_weights,
					    FALSE);
	    /* See if the chosen color scheme is already in use. */
	    /* Library entries are always one string. */
	    /* (should complain if not, instead of ignoring) */
	    if (consp(col))
	      col = car(col);
	    if (stringp(col)) {
		colstr = c_string(col);
		/* Note that this also rejects colors that are used
		   as the names of sides, but that's OK. */
		if (!name_in_use(side, colstr)) {
		    side->colorscheme = colstr;
		    break;
		}
	    }
	}
    }
}

/* This tests whether a given string is already being used by a side. */

int
name_in_use(Side *side, const char *str)
{
    Side *side2;

    if (empty_string(str))
      return FALSE;
    for_all_sides(side2) {
	if (side2 != side) {
	    if ((side2->name && strcmp(str, side2->name) == 0)
		|| (side2->noun && strcmp(str, side2->noun) == 0)
		|| (side2->pluralnoun && strcmp(str, side2->pluralnoun) == 0)
		|| (side2->adjective && strcmp(str, side2->adjective) == 0)
		|| (side2->colorscheme && strcmp(str, side2->colorscheme) == 0)
		)
	      return TRUE;
	}
    }
    return FALSE;
}

Obj *
make_namer(Obj *sym, Obj *meth)
{
    Obj *namer = new_pointer(sym, (char *) meth), *tmpobj;

    /* Append the new namer to the end of the list of namers. */
    tmpobj = cons(namer, lispnil);
    if (lastnamer != lispnil) {
	set_cdr(lastnamer, tmpobj);
	lastnamer = tmpobj;
    } else {
	namerlist = lastnamer = tmpobj;
    }
    return namer;
}

/* Method to add names to units that want them and don't have them already. */

int
name_units_randomly(int calls, int runs)
{
    Unit *unit;
    Side *side;

    /* Make sure each side's units are in order before we 
    name them. */
    sort_units(TRUE);
    /* There's never any reason not to run this method. */
    /* (should this announce progress?) */
    for_all_sides(side) {
	/* sort_units only puts the side unit lists in order, so 
	we need to use them instead of the global unit list. */
	for_all_side_units(side, unit) {
		make_up_unit_name(unit);
		assign_unit_number(unit);
	}
    }
    return TRUE;
}

/* Given a unit, return its naming method if it has one. */

const char *
unit_namer(Unit *unit)
{
    Side *side = unit->side;

    if (unit == NULL)
      return NULL;
    /* Look for and return a side-specific namer if found. */
    if (side->unitnamers != NULL && side->unitnamers[unit->type] != NULL) {
    	return side->unitnamers[unit->type];
    }
    return u_namer(unit->type);
}

/* Generate a name for a unit, using an appropriate method.

   It is possible (in fact encouraged) to add cool new unit name generation
   methods in here, especially when the grammar-based or thematic methods
   don't give the desired results. */

const char *
propose_unit_name(Unit *unit)
{
    int u;
    const char *method, *sidename;

    if (unit == NULL)
      return NULL;
    u = unit->type;
    method = unit_namer(unit);
    if (empty_string(method)) {
	/* Nothing to work with. */
    } else if (boundp(intern_symbol(method))) {
	return run_namer(symbol_value(intern_symbol(method)));
    } else {
	/* Do builtin naming methods. */
	switch (keyword_code(method)) {
	  case K_JUNKY:
	    /* Kind of a bizarre thing, but flavorful sometimes. */
	    sidename = side_name(unit->side);
	    if (!empty_string(sidename) && strlen(sidename) >= 2) {
		sprintf(spbuf, "%c%c-%s-%02d",
			uppercase(sidename[0]), uppercase(sidename[1]),
			utype_name_n(u, 3), unit->number);
	    } else {
		/* If no side name, use unit id to get better chance
		   of uniqueness. */
		sprintf(spbuf, "%s-%d", utype_name_n(u, 3), unit->id);
	    }
	    return copy_string(spbuf);
	  default:
	    init_warning("No naming method `%s', ignoring", method);
	    break;
	}
    }
    return NULL;
}

/* This names only units that do not already have names. */

void
make_up_unit_name(Unit *unit)
{
    if (unit == NULL || unit->name != NULL)
      return;
    /* (should check that proposed name is not in use by matching side and type?) */
    unit->name = propose_unit_name(unit);
}

/* Unit numbering only happens to designated types that are on a side. */

void
assign_unit_number(Unit *unit)
{
    if (u_assign_number(unit->type)
	&& unit->side != indepside) {
	/* If unnumbered, give it the next available number and increment. */
	if (unit->number == 0)
	  unit->number = (unit->side->counts)[unit->type]++;
    } else {
	/* Note that this will erase any already-assigned number,
	   if the type is one that is not supposed to be numbered. */
	unit->number = 0;
    }
}

/* Given a naming method, run it and get back a string. */

const char *
run_namer(Obj *namer)
{
    int len, ix;
    Obj *prev, *rslt, *type, *sym;
    Obj *code = (Obj *) namer->v.ptr.data;

    if (!consp(code))
      return "?format?";
    type = car(code);
    if (!symbolp(type))
      return "?type?";
    switch (keyword_code(c_string(type))) {
      case K_JUNKY:
      case K_RANDOM:
        len = length(cdr(code));
	if (len > 0) {
	    ix = xrandom(len - 1) + 1;
	    prev = code;
	    while (--ix)
	      prev = cdr(prev);
	    rslt = cadr(prev);
	    /* Splice out our desired name. */
	    set_cdr(prev, cddr(prev));
	    return c_string(rslt);
	} else if (!empty_string(g_default_namer())
		   && boundp(sym = intern_symbol(g_default_namer()))
		   /* Note that this test prevents infinite recursion. */
		   && namer != symbol_value(sym)) {
	    /* The default namer comes in to help out when the namer is
	       exhausted. */
	    return run_namer(symbol_value(sym));
	} else {
	    return "?no more names?";
	}
	break;
      case K_IN_ORDER:
        len = length(cdr(code));
	if (len > 0) {
	    ix = 1;
	    prev = code;
	    while (--ix)
	      prev = cdr(prev);
	    rslt = cadr(prev);
	    /* Splice out our desired name. */
	    set_cdr(prev, cddr(prev));
	    return c_string(rslt);
	} else if (!empty_string(g_default_namer())
		   && boundp(sym = intern_symbol(g_default_namer()))
		   /* Note that this test prevents infinite recursion. */
		   && namer != symbol_value(sym)) {
	    /* The default namer comes in to help out when the namer is
	       exhausted. */
	    return run_namer(symbol_value(sym));
	} else {
	    return "?no more names?";
	}
	break;
      case K_GRAMMAR:
	return name_from_grammar(code);
      default:
	return "?method?";
    }
}

static int maxdepth;

char *
name_from_grammar(Obj *grammar)
{
    char rslt[500];  /* not really safe... */
    Obj *root = cadr(grammar);
    Obj *depth = caddr(grammar);
    Obj *rules = cdr(cddr(grammar));

    maxdepth = 5;
    if (numberp(depth))
      maxdepth = c_number(depth);
    rslt[0] = '\0';
    gen_name(root, rules, 0, rslt);
    /* This should be optional maybe. */
    rslt[0] = uppercase(rslt[0]);
    return copy_string(rslt);
}

/* Given a nonterminal and a set of rules, find and apply the right
   rule. */

void
gen_name(Obj *nonterm, Obj *rules, int depth, char *rslt)
{
    Obj *rest, *rule;

    /* Look for the first rule whose head matches the nonterminal. */
    for_all_list(rules, rest) {
	rule = car(rest);
	if (equal(nonterm, car(rule))) {
	    gen_from_rule(cadr(rule), rules, depth, rslt);
	    return;
	}
    }
    /* If we didn't find the symbol mentioned as a nonterminal, see if
       it's a namer itself, and run it if so. */
    if (symbolp(nonterm)
	&& boundp(nonterm)
	&& pointerp(symbol_value(nonterm))) {
	strcat(rslt, run_namer(symbol_value(nonterm)));
    } else {
	/* Assume that the purported nonterm symbol is actually a
           terminal and just concatenate it. */
	strcat(rslt, c_string(nonterm));
    }
}

/* Given a rule body, decide how to add to the output string.  This may
   recurse, so there is a limit check. */

void
gen_from_rule(Obj *rule, Obj *rules, int depth, char *rslt)
{
    int total, oldlen, maxtries = 100, tries, found;
    char buf[BUFSIZE];
    const char *rejstr;
    Obj *rest, *subrule, *rejrest;
    
    if (depth >= maxdepth)
      return;
    switch (rule->type) {
      case NIL:
	/* Assume an empty rule body to be a deliberate no-op. */
	break;
      case STRING:
	/* Strings are always terminals, just append them directly. */
	strcat(rslt, c_string(rule));
	break;
      case SYMBOL:
	/* Assume that a symbol might be a nonterminal, and recurse. */
	gen_name(rule, rules, depth, rslt);
	break;
      case CONS:
        if (symbolp(car(rule))) {
	    switch (keyword_code(c_string(car(rule)))) {
	      case K_OR:
		/* Make a weighted selection from the alternatives. */
		total = 0;
		subrule = choose_from_weighted_list(cdr(rule), &total, TRUE);
		if (subrule != lispnil)
		  gen_from_rule(subrule, rules, depth + 1, rslt);
		break;
	      case K_CAPITALIZE:
		/* Run the rule, then capitalize the result. */
		if (cdr(rule) == lispnil) {
		    run_warning("`capitalize' form missing argument");
		    break;
		}
		oldlen = strlen(rslt);
		gen_from_rule(cadr(rule), rules, depth + 1, rslt);
		if (islower(rslt[oldlen]))
		  rslt[oldlen] = uppercase(rslt[oldlen]);
		break;
	      case K_REJECT:
		/* Run the rule, then retry if the result includes any
		   substrings that we want to exclude. */
		if (cdr(rule) == lispnil) {
		    run_warning("`reject' form missing argument");
		    break;
		}
		for (tries = 0; tries < maxtries; ++tries) {
		    /* Only look at the just-generated part of the result. */
		    oldlen = strlen(rslt);
		    /* Generate a name to test. */
		    gen_from_rule(cadr(rule), rules, depth + 1, rslt);
		    found = FALSE;
		    for_all_list(cddr(rule), rejrest) {
			if (symbolp(car(rejrest)) || stringp(car(rejrest))) {
			    rejstr = c_string(car(rejrest));
			    if (!empty_string(rejstr)
				&& strstr(rslt + oldlen, rejstr)) {
				found = TRUE;
				break;
			    }
			}
		    }
		    if (found) {
			if (tries >= maxtries - 1) {
			    sprintlisp(buf, rule, BUFSIZE);
			    run_warning("Grammar rule `%s' still got rejects"
					" after %d tries, accepting last try",
					buf, maxtries - 1);
			    break;
			}
			/* "Erase" result and start over. */
			rslt[oldlen] = '\0';
		    } else {
			/* Name looks good, we're done here. */
			break;
		    }
		}
		break;
	      default:
		/* Assume anything else to be a nested subsequence. */
		for_all_list(rule, rest)
		  gen_from_rule(car(rest), rules, depth + 1, rslt);
		break;
	    }
	} else if (stringp(car(rule))) {
	    /* Assume a nested subsequence. */
	    for_all_list(rule, rest)
	      gen_from_rule(car(rest), rules, depth + 1, rslt);
	} else {
	    sprintlisp(buf, rule, BUFSIZE);
	    run_warning("Grammar rule `%s' does not begin"
			" with a symbol or string",
			buf);
	}
	break;
      default:
	sprintlisp(buf, rule, BUFSIZE);
	run_warning("`%s' is not a valid grammar rule", buf);
	break;
    }
}
