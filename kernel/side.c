/* Sides in Xconq.
   Copyright (C) 1987-1989, 1991-2000 Stanley T. Shebs.
   Copyright (C) 2004-2005 Eric A. McDonald.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/* This file implements sides and functionality relating to sides in
   general. */

#include "conq.h"
#include "kernel.h"

#define checked_elev_at(x, y) (elevations_defined() ? elev_at(x, y) : 0)

namespace Xconq {
    int people_always_see;
    int any_people_see_chances = -1;
    int any_see_chances = -1;
    int max_see_chance_range;
    int any_see_mistake_chances = -1;
    int max_see_mistake_range;
    int any_overwatch_cache = -1;
    PackedBoolTable *any_overwatch_chances = NULL;
    int *overwatch_against_range_max = NULL;
    int *overwatch_against_range_min = NULL;
}

/* From 'ui.h'. */
extern void set_unit_image(Unit *unit);
extern void set_unit_view_image(UnitView *uview);

void return_default_colorname(Side *side);

static char *pick_default_colorname(Side *side);
static void allocate_unit_view_block(void);
static UnitView *create_bare_unit_view(void);
static void init_visible_elevation(int x, int y);
static void init_visible_elevation_2(int x, int y);
static void calc_visible_elevation(int x, int y);
static void calc_visible_elevation_2(int x, int y);
static void cover_area_1(Side *side, struct a_unit *unit, int x0, int y0,
			 int x1, int y1);
static int see_materials(Side *side, int x, int y);
static int see_weather(Side *side, int x, int y);
static int mistaken_type(int u2);
static void update_master_uview(Unit *unit, int lookabove);
static int remove_unit_view(Side *side, UnitView *olduview);
static void flush_one_view(UnitView *uview);

static int see_unit(Unit *seer, Unit *tosee, SeerNode *seers, int *numseers);
static int side_occs_see_unit(int x, int y, int *numseers, ParamBox *parambox);
static int side_ustack_see_unit(int x, int y, int *numseers, 
				ParamBox *parambox);
static void compute_overwatch_cache(void);

#ifndef INITMAXVIEWS
#define INITMAXVIEWS 200
#endif

#define VIEW_HASH_SIZE 257

/* Head of the list of all sides. */

Side *sidelist;

/* Pointer to the side representing independent units. */

Side *indepside;

/* Pointer to the last side of the list. */

Side *lastside;

/* Temporary used in many places. */
     
Side *tmpside;

/* The actual number of sides in a game.  This number never decreases;
   sides no longer playing need to be around for recordskeeping purposes. */

int numsides;

/* This is the number of sides including indepside. We need it for the
   AI assignment in init.c. We want to keep numsides referring to
   normal sides only, since it is used a lot in the interface code,
   where we typically don't want to include indepside. */

int numtotsides;

/* Used to generate the id number of the side. */

int nextsideid;

/* Test if a given side's sideclass matches a given sideclass. */

int
fn_test_side_in_sideclass(Obj *osclass, ParamBox *pbox)
{
    char *sclass = NULL;
    int sid = -1;
    ParamBoxSide *paramboxs = NULL;
    Obj *rest = lispnil;

    assert_error(osclass, "Attempted to access a NULL GDL object");
    assert_error(pbox, "Attempted to access a NULL parameter box");
    assert_error(pbox->get_type() == PBOX_TYPE_SIDE,
		 "Wrong type of paramter box passed to test function");
    paramboxs = (ParamBoxSide *)pbox;
    /* If object is NIL, then return immediately. */
    if (osclass == lispnil)
      return FALSE;
    /* If the object is a string, then assume it is a sideclass. */
    if (stringp(osclass)) {
        sclass = c_string(osclass);
        if (paramboxs->side && (paramboxs->side != indepside)) {
            if (empty_string(paramboxs->side->sideclass))
              return FALSE;
            return (!strcmp(sclass, paramboxs->side->sideclass));
        }
	else 
          return (!strcmp(sclass, "independent"));
    }
    /* If the object is a number, then assume it is a side ID. */
    else if (numberp(osclass)) {
	sid = c_number(osclass);
	if (paramboxs->side)
	  return (sid == paramboxs->side->id);
	else
	  return (!sid);
    }
    /* If the object is a cons, then assume it is a list of side ID's 
       inclusive-or side classes, and iterate through list until a 
       match is found or end of list is encountered. */
    else if (consp(osclass)) {
	for_all_list(osclass, rest) {
	    if (fn_test_side_in_sideclass(car(rest), pbox))
	      return TRUE;
	}
	return FALSE;
    }
    /* Else, we can't use the object. */
    else {
        init_warning("Testing side against garbled sideclass expression");
        /* Be permissive if continued. */
        return TRUE;
    }
    return FALSE;
}

/* Used for solid color emblems. These colors have been carefully
   tested so that they stand out against most backgrounds and also can be
   easily distinguished from each other. The indepside default emblem is
   set to "none" since it usually goes without emblem. */

char *default_colornames[MAXSIDES + 1] = {
	"lemon-chiffon",
	"blue",
	"red",
	"yellow",
	"green",
	"dark-orange",
	"deep-pink",
	"cyan",
	"purple",
	"tomato",
	"violet",
	"dodger-blue",
	"light-sea-green",
	"rosy-brown",
	"slate-blue",
	"firebrick"
};

/* Default colors used so far by any non-removed side. Stores the 
   number of the side using the color, or else zero. */

static short default_colornames_used[MAXSIDES + 1];

static char
*pick_default_colorname(Side *side) 
{
	int i;
	
	if (side == indepside) {
		return default_colornames[0];
	} 
	/* We start at 1 since default_colors[0] is reserved for 
	indepside, whether or not it is present in the game. */
	for (i = 1; i <= MAXSIDES +1; i++) {
		if (default_colornames_used[i] == FALSE) {
			default_colornames_used[i] = TRUE;
			return default_colornames[i];
		}
	}
	/* Should never happen, but humor the compiler. */
	return "black";
}

void
return_default_colorname(Side *side) 
{
	int i;
	
	/* We start at 1 since default_colors[0] is reserved for 
	indepside, whether or not it is present in the game. */
	for (i = 1; i <= MAXSIDES + 1; i++) {
	    if (default_colornames_used[i] == TRUE
		&& strcmp(side->default_color, default_colornames[i]) == 0) {
		default_colornames_used[i] = FALSE;
		return;
	    }
	}
}

/* Cached values of global vision vars. */

short any_los = -1;

int any_material_views = -1;

char *any_material_views_by_m = NULL;

/* Pointer to buffer used for readable side description (for debugging). */

char *sidedesigbuf = NULL;

/* Pointer to the head of the list of players. */

Player *playerlist;

/* Pointer to the last player of the list, used to add players at end. */

Player *last_player;

/* The total number of players. */

int numplayers;

/* Use to generate the id number of each player. */

int nextplayerid;

char *playerdesigbuf = NULL;

/* The list of doctrine objects. */

Doctrine *doctrine_list;

Doctrine *last_doctrine;

int next_doctrine_id;

/* The list of available unit views. */

UnitView *freeviews;

/* The global linked list of all unit views. */

UnitView *viewlist = NULL;

/* The number of views in viewlist. */

static int numunitviews = 0;

/* Grab a block of unit view objects to work with. */

static void
allocate_unit_view_block(void)
{
    int i;
    UnitView *viewblock = (UnitView *) xmalloc(INITMAXVIEWS * sizeof(UnitView));

    for (i = 0; i < INITMAXVIEWS; ++i) {
        viewblock[i].id = -1;
        viewblock[i].nexthere = &viewblock[i+1];
    }
    viewblock[INITMAXVIEWS-1].nexthere = NULL;
    freeviews = viewblock;
    Dprintf("Allocated space for %d unit views.\n", INITMAXVIEWS);
}

/* The primitive unit view creator. */

UnitView *
create_bare_unit_view(void)
{
    UnitView *newview;

    /* If our free list is empty, go and get some more units. */
    if (freeviews == NULL) {
	allocate_unit_view_block();
    }
    /* Take the first unit off the free list. */
    newview = freeviews;
    freeviews = freeviews->nexthere;
    /* ...but an invalid id. */
    newview->id = -1;
    return newview;
}

/* Count how many units total are on a side. */

int
n_units_on_side(Side *side)
{
    int n = 0;
    Unit *unit = NULL;
    /* (NOTE: Probably faster to sum up the side totals for each type. 
	The question is whether those tallies are accurate.) */
    for_all_side_units(side, unit) 
      ++n;
    return n;
}

/* Init side machinery. Don't fill in indepside yet since numutypes
   etc. may not be defined.  */

void
init_sides(void)
{
    /* Set up the list of sides. */
    sidelist = lastside = (Side *) xmalloc(sizeof(Side));
    /* The independent units' side will be the first one in the list,
       is always created.  It cannot be filled in yet, because the
       numbers of the various types is not yet known. */
    indepside = sidelist;
    /* Note: indepside is included in numtotsides, but not numsides. */
    numtotsides = 0;
    numsides = 0;
    nextsideid = 0;
    /* Set up the list of players. */
    playerlist = last_player = NULL;
    numplayers = 0;
    /* Indepside also needs a player, so we now start with id 0. */
    nextplayerid = 0;
    /* Set up the player/side assignment array. */
    assignments = (Assign *) xmalloc((MAXSIDES + 1) * sizeof(Assign));
    /* Set up the list of doctrines. */
    doctrine_list = last_doctrine = NULL;
    /* Doctrine ids must start at 1. */
    next_doctrine_id = 1;
}

/* Create an object representing a side. */

Side *
create_side(void)
{
    int u, m;
    Side *newside;

    /* Too many sides. */
    if (numsides >=  g_sides_max()) {
	run_error("Cannot have more than %d sides total!", g_sides_max());
    }
    /* Prevent many crashes and much hair-pulling. */
    if (numutypes <= 0) {
	/* (Should probably be regarded as an init_error; however, this 
	    segment of code may be executed "late".) */
	run_error("Unit types must be defined before defining sides!");
    }
    /* Just fill in the names if dealing with indepside. */
    if (nextsideid == 0) {
    	newside = indepside;
	newside->name = "Independents";
	newside->noun = "Independent";
	newside->pluralnoun = "Independents";
	newside->adjective = "independent";
	newside->colorscheme = "black,black,white";
	newside->emblemname = "none";
    } else {
	/* Else allocate a new side object and increase numsides. */
	newside = (Side *) xmalloc(sizeof(Side));
	/* numsides does not include indepside, thus increment it here. */
    	++numsides;
    }
    /* Fill in various side slots.  Only those with non-zero/non-NULL
       defaults need have anything done to them. */
    newside->id = nextsideid++;
    /* Always start sides IN the game. */
    newside->ingame = TRUE;
    /* Note that "everingame" is only set at the beginning of a turn. */
    /* Set up the relationships with other sides. */
    newside->knows_about = ALLSIDES; /* for now */
    newside->trusts = (short *) xmalloc((g_sides_max() + 1) * sizeof(short));
    newside->trades = (short *) xmalloc((g_sides_max() + 1) * sizeof(short));
    /* Set up per-unit-type slots. */
    newside->counts = (short *) xmalloc(numutypes * sizeof(short));
    newside->tech = (short *) xmalloc(numutypes * sizeof(short));
    newside->inittech = (short *) xmalloc(numutypes * sizeof(short));
    newside->numunits = (short *) xmalloc(numutypes * sizeof(short));
    newside->default_color = pick_default_colorname(newside);
    if (newside != indepside) {
	newside->emblemname = newside->default_color;
    }
    for_all_unit_types(u) {
	/* Start unit numbering at 1, not 0. */
	newside->counts[u] = 1;
    }
    newside->priority = -1;
    /* All sides should auto-finish by default. */
    newside->autofinish = TRUE;
    /* True by default, players should disable manually. */
    newside->willingtosave = TRUE;
    /* Put valid Lisp data into slots that need it. */
    newside->symbol = lispnil;
    newside->instructions = lispnil;
    newside->rawscores = lispnil;
    newside->aidata = lispnil;
    newside->uidata = lispnil;
    newside->possible_units = lispnil;
    newside->startx = newside->starty = -1;
    newside->init_center_x = newside->init_center_y = -1;
    newside->gaincounts = 
	(short *) xmalloc(numutypes * num_gain_reasons * sizeof(short));
    newside->losscounts = 
	(short *) xmalloc(numutypes * num_loss_reasons * sizeof(short));
    newside->atkstats = (long **) xmalloc(numutypes * sizeof(long *));
    newside->hitstats = (long **) xmalloc(numutypes * sizeof(long *));
    /* Necessary to enable AI control toggling of selected units! */
    newside->prefixarg = -1; 
    if (numatypes > 0) {
	newside->advance = (short *) xmalloc(numatypes * sizeof(short));
	newside->canresearch = (short *) xmalloc(numatypes * sizeof(short));
	newside->research_precluded = 
	    (short *)xmalloc(numatypes * sizeof(short));
	newside->research_topic = NOADVANCE;
	newside->autoresearch = FALSE;
	newside->research_goal = NONATYPE;
    }
    newside->canbuild = (short *) xmalloc(numutypes * sizeof(short));
    newside->cancarry = (short *) xmalloc(numutypes * sizeof(short));
    newside->candevelop = (short *) xmalloc(numutypes * sizeof(short));
    // Fill out side action limitations.
    // Note that indepside cannot act be default.
    newside->could_act_with = (short *) xmalloc(numutypes * sizeof(short));
    if (indepside != newside)
	memset(newside->could_act_with, TRUE, numutypes * sizeof(short));
    newside->could_construct = (short *) xmalloc(numutypes *sizeof(short));
    memset(newside->could_construct, TRUE, numutypes * sizeof(short));
    newside->could_develop = (short *) xmalloc(numutypes *sizeof(short));
    memset(newside->could_develop, TRUE, numutypes * sizeof(short));
    newside->could_research = (short *) xmalloc(numatypes *sizeof(short));
    memset(newside->could_research, TRUE, numatypes * sizeof(short));
    /* We can't update the canbuild vector yet, because its contents
       may depend on side class, which isn't set yet. */
    if (nummtypes > 0) {
	/* Set up the side's supply of materials. */
	newside->treasury = (long *) xmalloc(nummtypes * sizeof(long));
	for_all_material_types(m) {
	    if (side_has_treasury(newside, m)) {
		newside->treasury[m] = m_initial_treasury(m);
	    }
	}
	/* Make space for default conversions and copy them over. */
	newside->c_rates = (short *) xmalloc(nummtypes * sizeof(short));
	{
	    int m1, m2;
	    
	    /* First initilaize m. */
	    m = NONMTYPE;
	    /* For now, assume only one m. */
	    for_all_material_types(m1) {
		for_all_material_types(m2) {
		    if (mm_conversion(m1, m2) > 0) {
			m = m1;
			break;
		    }
		}
	    }
	    if (m != NONMTYPE) {
		for_all_material_types(m2) {
		    newside->c_rates[m2] = mm_conversion_default(m, m2);
		}
	    }
	}
    }
    newside->controlled_by_id = -1;
    newside->playerid = -1;
    /* Link in at the end of the list of sides. */
    newside->next = NULL;
    /* Important to avoid the indepside->next = indepside loop! */
    if (newside != indepside) {
	lastside->next = newside;
	lastside = newside;
    }
    init_side_unithead(newside);
    /* numtotsides also includes indepside. */
    ++numtotsides;
    return newside;
}

/* To make the double links work, we have to have one pseudo-unit to
   serve as a head.  This unit should not be seen by any outside
   code. */

void
init_side_unithead(Side *side)
{
    if (side->unithead == NULL) {
	side->unithead = create_bare_unit(0);
	side->unithead->next = side->unithead;
	side->unithead->prev = side->unithead;
    }
}

/* Quick check to see if we have units. */

/* This should be improved to not be fooled by dead units? */

int
side_has_units(Side *side)
{
    return (side->unithead != NULL
	    && (side->unithead->next != side->unithead));
}

/* Set up doctrine structures. */

void
init_doctrine(Side *side)
{
    int u;

    /* Every side must have a default doctrine to fall back on. */
    if (side->default_doctrine == NULL) {
	side->default_doctrine = new_doctrine(0);
    }
    /* Make each individual unit doctrine point to the generic
       doctrine by default. */
    if (side->udoctrine == NULL) {
	side->udoctrine =
	  (Doctrine **) xmalloc(numutypes * sizeof(Doctrine *));
    }
    for_all_unit_types(u) {
	if (side->udoctrine[u] == NULL)
	  side->udoctrine[u] = side->default_doctrine;
    }
}

void
init_self_unit(Side *side)
{
    Unit *unit;

    if ((g_self_required() /* || side prop? */)
    	&& side != indepside
	&& side->self_unit == NULL) {
	for_all_side_units(side, unit) {
	    if (u_can_be_self(unit->type)
		&& in_play(unit)
		&& completed(unit)) {
		side->self_unit = unit;
		break;
	    }	
	}
    }
}

/* Initialize basic viewing structures for a side.  This happens when
   the side is created (during/after reading but before synthesis).
   At this time we're mostly allocating space; filling in the views
   happens once all the units are in position.  We don't know the
   final values of parameters like side->see_all, so we have to
   allocate some data that might never be used. */

int
init_view(Side *side)
{
    int terrainset = FALSE, t, u, m;

    /* Be sure that we're not trying to set this up too soon. */
    check_area_shape();
    /* Allocate terrain view layers. */
    if ((!g_terrain_seen() || !g_see_terrain_always())
	&& side->terrview == NULL) {
	side->terrview = malloc_area_layer(char);
	if (!g_see_terrain_always()) {
	    side->terrviewdate = malloc_area_layer(short);
	    /* The terrview also holds the "have we seen it" flag, so
	       it must always be allocated, but the aux terrain view
	       need not be. */
	    if (numcelltypes < numttypes) {
		side->auxterrview =
		  (char **) xmalloc(numttypes * sizeof(short *));
		for_all_terrain_types(t) {
		    if (!t_is_cell(t)) {
			side->auxterrview[t] = malloc_area_layer(char);
		    }
		}
		if (0 /* aux terrain seen separately from main terrain */) {
		    side->auxterrviewdate =
		      (short **) xmalloc(numttypes * sizeof(short *));
		    for_all_terrain_types(t) {
			if (!t_is_cell(t)) {
			    side->auxterrviewdate[t] =
			      malloc_area_layer(short);
			}
		    }
		}
	    }
	}
    } else {
	terrainset = TRUE;
    }
    /* Allocate material view layers. */
    if (any_material_views < 0) {
	any_material_views = FALSE;
	for_all_material_types(m) {
	    for_all_terrain_types(t) {
		if (tm_storage_x(t, m) > 0 && tm_see_always(t, m) == 0) {
		    any_material_views = TRUE;
		    if (any_material_views_by_m == NULL)
		      any_material_views_by_m = (char *)xmalloc(nummtypes);
		    any_material_views_by_m[m] = TRUE;
		    break;
		}
	    }
	}
    }
    if (any_material_views) {
	if (side->materialview == NULL)
	  side->materialview = (short **) xmalloc(nummtypes * sizeof(short *));
	if (side->materialviewdate == NULL)
	  side->materialviewdate =
	    (short **) xmalloc(nummtypes * sizeof(short *));
	for_all_material_types(m) {
	    if (any_material_views_by_m[m]) {
		if (side->materialview[m] == NULL) {
		    side->materialview[m] = malloc_area_layer(short);
		}
		if (side->materialviewdate[m] == NULL) {
		    side->materialviewdate[m] = malloc_area_layer(short);
		}
	    }
	}
    }
    /* Allocate weather view layers. */
    if (any_temp_variation && !g_see_weather_always()) {
	if (side->tempview == NULL) {
	    side->tempview = malloc_area_layer(short);
	}
	if (side->tempviewdate == NULL) {
	    side->tempviewdate = malloc_area_layer(short);
	}
    }
    if (any_clouds && !g_see_weather_always()) {
	if (side->cloudview == NULL) {
	    side->cloudview = malloc_area_layer(short);
	}
	if (side->cloudbottomview == NULL) {
	    side->cloudbottomview = malloc_area_layer(short);
	}
	if (side->cloudheightview == NULL) {
	    side->cloudheightview = malloc_area_layer(short);
	}
	if (side->cloudviewdate == NULL) {
	    side->cloudviewdate = malloc_area_layer(short);
	}
    }
    if (any_wind_variation && !g_see_weather_always()) {
	if (side->windview == NULL) {
	    side->windview = malloc_area_layer(short);
	}
	if (side->windviewdate == NULL) {
	    side->windviewdate = malloc_area_layer(short);
	}
    }
    /* Allocate the vision coverage cache. */
    if (side->coverage == NULL) {
	side->coverage = malloc_area_layer(short);
    }
    /* Allocate vision altitude coverage if needed */
    if (any_los < 0) {
	any_los = FALSE;
	for_all_unit_types(u) {
	    if (!u_can_see_behind(u)) {
		any_los = TRUE;
		break;
	    }
	}
    }
    if (side->alt_coverage == NULL && any_los) {
	side->alt_coverage = malloc_area_layer(short);
    }
    return terrainset;
}

/* Calculate the centroid of all the starting units. */

void
calc_start_xy(Side *side)
{
    int num = 0, sumx = 0, sumy = 0;
    Unit *unit;

    for_all_side_units(side, unit) {
	if (in_play(unit)) {
	    sumx += unit->x;  sumy += unit->y;
	    ++num;
	}
    }
    if (num > 0) {
	side->startx = wrapx(sumx / num);  side->starty = sumy / num;
    }
}

/* Given a side, get its "number", which is same as its "id". */

int
side_number(Side *side)
{
    return side->id;
}

/* The inverse function - given a number, figure out which side it is.
   Returns NULL for any failure. */
/* (should cache values in a table, do direct lookup) */

Side *
side_n(int n)
{
    Side *side;

    for_all_sides(side)
      if (side->id == n)
        return side;
    return NULL;
}

Side *
find_side_by_name(char *str)
{
    Side *side;

    if (empty_string(str))
      return NULL;
    for_all_sides(side) {
	if (!empty_string(side->name) && strcmp(side->name, str) == 0)
	  return side;
	if (!empty_string(side->noun) && strcmp(side->noun, str) == 0)
	  return side;
	if (!empty_string(side->pluralnoun) && strcmp(side->pluralnoun, str) == 0)
	  return side;
	if (!empty_string(side->adjective) && strcmp(side->adjective, str) == 0)
	  return side;
    }
    return NULL;
}

Side *
parse_side_spec(char *str)
{
    int s;
    char *reststr;

    if (isdigit(str[0])) {
	s = strtol(str, &reststr, 10);
	if (between(1, s, numsides)) {
	    return side_n(s);
	}
    } else {
	return find_side_by_name(str);
    }
    return NULL;
}

/* This is true when one side controls another. */

int
side_controls_side(Side *side, Side *side2)
{
    return (side == side2 || side2->controlled_by == side);
}

short *max_control_ranges;

static int controller_here(int x, int y);

static int
controller_here(int x, int y)
{
    Unit *unit2;

    if (distance(x, y, tmpunit->x, tmpunit->y) < 2)
      return FALSE;
    for_all_stack(x, y, unit2) {
	if (side_controls_unit(tmpside, unit2)
	    && probability(uu_control(unit2->type, tmpunit->type)))
	  return TRUE;
    }
    return FALSE;
}

/* This is true if the given side may operate on the given unit. */

int
side_controls_unit(Side *side, Unit *unit)
{
    int dir, x1, y1;
    Unit *unit2;

    if (unit == NULL)
      return FALSE;
    if (is_designer(side))
      return TRUE;
    /* Also give debuggers full control. */
    if (Debug || DebugG || DebugM)
      return TRUE;
    if (side_controls_side(side, unit->side)) {
	/* The *unit* side must have the tech to use the unit, the controlling
	   side would have to actually take over the unit if it wants to use
	   its tech level to control the unit. */
	if (unit->side != NULL
	    && unit->side->tech[unit->type] < u_tech_to_use(unit->type))
	  return FALSE;
	if (u_direct_control(unit->type) || unit == side->self_unit)
	  return TRUE;
	/* Unit is not under direct control of the side; look for a
	   controlled unit that can control this unit. */
	if (max_control_ranges == NULL) {
	    int u1, u2;

	    max_control_ranges = (short *) xmalloc(numutypes * sizeof(short));
	    for_all_unit_types(u2) {
		max_control_ranges[u2] = -1;
		for_all_unit_types(u1) {
		    max_control_ranges[u2] =
		      max(max_control_ranges[u2], uu_control_range(u1, u2));
		}
	    }
	}
	if (max_control_ranges[unit->type] >= 0) {
	    for_all_stack(unit->x, unit->y, unit2) {
		if (unit != unit2
		    && side_controls_unit(side, unit2)
		    && probability(uu_control_at(unit2->type, unit->type)))
		  return TRUE;
	    }
	    /* (what about occupants that could be controllers?) */
	}
	if (max_control_ranges[unit->type] >= 1) {
	    for_all_directions(dir) {
		if (interior_point_in_dir(unit->x, unit->y, dir, &x1, &y1)) {
		    for_all_stack(x1, y1, unit2) {
			if (side_controls_unit(side, unit2)
			    && probability(uu_control_adj(unit2->type, unit->type)))
			  return TRUE;
		    }
		}
	    }
	}
	if (max_control_ranges[unit->type] >= 2) {
	    tmpside = side;
	    tmpunit = unit;
	    return search_around(unit->x, unit->y,
				 max_control_ranges[unit->type],
	    			 controller_here, &x1, &y1, 1);
	}
    }
    return FALSE;
}

/* This is true if the given side may examine the given unit. */

int
side_sees_unit(Side *side, Unit *unit)
{
    int x = -1, y = -1;

    if (unit == NULL)
      return FALSE;
    /* If the side is omniscient. */
    if (side->see_all || side->show_all)
      return TRUE;
    /* If the unit is 'see-always' and on known terrain in the arena. */
    x = unit->x;  y = unit->y;
    if (u_see_always(unit->type) && inside_area(x, y)
	&& (UNSEEN != terrain_view(side, x, y)))
      return TRUE;
    /* If the side controls the unit's side. */
    if (side_controls_side(side, unit->side))
      return TRUE;
    /* If the unit's side trusts the side. */
    if (trusted_side(unit->side, side))
      return TRUE;
    /* If the game is over. */
    if (endofgame)
      return TRUE;      
    /* If the side's people can see the unit. */
    if (Xconq::people_always_see && people_sides_defined() 
	&& (people_side_at(unit->x, unit->y) == side->id))
      return TRUE;
    /* Else, side cannot see. */
    return FALSE;
}

/* This is true if the side can see the image of a unit. */

int
side_sees_image(Side *side, Unit *unit)
{
    UnitView *uview;
    int x = -1, y = -1;

    if (side == NULL)
      run_error("NULL side to side_sees_image");
    if (unit == NULL)
      return FALSE;
    if (side->see_all || side->show_all)
      return TRUE;
    /* If the unit is 'see-always' and on known terrain in the arena. */
    x = unit->x;  y = unit->y;
    if (u_see_always(unit->type) && inside_area(x, y)
	&& (UNSEEN != terrain_view(side, x, y)))
      return TRUE;
    if (side_controls_side(side, unit->side))
      return TRUE;
#if (0)
    if (u_see_always(unit->type))
      return TRUE;
#endif
    for_all_view_stack_with_occs(side, unit->x, unit->y, uview) {
	if (unit->type == uview->type 
	    && unit->side == side_n(uview->siden)
	    && unit->id == uview->id) {
		return TRUE;
    	}
    }
    return FALSE;
}

/* Test whether the occupants of the given unit are visible to the
   given side. */

int
occupants_visible(Side *side, Unit *unit)
{
    if (unit->occupant == NULL)
      return FALSE;
    /* If the side is omniscient. */
    if (side->see_all)
      return TRUE;
    /* If the side sees the transport and the occupants are to be revealed. */
    if (side_sees_unit(side, unit) && u_see_occupants(unit->type))
      return TRUE;
    /* If the side owns a seeing unit in the transport. */
    /* (TODO: Handle the case of a trusted unit providing info.) */
    if (side_owns_viewer_in_unit(side, unit))
      return TRUE;
    return FALSE;
}

int
num_units_in_play(Side *side, int u)
{
    int num = 0;
    Unit *unit;

    if (side != NULL && side->ingame) {
      for_all_side_units(side, unit) {
	if (unit->type == u
	    && in_play(unit)
	    && completed(unit))
	  ++num;
      }
    }
    return num;
}

int
num_units_incomplete(Side *side, int u)
{
    int num = 0;
    Unit *unit;

    if (side != NULL && side->ingame) {
      for_all_side_units(side, unit) {
	if (unit->type == u 
	    && in_play(unit) 
	    && !completed(unit)) {
	    ++num;
      	}
      }
    }
    return num;
}

Unit *
find_next_unit(Side *side, Unit *prevunit)
{
    Unit *unit = NULL;

    if (side != NULL) {
	if (prevunit == NULL)
	  prevunit = side->unithead;
	for (unit = prevunit->next; unit != prevunit; unit = unit->next) {
	    if (is_unit(unit) && unit->id > 0
		&& alive(unit)
		&& inside_area(unit->x, unit->y)) {
		return unit;
	    }
	}
    }
    return NULL;
}

Unit *
find_prev_unit(Side *side, Unit *nextunit)
{
    Unit *unit = NULL;

    if (side != NULL) {
	if (nextunit == NULL) nextunit = side->unithead;
	for (unit = nextunit->prev; unit != nextunit; unit = unit->prev) {
	    if (is_unit(unit) && unit->id > 0
		&& alive(unit)
		&& inside_area(unit->x, unit->y)) {
		return unit;
	    }
	}
    }
    return NULL;
}

Unit *
find_next_actor(Side *side, Unit *prevunit)
{
    Unit *unit = NULL;

    if (side != NULL) {
	if (prevunit == NULL) prevunit = side->unithead;
	for (unit = prevunit->next; unit != prevunit; unit = unit->next) {
	    if (is_unit(unit) && unit->id > 0
		&& alive(unit)
		&& inside_area(unit->x, unit->y)
		&& (unit->act && unit->act->initacp)) {
		return unit;
	    }
	}
    }
    return NULL;
}

Unit *
find_prev_actor(Side *side, Unit *nextunit)
{
    Unit *unit = NULL;

    if (side != NULL) {
	if (nextunit == NULL)
	  nextunit = side->unithead;
	for (unit = nextunit->prev; unit != nextunit; unit = unit->prev) {
	    if (is_unit(unit) && unit->id > 0
		&& alive(unit)
		&& inside_area(unit->x, unit->y)
		&& (unit->act && unit->act->initacp)) {
		return unit;
	    }
	}
    }
    return NULL;
}

Unit *
find_next_mover(Side *side, Unit *prevunit)
{
    Unit *unit = NULL;

    if (side != NULL) {
	if (prevunit == NULL)
	  prevunit = side->unithead;
	for (unit = prevunit->next; unit != prevunit; unit = unit->next) {
	    if (is_unit(unit) && unit->id > 0
		&& alive(unit)
		&& inside_area(unit->x, unit->y)
		&& has_acp_left(unit)) {
		return unit;
	    }
	}
    }
    return NULL;
}

Unit *
find_prev_mover(Side *side, Unit *nextunit)
{
    Unit *unit = NULL;

    if (side != NULL) {
	if (nextunit == NULL)
	  nextunit = side->unithead;
	for (unit = nextunit->prev; unit != nextunit; unit = unit->prev) {
	    if (is_unit(unit) && unit->id > 0
		&& alive(unit)
		&& inside_area(unit->x, unit->y)
		&& has_acp_left(unit)) {
		return unit;
	    }
	}
    }
    return NULL;
}

Unit *
find_next_awake_mover(Side *side, Unit *prevunit)
{
    Unit *unit = NULL;

    if (side != NULL) {
	if (prevunit == NULL)
	  prevunit = side->unithead;
	for (unit = prevunit->next; unit != prevunit; unit = unit->next) {
	    if (is_unit(unit)
		&& unit->id > 0
		&& alive(unit)
		&& inside_area(unit->x, unit->y)
		&& has_acp_left(unit)
		&& (unit->plan
		    && !unit->plan->asleep
		    && !unit->plan->reserve
		    && !unit->plan->delayed)) {
		return unit;
	    }
	}
    }
    return NULL;
}

Unit *
find_prev_awake_mover(Side *side, Unit *nextunit)
{
    Unit *unit = NULL;

    if (side != NULL) {
	if (nextunit == NULL) nextunit = side->unithead;
	for (unit = nextunit->prev; unit != nextunit; unit = unit->prev) {
	    if (is_unit(unit) && unit->id > 0
		&& alive(unit)
		&& inside_area(unit->x, unit->y)
		&& has_acp_left(unit)
		&& (unit->plan
		    && !unit->plan->asleep
		    && !unit->plan->reserve
		    && !unit->plan->delayed)) {
		return unit;
	    }
	}
    }
    return NULL;
}

/* Compute the total number of action points available to the side at the
   beginning of the turn. */

int
side_initacp(Side *side)
{
    int totacp = 0;
    Unit *unit;

    for_all_side_units(side, unit) {
	if (alive(unit) && unit->act) {
	    totacp += unit->act->initacp;
	}
    }
    return totacp;
}

/* Return the total of acp still unused by the side. */

int
side_acp(Side *side)
{
    int acpleft = 0;
    Unit *unit;

    for_all_side_units(side, unit) {
	if (alive(unit) && unit->act) {
	    acpleft += unit->act->acp;
	}
    }
    return acpleft;
}

int
side_acp_reserved(Side *side)
{
    int acpleft = 0;
    Unit *unit;

    for_all_side_units(side, unit) {
	if (alive(unit) && unit->act) {
		if (unit->plan
		    && (unit->plan->reserve || unit->plan->asleep)) {
	    	acpleft += unit->act->acp;
	    }
	}
    }
    return acpleft;
}

#if 0  /* not presently needed, but possibly useful */
/* Return the total of acp still expected to be used by a human player. */

int
side_acp_human(Side *side)
{
    int acpleft = 0;
    Unit *unit;

    if (!side_has_display(side))
      return acpleft;

    for_all_side_units(side, unit) {
	if (unit != NULL
	    && alive(unit)
	    && inside_area(unit->x, unit->y)
	    && has_acp_left(unit)
	    && (unit->plan
		&& !unit->plan->asleep
		&& !unit->plan->reserve
		&& unit->plan->tasks == NULL)) {
		acpleft += unit->act->acp;
	}
    }
    return acpleft;
}
#endif

/* (note that technology could be "factored out" of a game if all sides
   reach max tech at some point) */
/* (otherwise should compute once and cache) */
/* (note that once tech factors out, can never factor in again) */

int
using_tech_levels(void)
{
    int u;
    Side *side;

    for_all_sides(side) {
	for_all_unit_types(u) {
	    if (side->tech[u] < u_tech_max(u))
	      return TRUE;
	}
    }
    return FALSE;
}

/* Take the given side out of the game entirely.  This does not imply
   winning or losing, nor does it take down the side's display or AI. */

void
remove_side_from_game(Side *side)
{
    Side *side2;

    /* Officially flag this side as being no longer in the game. */
    side->ingame = FALSE;
    /* Update everybody on this. */
    for_all_sides(side2) {
	update_side_display(side2, side, TRUE);
    }
    /* Note that we no longer try to remove any images from other sides'
       views, because even with the side gone, the images may be useful
       information about where its units had gotten to.  For instance,
       if a unit had been captured shortly before the side lost, then its
       image might still correspond to an actual unit, with only its side
       changed, and other sides may want to investigate for themselves. */
}

int
num_displayed_sides(void)
{
    int n = 0;
    Side *side;

    for_all_sides(side) {
	if (side_has_display(side))
	  ++n;
    }
    return n;
}

void
update_side_display_all_sides(Side *side, int rightnow)
{
    Side *side2;

    for_all_sides(side2) {
	if (side_has_display(side2)) {
	    update_side_display(side2, side, rightnow);
	}
    }
}

void
set_side_name(Side *side, Side *side2, char *newname)
{
    side2->name = newname;
    update_side_display_all_sides(side2, TRUE);
}

void
set_side_longname(Side *side, Side *side2, char *newname)
{
    side2->longname = newname;
    update_side_display_all_sides(side2, TRUE);
}

void
set_side_shortname(Side *side, Side *side2, char *newname)
{
    side2->shortname = newname;
    update_side_display_all_sides(side2, TRUE);
}

void
set_side_noun(Side *side, Side *side2, char *newname)
{
    side2->noun = newname;
    update_side_display_all_sides(side2, TRUE);
}

void
set_side_pluralnoun(Side *side, Side *side2, char *newname)
{
    side2->pluralnoun = newname;
    update_side_display_all_sides(side2, TRUE);
}

void
set_side_adjective(Side *side, Side *side2, char *newname)
{
    side2->adjective = newname;
    update_side_display_all_sides(side2, TRUE);
}

void
set_side_emblemname(Side *side, Side *side2, char *newname)
{
    side2->emblemname = newname;
    update_side_display_all_sides(side2, TRUE);
}

void
set_side_colorscheme(Side *side, Side *side2, char *newname)
{
    side2->colorscheme = newname;
    update_side_display_all_sides(side2, TRUE);
}

#ifdef DESIGNERS

/* Implement the transition from regular side to a side that can
   alter the game's state directly (aka "designer"). */

void
become_designer(Side *side)
{
    Side *side2;

    if (side->designer)
      return;
    side->designer = TRUE;
    ++numdesigners;
    /* Designers have godlike power in the game, so mark it (permanently)
       as no longer a normal game. */
    compromised = TRUE;
    /* Designers get to see everything if they want, but it's useful
       to see a player view sometimes, so give designers the ability
       to toggle the accurate display.  Note that we must leave the
       side's see_all at its original value, so that unit views
       continue to be constructed and manipulated properly. */
    if (!side->see_all)
      side->may_set_show_all = TRUE;
    side->show_all = TRUE;
    place_legends(side);
    update_everything();
    /* Let everybody know of the change in status. */
    notify_all("%s IS NOW A DESIGNER.", short_side_title(side));
    for_all_sides(side2) {
	update_side_display(side2, side, TRUE);
    }
}

/* Give up the godlike powers. */

void
become_nondesigner(Side *side)
{
    Side *side2;

    if (!side->designer)
      return;
    side->designer = FALSE;
    --numdesigners;
    /* Go back to the original viewing capabilities. */
    side->may_set_show_all = FALSE;
    side->show_all = side->see_all;
    place_legends(side);
    update_everything();
    notify_all("%s is no longer a designer.", short_side_title(side));
    for_all_sides(side2) {
	update_side_display(side2, side, TRUE);
    }
}

#endif /* DESIGNERS */

/* Return TRUE if side1 trusts side2.  Note that the nature of trust
   is such that we don't want to check that the trust is mutual
   (although in practice it would be a foolish player who would trust
   a side that doesn't reciprocate!) */

int
trusted_side(Side *side1, Side *side2)
{
    if (side1 == side2)
      return TRUE;
    if (side1 == NULL || side2 == NULL || side1->trusts == NULL)
      return FALSE;
    return (side1->trusts[side2->id]);
}

void
set_trust(Side *side, Side *side2, int val)
{
    int oldval;
    Side *side3;

    if (side == side2)
      return;
    if (side->trusts == NULL)
      return;
    oldval = side->trusts[side2->id];
    if (val == oldval)
      return;
    side->trusts[side2->id] = val;
    /* This is a major change that all other sides will know about. */
    for_all_sides(side3) {
	if (active_display(side3)) {
	    /* (should be handled by nlang.c) */
	    notify(side3, "%s %s %s",
		   side_name(side),
		   (val ? "now trusts" : "no longer trusts"),
		   side_name(side2));
	    update_side_display(side3, side, FALSE);
	    update_side_display(side3, side2, TRUE);
	}
    }
    /* (should update views, list of units known about, etc) */
    /* (if cell goes from exact to only recorded, update display anyhow) */
}

void
set_controlled_by(Side *side, Side *side2, int val)
{
    char tmpbuf[BUFSIZE];
    int changed = FALSE;

    if (side == NULL || side2 == NULL || side == side2)
      return;
    if (val) {
	/* Make side be controlled. */
	if (side->controlled_by == NULL) {
	    side->controlled_by = side2;
	    /* (should rework unit movement vectors now?) */
	    changed = TRUE;
	} else {
	    /* Can't happen. */
	}
    } else {
	/* Make side be uncontrolled. */
	if (side->controlled_by == side2) {
	    side->controlled_by = NULL;
	    /* (should rework unit movement vectors now?) */
	    changed = TRUE;
	} else {
	    /* Can't happen. */
	}
    }
    if (changed) {
	strcpy(tmpbuf, short_side_title(side2));
	/* (should be handled by nlang.c) */
	notify_all("%s%s control%s %s now.",
		   tmpbuf,
		   (val ? "" : " no longer"),
		   (short_side_title_plural_p(side2) ? "" : "s"),
		   short_side_title(side));
    }
}

/* What interfaces should use to tweak the autofinish flag. */

void
set_autofinish(Side *side, int value)
{
    side->autofinish = value;
}

/* What interfaces should use to tweak the autoresearch flag. */

void
set_autoresearch(Side *side, int value)
{
    side->autoresearch = value;
}

/* Being at war requires only ones of the sides to consider itself so. */

/* (Should the other side's relationships be tweaked also?) */

int
enemy_side(Side *side1, Side *side2)
{
    if (trusted_side(side1, side2))
      return FALSE;
    return TRUE;
}

/* A formal alliance requires the agreement of both sides. */

int
allied_side(Side *side1, Side *side2)
{
    if (trusted_side(side1, side2))
      return TRUE;
    return FALSE;
}

/* Neutralness is basically anything else. */

int
neutral_side(Side *side1, Side *side2)
{
    return (!enemy_side(side1, side2) && !allied_side(side1, side2));
}

void
set_willing_to_save(Side *side, int flag)
{
    int oldflag = side->willingtosave;
    Side *side2;

    if (flag != oldflag) {
	side->willingtosave = flag;
	/* Inform everybody of our willingness to save. */
	for_all_sides(side2) {
	    if (active_display(side2)) {
		notify(side2, "%s is%s willing to save the game.",
			      side_name(side), (flag ? "" : " not"));
		update_side_display(side2, side, TRUE);
	    }
	}
    }
}

void
set_willing_to_draw(Side *side, int flag)
{
    int oldflag = side->willingtodraw;
    Side *side2;

    if (flag != oldflag) {
	side->willingtodraw = flag;
	/* Inform everybody of our willingness to draw. */
	for_all_sides(side2) {
	    if (active_display(side2)) {
		notify(side2, "%s is%s willing to declare the game a draw.",
			      side_name(side), (flag ? "" : " not"));
		update_side_display(side2, side, TRUE);
	    }
	}
    }
}

/* Set the self-unit of the given side.  This is only called when done at
   the direction of the side, and may fail if the side can't change its
   self-unit voluntarily. */

void
set_side_self_unit(Side *side, Unit *unit)
{
    if (!in_play(unit))
      return;
    if (side->self_unit
        && in_play(side->self_unit)
        && !u_self_changeable(side->self_unit->type))
      return;
    side->self_unit = unit;
    /* (should update some part of display?) */
}

/* Message-forwarding function. */

void
send_message(Side *side, SideMask sidemask, char *str)
{
    char *sidedesc, buf[BUFSIZE];
    SideMask testmask;
    Side *side2, *sender;

    sender = side;
    testmask = add_side_to_set(side, NOSIDES);
    if (sidemask == NOSIDES || empty_string(str)) {
	notify(side, "You say nothing.");
	return;
    } else if (sidemask == testmask) {
	notify(side, "You mumble to yourself.");
	return;
    } else if (sidemask == ALLSIDES) {
	notify(side, "You broadcast \"%s\" to all.", str);
    } else {
    	sidedesc = sidemask_desc(buf, sidemask);
	notify(side, "You send \"%s\" to \"%s\".", str, sidedesc);
    }
    /* Handle messages that are to have anonymous senders. */
    if (strlen(str) > 6  && strncmp(str, "(anon)", 6) == 0) {
	str += 6;
	sender = NULL;
    }
    for_all_sides(side2) {
	if (side2 != side && side_in_set(side2, sidemask)) {
	    receive_message(side2, sender, str);
	}
    }
}

/* Handle the receipt of a message.  Some messages may result in specific
   actions, but the default is just to forward to AIs and displays. */

void
receive_message(Side *side, Side *sender, char *str)
{
    /* Look for specially-recognized messages. */
    if (strcmp("%reveal", str) == 0) {
	reveal_side(sender, side, NULL);
    } else {
	/* Give the message to interface if present. */
	if (side_has_display(side)) {
	    update_message_display(side, sender, str, TRUE);
	}
	/* Also give the message to any local AI. */
	if (side_has_ai(side)) {
	    ai_receive_message(side, sender, str);
	}
    }
}

/* General method for passing along info about one side to another. */

void
reveal_side(Side *sender, Side *recipient, int *types)
{
    int x, y;
    Unit *unit;

    if (sender == NULL)
      return;
    if (g_see_all())
      return;
    if (!g_terrain_seen()) {
	for_all_cells(x, y) {
	    if (terrain_view(sender, x, y) != UNSEEN
	        && terrain_view(recipient, x, y) == UNSEEN) {
	        set_terrain_view(recipient, x, y, terrain_view(sender, x, y));
	        /* (should update unit views also) */
	    }
	}
    }
    for_all_side_units(sender, unit) {
	if (in_play(unit) && (types == NULL || types[unit->type])) {
	    see_exact(recipient, unit->x, unit->y);
	}
    }
    /* Whether the amount of data is small or large, there is little value
       in being selective about updates - just redo entire maps. */
    update_area_display(recipient);
}

/* Modify doctrine according to a specification. */

void
set_doctrine(Side *side, char *spec)
{
    int u;
    char *arg, *arg2, *rest, substr[BUFSIZE];
    Doctrine *doctrine;

    rest = get_next_arg(spec, substr, &arg);
    if ((doctrine = find_doctrine_by_name(arg)) != NULL) {
	/* Found a specific named doctrine. */
    } else if ((u = utype_from_name(arg)) != NONUTYPE) {
	doctrine = side->udoctrine[u];
    } else if (strcmp(arg, "default") == 0) {
	doctrine = side->default_doctrine;
    }
    if (doctrine->locked) {
	/* (should mention name of doctrine) */
	notify(side, "This doctrine cannot be changed!");
	return;
    }
    rest = get_next_arg(rest, substr, &arg);
    if (strcmp(arg, "resupply") == 0) {
	rest = get_next_arg(rest, substr, &arg2);
	doctrine->resupply_percent = atoi(arg2);
    } else if (strcmp(arg, "rearm") == 0) {
	rest = get_next_arg(rest, substr, &arg2);
	doctrine->rearm_percent = atoi(arg2);
    } else if (strcmp(arg, "repair") == 0) {
	rest = get_next_arg(rest, substr, &arg2);
	doctrine->repair_percent = atoi(arg2);
    } else if (strcmp(arg, "run") == 0) {
	notify(side, "Can't modify construction runs yet");
    } else {
	notify(side, "\"%s\" not a known doctrine property", arg);
	notify(side, "Known ones are: ask, resupply, rearm, repair, run");
    }
}

void
set_side_research_topic(Side *side, int a)
{
    side->research_topic = a;
}

void
set_side_research_goal(Side *side, int a)
{
    side->research_goal = a;
}

void
set_side_startx(Side *side, int x)
{
    side->startx = x;
}

void
set_side_starty(Side *side, int y)
{
    side->starty = y;
}

/* Vision. */

UnitView *
unit_view_at(Side *side, int x, int y)
{
    int hash;
    UnitView *uv;

    /* This might be called during synthesis, before sides are set up. */
    if (side->unit_views == NULL)
      return NULL;
    hash = (wrapx(x) ^ y) % VIEW_HASH_SIZE;
    for (uv = side->unit_views[hash]; uv != NULL; uv = uv->nextinhash) {
	if (wrapx(x) == uv->x 
	    && y == uv->y
	    && uv->transport == NULL) {
		return uv;
    	}
    }
    return NULL;
}

UnitView *
unit_view_next(Side *side, int x, int y, UnitView *uview)
{
    UnitView *uv2;

    /* If we are looking among the side views. */
    if (side) {
	for (uv2 = uview->nextinhash; uv2 != NULL; uv2 = uv2->nextinhash) {
	    if (wrapx(x) == uv2->x && y == uv2->y && uv2->transport == NULL) 
	      return uv2;
	}
    }
    /* If we are looking among master views. */
    else
      return uview->nexthere;
    return NULL;
}

/* Look for an existing unit view that is tied to the given unit. */

UnitView *
find_unit_view(Side *side, Unit *unit)
{
    UnitView *uv, *ov;
    int i;

    if (!side) {
	update_master_uview(unit, FALSE);
	return unit->uview;
    }
    if (side->unit_views == NULL) {
	return NULL;
    }
    for (i = 0; i < VIEW_HASH_SIZE; ++i) {
	for (uv = side->unit_views[i]; uv != NULL; uv = uv->nextinhash) {
	    if (uv->id == unit->id) {
		return uv;
	    }
	    /* Also check all the occupant views. */
	    for_all_occupant_views_with_occs(uv, ov) {
		if (ov->id == unit->id) {
		    return ov;
		}
	    }
	}
    }
    return NULL;
}

/*! \todo This function will need to be modified to behave differently for 
	    clients and servers once we are about to do the client-server 
	    separation. For clients, it will actually send a network query 
	    to the server asking if we can get back a shadow unit for the 
	    unit handle stored with the uview. NULL will be returned by the 
	    function if the network response is no unit, else a pointer to 
	    the shadow unit will be returned. For the server, it will simply 
	    return a pointer to the actual unit associated with the uview.
	    For now, we default to the server behavior.
*/

Unit *
query_unit_from_uview(UnitView *uview)
{
    return view_unit(uview);
}

/* Get the updated master uview for a given unit. */
/*! \note This function (as well as a number of others) will have to be 
	  changed to take a unit handle of some sort (perhaps unit ID) when 
	  true client-server separation is done. There will also be two 
	  separate functions: one that does the client query and gets back 
	  the mashalled uview, and one that handles the query on the server 
	  side, and marshals an appropriate response back to the client. */
/* {Client and Arbiter Function} */

UnitView *
query_uvstack_from_unit(Unit *unit)
{
    /* If an out-of-play unit was given, then return a NULL uview. */
    if (!in_play(unit))
      return NULL;
    /* Update the master uview first. */
    update_master_uview(unit, FALSE);
    /* Return the master uview. */
    return unit->uview;
}

/* Get the uvstack for a given location from an omniscient perspective. */
/*! \note See note for 'query_uvstack_from_unit'. */
/* {Client and Arbiter Function} */

UnitView *
query_uvstack_at(int x, int y)
{
    Unit *unit = NULL;

    /* If x and y are out-of-area, then return a NULL uview. */
    if (!inside_area(x, y))
      return NULL;
    /* Get the unit at given location, if any. */
    unit = unit_at(x, y);
    /* If no unit is there, then return NULL. */
    if (!unit)
      return NULL;
    /* Else, get uview stack from unit. */
    return query_uvstack_from_unit(unit);
}

/* Find out if an uview is in the correct position in a stack (cell or 
   transport), relative to another uview. */

int
is_at_correct_uvstack_position(UnitView *uview, UnitView *uview2)
{
    int u = NONUTYPE, u2 = NONUTYPE;
    int uso = -1, u2so = -1;

    assert_error(uview, "Attempted to place a NULL uview in a uvstack");
    assert_error(uview2, "Attempted to compare an uview in a NULL uvstack");
    u = uview->type;
    u2 = uview2->type;
    uso = u_stack_order(u);
    u2so = u_stack_order(u2);
    /* Enforce side ordering 1st. */
    /*! \todo Add support for holding units belonging to the dside first in 
	      the stack, last in the stack, or in the natural side order. 
	      Currently, only the natural side order is supported. */
    if (uview->siden > uview2->siden)
      return FALSE;
    if (uview->siden < uview2->siden)
      return TRUE;
    /* Enforce stack ordering 2nd. */
    if (uso < u2so)
      return FALSE;
    if (uso > u2so)
      return TRUE;
    /* Enforce utype ordering 3rd. */
    if (u > u2)
      return FALSE;
    if (u < u2)
      return TRUE;
    /* Enforce uview ID ordering 4th. */
    if (uview->id < uview2->id)
      return TRUE;
    return FALSE;
}

/* Add uview to the given transport's uvstack. */

void
add_uview_to_uvstack(UnitView *uview, UnitView *tsptview)
{
    UnitView *topview = NULL, *occview = NULL, *nextview = NULL, 
	     *prevview = NULL;

    assert_error(uview, "Attempted to add a NULL uview to a uvstack");
    assert_error(tsptview, 
		 "Attempted to add an uview to a NULL transport uvstack");
    topview = tsptview->occupant;
    if (topview) {
	/* Insert uview into the occupant list at its correct position. */
	for_all_occupant_views(tsptview, occview) {
	    if (is_at_correct_uvstack_position(uview, occview)) {
		nextview = occview;
#if (0)
		if (uview == nextview->nexthere)
		  nextview->nexthere = NULL;
#endif
		if (occview == topview) 
		  topview = uview;
		break;
	    }
	    prevview = occview;
	}
	if (prevview != NULL) {
	    prevview->nexthere = uview;
	}
    } else {
	topview = uview;
    }
    uview->nexthere = nextview;
    tsptview->occupant = topview;
}

/* Initialize and fill out an uview from unit data. */

void
fill_out_uview(Unit *unit, Side *side, UnitView *uview)
{
    /* Sanity checks. */
    assert_error(unit, "Attempted to fill out an unit view with a NULL unit");
    assert_error(uview, "Attempted to fill out a NULL unit view");
    /* Fill out uview. */
    uview->observer = (side ? side->id : -1);
    uview->unit = unit;
    uview->x = unit->x;
    uview->y = unit->y;
    uview->siden = unit->side->id;
    uview->type = unit->type;
    uview->size = unit->size;
    uview->id = unit->id;
    uview->imf = unit->imf;
    uview->image_name = unit->image_name;
    uview->complete = completed(unit);
    uview->occupant = NULL;
    uview->transport = NULL;
    uview->nextinhash = NULL;
    uview->nexthere = NULL;
    uview->vnext = NULL;
}

/* Update the master uview of an unit. */

void
update_master_uview(Unit *unit, int lookabove)
{
    UnitView *uview = NULL;

    /* Return immediately if no unit to update. */
    if (!unit)
      return;
    /* If we are to look above and the unit has a transport, 
       then update the transport's view first. Once the top of the transport 
       chain is reached, then occs (including this unit) will be filled out, 
       and so there is no need to duplicate work. Thus, return afterword. */
    if (lookabove && unit->transport) {
	update_master_uview(unit->transport, TRUE);
	return;
    }
    /* Allocate the uview, if it does already exist. */
    uview = unit->uview;
    if (!uview) {
	uview = (UnitView *)xmalloc(sizeof(UnitView));
	unit->uview = uview;
    }
    /* Fill out the uview from unit. */
    fill_out_uview(unit, NULL, uview);
    /* Link with transport, if any. */
    uview->transport = (unit->transport ? unit->transport->uview : NULL);
    /* Recurse into and link with first occ, if any. */
    /* This will update all of the occ's occs and next neighbors. */
    if (unit->occupant)
      update_master_uview(unit->occupant, FALSE);
    uview->occupant = (unit->occupant ? unit->occupant->uview : NULL);
    /* Recurse into and link with next neighbor, if any. */
    /* This will update all of the neighbor's occs and next neighbors. */
    /* The 'nextinhash' field is also updated with the value of the next 
       neighbor so that 'unit_view_next' will not be broken. */
    if (unit->nexthere)
      update_master_uview(unit->nexthere, FALSE);
    uview->nexthere = (unit->nexthere ? unit->nexthere->uview : NULL);
    uview->nextinhash = uview->nexthere;
}

/* Add a new unit view to the side's unit views. */

UnitView *
add_unit_view(Side *side, Unit *unit)
{
    int changed = FALSE, rehash = FALSE, x, y;
    UnitView *tranview = NULL, *uview = NULL, *occview = NULL;
    Unit *occ;

    /* We need to create any transport view first to avoid an infinite
       loop between transport and occupant. */
    if (unit->transport) {
	tranview = find_unit_view(side, unit->transport);
	if (tranview == NULL) {
	    /* Recursive. Will climb up transport tree if necessary. */
	    tranview = add_unit_view(side, unit->transport);
	    /* Since occupant views also get generated,
	       we can return after this. */
	    return find_unit_view(side, unit);
    	}
    }
    /* Now we check if our unit has a view. */
    uview = find_unit_view(side, unit);
    if (uview) {
    	x = uview->x;
    	y = uview->y;
	/* Blast the view if it is in the wrong location, 
	   or if it has the wrong transport. */
	if (x != unit->x 
	    || y != unit->y
	    || (uview->transport && !unit->transport)
	    || (!uview->transport && unit->transport)
	    || (uview->transport && unit->transport 
	    	&& uview->transport->id != unit->transport->id)) {
		remove_unit_view(side, uview);
		uview = NULL;
		/* Update the old cell display. */
		update_cell_display(side, x, y, UPDATE_ALWAYS);
	}
    }
    /* We need a new unit view. */
    if (uview == NULL) {
    	++numunitviews;
	uview = create_bare_unit_view();
	fill_out_uview(unit, side, uview);
#if (0)
	uview->observer = side->id;
	uview->unit = unit;
	uview->type = unit->type;
	uview->size = unit->size;
	uview->id = unit->id;
	uview->imf = unit->imf;
	uview->image_name = unit->image_name;
	uview->complete = completed(unit);
	uview->occupant = NULL;
	uview->transport = tranview;
	uview->nextinhash = NULL;
	uview->nexthere = NULL;
#endif
	uview->transport = tranview;
	/* Insert it into the global list. */
	uview->vnext = viewlist;
	viewlist = uview;
	/* Splice uview into the hash table, but only if it is the image of a 
	   top unit. */
	if (unit->transport == NULL) {
	    add_unit_view_raw(side, uview, unit->x, unit->y);
	/* Else splice uview into the occupant list of its transport view. */
	} else {
	    add_uview_to_uvstack(uview, tranview);
	}
	changed = TRUE;
    }
    if (uview->type != unit->type) {
	uview->type = unit->type;
	changed = TRUE;
    }
    if (uview->siden != unit->side->id) {
	uview->siden = unit->side->id;
	changed = TRUE;
    }
    if (uview->name != unit->name) {
	uview->name = unit->name;
	changed = TRUE;
    }
    if (uview->imf != unit->imf) {
	uview->imf = unit->imf;
	changed = TRUE;
    }
    if (uview->image_name != unit->image_name) {
	uview->image_name = unit->image_name;
	changed = TRUE;
    }
    if (uview->size != unit->size) {
	uview->size = unit->size;
	changed = TRUE;
    }
    if (uview->complete != completed(unit)) {
	uview->complete = completed(unit);
	changed = TRUE;
    }
    /* Clean out any stale occupant views. */
    for_all_occupant_views(uview, occview) {
	remove_unit_view(side, occview);
    	changed = TRUE;
    }
    /* Add any occupant views that should be added. */
    if (unit->occupant && occupants_visible(side, unit)) {
	uview->occupant = NULL;
	for_all_occupants(unit, occ) 
	  add_unit_view(side, occ);
	if (uview->occupant)
	  changed = TRUE;
    }    
    /* Irrespective of whether any view content changed, we now know
       that the view of this unit is current, so date it. */
    uview->date = g_turn();
    return (changed ? uview : NULL);
}

void
add_unit_view_raw(Side *side, UnitView *uview, int x, int y)
{
    int hash;
    UnitView *uv2, *prevview;

    if (side->unit_views == NULL)
      side->unit_views =
	(UnitView **) xmalloc(VIEW_HASH_SIZE * sizeof(UnitView *));
    prevview = NULL;
    hash = (wrapx(x) ^ y) % VIEW_HASH_SIZE;
    for (uv2 = side->unit_views[hash]; uv2 != NULL; uv2 = uv2->nextinhash) {
	if (is_at_correct_uvstack_position(uview, uv2)) {
	    break;
	}
	prevview = uv2;
    }
    uview->nextinhash = uv2;
    if (prevview == NULL) {
	side->unit_views[hash] = uview;
    } else {
	prevview->nextinhash = uview;
    }
    uview->x = x;  
    uview->y = y;
}

int
remove_unit_view(Side *side, UnitView *olduview)
{
    int hash;
    UnitView *uv = NULL, *prevview = NULL;

    if (side->unit_views == NULL)
      return FALSE;
    prevview = uv = NULL;
    /* Splice the unit out of any occupant list. */
    if (olduview->transport) {
    	for_all_occupant_views(olduview->transport, uv) {
	    if (olduview == uv) {
		if (prevview == NULL) {
		    olduview->transport->occupant = uv->nexthere;
		} else {
		    prevview->nexthere = uv->nexthere;
		}
		break;
	    }
	    prevview = uv;
     	}
    /* Else splice out the view from the hash table. */     
    } else {
	hash = (wrapx(olduview->x) ^ olduview->y) % VIEW_HASH_SIZE;
	for (uv = side->unit_views[hash]; uv != NULL; uv = uv->nextinhash) {
	    if (uv && uv == olduview) {
		if (prevview == NULL) {
		    side->unit_views[hash] = uv->nextinhash;
		} else {
		    prevview->nextinhash = uv->nextinhash;
		}
		break;
	    }
	    prevview = uv;
	}
    }
    /* Then clean out the occupant views. */
    for_all_occupant_views(olduview, uv) {
    	remove_unit_view(side, uv);
    }
    /* Finally mark the view as garbage. But don't hit its links yet, 
       since we might be traversing an occupant list here. */
    olduview->id = -1;
    return TRUE;
}

/* Get rid of all stale unit views at once. */

void
flush_stale_views(void)
{
    UnitView *uview, *prevview, *nextuv;
    int n = 0;

    if (viewlist == NULL)
      return;
    uview = viewlist;
    while (uview->id == -1) {
	nextuv = uview->vnext;
	flush_one_view(uview);
	uview = nextuv;
	if (uview == NULL)
	  break;
    }
    viewlist = uview;
    prevview = NULL;
    for_all_unit_views(uview) {
	if (uview->id == -1) {
	    nextuv = uview->vnext;
	    prevview->vnext = uview->vnext;
	    flush_one_view(uview);
	    uview = prevview;
	    ++n;
	} else {
	    prevview = uview;
	}
    }
    Dprintf("%d stale unit views flushed.\n", n);
    Dprintf("%d unit views left.\n", numunitviews);
}

/* Keep it clean - hit all links to other places.  Some might not be
   strictly necessary, but this is not an area to take chances with. */

static void
flush_one_view(UnitView *uview)
{
    uview->occupant = NULL;
    uview->transport = NULL;
    uview->nextinhash = NULL;
    uview->vnext = NULL;
    /* Add it on the front of the list of available views. */
    uview->nexthere = freeviews;
    freeviews = uview;
    --numunitviews;
}

extern int do_fire_at_action(Unit *unit, Unit *unit2, Unit *unit3, int m);

/* Check if one unit can perform overwatch fire upon another? */

int
can_overwatch(Unit *ounit, Unit *unit)
{
    int u = NONUTYPE, ou = NONUTYPE;
    int range = -1;
    int rslt = A_ANY_OK;

    assert_error(ounit, "Attempted to access a NULL overwatcher");
    assert_error(unit, "Attempted to access a NULL unit");
    ou = ounit->type;
    u = unit->type;
    range = distance(ounit->x, ounit->y, unit->x, unit->y);
    if (!between(u_range_min(ou), range, u_range(ou)))
      return FALSE;
    if (!between(uu_zoo_range_min(ou, u), range, uu_zoo_range(ou, u)))
      return FALSE;
    if (0 >= fire_hit_chance(ou, u))
      return FALSE;
    /*! \todo Handle any other probablility checks. */
    /* Check if overwatcher can perform a valid fire. */
    rslt = check_fire_at_action(ounit, ounit, unit, -1);
    /* If so, then go for it. */
    if (valid(rslt))
      return TRUE;
    /* If not, it may simply be because of ACP. We need to check this case. */
    if ((A_ANY_NO_ACP == rslt) && unit->act) {
	if (u_acp_to_fire(ou) <= (unit->act->acp + uu_acp_overwatch(ou, u)))
	  return TRUE;
    }
    /* Else, assume that we can't. */
    return FALSE;
}

/* Check if any overwatchers in a particular cell. */
/* (Search Predicate) */
/* {Arbiter Function} */

int
try_overwatch_from(int x, int y, int *counter, ParamBox *parambox)
{
    Unit *unit = NULL, *unit2 = NULL;
    int u = NONUTYPE;
    Side *oside = NULL;
    ParamBoxUnitSide *paramboxus = NULL;

#if (0)
    assert_warning_return(inside_area(x, y), 
			  "Tried to overwatch from outside playing area", 
			  FALSE);
#endif
    if (!inside_area(x, y))
	return FALSE;
    assert_error(parambox, "Attempted to use a NULL parambox");
    assert_error(PBOX_TYPE_UNIT_SIDE == parambox->get_type(), 
		 "Attempted to use wrong type of parambox in a search");
    paramboxus = (ParamBoxUnitSide *)parambox;
    assert_error(paramboxus->unit, "Attempted to access a NULL unit");
    assert_error(paramboxus->side, 
		 "Attempted to access a NULL overwatcher side");
    unit = paramboxus->unit;
    oside = paramboxus->side;
    u = unit->type;
    for_all_stack_with_occs(x, y, unit2) {
	/* Skip any units that are not on the overwatching side. */
	if (unit2->side != oside)
	  continue;
	/* Can the enemy unit perform overwatch fire against the 
	   given unit? */
	if (can_overwatch(unit2, unit)) {
	    if (unit2->act)
	      unit2->act->acp += uu_acp_overwatch(unit2->type, u);
	    do_fire_at_action(unit2, unit2, unit, -1);
	    if (counter)
	      ++(*counter);
	    /*! \todo Limit by max num of overwatch firings for victim type. */
	}
    }
    return FALSE;
}

/* Check if there any overwatch fire attempts against a given unit. 
   If so, check if the fire action is valid by the overwatcher against the 
   given unit. */
/* {Arbiter Function} */

void
try_overwatch_against(Unit *unit)
{
    Side *oside = NULL;
    UnitView *uview = NULL;
    int u = NONUTYPE;
    ParamBoxUnitSide paramboxus;
    int counter = 0;

    /* Sanity checks. */
    assert_error(unit, "Attempted to overwatch a NULL unit");
    u = unit->type;
    /* Compute cache, if necessary. */
    if (Xconq::any_overwatch_cache == -1)
      compute_overwatch_cache();
    /* Look through each side's view of the unit, if any. */
    for_all_sides(oside) {
	/* Skip unit's side. */
	if (unit->side == oside)
	  continue;
	/* If the side is not an enemy then skip its reaction. */
	if (!enemy_side(unit->side, oside))
	  continue;
	/* Check if the side can see the given unit. */
	uview = find_unit_view(oside, unit);
	/* If not, then skip side's overwatch reaction. */
	if (!uview)
	  continue;
	/* Setup the parambox for searching. */
	paramboxus.unit = unit;
	paramboxus.side = oside;
	/* If side can see given unit, then start searching for overwatchers. */
	if (0 >= Xconq::overwatch_against_range_min[u])
	  try_overwatch_from(unit->x, unit->y, &counter, &paramboxus);
	/*! \todo Search in ring starting from min range. */
	/*! \todo Limit by max num of overwatch firings for victim type. */
	limited_search_around(unit->x, unit->y, 
			      Xconq::overwatch_against_range_max[u], 
			      try_overwatch_from, 1, &counter, 
			      cover(oside, unit->x, unit->y), 
			      (ParamBox *)&paramboxus);
    }
    if (counter)
      Dprintf("%s received overwatch fire from %d units.\n", 
	      unit_desig(unit), counter);
}

/* What happens when a unit appears on a given cell. */

/* An always-seen unit has builtin spies/tracers to inform everybody
   else of all its movements.  When such a unit occupies a cell,
   coverage is turned on and remains on until the unit leaves that
   cell. */

/* If this unit with onboard spies wanders into unknown territory,
   shouldn't that territory become known as well?  I think the unseen
   test should only apply during initialization. */
/* But if always-seen unit concealed during init, will magically appear
   when it first moves! */

void
all_see_occupy(Unit *unit, int x, int y, int inopen)
{
    Side *side;
    int dir, x1, y1;
    Unit *unit2, *unit3;
    int u = NONUTYPE;
    int always = FALSE;
    
    assert_error(unit, "Attempted to create views of a NULL unit");
    u = unit->type;
    always = u_see_always(u);
    for_all_sides(side) {
	if (side_sees_unit(side, unit)) {
	    see_cell(side, x, y);
	} else if (side_tracking_unit(side, unit)) {
	    see_cell(side, x, y);
	} else if (cover(side, x, y) > 0) {
	    see_cell(side, x, y);
	} else {
	    if (always && terrain_view(side, x, y) != UNSEEN) {
		add_cover(side, x, y, 1);
		set_alt_cover(side, x, y, 0);
	    }
	    if (inopen || always) {
		see_cell(side, x, y);
	    }
	}
    }
    /* If the occupation happens during the game, nearby units that
       are on unfriendly sides should wake up.  (should also be:
       "unless they were directed to sleep in presence of enemy") */
    if (gameinited) {
	for_all_directions(dir) {
	    if (interior_point_in_dir(x, y, dir, &x1, &y1)) {
	   	for_all_stack(x1, y1, unit2) {
	   	    if (unit2->side != NULL
	   	    	&& unit2->side != unit->side
	   	    	&& units_visible(unit2->side, x, y)
	   	    	&& !unit_trusts_unit(unit2, unit)) {
	   	    	wake_unit(unit2->side, unit2, FALSE);
	   	    }
	   	    for_all_occupants(unit2, unit3) {
			if (unit3->side != NULL
			    && unit3->side != unit->side
			    && units_visible(unit3->side, x, y)
			    && !unit_trusts_unit(unit3, unit)) {
			    wake_unit(unit3->side, unit3, FALSE);
			}
	   	    }
	   	}
	    }
	}
	/* Check for any overwatch against the occupying unit. */
	if (Xconq::any_overwatch_cache == -1)
	  compute_overwatch_cache();
	if (Xconq::any_overwatch_cache 
	    && get_packed_bool(Xconq::any_overwatch_chances, 0, u)
	    && !Xconq::suppress_reactions
	    && in_play(unit))
	  try_overwatch_against(unit);
    }
}

/* Some highly visible unit types cannot leave a cell without everybody
   knowing about the event.  The visibility is attached to the unit, not
   the cell, so first the newly-empty cell is viewed, then view coverage
   is decremented. */

void
all_see_leave(Unit *unit, int x, int y, int inopen)
{
    Side *side;
    int always = u_see_always(unit->type), domore, update;
    UnitView *uview;

    for_all_sides(side) {
	if (side_sees_unit(side, unit)
		   && in_area(unit->x, unit->y)) {
	    see_cell(side, x, y);
	} else if (side_tracking_unit(side, unit)) {
	    see_cell(side, x, y);
	} else if (cover(side, x, y) > 0) {
	    see_cell(side, x, y);
	} else {
	    if (always && terrain_view(side, x, y) != UNSEEN) {
		see_cell(side, x, y);
		if (cover(side, x, y) > 0)
		  add_cover(side, x, y, -1);
		if (side->alt_coverage)
		  /* (should recalc alt coverage, since was 0) */;
	    }
	    /* Won't be called twice, because view coverage is 0 now. */
	    if (inopen) {
		see_cell(side, x, y);
	    }
	    /* special hack to flush images we *know* are garbage */
	    if (cover(side, x, y) < 1) {
		domore = TRUE;
		update = FALSE;
		while (domore) {
		    domore = FALSE;
		    for_all_view_stack_with_occs(side, x, y, uview) {
			if (side->id == uview->siden) {
			    remove_unit_view(side, uview);
			    domore = TRUE;
			    update = TRUE;
			    break;
			}
		    }
		}
		if (update)
		  update_cell_display(side, x, y, UPDATE_ALWAYS);
	    }
	}
    }
}

static int tmpx0, tmpy0, tmpz0;
static int tmpnx, tmpny, tmpnz;

static void
init_visible_elevation(int x, int y)
{
    int elev = checked_elev_at(x, y);

    set_tmp2_at(x, y, elev - tmpz0);
}

static void
init_visible_elevation_2(int x, int y)
{
    int elev = checked_elev_at(x, y);

    set_tmp3_at(x, y, elev - tmpnz);
}


static void
calc_visible_elevation(int x, int y)
{
    int dir, x1, y1, elev, tmp, tmpa, tmpb;
    int adjelev = 9999, viselev, dist, dist1, cellwid = area.cellwidth;

    elev = checked_elev_at(x, y);
    dist = distance(x, y, tmpx0, tmpy0);
    if (cellwid <= 0)
      cellwid = 1;
    for_all_directions(dir) {
	if (point_in_dir(x, y, dir, &x1, &y1)) {
	    dist1 = distance(x1, y1, tmpx0, tmpy0);
	    if (dist1 < dist) {
		tmpa = tmp2_at(x1, y1);
		/* Account for the screening effect of the elevation
                   difference. */
		/* (dist1 will never be zero) */
		tmpa = (tmpa * dist * cellwid) / (dist1 * cellwid);
		tmpb = checked_elev_at(x1, y1)
		  + t_thickness(terrain_at(x1, y1))
		  - tmpz0;
		tmpb = (tmpb * dist * cellwid) / (dist1 * cellwid);
		tmp = max(tmpa, tmpb);
		adjelev = min(adjelev, tmp + tmpz0);
	    }
	}
    }
    viselev = max(adjelev, elev);
    set_tmp2_at(x, y, viselev - tmpz0);
}

static void
calc_visible_elevation_2(int x, int y)
{
    int dir, x1, y1, elev, tmp, tmpa, tmpb;
    int adjelev = 9999, viselev, dist, dist1, cellwid = area.cellwidth;

    elev = checked_elev_at(x, y);
    dist = distance(x, y, tmpnx, tmpny);
    if (cellwid <= 0)
      cellwid = 1;
    for_all_directions(dir) {
	if (point_in_dir(x, y, dir, &x1, &y1)) {
	    dist1 = distance(x1, y1, tmpnx, tmpny);
	    if (dist1 < dist) {
		tmpa = tmp3_at(x1, y1);
		/* Account for the screening effect of the elevation
                   difference. */
		/* (dist1 will never be zero) */
		tmpa = (tmpa * dist * cellwid) / (dist1 * cellwid);
		tmpb = checked_elev_at(x1, y1)
		  + t_thickness(terrain_at(x1, y1))
		  - tmpnz;
		tmpb = (tmpb * dist * cellwid) / (dist1 * cellwid);
		tmp = max(tmpa, tmpb);
		adjelev = min(adjelev, tmp + tmpnz);
	    }
	}
    }
    viselev = max(adjelev, elev);
    set_tmp3_at(x, y, viselev - tmpnz);
}

/* Unit's beady eyes are shifting from one location to another.  Since
   new things may be coming into view, we have to check and maybe draw
   lots of cells (but only need the one output flush, fortunately). */

/* (LOS comes in here, to make irregular coverage areas) */

void
cover_area(Side *side, Unit *unit, Unit *oldtransport, int x0, int y0,
	   int nx, int ny)
{
    Side *side2;

    if (side != NULL
	&& completed(unit)
	&& (oldtransport == NULL
	   || uu_occ_vision(unit->type, oldtransport->type) > 0)) {
	if (side->ingame) {
	    /* Active sides keep their allies informed.  Note that
	       this will effectively leak information to allies of
	       allies, even if the unit's side is not an ally
	       directly. */
	    for_all_sides(side2) {
		if (trusted_side(side, side2)
		    || side2->see_all) {
		    cover_area_1(side2, unit, x0, y0, nx, ny);
		}
	    }
	} else {
	    /* Inactive sides just have their own data maintained
	       (not clear if this is critical, but seems sensible) */
	    cover_area_1(side, unit, x0, y0, nx, ny);
	}
    }
}

/* Set this flag to redo coverage without doing all the side effects
   that come with see_cell. */

static int suppress_see_cell;

/* Set this flag to initialize views without waking any units.  This
   is necessary when restoring a game, since we construct the view
   using this code, but don't want to modify the units' plans from
   what they were when the game was saved. */

int suppress_see_wakeup;

static void
cover_area_1(Side *side, Unit *unit, int x0, int y0, int nx, int ny)
{
    int u = unit->type, range0, nrange, range, x, y, x1, y1, x2, y2;
    int y1c, y2c, cov, los, r;
    int xmin, ymin, xmax, ymax, oldcov, newcov, anychanges;

    if (side->coverage == NULL)
      return;
    range0 = nrange = u_vision_range(u);
    /* Adjust for the effects of nighttime on vision range. */
    if (in_area(x0, y0)) {
	if (night_at(x0, y0)) {
	    range0 =
	      (range0 * ut_vision_night_effect(u, terrain_at(x0, y0))) / 100;
	}
    } else {
	range0 = 0;
    }
    if (in_area(nx, ny)) {
	if (night_at(nx, ny)) {
	    nrange =
	      (nrange * ut_vision_night_effect(u, terrain_at(nx, ny))) / 100;
	}
    } else {
	nrange = 0;
    }
    range = max(range0, nrange);
    allocate_area_scratch(1);
    anychanges = FALSE;
    /* First, set the union of the from and to areas to the existing
       coverage. */
    /* These may be outside the area - necessary since units may be able
       to see farther in x than the height of the area. */
    /* Compute the maximum bounds that may be affected. */
    if (y0 >= 0) {
	if (ny >= 0) {
	    ymin = min(y0, ny);
	    ymax = max(y0, ny);
	} else {
	    ymin = ymax = y0;
	}
    } else if (ny >= 0) {
	ymin = ymax = ny;
    }
    if (x0 >= 0) {
	if (nx >= 0) {
	    xmin = min(x0, nx);
	    xmax = max(x0, nx);
	} else {
	    xmin = xmax = x0;
	}
    } else if (nx >= 0) {
	xmin = xmax = nx;
    }
    if (any_los) {
	/* Need extra scratch layers, will be used for visible
           elevation cache. */
	allocate_area_scratch(3);
    }
    los = FALSE;
    /* (should also adjust for effect of clouds here) */
    if (!u_can_see_behind(u)) {
	los = TRUE;
	/* Compute the minimum elevation for visibility at each cell. */
	if (in_area(x0, y0)) {
	    tmpx0 = x0;  tmpy0 = y0;
	    tmpz0 = checked_elev_at(x0, y0)
		+ unit_alt(unit)
		+ ut_eye_height(u, terrain_at(x0, y0));
	    apply_to_area(x0, y0, range0, init_visible_elevation);
	    /* Leave own and adj cells alone, they will always be visible. */
	    for (r = 2; r <= range0; ++r) {
		apply_to_ring(x0, y0, r, r, calc_visible_elevation);
	    }
	    /* We now have a layer indicating how high things must be
               to be visible. */
	}
	if (in_area(nx, ny)) {
	    tmpnx = nx;  tmpny = ny;
	    tmpnz = checked_elev_at(nx, ny)
		+ unit_alt(unit)
		+ ut_eye_height(u, terrain_at(nx, ny));
	    apply_to_area(nx, ny, nrange, init_visible_elevation_2);
	    /* Leave own and adj cells alone, they will always be visible. */
	    for (r = 2; r <= nrange; ++r) {
		apply_to_ring(nx, ny, r, r, calc_visible_elevation_2);
	    }
	    /* We now have another layer, indicating how high things must be
               to be visible from the new location. */
	}
    }
    /* Copy the current coverage into the tmp layer. */
    y1 = y1c = ymin - range;
    y2 = y2c = ymax + range;
    /* Clip the iteration bounds. */
    if (y1c < 0)
      y1c = 0;
    if (y2c > area.height - 1)
      y2c = area.height - 1;
    for (y = y1c; y <= y2c; ++y) {
	x1 = xmin - range;
	x2 = xmax + range;
	for (x = x1; x <= x2; ++x) {
	    if (in_area(x, y)) {
		set_tmp1_at(x, y, cover(side, x, y));
	    }
	}
    }
    /* Decrement coverage around the old location. */
    if (in_area(x0, y0)) {
	y1 = y1c = y0 - range0;
	y2 = y2c = y0 + range0;
	/* Clip the iteration bounds. */
	if (y1c < 0)
	  y1c = 0;
	if (y2c > area.height - 1)
	  y2c = area.height - 1;
	for (y = y1c; y <= y2c; ++y) {
	    x1 = x0 - (y < y0 ? (y - y1) : range0);
	    x2 = x0 + (y > y0 ? (y2 - y) : range0);
	    for (x = x1; x <= x2; ++x) {
		if (in_area(x, y)) {
		    if (!los
			|| ((tmp2_at(x, y) + tmpz0)
			    <= (checked_elev_at(x, y)
				+ t_thickness(terrain_at(x, y))))) {
			cov = tmp1_at(x, y) - 1;
			/* Should never go negative, detect if so. */
			if (cov < 0) {
			    Dprintf("Negative coverage for %s at %d,%d\n",
				    side_desig(side), wrapx(x), y);
			}
			set_tmp1_at(x, y, cov);
		    }
		    if (los && (alt_cover(side, x, y) == (tmp2_at(x, y) + tmpz0)))
		      /* this unit set the min, should recalc alt
                         coverage now */;
		}
	    }
	}
    }
    /* Increment coverage around the new location. */
    if (in_area(nx, ny)) {
	y1 = y1c = ny - nrange;
	y2 = y2c = ny + nrange;
	/* Clip the iteration bounds. */
	if (y1c < 0)
	  y1c = 0;
	if (y2c > area.height - 1)
	  y2c = area.height - 1;
	for (y = y1c; y <= y2c; ++y) {
	    x1 = nx - (y < ny ? (y - y1) : nrange);
	    x2 = nx + (y > ny ? (y2 - y) : nrange);
	    for (x = x1; x <= x2; ++x) {
		if (in_area(x, y)) {
		    if (!los
			|| ((tmp3_at(x, y) + tmpnz)
			    <= (checked_elev_at(x, y)
				+ t_thickness(terrain_at(x, y))))) {
			cov = tmp1_at(x, y) + 1;
			set_tmp1_at(x, y, cov);
		    }
		    if (los)
		      set_alt_cover(side, x, y,
				    min(alt_cover(side, x, y),
					(tmp3_at(x, y) + tmpnz)));
		}
	    }
	}
    }
    /* Now update the actual coverage.  Do this over an area that includes
       both the decrement and increment changes. */
    y1 = y1c = ymin - range;
    y2 = y2c = ymax + range;
    /* Clip the iteration bounds. */
    if (y1c < 0)
      y1c = 0;
    if (y2c > area.height - 1)
      y2c = area.height - 1;
    for (y = y1c; y <= y2c; ++y) {
	x1 = xmin - range;
	x2 = xmax + range;
	for (x = x1; x <= x2; ++x) {
	    if (in_area(x, y)) {
		oldcov = cover(side, x, y);
		newcov = tmp1_at(x, y);
		if (newcov != oldcov) {
		    set_cover(side, x, y, newcov);
		    /* Skip over unit view updating if we're just repairing
		       the coverage layer. */
		    if (suppress_see_cell)
		      continue;
		    if (newcov > oldcov
			&& see_cell(side, x, y)
			&& !suppress_see_wakeup) {
				react_to_seen_unit(side, unit, x, y);
		    }
		    if ((newcov > 0 && oldcov == 0)
			|| (newcov == 0 && oldcov > 0)
			|| (DebugG && newcov != oldcov)) {
				update_cell_display(side, x, y, UPDATE_COVER);
		    }
		    anychanges = TRUE;
		}
	    }
	}
    }
    /* If we're seeing new things, make sure they're on the display. */
    if (anychanges) {
	flush_display_buffers(side);
	/* Also flush the unit view list. */
	flush_stale_views();
    }
}

/* Use this to clear out garbled view coverage.  Note however that this
   may not touch any unit views, otherwise players could collect info
   about hard-to-see units by refreshing/recomputing over and over. */

void
reset_coverage(void)
{
    Side *side;

    if (g_see_all())
      return;
    suppress_see_cell = TRUE;
    for_all_sides(side)
      calc_coverage(side);
    suppress_see_cell = FALSE;
}

/* Same as above but does not suppress see-cell. Needs to be called on
sunrise etc. when unit views should be updated. */

void
really_reset_coverage(void)
{
    Side *side;

    if (g_see_all())
      return;
    for_all_sides(side)
      calc_coverage(side);
}

/* Calculate/recalculate the view coverage layers of a side. */

/* Note: this code is very inefficient in that each unit view is added
and then removed about 50 times. Should fix this. */

void
calc_coverage(Side *side)
{
    int x, y, pop, visible[MAXSIDES + 1];
    Unit *unit;
    Side *side2;

    if (side->coverage == NULL)
      return;
    /* Should figure out why this does not work. */
    if (0/*side->see_all*/) {
    	for_all_cells(x, y) {
    		set_cover(side, x, y, 1);
    	}
    	return;
    }
    Dprintf("Calculating all view coverage for %s\n", side_desig(side));
    /* Either init all cells to 0, or use populations to decide. */
    if (people_sides_defined()) {
	/* First figure out which sides' people will tell us stuff and
	   cache it, so as to speed up the init. */
	for_all_sides(side2) {
	    visible[side2->id] = FALSE;
	    if (trusted_side(side2, side) || side2->controlled_by == side)
	      visible[side2->id] = TRUE;
	}
	for_all_cells(x, y) {
	    pop = people_side_at(x, y);
	    set_cover(side, x, y, ((pop != NOBODY && visible[pop]) ? 1 : 0));
	}
    } else {
	for_all_cells(x, y) {
	    set_cover(side, x, y, 0);
	}
    }
    /* Add coverage by, and of, the units already in place. */
    for_all_units(unit) {
	if (in_play(unit)) {
	    x = unit->x;  y = unit->y;
	    if (trusted_side(unit->side, side)
	    	|| side->see_all) {
		/* Units that trust us tell us stuff. */
		cover_area(side, unit, unit->transport, -1, -1, x, y);
	    } else if (u_see_always(unit->type)
		       && terrain_view(side, x, y) != UNSEEN) {
		/* Always-seen units effectively monitor their current
		   location for us, even if they don't like us. */
		add_cover(side, x, y, 1);
	    }
	}
    }
}

/* Look for and clear out any bogus view data. */

void
reset_all_views(void)
{
    Side *side;

    for_all_sides(side) {
	if (!side->see_all) {
	    reset_view(side);
	}
    }
}

void
reset_view(Side *side)
{
    int x, y, domore;
    UnitView *uview;

    for_all_cells(x, y) {
	/* Remove any leftover images of our own units. */
	if (cover(side, x, y) < 1) {
	    domore = TRUE;
	    while (domore) {
		domore = FALSE;
		for_all_view_stack_with_occs(side, x, y, uview) {
		    if (side->id == uview->siden) {
			remove_unit_view(side, uview);
			domore = TRUE;
			break;
		    }
		}
	    }
	}
    }
}

void
react_to_seen_unit(Side *side, Unit *unit, int x, int y)
{
    int eu;
    Unit *eunit;
    UnitView *uview;
    Side *es;

    /* (The g_see_all() branch appears to be unused, because 
	'calc_coverage' is only called when g_see_all() is not true.) */
    if (g_see_all() /* see real unit */) {
    	/* (should look at all of stack if can be mixed) */
	if ((eunit = unit_at(x, y)) != NULL) {
	    if (unit->plan && !allied_side(eunit->side, side)) {
		/* should do a more general alarm */
		selectively_wake_unit(unit->side, unit, TRUE, FALSE);
	    }
	}
    } else if (side->coverage != NULL) {
	uview = unit_view_at(side, x, y);
    	if (uview != NULL) {
    	    eu = uview->type;  
    	    es = side_n(uview->siden);
    	    /* react only to certain utypes? */
	    if (unit->plan && !allied_side(es, side)) {
		/* should do a more general alarm */
		selectively_wake_unit(unit->side, unit, TRUE, FALSE);
	    }
    	}
    } else {
    	/* ??? */
    }
}

/* Decide whether any side acquires tracking on a unit. */

int any_tracking = -1;

void
maybe_track(Unit *unit)
{
    int x0, y0, dir, x1, y1, chance;
    Unit *unit2;

    if (any_tracking < 0) {
	int u1, u2;

	any_tracking = FALSE;
	for_all_unit_types(u1) {
	    for_all_unit_types(u2) {
		if (uu_track(u1, u2) > 0) {
		    any_tracking = TRUE;
		    break;
		}
	    }
	    if (any_tracking)
	      break;
	}
    }
    if (!any_tracking)
      return;
    x0 = unit->x;  y0 = unit->y;
    for_all_stack(x0, y0, unit2) {
	if (in_play(unit2)
	    && unit2 != unit
	    && unit2->side != unit->side
	    && unit2->side != NULL) {
	    chance = uu_track(unit2->type, unit->type);
	    if (xrandom(10000) < chance) {
		add_side_to_set(unit2->side, unit->tracking);
	    }
	}
    }
    for_all_directions(dir) {
	if (interior_point_in_dir(x0, y0, dir, &x1, &y1)) {
	    for_all_stack(x1, y1, unit2) {
		if (in_play(unit2)
		    && unit2 != unit
		    && unit2->side != unit->side
		    && unit2->side != NULL) {
		    chance = uu_track(unit2->type, unit->type);
		    if (xrandom(10000) < chance) {
			add_side_to_set(unit2->side, unit->tracking);
		    }
		}
	    }
	}
    }
}

/* Decide whether any side tracking a unit lost it. */

void
maybe_lose_track(Unit *unit, int nx, int ny)
{
    int t = terrain_at(nx, ny), chance;
    Side *side;

    chance = ut_lose_track(unit->type, t);
    if (chance > 0) {
	for_all_sides(side) {
	    if (side_tracking_unit(side, unit)
		&& xrandom(10000) < chance) {
		remove_side_from_set(side, unit->tracking);
		/* (should notify?) */
	    }
	}
    }
}

extern void compute_see_chances(void);

/* Determine whether there is any possibility of an uncertain sighting,
   and cache the conclusion. */
/*! \note 'any_see_chances' will be only be set if a see chance is not 100%. 
	  Thus, if all the see chances are 0, it will be set. The variable 
	  name is somewhat misleading. */

void
compute_see_chances(void)
{
    using namespace Xconq;
    int u1, u2, u3, m1;

    any_see_chances = FALSE;
    any_people_see_chances = FALSE;
    people_always_see = TRUE;
    max_see_chance_range = -1;
    any_see_mistake_chances = FALSE;
    max_see_mistake_range = -1;
    for_all_unit_types(u2) {
	for_all_unit_types(u1) {
	    if (uu_see_at(u1, u2) != 100) {
		any_see_chances = TRUE;
		max_see_chance_range = max(max_see_chance_range, 0);
	    }
	    if (uu_see_adj(u1, u2) != 100) {
		any_see_chances = TRUE;
		max_see_chance_range = max(max_see_chance_range, 1);
	    }
	    if (uu_see(u1, u2) != 100) {
		any_see_chances = TRUE;
	    }
	    max_see_chance_range = max(max_see_chance_range, 
				       u_vision_range(u1));
	    if (uu_see_mistake(u1, u2) > 0) {
		for_all_unit_types(u3) {
		    if (uu_looks_like(u2, u3) > 0) {
			any_see_mistake_chances = TRUE;
			break;
		    }
		}
		max_see_mistake_range = max(max_see_mistake_range, 
					    u_vision_range(u1));
	    }
	}
	for_all_material_types(m1) {
	    if (m_people(m1) > 0 && um_people_see(u2, m1) > 0) {
		any_people_see_chances = TRUE;
		if (um_people_see(u2, m1) < 100)
		  people_always_see = FALSE;
	    }
	}
	if (people_sides_defined())
	  any_people_see_chances = TRUE;
    }
}

/* Compute the overwatch cache. Useful for optimizing searches for 
   overwatchers. */

void
compute_overwatch_cache(void)
{
    using namespace Xconq;
    int u1 = NONUTYPE, u2 = NONUTYPE;
    int range = -1;

    /* Allocate the overwatch-against range arrays. */
    any_overwatch_chances = create_packed_bool_table(1, numutypes);
    overwatch_against_range_max = (int *)xmalloc(numutypes * sizeof(int));
    overwatch_against_range_min = (int *)xmalloc(numutypes * sizeof(int));
    init_packed_bool_table(any_overwatch_chances);
    memset(overwatch_against_range_max, -1, numutypes * sizeof(int));
    memset(overwatch_against_range_min, INT_MAX, numutypes * sizeof(int));
    /* Find out if any overwatch is being done. */
    any_overwatch_cache = FALSE;
    for_all_unit_types(u1) {
	/* If an unit cannot fire, then it cannot overwatch by definition. */
	if (0 >= u_range(u1))
	  continue;
	/* Check if the firing unit can engage in any overwatch. */
	for_all_unit_types(u2) {
	    if ((-1 < uu_zoo_range(u1, u2)) && (0 < fire_hit_chance(u1, u2))) {
		/*! \todo Also consider an actual probability. */
		any_overwatch_cache = TRUE;
		set_packed_bool(any_overwatch_chances, 0, u2, TRUE);
		range = min(uu_zoo_range(u1, u2), u_range(u1));
		overwatch_against_range_max[u2] = 
		    max(overwatch_against_range_max[u2], range);
		range = max(uu_zoo_range_min(u1, u2), u_range_min(u1));
		overwatch_against_range_min[u2] =
		    min(overwatch_against_range_min[u2], range);
	    }
	}
    }
}

/* Update the view of this cell for everybody's benefit.  May have to write
   to many displays. */

void
all_see_cell(int x, int y)
{
    Side *side;

    for_all_sides(side) {
	see_cell(side, x, y);
    }
}

static void mistake_view(Side *side, Unit *seer, UnitView *uview);

/* Check if one unit sees another, and flag whether that vision is clear 
   or not. */
/* {Arbiter Function} */

int
see_unit(Unit *seer, Unit *tosee, SeerNode *seers, int *numseers)
{
    int dist = -1;
    int u = NONUTYPE, us = NONUTYPE;

    assert_error(seers, "Attempted to access a NULL array in seeing code");
    assert_error(numseers && (*numseers >= 0), 
		 "Attempted to access a bad counter in seeing code");
    assert_warning_return(in_play(seer), 
			  "Attempted to access an out-of-play unit", FALSE);
    assert_warning_return(in_play(tosee), 
			  "Attempted to access an out-of-play unit", FALSE);
    if (seer == tosee)
      return FALSE;
    if (seer->side == tosee->side)
      return FALSE;
    u = tosee->type;
    us = seer->type;
    dist = distance(tosee->x, tosee->y, seer->x, seer->y);
    if (probability(see_chance(seer, tosee))) {
	seers[*numseers].seer = seer;
	if ((uu_see_mistake_range_min(us, u) <= dist)
	    && uu_see_mistake(us, u)) {
	    if (xrandom(10000) < uu_see_mistake(us, u))
	      seers[*numseers].mistakes = TRUE;
	}
	++(*numseers);
	return TRUE;
    }
    return FALSE;
}

/* Iterate 'see_unit' over all the occs of a transport. */
/* (Can be used as a search predicate.) */
/* {Arbiter Function} */

int
side_occs_see_unit(int x, int y, int *numseers, ParamBox *parambox)
{
    ParamBoxUnitUnitSeers *paramboxuu = NULL;
    ParamBoxUnitUnitSeers *paramboxuu2 = NULL;
    int foundseer = FALSE;
    Unit *seer = NULL;

    assert_warning_return(in_area(x, y), "Attempted to use illegal coordinates",
			  FALSE);
    assert_warning_return(parambox, "Attempted to access a NULL parambox",
			  FALSE);
#if (0)
    assert_warning_return(PBOX_TYPE_UNIT_UNIT_SEERS == parambox->get_type(), 
			  "Attempted to use wrong type of parambox",
			  FALSE);
#endif
    paramboxuu = (ParamBoxUnitUnitSeers *)parambox;
    assert_warning_return(paramboxuu->unit1, 
			  "Could not check visibility of an unit", FALSE);
    assert_warning_return(paramboxuu->unit2, 
			  "Attempted to access a NULL unit", FALSE);
    assert_warning_return(numseers && (*numseers >= 0), 
			  "Invalid parambox data encountered", FALSE);
    assert_warning_return(paramboxuu->seers,
			  "Attempted to access a NULL array", FALSE);
    /* If no possible seers, then return now. */
    if (!paramboxuu->unit2->occupant)
      return FALSE;
    /* Iterate over all occupants. */
    for_all_occupants(paramboxuu->unit2, seer) {
	/* Paranoia. Skip any out-of-play unit. */
	if (!in_play(seer))
	  continue;
	/* HACK: We need to pass more information into the recursive routine. 
	   The seer may not be of the same side as the side that is supposed 
	   to be seeing. For now we must assume that an occ of the correct 
	   side is not lurking in an unit of the wrong side. */
	if (seer->side != paramboxuu->unit2->side) {
	    continue;
	}
	/* Descend depth-first before checking anything else. A seer of the 
	   correct side may be lurking in a transport of an incorrect side. */
	if (seer->occupant) {
	    paramboxuu2 = 
		(ParamBoxUnitUnitSeers *)xmalloc(sizeof(ParamBoxUnitUnitSeers));
	    paramboxuu2->unit1 = paramboxuu->unit1;
	    paramboxuu2->unit2 = seer;
	    paramboxuu2->seers = paramboxuu->seers;
	    if (side_occs_see_unit(x, y, numseers, (ParamBox *)paramboxuu2))
	      foundseer = TRUE;
	    if (paramboxuu2)
	      free(paramboxuu2);
	}
	/*! \todo Put the side filter back in the correct order once the 
		  proper side info is being passed to 'side_occs_see_unit'. */
#if (0)
	/* Skip any units not on given side. */
	/* (TODO: Also account for trusted sides.) */
	if (seer->side != paramboxuu->unit2->side) {
	    continue;
	}
#endif
	/* Try seeing with the current seer. */
	if (see_unit(seer, paramboxuu->unit1, paramboxuu->seers, numseers))
	  foundseer = TRUE;
	/* Check if we should conitnue evaluating. */
	if (*numseers >= cover(paramboxuu->unit2->side, x, y))
	  break;
    }
    /* return foundseer; */
    return FALSE; 
}

/* Iterate 'see_unit' over all the units of a stack. */
/* (Can be used as a search predicate.) */
/* {Arbiter Function} */

int
side_ustack_see_unit(int x, int y, int *numseers, ParamBox *parambox)
{
    ParamBoxUnitSideSeers *paramboxus = NULL;
    ParamBoxUnitUnitSeers *paramboxuu = NULL;
    int foundseer = FALSE;
    Unit *seer = NULL;

    assert_warning_return(in_area(x, y), "Attempted to use illegal coordinates",
			  FALSE);
    assert_warning_return(parambox, "Attempted to access a NULL parambox",
			  FALSE);
    assert_warning_return(PBOX_TYPE_UNIT_SIDE_SEERS == parambox->get_type(), 
			  "Attempted to use wrong type of parambox",
			  FALSE);
    paramboxus = (ParamBoxUnitSideSeers *)parambox;
    assert_warning_return(paramboxus->unit, 
			  "Could not check visibility of an unit", FALSE);
    assert_warning_return(paramboxus->side, 
			  "Attempted to access a NULL side", FALSE);
    assert_warning_return(numseers && (*numseers >= 0), 
			  "Invalid parambox data encountered", FALSE);
    assert_warning_return(paramboxus->seers,
			  "Attempted to access a NULL array", FALSE);
    /* If no possible seers, then return now. */
    if (!unit_at(x, y))
      return FALSE;
    /* Iterate over all stack members. */
    for_all_stack(x, y, seer) {
	/* Paranoia. Skip any out-of-play units in stack. */
	if (!in_play(seer))
	  continue;
	/* HACK: We need to pass more information into the recursive routine. 
	   The seer may not be of the same side as the side that is supposed 
	   to be seeing. For now we must assume that an occ of the correct 
	   side is not lurking in an unit of the wrong side. */
	if (seer->side != paramboxus->side) {
	    continue;
	}
	/* Descend depth-first before checking anything else. A seer of the 
	   correct side may be lurking in a transport of an incorrect side. */
	if (seer->occupant) {
	    paramboxuu = 
		(ParamBoxUnitUnitSeers *)xmalloc(sizeof(ParamBoxUnitUnitSeers));
	    paramboxuu->unit1 = paramboxus->unit;
	    paramboxuu->unit2 = seer;
	    paramboxuu->seers = paramboxus->seers;
	    if (side_occs_see_unit(x, y, numseers, (ParamBox *)paramboxuu))
	      foundseer = TRUE;
	    if (paramboxuu)
	      free(paramboxuu);
	}
	/*! \todo Put the side filter back in the correct order once the 
		  proper side info is being passed to 'side_occs_see_unit'. */
#if (0)
	/* Skip any units not on given side. */
	/* (TODO: Also account for trusted sides.) */
	if (seer->side != paramboxus->side) {
	    continue;
	}
#endif
	/* Try seeing with the current seer. */
	if (see_unit(seer, paramboxus->unit, paramboxus->seers, numseers))
	  foundseer = TRUE;
	/* Check if we should conitnue evaluating. */
	if (*numseers >= cover(paramboxus->side, x, y))
	  break;
    }
    /* return foundseer; */
    return FALSE;
}

/* Look at the given position, possibly not seeing anything.  Return
   true if a unit was spotted. */

int
see_cell(Side *side, int x, int y)
{
    int update, updatet, chance, curview;
    int m, mupdate, wupdate, flags, domore, rslt, sawthis;
    Unit *unit;
    UnitView *uview, *newuview;
    SeerNode seers [BUFSIZE];
    int numseers = 0, i = 0, seenclearly = FALSE, cellcover = 0;
    ParamBoxUnitSideSeers paramboxus;

    if (!in_area(x, y))
      return FALSE;
    update = updatet = rslt = FALSE;
    if (cover(side, x, y) > 0) {
    	/* Always update our knowledge of the terrain. */
    	curview = terrain_view(side, x, y);
 	if (curview == UNSEEN || !g_see_terrain_always()) {
	    set_terrain_view(side, x, y, buildtview(terrain_at(x, y)));
	    if (!g_see_terrain_always()) {
		set_terrain_view_date(side, x, y, g_turn());
	    }
	    update = updatet = TRUE;
	    if (side->see_all) {
		add_cover(side, x, y, 1);
	    } else {
		for_all_stack(x, y, unit) {
		    if (u_see_always(unit->type)) {
			add_cover(side, x, y, 1);
			break;
		    }
		}
	    }
	}
	if (any_material_views) {
	    mupdate = see_materials(side, x, y);
	    if (mupdate)
	      update = TRUE;
	}
	if (temperatures_defined() || clouds_defined() || winds_defined()) {
	    wupdate = see_weather(side, x, y);
	    if (wupdate)
	      update = TRUE;
	}
	if (Xconq::any_see_chances < 0)
	  compute_see_chances();
	/* Get rid of any unit views that are no longer valid here. */
	domore = TRUE;
	while (domore) {
	    domore = FALSE;
	    for_all_view_stack(side, x, y, uview) {
		/* Conditionally flush unit views. */
		/*! \note Improve this code to consider view flushing 
			  granularity. */
		unit = view_unit(uview);
		if (!unit || (uview->x != unit->x) || (uview->y != unit->y)
		    || (uview->transport 
			&& (view_unit(uview->transport) != unit->transport))
		    || (!u_see_always(uview->type) 
			&& (uview->date < g_turn()))) {
		    if (remove_unit_view(side, uview)) {
			update = TRUE;
		    }
		    domore = TRUE;
		    break;
		}
	    }
	}
	/* Iterate over all units-to-see in a given location. */
	for_all_stack(x, y, unit) {
	    /* Reset the flag that says whether we've spotted this
	       particular unit in the stack. */
	    sawthis = FALSE;
	    /* If any definite sightings. */
	    if (side_sees_unit(side, unit)) {
		newuview = add_unit_view(side, unit);
		if (newuview)
		  update = TRUE;
		rslt = sawthis = TRUE;
	    /* If any possible sightings. */
	    /* All see chances may be perfect by default ('any_see_chances' 
	       will be FALSE), but terrain, night, etc... modifications still 
	       take place, and mistaken viewings can still occur. */
	    }
	    else {
		numseers = 0;
		cellcover = cover(side, x, y);
#if (0)
		/* Allocate array of max possible seers. */
		if (cellcover > 0)
		  seers = (SeerNode *)xmalloc(cellcover * sizeof(SeerNode));
#endif
		/* Reset the seers array. */
		memset(seers, 0, BUFSIZE*sizeof(SeerNode));
		/* Setup parambox to iterate through ustacks. */
		paramboxus.unit = unit;
		paramboxus.side = side;
		paramboxus.seers = seers;
		/* Look in the same cell. */
		if (cellcover)
		  side_ustack_see_unit(x, y, &numseers, 
				       (ParamBox *)(&paramboxus));
		/* Look in other cells out to the maximum vision range. */
		/* (NOTE: There should probably be a performance choice 
		    here. If the max vision range is sufficiently large, then 
		    it may be cheaper to iterate through the side units 
		    instead of searching the area.) */
		if ((numseers < cellcover) 
		    && (0 < Xconq::max_see_chance_range)) {
		    limited_search_around(x, y, Xconq::max_see_chance_range, 
					  side_ustack_see_unit, 1, &numseers, 
					  cellcover, (ParamBox *)&paramboxus);
		}
		/* Let the caller know that something is seen. */
		if (numseers)
		  rslt = TRUE;
		seenclearly = FALSE;
		/* Now that we have gathered up all the seers, start 
		   building unmistaken unit views. */
		for (i = 0; i < numseers; ++i) {
		    /* Skip over mistaken views. */
		    if (seers[i].mistakes)
		      continue;
		    /* Construct an unit view. */
		    /* If coordinated vision, then we build the new uview 
		       without hesitation. */
		    /* (NOTE: Currently, uncoordinated vision is not supported 
			by the Xconq uview code.) */
		    if (1 /* (TODO: Replace with appropriate flag.) */) {
			newuview = add_unit_view(side, unit);
			seenclearly = TRUE;
		    }
		    /* If this is a new unit view, then a display update is 
		       needed. */
		    /* (NOTE: This machinery will probably need to be 
			changed if uncoordinated vision or advanced fog-of-war 
			make it into Xconq.) */
		    if (newuview)
		      update = TRUE;
		    /* If coordinated vision, then any single accurate 
		       sighting will be sufficient, and we don't need to 
		       build additional unit views. */
		    if (1 /* (TODO: Replace with appropriate flag.) */)
		      break;
		} /* for all unmistaken views */
		/* If no one saw anything clearly, then investigate the 
		   possibility of mistaken views. */
		for (i = 0; i < numseers; ++i) {
		    /* If coordinated vision and something else already 
		       clearly seen, then what are we doing here? Get out. */
		    if (1 /* (TODO: Replace with appropriate flag.) */
			&& seenclearly)
		      break;
		    /* If coordinated vision and we are still in this loop, 
		       then all seers must be mistaken about what they see. */
		    if (1 /* (TODO: Replace with appropriate flag.) */) {
			newuview = add_unit_view(side, unit);
			/* If this is a new unit view, then a display update is 
			   needed. */
			/* (NOTE: This machinery will probably need to be 
			    changed if uncoordinated vision or advanced 
			    fog-of-war make it into Xconq.) */
			if (newuview)
			  update = TRUE;
			/* If no advanced fog-of-war, then we pick one of the 
			   seers at random and agree upon its view of things, 
			   and don't bother with other possible views. */
			if (newuview
			    && 1 /* (TODO: Replace with appropriate flag.) */) {
			    /* Mistake the view. */
			    mistake_view(side, seers[xrandom(numseers)].seer, 
					 newuview);
			    break;
			}
			/* (TODO: Handle advanced fog-of-war.) */
		    } /* Coordinated vision. */
		    /* Skip over unmistaken views. */
		    if (!(seers[i].mistakes))
		      continue;
		    /* (TODO: Handle uncoordinated vision.) */
		} /* for all mistaken views */
#if (0)
		/* Deallocate array of seers if necessary. */
		if (seers) {
		    free(seers);
		    seers = NULL;
		}
#endif
	    }
	} /* for all stack units */
	/* Check if any populations in the cell see something. */
	/* 'side_sees_unit' will detect an unit if the people of a side 
	    always see and are present in a cell. */
	if (!Xconq::people_always_see && Xconq::any_people_see_chances
	    && people_sides_defined()
	    && people_side_at(x, y) == side->id
	    && any_cell_materials_defined()) {
	    for_all_stack(x, y, unit) {
		for_all_material_types(m) {
		    if (cell_material_defined(m)
			&& material_at(x, y, m) > 0) {
			chance = um_people_see(unit->type, m);
			if (probability(chance)) {
			    newuview = add_unit_view(side, unit);
			    if (newuview)
			      update = TRUE;
			    /* Proceed to consider the next unit. */
			    break;
			}
		    }
		}
   	    }
	}
    }
    /* If there was any change in what was visible, tell the display. */
    if (update) {
	flags = UPDATE_ALWAYS;
	/* If the view of the terrain changed, adjacent cells probably
	   need to be redrawn as well. */
	if (updatet)
	  flags |= UPDATE_ADJ;
	update_cell_display(side, x, y, flags);
    }
    /* Indicate whether any units were seen at this location, even if
       no display updates were needed. */
    return rslt;
}

/* If the viewing unit might possibly misidentify the type of unit it
   saw, compute the chance and replace the view's type with one of the
   possible types that it could be mistaken for. */

static void
mistake_view(Side *side, Unit *seer, UnitView *uview)
{
    assert_warning_return(seer, "Attempted to see with a NULL unit",);
    assert_warning_return(side, "Attempted to access a NULL side",);
    assert_warning_return(uview, "Attempted to see a NULL unit view",);
    uview->type = mistaken_type(uview->type);
    uview->image_name = NULL;
    uview->imf = NULL;
    set_unit_view_image(uview);
}

/* Given a unit type, return a similar-looking type that it might be
   confused with. */

static int
mistaken_type(int u2)
{
    int u3, totalweight, randval;

    totalweight = 0;
    for_all_unit_types(u3) {
	totalweight += uu_looks_like(u2, u3);
    }
    /* If nothing that it looks like, return original type. */
    if (totalweight == 0)
      return u2;
    randval = xrandom(totalweight);
    totalweight = 0;
    for_all_unit_types(u3) {
	totalweight += uu_looks_like(u2, u3);
	if (randval < totalweight)
	  return u3;	
    }
    return u2;
}

/* "Bare-bones" viewing, for whenever you know exactly what's there.
   This is the lowest level of all viewing routines, and executed a lot. */

void
see_exact(Side *side, int x, int y)
{
    int oldtview, newtview, update, mupdate, wupdate, domore;
    Unit *unit;
    UnitView *uview;

    if (!in_area(x, y))
      return;
    if (side->see_all) {
	/* It may not really be necessary to do anything to the
	   display, but the kernel doesn't know if the interface is
	   drawing all the units that are visible, or is only drawing
	   "interesting" ones, or whatever.  It would be up to the
	   interface to decide that, say, its magnification power for
	   a map is such that only one unit is being displayed, and
	   that the update from here doesn't result in any visible
	   changes to what's already been drawn on the screen. */
    	update = TRUE;
    } else {
	update = FALSE;
    	oldtview = terrain_view(side, x, y);
    	newtview = buildtview(terrain_at(x, y));
    	set_terrain_view(side, x, y, newtview);
	set_terrain_view_date(side, x, y, g_turn());
	if (oldtview != newtview)
	  update = TRUE;
	mupdate = see_materials(side, x, y);
	if (mupdate)
	  update = TRUE;
	wupdate = see_weather(side, x, y);
	if (wupdate)
	  update = TRUE;
	/* Flush all unit views. */
	domore = TRUE;
	while (domore) {
	    domore = FALSE;
	    for_all_view_stack(side, x, y, uview) {
		if (remove_unit_view(side, uview)) {
		    update = TRUE;
		}
		domore = TRUE;
		break;
	    }
	}
	for_all_stack(x, y, unit) {
	    uview = add_unit_view(side, unit);
	    if (uview)
	      update = TRUE;
	}
    }
    /* If there was any change in what was visible, tell the display. */
    if (update) {
	update_cell_display(side, x, y, UPDATE_ALWAYS | UPDATE_ADJ);
    }
}

static int
see_materials(Side *side, int x, int y)
{
    int m, curview, update;

    update = FALSE;
    if (!any_material_views)
      return update;
    for_all_material_types(m) {
	if (any_material_views_by_m[m]) {
	    curview = material_view(side, x, y, m);
	    if (curview != material_at(x, y, m)) {
		set_material_view(side, x, y, m, material_at(x, y, m));
		update = TRUE;
	    }
	    /* Even if the data didn't change, our information is
	       now up-to-date. */
	    if (1)
	      set_material_view_date(side, x, y, m, g_turn());
	}
    }
    return update;
}

static int
see_weather(Side *side, int x, int y)
{
    int curview, update;

    update = FALSE;
    if (temperatures_defined()) {
	curview = temperature_view(side, x, y);
	if (curview != temperature_at(x, y)) {
	    set_temperature_view(side, x, y, temperature_at(x, y));
	    update = TRUE;
	}
	/* Even if the data didn't change, our information is
	   now up-to-date. */
	if (!g_see_weather_always())
	  set_temperature_view_date(side, x, y, g_turn());
    }
    if (clouds_defined()) {
	curview = cloud_view(side, x, y);
	if (curview != raw_cloud_at(x, y)) {
	    set_cloud_view(side, x, y, raw_cloud_at(x, y));
	    update = TRUE;
	}
	curview = cloud_bottom_view(side, x, y);
	if (cloud_bottoms_defined() 
	    && (curview != raw_cloud_bottom_at(x, y))) {
	    set_cloud_bottom_view(side, x, y, raw_cloud_bottom_at(x, y));
	    update = TRUE;
	}
	curview = cloud_height_view(side, x, y);
	if (cloud_heights_defined()
	    && (curview != raw_cloud_height_at(x, y))) {
	    set_cloud_height_view(side, x, y, raw_cloud_height_at(x, y));
	    update = TRUE;
	}
	/* Even if the data didn't change, our information is
	   now up-to-date. */
	if (!g_see_weather_always())
	  set_cloud_view_date(side, x, y, g_turn());
	/* Only need one date for the three layers of view data. */
    }
    if (winds_defined()) {
	curview = wind_view(side, x, y);
	if (curview != raw_wind_at(x, y)) {
	    set_wind_view(side, x, y, raw_wind_at(x, y));
	    update = TRUE;
	}
	/* Even if the data didn't change, our information is
	   now up-to-date. */
	if (!g_see_weather_always())
	  set_wind_view_date(side, x, y, g_turn());
    }
    return update;
}

/* A border has been seen if the cells on both sides has been seen. */

int
seen_border(Side *side, int x, int y, int dir)
{
    int x1, y1;

    if (side->see_all)
      return TRUE;
    if (terrain_view(side, x, y) == UNSEEN)
      return FALSE;
    if (point_in_dir(x, y, dir, &x1, &y1))
      if (terrain_view(side, x1, y1) == UNSEEN)
	return FALSE;
    return TRUE;
}

/* Make a printable identification of the given side.  This should be
   used for debugging and designing, not regular play. */

char *
side_desig(Side *side)
{
    Unit *unit;

    if (sidedesigbuf == NULL)
      sidedesigbuf = (char *)xmalloc(BUFSIZE);
    if (side != NULL) {
	/* The side name is all capitalized in the output. Why? */
	sprintf(sidedesigbuf, "s%d %s", side_number(side), side_name(side));
	if (side->self_unit) {
		unit = find_unit_dead_or_alive(side->self_unit->id);
		if (in_play(unit)) {
			if (!mobile(unit->type) || u_advanced(unit->type)) {
				tprintf(sidedesigbuf, " (capital %s %s)", 
						(side->ingame ? "is" : "was"),
						short_unit_handle(unit));
			} else {
				tprintf(sidedesigbuf, " (leader %s %s)", 
						(side->ingame ? "is" : "was"),
						short_unit_handle(unit));
			}
		}
	}
    } else {
	sprintf(sidedesigbuf, "nullside");
    }
    return sidedesigbuf;
}

/* Add a player into the list of players.  All values are defaults here. */

Player *
add_player(void)
{
    Player *player = (Player *) xmalloc(sizeof(Player));

    player->id = nextplayerid++;
    /* Note that all names and suchlike slots are NULL. */
    ++numplayers;
    /* Add this one to the end of the player list. */
    if (last_player == NULL) {
	playerlist = last_player = player;
    } else {
	last_player->next = player;
	last_player = player;
    }
    Dprintf("Added player #%d\n", player->id);
    return player;
}

Player *
find_player(int n)
{
    Player *player;

    for_all_players(player) {
       if (player->id == n)
	return player;
    }
    return NULL;
}

/* Transform a player object into a regularized form. */

void
canonicalize_player(Player *player)
{
    if (player == NULL)
      return;
    if (empty_string(player->displayname))
      player->displayname = NULL;
    if (empty_string(player->aitypename))
      player->aitypename = NULL;
    /* This seems like a logical place to canonicalize an AI type
       of "ai" into a specific type, but we don't necessarily know
       the best default until the game is closer to starting. */
}

/* Make a printable identification of the given player.  The output
   here must be correctly parseable according to player spec rules,
   although it doesn't have to be pretty or minimal. */

char *
player_desig(Player *player)
{
    if (playerdesigbuf == NULL)
      playerdesigbuf = (char *)xmalloc(BUFSIZE);
    if (player != NULL) {
	snprintf(playerdesigbuf, BUFSIZE, "%s,%s/%s@%s+%d",
		(player->name ? player->name : ""),
		(player->aitypename ? player->aitypename : ""),
		(player->configname ? player->configname : ""),
		(player->displayname ? player->displayname : ""),
		player->advantage);
    } else {
	sprintf(playerdesigbuf, "nullplayer");
    }
    return playerdesigbuf;
}

/* Return the net advantage of the side, accounting for both the
   side's predetermined advantage and the player's choice. */

int
actual_advantage(Side *side)
{
    int advantage = max(1, side->advantage);

    if (side->player)
      advantage = side->player->advantage;
    return advantage;
}

/* Doctrine handling. */

Doctrine *
new_doctrine(int id)
{
    Doctrine *doctrine = (Doctrine *) xmalloc(sizeof(Doctrine));

    /* Doctrine ids start at 1 and go up. */
    if (id <= 0)
      id = next_doctrine_id++;
    else
      next_doctrine_id = id + 1;
    doctrine->id = id;
    /* Fill in all the doctrine slots with their default values. */
    doctrine->resupply_percent = 50;
    doctrine->rearm_percent = 20;
    /* 35% is basically 1/3, rounded up. */
    doctrine->repair_percent = 35;
    doctrine->resupply_complete = 100;
    doctrine->rearm_complete = 100;
    doctrine->repair_complete = 100;
    doctrine->min_turns_food = 5;
    doctrine->min_distance_fuel = 20;
    doctrine->construction_run = (short *) xmalloc (numutypes * sizeof(short));
    /* We just committed on the number of unit types. */
    disallow_more_unit_types();
    /* Add the new doctrine to the end of the list of doctrines. */
    if (last_doctrine != NULL) {
	last_doctrine->next = doctrine;
	last_doctrine = doctrine;
    } else {
	doctrine_list = last_doctrine = doctrine;
    }
    return doctrine;
}

Doctrine *
find_doctrine_by_name(char *name)
{
    Doctrine *doctrine;

    if (name == NULL)
      return NULL;
    for_all_doctrines(doctrine) {
	if (doctrine->name != NULL && strcmp(name, doctrine->name) == 0)
	  return doctrine;
    }
    return NULL;
}

Doctrine *
find_doctrine(int id)
{
    Doctrine *doctrine;

    for_all_doctrines(doctrine) {
	if (doctrine->id == id)
	  return doctrine;
    }
    return NULL;
}

#if 0 /* seems useful, but never actually used */
Doctrine *
clone_doctrine(Doctrine *olddoc)
{
    int tmpid;
    Doctrine *newdoc, *tmpnext;

    newdoc = new_doctrine(0);
    tmpid = newdoc->id;
    tmpnext = newdoc->next;
    memcpy(newdoc, olddoc, sizeof(Doctrine));
    newdoc->id = tmpid;
    /* Always allocate and copy subarrays. */
    newdoc->construction_run = (short *) xmalloc (numutypes * sizeof(short));
    memcpy(newdoc->construction_run, olddoc->construction_run, numutypes * sizeof(short));
    newdoc->next = tmpnext;
    return newdoc;
}
#endif

/* Standing order handling. */

StandingOrder *
new_standing_order(void)
{
    StandingOrder *sorder;

    sorder = (StandingOrder *) xmalloc(sizeof(StandingOrder));
    sorder->types = (char *)xmalloc(numutypes);
    return sorder;
}

/* Add a new standing order for the side.  This function can add to front
   or back of existing list of orders. */

int order_conds_match(StandingOrder *sorder, StandingOrder *sorder2);

void
add_standing_order(Side *side, StandingOrder *sorder, int pos)
{
    StandingOrder *sorder2, *saved;

    if (sorder->task == NULL) {
	/* Cancelling an order. */
	saved = NULL;
	if (side->orders == NULL) {
	    /* No orders, so nothing to do. */
	    notify(side, "No orders to cancel");
	} else if (order_conds_match(sorder, side->orders)) {
	    /* Delete the first order in the list. */
	    saved = side->orders;
	    if (side->last_order == side->orders)
	      side->last_order = side->orders->next;
	    side->orders = side->orders->next;
	} else {
	    for (sorder2 = side->orders; sorder2->next != NULL; sorder2 = sorder2->next) {
		if (order_conds_match(sorder, sorder2->next)) {
		    saved = sorder2->next;
		    if (side->last_order == sorder2->next)
		      side->last_order = sorder2->next->next;
		    sorder2->next = sorder2->next->next;
		    break;
		}
	    }
	    /* If we're here, no match; might be user error, but can't be sure,
	       so don't say anything. */
	}
	if (saved != NULL) {
	    notify(side, "Cancelled order `%s'", standing_order_desc(saved, spbuf));
	}
    } else if (pos == 0) {
	/* Add order to front of list. */
	sorder->next = side->orders;
	side->orders = sorder;
	if (side->last_order == NULL)
	  side->last_order = sorder;
    } else if (side->last_order != NULL) {
	/* Add order to end of list. */
	side->last_order->next = sorder;
	side->last_order = sorder;
    } else {
	/* First standing order for the side. */
	side->orders = side->last_order = sorder;
    }
}

int
order_conds_match(StandingOrder *sorder, StandingOrder *sorder2)
{
    return (sorder->condtype == sorder2->condtype
	    && sorder->a1 == sorder2->a1
	    && sorder->a2 == sorder2->a2
	    && sorder->a3 == sorder2->a3);
}

int
parse_standing_order(Side *side, char *cmdstr)
{
    StandingOrder *sorder, *sorder2;

    if (cmdstr[0] == '?') {
	if (side->orders != NULL) {
	    notify(side, "Current standing orders:");
	    for (sorder2 = side->orders; sorder2 != NULL; sorder2 = sorder2->next) {
		notify(side, "  %s", standing_order_desc(sorder2, spbuf));
	    }
	} else {
	    notify(side, "No standing orders in effect.");
	}
	return 0;
    }
    sorder = new_standing_order();
    cmdstr = parse_unit_types(side, cmdstr, sorder->types);
    if (cmdstr == NULL)
      return (-1);
    cmdstr = parse_order_cond(side, cmdstr, sorder);
    if (cmdstr == NULL)
      return (-2);
    cmdstr = parse_task(side, cmdstr, &(sorder->task));
    if (cmdstr == NULL)
      return (-3);
    add_standing_order(side, sorder, 0);
    if (sorder->task != NULL) {
	notify(side, "New standing order: %s", standing_order_desc(sorder, spbuf));
    }
    return 0;
}

/* (should all go to nlang.c?) */

char *
parse_unit_types(Side *side, char *str, char *utypevec)
{
    char *arg, substr[BUFSIZE], *rest;
    int u;

    rest = get_next_arg(str, substr, &arg);
    u = utype_from_name(arg);
    if (u != NONUTYPE) {
	utypevec[u] = 1;
    } else if (strcmp(arg, "all") == 0) {
	for_all_unit_types(u)
	  utypevec[u] = 1;
    } else {
	notify(side, "Unit type \"%s\" not recognized", arg);
	return NULL;
    }
    return rest;
}

char *
parse_order_cond(Side *side, char *str, StandingOrder *sorder)
{
    int x = 0, y = 0, dist = 0;
    char *arg, *arg2, substr[BUFSIZE], *rest;
    Unit *unit;

    rest = get_next_arg(str, substr, &arg);
    if (strcmp(arg, "at") == 0 || strcmp(arg, "@") == 0) {
	sorder->condtype = sorder_at;
    } else if (strcmp(arg, "in") == 0) {
	sorder->condtype = sorder_in;
    } else if (strcmp(arg, "within") == 0 || strcmp(arg, "near") == 0) {
	sorder->condtype = sorder_near;
    } else {
	notify(side, "Condition type \"%s\" not recognized", arg);
	return NULL;
    }
    if (sorder->condtype == sorder_near) {
	rest = get_next_arg(rest, substr, &arg);
	dist = strtol(arg, &arg2, 10);
	sorder->a3 = dist;
    }
    rest = get_next_arg(rest, substr, &arg);
    x = strtol(arg, &arg2, 10);
    if (arg != arg2 && *arg2 == ',') {
	y = strtol(arg2 + 1, &arg, 10);
	sorder->a1 = x;  sorder->a2 = y;
	return rest;
    } else if ((unit = find_unit_by_name(arg)) != NULL) {
	if (sorder->condtype == sorder_at || sorder->condtype == sorder_near) {
	    sorder->a1 = x;  sorder->a2 = y;
	} else if (sorder->condtype == sorder_in) {
	    sorder->a1 = unit->id;
	} else {
	    return NULL;
	}
	return rest;
    } else {
	notify(side, "Condition argument \"%s\" not recognized", arg);
	return NULL;
    }
}

char *
standing_order_desc(StandingOrder *sorder, char *buf)
{
    int u, v = -1, i = 1;
    int *args;

    for_all_unit_types(u) {
	if (sorder->types[u]) {
	    if (v < 0)
	      v = u;
	} else {
	    i = 0;
	}
    }
    if (v < 0 || sorder->task == NULL) {
	strcpy(buf, "invalid");
	return buf;
    }
    sprintf(buf, "if %s ", (i ? "all" : u_type_name(v)));

    switch (sorder->condtype) {
    case sorder_at:
	tprintf(buf, "at %d,%d ", sorder->a1, sorder->a2);
	break;
    case sorder_in:
	tprintf(buf, "in %s ", short_unit_handle(find_unit(sorder->a1)));
	break;
    case sorder_near:
	tprintf(buf, "within %d %d,%d ", sorder->a3,  sorder->a1, sorder->a2);
	break;
    default:
	strcat(buf, "unknown");
	return buf;
    }

    i = sorder->task->type;
    args = sorder->task->args;
    switch (i) {
    case TASK_MOVE_TO:
	tprintf(buf, "%s %d,%d", taskdefns[i].display_name, args[0], args[1]);
	break;
    case TASK_SENTRY:
	tprintf(buf, "%s %d", taskdefns[i].display_name, args[0]);
	break;
    default:
	task_desc(buf+strlen(buf), NULL, NULL, sorder->task);
	break;
    }
    return buf;
}

/* Collect the next whitespace-separated argument. */
/* (should move to util.c or nlang.c) */

char *
get_next_arg(char *str, char *buf, char **rsltp)
{
    char *p;

    strcpy(buf, str);
    p = buf;
    /* Skip past any leading whitespace. */
    while (isspace(*p))
      ++p;
    if (*p == '"') {
	++p;
	*rsltp = p;
	while (*p != '"' && *p != '\0')
	  ++p;
	*p = '\0';
	/* Increment so later scanning looks past the closing quote. */ 
	++p;
    } else {
	*rsltp = p;
	while (!isspace(*p) && *p != '\0')
	  ++p;
	*p = '\0';
    }
    return str + (p - buf);
}

#if 0

/* Agreement handling here for now. */

Agreement *agreement_list = NULL;

Agreement *last_agreement;

int num_agreements = 0;

int next_agreement_id = 1;

char *agreement_desig_buf;

void
init_agreements(void)
{
    agreement_list = last_agreement = NULL;
    num_agreements = 0;
}

Agreement *
create_agreement(int id)
{
    Agreement *ag = (Agreement *) xmalloc(sizeof(Agreement));

    if (id == 0)
      id = next_agreement_id++;
    ag->id = id;
    ag->terms = lispnil;
    ag->next = NULL;
    if (agreement_list != NULL) {
	last_agreement->next = ag;
    } else {
	agreement_list = ag;
    }
    last_agreement = ag;
    ++num_agreements;
    return ag;
}

Agreement *
find_agreement(int id)
{
    Agreement *ag;

    for_all_agreements(ag) {
	if (ag->id == id)
	  return ag;
    }
    return NULL;
}

char *
agreement_desig(Agreement *ag)
{
    if (agreement_desig_buf == NULL)
      agreement_desig_buf = (char *)xmalloc(BUFSIZE);
    sprintf(agreement_desig_buf, "<ag %s %s %s>",
	    (ag->agtype ? ag->agtype : "(null)"),
	    (ag->name ? ag->name : "(null)"),
	    (ag->terms == lispnil ? "(no terms)" : "...terms..."));
    return agreement_desig_buf;
}

#endif

/* Helper functions to init view layers from rle encoding. */

void
fn_set_terrain_view(int x, int y, int val)
{
    int rawval;

    if (1)
      rawval = val;
    else
      /* This is a more efficient encoding, but only usable if can guarantee
	 see-terrain-always upon rereading. */
      rawval = (val ? buildtview(terrain_at(x, y)) : 0);
    set_terrain_view(tmpside, x, y, rawval);
}

void
fn_set_terrain_view_date(int x, int y, int val)
{
    set_terrain_view_date(tmpside, x, y, val);
}

void
fn_set_aux_terrain_view(int x, int y, int val)
{
    /* Filter anything but the basic six bits. */
    val &= 0x3f;
    set_aux_terrain_view(tmpside, x, y, tmpttype, val);
}

void
fn_set_aux_terrain_view_date(int x, int y, int val)
{
    set_aux_terrain_view_date(tmpside, x, y, tmpttype, val);
}

void
fn_set_material_view(int x, int y, int val)
{
    set_material_view(tmpside, x, y, tmpmtype, val);
}

void
fn_set_material_view_date(int x, int y, int val)
{
    set_material_view_date(tmpside, x, y, tmpmtype, val);
}

void
fn_set_temp_view(int x, int y, int val)
{
    set_temperature_view(tmpside, x, y, val);
}

void
fn_set_temp_view_date(int x, int y, int val)
{
    set_temperature_view_date(tmpside, x, y, val);
}

void
fn_set_cloud_view(int x, int y, int val)
{
    set_cloud_view(tmpside, x, y, val);
}

void
fn_set_cloud_bottom_view(int x, int y, int val)
{
    set_cloud_bottom_view(tmpside, x, y, val);
}

void
fn_set_cloud_height_view(int x, int y, int val)
{
    set_cloud_height_view(tmpside, x, y, val);
}

void
fn_set_cloud_view_date(int x, int y, int val)
{
    set_cloud_view_date(tmpside, x, y, val);
}

void
fn_set_wind_view(int x, int y, int val)
{
    set_wind_view(tmpside, x, y, val);
}

void
fn_set_wind_view_date(int x, int y, int val)
{
    set_wind_view_date(tmpside, x, y, val);
}

#ifdef DESIGNERS

static void paint_view_1(int x, int y);

static int tmptview;
static int tmpuviewflag;

/* Paint the side's view with given values. */

void
paint_view(Side *side, int x, int y, int r, int tview, int uviewflag)
{
    tmpside = side;
    tmptview = tview;
    tmpuviewflag = uviewflag;
    apply_to_area_plus_edge(x, y, r, paint_view_1);
}

static void
paint_view_1(int x, int y)
{
    int tview, oldtview = terrain_view(tmpside, x, y);

    if (oldtview != tmptview) {
	tview = tmptview;
	if (tview == 1234567) {
	    tview = buildtview(terrain_at(x, y));
	}
	set_terrain_view(tmpside, x, y, tview);
	/* If the flag says so, see all units in the cell, otherwise
	   leave all units unseen. */
	/* (should we do a see_cell in any case?) */
	if (tmpuviewflag && tview != UNSEEN) {
	    see_exact(tmpside, x, y);
	}
	update_cell_display(tmpside, x, y, UPDATE_ALWAYS | UPDATE_ADJ);
    }
}

#endif /* DESIGNERS */

/* Find a side's total supply of a given material.  */

int 
side_material_supply(Side * side, int m)
{
	Unit	* unit;
	int	supply = 0;

	if (!is_material_type(m)) {
		return -1;
	}
	/* Add up unit supplies. */
	for_all_side_units(side, unit) {
		/* This also counts units under construction, which
		can store materials. */
		if (in_play(unit)) {
			supply += unit->supply[m];
		}
	}
	/* Add stuff in the side's treasury. */
	if (side_has_treasury(side, m)) {
		supply += side->treasury[m];
	}
	return supply;
}

/* Find a side's total production of a given material. */

int side_material_production(Side * side, int m)
{
	Unit	*unit;
	int	x, y;
	int	production = 0;

	for_all_side_units(side, unit) {
		if(!is_active(unit)) {
			continue;
		}
		/* Only advanced units collect materials from cells. */
		if (u_advanced(unit->type)) {
			for_all_cells_within_reach(unit, x, y) {
				if (!inside_area(x, y)) {
					continue;
				}
				if (user_at(x, y) == unit->id) {
					production += production_at(x, y, m);
				}
			}
		}
		/* Both advanced and non-advanced units may do this. */
		production += base_production(unit, m);
	}
	return production;
}
/* Find a side's total storage capacity for a given material. */

int side_material_storage(Side * side, int m)
{
	Unit	*unit;
	int	storage = 0;

	for_all_side_units(side, unit) {
		/* This also counts units under construction, which
		can store materials. */
		if(!in_play(unit)) {
			continue;
		}
		storage += um_storage_x(unit->type, m);
	}
	/* Add room in the side's treasury if it exists. */
	if (side_has_treasury(side, m)) {
		/* Clip to VARHI to prevent numeric overflow. */
		storage += min(g_treasury_size(), VARHI - storage);
	}
	return storage;
}

/* Selectively choose an advance from a weighted list accounting for what 
   advances the side already has and could possibly research. */

Obj *
choose_side_research_goal_from_weighted_list(Obj *lis, int *totalweightp, 
					     Side *side)
{
    int n, sofar, weight, a = NONATYPE;
    char buf[BUFSIZE];
    Obj *rest, *head, *tail, *rslt, *adv;

    assert_error(lis, "Attempted to access a NULL list");
    assert_error(lis != lispnil, "Attempted to use a nil list");
    assert_error(side, "Attempted to access a NULL side");
    if (*totalweightp <= 0) {
	for_all_list(lis, rest) {
	    head = car(rest);
	    if (symbolp(head))
	      adv = eval_symbol(head);
	    else if (consp(head))
	      adv = cadr(head);
	    else
	      adv = head;
	    if (!atypep(adv) || !is_advance_type(c_number(adv))) {
		Dprintf(
"Illegal list element encountered in a side research goal list.");
		return lispnil;
	    }
	    else
	      a = c_number(adv);
	    if (has_advance(side, a) || side->research_precluded[a])
	      continue;
	    weight = ((consp(head) && numberp(car(head))) ? c_number(car(head)) 
							  : 1);
	    *totalweightp += weight;
	}
    }
    /* If total weight is 0, then return nil. */
    if (*totalweightp == 0) 
      return lispnil;
    n = xrandom(*totalweightp);
    sofar = 0;
    rslt = lispnil;
    for_all_list(lis, rest) {
	head = car(rest);
	if (symbolp(head))
	  adv = eval_symbol(head);
	else if (consp(head))
	  adv = cadr(head);
	else
	  adv = head;
	a = c_number(adv);
	if (has_advance(side, a) || side->research_precluded[a])
	  continue;
	if (consp(head) && numberp(car(head))) {
	    sofar += c_number(car(head));
	    tail = cadr(head);
	} else {
	    sofar += 1;
	    tail = head;
	}
	if (sofar > n) {
	    rslt = tail;
	    break;
	}
    }
    return rslt;
}
