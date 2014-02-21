/* Unit action buttons for the SDL interface to Xconq.
   Copyright (C) 2004 Eric A. McDonald.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "sdlpreconq.h"
#include "conq.h"
#include "sdlconq.h"

/* (Temporary hack. Need to create an 'impl_develop' function.) */
extern void net_set_develop_task(Unit *unit, int u2, int techgoal);

/* (Temporary hack. Need to create an 'impl_unit_research' function.) */
extern void net_set_unit_curadvance(Side *side, Unit *unit, int a);

/* (Temporary hack. Need to publicly declare 'net_set_unit_asleep'.) */
extern void net_set_unit_asleep(Side *side, Unit *unit, int flag, int recurse);

/* (Temporary hack. Need to publicly declare 'net_set_unit_reserve'.) */
extern void net_set_unit_reserve(Side *side, Unit *unit, int flag, int recurse);

/* (Temporary hack. Need to publicly declare 'net_delay_unit'.) */
extern void net_delay_unit(Unit *unit, int flag);

/** SECTION: Available button click handlers. **/
/* (TODO: Most of the click handlers herein need to be modified to account 
    for multiple unit selections, and not just the current unit.) */

static void delay_button_fn(SDLButton *button);
static void skip_button_fn(SDLButton *button);
static void sleep_button_fn(SDLButton *button);
static void wake_button_fn(SDLButton *button);
static void research_button_fn(SDLButton *button);
static void auto_research_button_fn(SDLButton *button);
static void construct_button_fn(SDLButton *button);
static void develop_button_fn(SDLButton *button);
static void change_type_button_fn(SDLButton *button);

/* Delay. */

static void
delay_button_fn(SDLButton *button)
{
    Unit *unit = NULL;

    assert_error(button, "Attempted to access a NULL button");
    unit = sscreen->curunit;
    if (unit)
      net_delay_unit(unit, TRUE);
    reset_screen_input(sscreen);
}

/* Skip. */

static void
skip_button_fn(SDLButton *button)
{
    Unit *unit = NULL;

    assert_error(button, "Attempted to access a NULL button");
    unit = sscreen->curunit;
    if (unit)
      net_set_unit_reserve(unit->side, unit, TRUE, FALSE);
    reset_screen_input(sscreen);
}

/* Sleep. */

static void
sleep_button_fn(SDLButton *button)
{
    Unit *unit = NULL;

    assert_error(button, "Attempted to access a NULL button");
    unit = sscreen->curunit;
    if (unit)
      net_set_unit_asleep(unit->side, unit, TRUE, FALSE);
    reset_screen_input(sscreen);
}

/* Wake. */

static void
wake_button_fn(SDLButton *button)
{
    Unit *unit = NULL;

    assert_error(button, "Attempted to access a NULL button");
    unit = sscreen->curunit;
    if (unit)
      net_set_unit_asleep(unit->side, unit, FALSE, FALSE);
    reset_screen_input(sscreen);
}

/* Research an advance. */

static void
research_button_fn(SDLButton *button)
{
    Unit *unit = NULL;

    assert_error(button, "Attempted to access a NULL button");
    assert_error(is_advance_type(button->data), 
		 "Illegal button data encountered");
    unit = sscreen->curunit;
    if (unit)
      net_set_unit_curadvance(unit->side, unit, button->data);
    reset_screen_input(sscreen);
}

/* Automatically research an advance. */

static void
auto_research_button_fn(SDLButton *button)
{
    Unit *unit = NULL;
    /* int a = NONATYPE; */

    assert_error(button, "Attempted to access a NULL button");
    unit = sscreen->curunit;
    if (unit) {
	auto_pick_unit_research(unit);
	assert_warning_return(is_advance_type(unit->curadvance), 
		     "Expected to choose advance but could not",);
	net_set_unit_curadvance(unit->side, unit, unit->curadvance);
    }
    reset_screen_input(sscreen);
}

/* Initiate a build using the current unit. */

static void
construct_button_fn(SDLButton *button)
{
    Unit *unit = sscreen->curunit;

    assert_error(button, "Attempted to access a NULL button");
    if (unit == NULL)
      return;
    if (sscreen->prefixarg < -1) {
	Dprintf("Illegal screen prefixarg detected: %d. Resetting.\n", 
		sscreen->prefixarg);
	sscreen->prefixarg = -1;
    }
    maybe_ask_construct_location(sscreen, unit, button->data);
}

/* Initiate development using the current unit. */

static void
develop_button_fn(SDLButton *button)
{
    Unit *unit = sscreen->curunit;
    int u2 = NONUTYPE;

    assert_error(button, "Attempted to access a NULL button");
    if (unit == NULL)
      return;
    assert_error(button, "Attempted to use a NULL button");
    u2 = button->data;
    /* (TODO: Make an 'impl_develop' so that side can get proper feedback.) */
    net_set_develop_task(unit, u2, u_tech_to_build(u2));
    reset_screen_input(sscreen);
}

/* Perform a change type of the current unit. */

static void
change_type_button_fn(SDLButton *button)
{
    Unit *unit = sscreen->curunit;

    assert_error(button, "Attempted to access a NULL button");
    if (unit == NULL)
      return;
    impl_change_type(dside, unit, button->data);
    reset_screen_input(sscreen);
}

/** SECTION: Builders of lists of unit action protobuttons. **/

/* Add delay button to the unit action panel. */

void
add_delay_button(Screen *screen, Unit **units, int uvecsz)
{
    Unit *unit = NULL;
    int i = 0, oneready = FALSE;
    struct a_protobutton *protobutton = NULL;

    if (!screen)
      return;
    assert_warning_return(units, "Attempted to access a NULL unit vector",);
    assert_warning_return(uvecsz > 0, "Encountered illegal unit count",);
    for (i = 0; i < uvecsz; ++i) {
	unit = units[i];
	assert_warning_return(unit, "Attempted to access a NULL unit",);
	if (unit->plan && !(unit->plan->delayed)) {
	    oneready = TRUE;
	    break;
	}
    }
    if (!oneready)
      return;
    protobutton = &(G_protobuttons[G_num_protobuttons++]);
    memset(protobutton, 0, sizeof(struct a_protobutton));
    protobutton->data = TRUE;
    protobutton->label = "Delay";
    protobutton->picture = NULL;
    sprintf(protobutton->help, "Delay");
    protobutton->click_fn = delay_button_fn;

}

/* Add skip button to the unit action panel. */

void
add_skip_button(Screen *screen, Unit **units, int uvecsz)
{
    /* Unit *unit = NULL; */
    struct a_protobutton *protobutton = NULL;

    if (!screen)
      return;
    assert_warning_return(units, "Attempted to access a NULL unit vector",);
    assert_warning_return(uvecsz > 0, "Encountered illegal unit count",);
    protobutton = &(G_protobuttons[G_num_protobuttons++]);
    memset(protobutton, 0, sizeof(struct a_protobutton));
    protobutton->data = TRUE;
    protobutton->label = "Skip";
    protobutton->picture = NULL;
    sprintf(protobutton->help, "Skip");
    protobutton->click_fn = skip_button_fn;
}

/* Add sleep button to the unit action panel. */

void
add_sleep_button(Screen *screen, Unit **units, int uvecsz)
{
    Unit *unit = NULL;
    int i = 0, oneawake = FALSE;
    struct a_protobutton *protobutton = NULL;

    if (!screen)
      return;
    assert_warning_return(units, "Attempted to access a NULL unit vector",);
    assert_warning_return(uvecsz > 0, "Encountered illegal unit count",);
    for (i = 0; i < uvecsz; ++i) {
	unit = units[i];
	assert_warning_return(unit, "Attempted to access a NULL unit",);
	if (unit->plan && !(unit->plan->asleep)) {
	    oneawake = TRUE;
	    break;
	}
    }
    if (!oneawake)
      return;
    protobutton = &(G_protobuttons[G_num_protobuttons++]);
    memset(protobutton, 0, sizeof(struct a_protobutton));
    protobutton->data = TRUE;
    protobutton->label = "Sleep";
    protobutton->picture = NULL;
    sprintf(protobutton->help, "Sleep");
    protobutton->click_fn = sleep_button_fn;
}

/* Add wake button to the unit action panel. */

void
add_wake_button(Screen *screen, Unit **units, int uvecsz)
{
    Unit *unit = NULL;
    int i = 0, oneasleep = FALSE;
    struct a_protobutton *protobutton = NULL;

    if (!screen)
      return;
    assert_warning_return(units, "Attempted to access a NULL unit vector",);
    assert_warning_return(uvecsz > 0, "Encountered illegal unit count",);
    for (i = 0; i < uvecsz; ++i) {
	unit = units[i];
	assert_warning_return(unit, "Attempted to access a NULL unit",);
	if (unit->plan && unit->plan->asleep) {
	    oneasleep = TRUE;
	    break;
	}
    }
    if (!oneasleep)
      return;
    protobutton = &(G_protobuttons[G_num_protobuttons++]);
    memset(protobutton, 0, sizeof(struct a_protobutton));
    protobutton->data = TRUE;
    protobutton->label = "Wake";
    protobutton->picture = NULL;
    sprintf(protobutton->help, "Wake");
    protobutton->click_fn = wake_button_fn;
}

/* Add research buttons to the unit action panel. */

void
add_research_buttons(Screen *screen, Unit **units, int uvecsz)
{
    Unit *unit = NULL;
    Side *side = NULL;
    int i = 0, allcanresearch = FALSE, a = NONATYPE;
    struct a_protobutton *protobutton = NULL;

    if (!screen)
      return;
    assert_warning_return(units, "Attempted to access a NULL unit vector",);
    assert_warning_return(uvecsz > 0, "Encountered illegal unit count",);
    for_all_advance_types(a) {
	allcanresearch = TRUE;
	for (i = 0; i < uvecsz; ++i) {
	    unit = units[i];
	    assert_warning_return(unit, "Attempted to access a NULL unit",);
	    side = unit->side;
	    assert_error(side, "Attempted to access a NULL side");
	    if (!u_can_research(unit->type) || !side_can_research(side, a)) {
		allcanresearch = FALSE;
		break;
	    }
	}
	if (allcanresearch) {
		protobutton = &(G_protobuttons[G_num_protobuttons++]);
		memset(protobutton, 0, sizeof(struct a_protobutton));
		protobutton->data = a;
		protobutton->label = "R&D";
		protobutton->picture = NULL;
		sprintf(protobutton->help, "Research %s", a_type_name(a));
		protobutton->click_fn = research_button_fn;
	}
    }
    /* If there is more than one advance to choose from, then create an 
       autopicker button. */
    if ((G_protobuttons[G_num_protobuttons - 1].click_fn == 
	    research_button_fn)
	&& (G_protobuttons[G_num_protobuttons - 2].click_fn == 
	    research_button_fn)) {
	protobutton = &(G_protobuttons[G_num_protobuttons++]);
	memset(protobutton, 0, sizeof(struct a_protobutton));
	protobutton->label = "Auto\nR&D";
	protobutton->picture = NULL;
	strcpy(protobutton->help, "Automatically Pick Research Topic");
	protobutton->click_fn = auto_research_button_fn;
    }
}

/* Add construct buttons to the unit action panel. */
/* Processes unit vectors. */

void
add_construct_buttons(Screen *screen, Unit **units, int uvecsz)
{
    int i = 0, allcanbuild = FALSE, n = -1, u = NONUTYPE;
    Unit *unit = NULL;
    struct a_protobutton *protobutton = NULL;

    if (!screen)
      return;
    assert_warning_return(units, "Attempted to access a NULL unit vector",);
    assert_warning_return(uvecsz > 0, "Encountered illegal unit count",);
    for_all_unit_types(u) {
	allcanbuild = TRUE;
	for (i = 0; i < uvecsz; ++i) {
	    unit = units[i];
	    assert_warning_return(unit, "Attempted to access a NULL unit",);
	    if (!can_build_type(unit, u)) {
		allcanbuild = FALSE;
		break;
	    }
	}
	if (allcanbuild) {
	    protobutton = &(G_protobuttons[G_num_protobuttons++]);
	    memset(protobutton, 0, sizeof(struct a_protobutton));
	    protobutton->data = u;
	    protobutton->label = shortest_unique_name(u);
	    protobutton->picture = uimages[u];
	    n = construction_run_doctrine(unit, u);
	    if (n <= 0)
	      n = 1;
	    sprintf(protobutton->help, "Construct %s (x %d)",
		    u_type_name(u),
		    (screen->prefixarg > 0) ? screen->prefixarg
					    : n);
	    protobutton->click_fn = construct_button_fn;
	}
    }
}

/* Add develop buttons to the unit action panel. */
/* Processes unit vectors. */

void
add_develop_buttons(Screen *screen, Unit **units, int uvecsz)
{
    int i = 0, allcandevelop = FALSE, u = NONUTYPE;
    Unit *unit = NULL;
    Side *side = NULL;
    struct a_protobutton *protobutton = NULL;

    if (!screen)
      return;
    assert_warning_return(units, "Attempted to access a NULL unit vector",);
    assert_warning_return(uvecsz > 0, "Encountered illegal unit count",);
    for_all_unit_types(u) {
	allcandevelop = TRUE;
	for (i = 0; i < uvecsz; ++i) {
	    unit = units[i];
	    side = unit->side;
	    assert_warning_return(unit, "Attempted to access a NULL unit",);
	    assert_error(side, "Attempted to access a NULL side");
	    if ((side->tech[u] >= u_tech_to_build(u))
		|| !could_develop(unit->type, u)) {
		allcandevelop = FALSE;
		break;
	    }
	}
	if (allcandevelop) {
	    protobutton = &(G_protobuttons[G_num_protobuttons++]);
	    memset(protobutton, 0, sizeof(struct a_protobutton));
	    protobutton->data = u;
	    protobutton->label = shortest_unique_name(u);
	    protobutton->picture = uimages[u];
	    sprintf(protobutton->help, "Develop %s", u_type_name(u));
	    protobutton->click_fn = develop_button_fn;
	}
    }
}

/* Add change type buttons to the unit action panel. */
/* Processes unit vectors. */

void
add_change_type_buttons(Screen *screen, Unit **units, int uvecsz)
{
    int i = 0, allcanmorph = FALSE, u = NONUTYPE;
    Unit *unit = NULL;
    struct a_protobutton *protobutton = NULL;

    if (!screen)
      return;
    assert_warning_return(units, "Attempted to access a NULL unit vector",);
    assert_warning_return(uvecsz > 0, "Encountered illegal unit count",);
    for_all_unit_types(u) {
	allcanmorph = TRUE;
	for (i = 0; i < uvecsz; ++i) {
	    unit = units[i];
	    assert_warning_return(unit, "Attempted to access a NULL unit",);
	    if (!can_change_type_to(unit, u)) {
		allcanmorph = FALSE;
		break;
	    }
	}
	if (allcanmorph) {
	    protobutton = &(G_protobuttons[G_num_protobuttons++]);
	    memset(protobutton, 0, sizeof(struct a_protobutton));
	    protobutton->data = u;
	    protobutton->label = shortest_unique_name(u);
	    protobutton->picture = uimages[u];
	    sprintf(protobutton->help, "Change Type to %s", u_type_name(u));
	    protobutton->click_fn = change_type_button_fn;
	}
    }
}
