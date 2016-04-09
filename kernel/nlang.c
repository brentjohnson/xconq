/* Interface-independent natural language handling for Xconq.
   Copyright (C) 1987-1989, 1991-2000 Stanley T. Shebs.
   Copyright (C) 2004-2005 Eric A. McDonald.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/* This file should be entirely replaced for non-English Xconq. */
/* (One way to do this would be to call this file "nlang-en.c", then
   symlink this or nlang-fr.c, etc to nlang.c when configuring;
   similarly for help.c.) */
/* (A better way to would be to run all these through a dispatch
   vector, then each player could get messages and such in a
   different language.) */
/* (Or just use the 'msgcat' and other GNU NLS stuff.) */

#include "conq.h"
#include "kernel.h"

#include "aiscore.h"
#include "aiunit.h"
#include "aiunit2.h"
#include "aitact.h"
#include "aioprt.h"

static void notify_combat(Unit *unit, Unit *atker, const char *str);
static int pattern_matches_combat(Obj *pattern, Unit *unit, Unit *unit2);
static void combat_desc_from_list(Side *side, Obj *lis, Unit *unit,
				  Unit *unit2, const char *str, char *buf);
static void init_calendar(void);
static void parse_date_step_range(Obj *form);
static void maybe_mention_date(Side *side);
static int gain_count(Side *side, int u, int r);
static int loss_count(Side *side, int u, int r);
static int atkstats(Side *side, int a, int d);
static int hitstats(Side *side, int a, int d);
static void pad_out(char *buf, int n);
static const char *past_unit_handle(Side *side, PastUnit *past_unit);
static const char *short_side_title_with_adjective(Side *side, char *adjective);

static char *tmpnbuf;

static char *tmpdbuf;

static char *pluralbuf;

/* Short names of directions. */

const char *dirnames[] = DIRNAMES;

char *unitbuf = NULL;

char *past_unitbuf = NULL;

static char *side_short_title = NULL;

static const char *gain_reason_names[] = { "Ini", "Bld", "Cap", "Oth" };

static const char *loss_reason_names[] = { "Cbt", "Cap", "Stv", "Acc", "Att", "Dis", "Oth" };

/* Calendar handling. */

typedef enum {
    cal_unknown,
    cal_number,
    cal_usual
} CalendarType;

static CalendarType calendar_type = cal_unknown;

typedef enum {
    ds_second,
    ds_minute,
    ds_hour,
    ds_day,
    ds_week,
    ds_month,
    ds_season,
    ds_year
} UsualDateStepType;

typedef struct a_usualdate {
    int second;
    int minute;
    int hour;
    int day;
    int month;
    int year;
} UsualDate;

typedef struct a_usualdatesteprange {
    int turn_start;
    int turn_end;
    UsualDateStepType step_type;
    int step_size;
} UsualDateStepRange;

static const char *usual_date_string(int date);
static void parse_usual_date(const char *datestr, int range, UsualDate *udate);

static int turn_initial;

static UsualDateStepRange date_step_ranges[20];

static int num_date_step_ranges;

static const char *months[] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec", "???" };

static short monthdays[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31, 0 };

static const char *seasons[] = { "Win", "Spr", "Sum", "Aut" };

static UsualDate *usual_initial;

char *datebuf;

const char *turn_name;

char *featurebuf;

/* This is the number of types to mention by name; any others will
   just be included in the count of missing images. */

#define NUMTOLIST 5

static char *missinglist;

/* This array allows for counting up to 4 classes of missing images. */

static int missing[4];

static int totlisted = 0;

/* Initialize things.  Note that this happens before any game is loaded, so
   can't do game-specific init here. */

void
init_nlang(void)
{
    if (tmpnbuf == NULL)
      tmpnbuf = (char *)xmalloc(BUFSIZE);
    if (tmpdbuf == NULL)
      tmpdbuf = (char *)xmalloc(BUFSIZE);
    if (pluralbuf == NULL)
      pluralbuf = (char *)xmalloc(BUFSIZE);
    if (datebuf == NULL)
      datebuf = (char *)xmalloc(BUFSIZE);
}

/* Send a message to everybody who's got a screen. */

void
notify_all(const char *fmt, ...)
{
    va_list ap;
    Side *side;

    for_all_sides(side) {
	if (active_display(side)) {
	    maybe_mention_date(side);
	    va_start(ap, fmt);
	    vsprintf(tmpnbuf, fmt, ap);
	    va_end(ap);
	    /* Always capitalize first char of notice. */
	    capitalize(tmpnbuf);
	    low_notify(side, tmpnbuf);
	}
    }
}

void
notify(Side *side, const char *fmt, ...)
{
    va_list ap;

    if (!active_display(side))
      return;
    maybe_mention_date(side);
    va_start(ap, fmt);
    vsprintf(tmpnbuf, fmt, ap);
    va_end(ap);
    /* Always capitalize first char of notice. */
    capitalize(tmpnbuf);
    low_notify(side, tmpnbuf);
}

void
vnotify(Side *side, const char *fmt, va_list ap)
{
    if (!active_display(side))
      return;
    maybe_mention_date(side);
    vsprintf(tmpnbuf, fmt, ap);
    /* Always capitalize first char of notice. */
    capitalize(tmpnbuf);
    low_notify(side, tmpnbuf);
}

static void
maybe_mention_date(Side *side)
{
    /* Note that last_notice_date defaults to 0, so this means that
       any turn 0 notices will not have a date prepended (which is good). */
    if (g_turn() != side->last_notice_date) {
	sprintf(tmpdbuf, "%s:", absolute_date_string(g_turn()));
	low_notify(side, tmpdbuf);
	side->last_notice_date = g_turn();
    }
}

/* Pad a given buffer with blanks out to the given position, and cut
   off any additional content. */

void
pad_out(char *buf, int n)
{
    int i, len = strlen(buf);

    if (n < 1)
      return;
    for (i = len; i < n; ++i) {
	buf[i] = ' ';
    }
    buf[n - 1] = '\0';
}

/* Get a char string naming the side.  Doesn't have to be pretty. */

/* (should synth complete name/adjective from other parts of speech) */

const char *
side_name(Side *side)
{
    return (side->name ? side->name :
	    (side->adjective ? side->adjective :
	     (side->pluralnoun ? side->pluralnoun :
	      (side->noun ? side->noun :
	       (char *)""))));
}

const char *
side_adjective(Side *side)
{
    return (side->adjective ? side->adjective :
	    (side->noun ? side->noun :
	     (side->pluralnoun ? side->pluralnoun :
	      (side->name ? side->name :
	       (const char *)""))));
}

const char *
short_side_title(Side *side)
{
    if (side_short_title == NULL)
      side_short_title = (char *)xmalloc(BUFSIZE);
    if (side->name) {
	return side->name;
    } else if (side->pluralnoun) {
	sprintf(side_short_title, "the %s", side->pluralnoun);
    } else if (side->noun) {
	sprintf(side_short_title, "the %s", plural_form(side->noun));
    } else if (side->adjective) {
	sprintf(side_short_title, "the %s side", side->adjective);
    } else {
	return " - ";
    }
    return side_short_title;
}

const char *
short_side_title_with_adjective(Side *side, char *adjective)
{
    if (side_short_title == NULL)
      side_short_title = (char *)xmalloc(BUFSIZE);
    side_short_title[0] = '\0';
    if (side->name) {
	if (empty_string(adjective))
	  return side->name;
	else {
	    strcat(side_short_title, adjective);
	    strcat(side_short_title, " ");
	    strcat(side_short_title, side->name);
	}
    } else if (side->pluralnoun) {
	strcat(side_short_title, "the ");
	if (!empty_string(adjective)) {
	    strcat(side_short_title, adjective);
	    strcat(side_short_title, " ");
	}
	strcat(side_short_title, side->pluralnoun);
    } else if (side->noun) {
	strcat(side_short_title, "the ");
	if (!empty_string(adjective)) {
	    strcat(side_short_title, adjective);
	    strcat(side_short_title, " ");
	}
	strcat(side_short_title, side->noun);
    } else if (side->adjective) {
	strcat(side_short_title, "the ");
	if (!empty_string(adjective)) {
	    strcat(side_short_title, adjective);
	    strcat(side_short_title, " ");
	}
	strcat(side_short_title, side->adjective);
	strcat(side_short_title, " side");
    } else {
	return " - ";
    }
    return side_short_title;
}

/* This indicates whether the above routine returns a singular or plural form
   of title. */

int
short_side_title_plural_p(Side *side)
{
    if (side->name) {
	return FALSE;
    } else if (side->pluralnoun) {
	return TRUE;
    } else if (side->noun) {
	return TRUE;
    } else if (side->adjective) {
	sprintf(side_short_title, "the %s side", side->adjective);
	return FALSE;
    } else {
	return FALSE;
    }
}

const char *
shortest_side_title(Side *side2, char *buf)
{
    if (side2->name) {
	return side2->name;
    } else if (side2->adjective) {
	return side2->adjective;
    } else if (side2->noun) {
	return side2->noun;
    } else if (side2->pluralnoun) {
	return side2->pluralnoun;
    } else {
	sprintf(buf, "(#%d)", side_number(side2));
    }
    return buf;
}

const char *
sidemask_desc(char *buf, SideMask sidemask)
{
    int first = TRUE;
    Side *side2;

    if (sidemask < 0 || sidemask >= ((1 << (numsides + 1)) - 2))
      return "all";
    buf[0] = '\0';
    for_all_sides(side2) {
	if (side_in_set(side2, sidemask)) {
	    if (first)
	      first = FALSE;
	    else
	      strcat(buf, ", ");
	    strcat(buf, short_side_title(side2));
	}
    }
    return buf;
}

char *
side_score_desc(char *buf, Side *side, Scorekeeper *sk)
{
    if (!sk->keepscore) {
	buf[0] = 0;
	return buf;
    }
    if (!empty_string(sk->title)) {
    	strcpy(buf, sk->title);
    } else {
    	sprintf(buf, "Score");
    	if (numscorekeepers > 1) {
    		tprintf(buf, " %d", sk->id);
    	}
    }
    tprintf(buf, ": ");
    if (symbolp(sk->body)
        && match_keyword(sk->body, K_LAST_SIDE_WINS)) {
	tprintf(buf, "%d", side_point_value(side));
    } else if (symbolp(sk->body)
        && match_keyword(sk->body, K_LAST_ALLIANCE_WINS)) {
	if (has_allies(side)) {
		tprintf(buf, "%d", alliance_point_value(side));
		tprintf(buf, " (%d alone)", side_point_value(side));
    	} else {
		tprintf(buf, "%d", side_point_value(side));
       	}
    } else {
	/* Compose the generic scorekeeper status display. */
	if (sk->scorenum >= 0) {
	    tprintf(buf, "%d", side->scores[sk->scorenum]);
	}
    }
    return buf;
}

char *
long_player_title(char *buf, Player *player, char *thisdisplayname)
{
    buf[0] = '\0';
    if (player == NULL) {
	/* Do nothing */
    } else if (player->displayname != NULL) {
	if (player->name != NULL) {
	    strcat(buf, player->name);
	    strcat(buf, "@");
	}
	if (thisdisplayname != NULL
	    && strcmp(player->displayname, thisdisplayname) == 0
	    && player->rid == my_rid) {
	    strcat(buf, "You");
	} else {
	    strcat(buf, player->displayname);
	}
	if (player->aitypename != NULL) {
	    strcat(buf, "(& AI ");
	    strcat(buf, player->aitypename);
	    strcat(buf, ")");
	}
    } else if (player->aitypename != NULL) {
	strcat(buf, "AI ");
	strcat(buf, player->aitypename);
    } else {
	strcat(buf, "-");
    }
    return buf;
}

char *
short_player_title(char *buf, Player *player, char *thisdisplayname)
{
    buf[0] = '\0';
    if (player == NULL)
      return buf;
    if (player->name != NULL) {
	strcat(buf, player->name);
    }
    if (player->aitypename != NULL) {
	strcat(buf, ",");
	strcat(buf, player->aitypename);
    }
    if ((player->name != NULL || player->aitypename != NULL)
	&& player->displayname != NULL) {
	strcat(buf, "@");
    }
    if (thisdisplayname != NULL
	&& player->displayname != NULL
	&& strcmp(player->displayname, thisdisplayname) == 0) {
	strcat(buf, "You");
    } else if (player->displayname != NULL) {
	strcat(buf, player->displayname);
    }
    if (strlen(buf) == 0) {
	strcat(buf, "-");
    }
    return buf;
}

/* If we are not using the X11 interface, we don't need to pass thisdisplayname
in order to figure out if we are "You". This make things a lot simpler since the. */

char *
simple_player_title(char *buf, Player *player)
{
    buf[0] = '\0';
    if (player == NULL)
      return buf;
    /* First handle human players. */
    if (player->displayname) {
    	/* If we are "You" we are done. */
    	if (player->rid == my_rid) {
    		strcat(buf, "You");
    	/* If we have a "name" use it. */
    	} else if (player->name) {
    		strcat(buf, player->name);
	/* Else use the "displayname". This is where we usually end up, since
	whatever is passed in default_player_spec by default is used to set the 
	"displayname" and not the "name". */
	} else {
    		strcat(buf, player->displayname);
	}
	/* Add the ai name if it exists. */
	if (player->aitypename) {
		strcat(buf, " + ");
		strcat(buf, player->aitypename);	
	}
    /* Then handle ai players. */
    } else if (player->aitypename) { 		
	strcat(buf, player->aitypename);
    }
    if (strlen(buf) == 0) {
	strcat(buf, "-");
    }
    return buf;
}

/* Used in player setup. Returns "You" or name for human players
and "Computer" or "Nobody" for non-human players. */

char *
simple_player_name(char *buf, Player *player)
{
    buf[0] = '\0';
    if (player == NULL)
      return buf;
    /* First handle human players. */
    if (player->displayname) {
    	/* If we are "You" we are done. */
    	if (player->rid == my_rid) {
    		strcat(buf, "You");
    	/* If we have a "name" use it. */
    	} else if (player->name) {
    		strcat(buf, player->name);
	/* Else use the "displayname". This is where we usually end up, since
	whatever is passed in default_player_spec by default is used to set the 
	"displayname" and not the "name". */
	} else {
    		strcat(buf, player->displayname);
	}
    /* Then handle ai players. */
    } else if (player->aitypename) { 		
	strcat(buf, "Computer");
    }
    if (strlen(buf) == 0) {
	strcat(buf, "Nobody");
    }
    return buf;
}

#if 0 	/* Unused. */

void
side_and_type_name(char *buf, Side *side, int u, Side *side2)
{
    /* Decide how to identify the side. */
    if (side2 == NULL) {
	sprintf(buf, "independent ");
    } else if (side == side2) {
	sprintf(buf, "your ");
    } else {
	sprintf(buf, "%s ", side_adjective(side2));
    }
    /* Glue the pieces together and return it. */
    strcat(buf, u_type_name(u));
}

#endif

/* Build a short phrase describing a given unit to a given side,
   basically consisting of indication of unit's side, then of unit
   itself. */

const char *
unit_handle(Side *side, Unit *unit)
{
    Side *side2 = NULL;

    if (unit != NULL)
      side2 = unit->side;
    return apparent_unit_handle(side, unit, side2);
}

/* This version allows the caller to supply a side other than the
   unit's actual side, such as when describing an out-of-date image of
   a unit that may have been captured. */

const char *
apparent_unit_handle(Side *side, Unit *unit, Side *side2)
{
    const char *utypename;
    char smallbuf[40], sidebuf[100];
    const char *fmtstr;
    Side *side3;
    Obj *frest, *fmt1;

    /* This should be impossible, be really obvious if it happens. */
    if (side2 == NULL)
      return "null side2 in apparent_unit_handle?";
    /* Make sure our working space exists. */
    if (unitbuf == NULL)
      unitbuf = (char *)xmalloc(BUFSIZE);
    /* Handle various weird situations. */
    if (unit == NULL)
      return "???";
    if (!alive(unit)) {
    	sprintf(unitbuf, "dead #%d", unit->id);
        return unitbuf;
    }
    unitbuf[0] = '\0';
    /* Decide how to identify the side.  If the unit's original side
       is not its current side, list both of them. */
    side3 = NULL;
    if (unit->origside != NULL && side2 != unit->origside)
      side3 = unit->origside;
    sidebuf[0] = '\0';
    if (side2 == side) {
	strcat(sidebuf, "your");
    } else {
	/* If the side adjective is a genitive (ends in 's, s' or z')
	   we should skip the definite article. */
	int len = strlen(side_adjective(side2));
	const char *end = side_adjective(side2) + len - 2;

	if (strcmp(end, "'s") != 0
	    && strcmp(end, "s'") != 0
	    && strcmp(end, "z'") != 0)
	  strcat(sidebuf, "the ");
	strcat(sidebuf, side_adjective(side2));
    }

	/* Generates a lot of extra text of limited interest to the player. */
#if 0
    if (side3 != NULL) {
	if (side3 == side) {
	    strcat(sidebuf, " (formerly your)");
	} else if (side3 == indepside) {
	    /* Don't add anything for captured independents.  While
	       technically there's no reason not to do this, in
	       practice a side will often have many captured indeps,
	       and this keeps the text from getting too cluttered. */
	} else {
	    strcat(sidebuf, " (formerly ");
	    strcat(sidebuf, side_adjective(side3));
	    strcat(sidebuf, ")");
	}
    }
#endif

    /* Now add the unit's unique description. */
    utypename = u_type_name(unit->type);
    /* If we have special formatting info, interpret it. */
    if (u_desc_format(unit->type) != lispnil) {
	for_all_list(u_desc_format(unit->type), frest) {
	    fmt1 = car(frest);
	    if (stringp(fmt1)) {
		/* Append strings verbatim. */
		strcat(unitbuf, c_string(fmt1));
	    } else if (symbolp(fmt1)) {
		/* Symbols indicate the types of data to format and
                   output. */
		fmtstr = c_string(fmt1);
		if (strcmp(fmtstr, "name") == 0) {
		    strcat(unitbuf, (unit->name ? unit->name : "anon"));
		} else if (strcmp(fmtstr, "position") == 0) {
		    sprintf(smallbuf, "%d,%d", unit->x, unit->y);
		    strcat(unitbuf, smallbuf);
		} else if (strcmp(fmtstr, "side") == 0) {
		    strcat(unitbuf, sidebuf);
		} else if (strcmp(fmtstr, "type") == 0) {
		    strcat(unitbuf, utypename);
		} else if (strcmp(fmtstr, "side-name") == 0) {
		    strcat(unitbuf, side_name(unit->side));
		} else if (strcmp(fmtstr, "side-adjective") == 0) {
		    strcat(unitbuf, side_adjective(unit->side));
		} else {
		    strcat(unitbuf, "??description-format??");
		}
	    } else {
		strcat(unitbuf, "??description-format??");
	    }
	}
	return unitbuf;
    } else {
	strcat(unitbuf, sidebuf);
	strcat(unitbuf, " ");
    }
    /* If this unit is a self unit, say so. */
    if (unit->side != NULL && unit == unit->side->self_unit) {
    	if (!mobile(unit->type) || u_advanced(unit->type)) {
	  	strcat(unitbuf, "capital ");
	} else {
		strcat(unitbuf, "leader ");
    	}
	if (unit->name) {
		tprintf(unitbuf, "%s", unit->name);
	} else if (unit->number > 0) {
		tprintf(unitbuf, "%d%s %s",
			unit->number, ordinal_suffix(unit->number), utypename);
	} else {
		strcat(unitbuf, utypename);
	}
    /* Default formats for units. */
    } else {
	    if (unit->name) {
		tprintf(unitbuf, "%s %s", utypename, unit->name);
	    } else if (unit->number > 0) {
		tprintf(unitbuf, "%d%s %s",
			unit->number, ordinal_suffix(unit->number), utypename);
	    } else {
		strcat(unitbuf, utypename);
	    }
    }
    return unitbuf;
}

/* Shorter unit description omits side name, but uses same buffer.
   This is mainly useful for describing the transport of a unit and
   suchlike, where the player will likely already know the side of the
   unit. */

const char *
short_unit_handle(Unit *unit)
{
    int u;

    if (unitbuf == NULL)
      unitbuf = (char *)xmalloc(BUFSIZE);
    if (unit == NULL)
      return "???";
    if (!alive(unit)) {
    	sprintf(unitbuf, "dead #%d", unit->id);
        return unitbuf;
    }
    u = unit->type;
    /* Use the name alone if the unit is named, or else use optional
       ordinal and the shortest type name available. */
    if (!empty_string(unit->name)) {
	strcpy(unitbuf, unit->name);
    } else {
	unitbuf[0] = '\0';
	if (unit->number > 0) {
	    sprintf(unitbuf, "%d%s ",
		    unit->number, ordinal_suffix(unit->number));
	}
	if (!empty_string(u_short_name(u)))
	  strcat(unitbuf, u_short_name(u));
	else
	  strcat(unitbuf, u_type_name(u));
    }
    return unitbuf;
}

/* This version lists the side but skips original side etc. */

const char *
medium_long_unit_handle(Unit *unit)
{
    if (unitbuf == NULL)
      unitbuf = (char *)xmalloc(BUFSIZE);
    if (unit == NULL)
      return "???";
    if (!alive(unit)) {
    	sprintf(unitbuf, "dead #%d", unit->id);
        return unitbuf;
    }
    strcpy(unitbuf, side_adjective(unit->side));
    strcat(unitbuf, " ");
    /* If this unit is a self unit, say so. */
    if (unit->side != NULL && unit == unit->side->self_unit) {
	if (!mobile(unit->type) || u_advanced(unit->type)) {
		strcat(unitbuf, "capital ");
	} else {
		strcat(unitbuf, "leader ");
	}
	/* If the unit has a name, write its name. */
	if (!empty_string(unit->name)) {
		strcat(unitbuf, unit->name);
		/* If the unit has a number, write it followed by the type. */
	} else if (unit->number > 0) {
		tprintf(unitbuf, "%d%s %s",
			unit->number, ordinal_suffix(unit->number),
			u_type_name(unit->type));
		/* Else just write the unit type. */
	} else {
		strcat(unitbuf, u_type_name(unit->type));
	}
    } else {
	/* If the unit has a name, write its type followed by the name. */
	if (!empty_string(unit->name)) {
		strcat(unitbuf, u_type_name(unit->type));
		strcat(unitbuf, " ");
		strcat(unitbuf, unit->name);
		/* If the unit has a number, write it followed by the type. */
	} else if (unit->number > 0) {
		tprintf(unitbuf, "%d%s %s",
			unit->number, ordinal_suffix(unit->number),
			u_type_name(unit->type));
		/* Else just write the unit type. */
	} else {
		strcat(unitbuf, u_type_name(unit->type));
	}
    }
    return unitbuf;
}

/* Put either the unit's name or its number into the given buffer. */

void
name_or_number(Unit *unit, char *buf)
{
    if (unit->name) {
	strcpy(buf, unit->name);
    } else if (unit->number > 0) {
	sprintf(buf, "%d%s", unit->number, ordinal_suffix(unit->number));
    } else {
	buf[0] = '\0';
    }
}

/* Build a short phrase describing a given past unit to a given side,
   basically consisting of indication of unit's side, then of unit
   itself. */

const char *
past_unit_handle(Side *side, PastUnit *past_unit)
{
    const char *utypename;
    Side *side2;

    if (past_unitbuf == NULL)
      past_unitbuf = (char *)xmalloc(BUFSIZE);
    /* Handle various weird situations. */
    if (past_unit == NULL)
      return "???";
    /* Decide how to identify the side. */
    side2 = past_unit->side;
    if (side2 == NULL) {
	sprintf(past_unitbuf, "the ");
    } else if (side2 == side) {
	sprintf(past_unitbuf, "your ");
    } else {
	/* If the side adjective is a genitive (ends in 's, s' or z')
	   we should skip the definite article. */
	int len = strlen(side_adjective(side2));
	const char *end = side_adjective(side2) + len - 2;

	if (strcmp(end, "'s") != 0
	    && strcmp(end, "s'") != 0
	    && strcmp(end, "z'") != 0)
	  sprintf(past_unitbuf, "the ");
	sprintf(past_unitbuf, side_adjective(side2));
	strcat(past_unitbuf, " ");
    }
    /* Now add the past_unit's unique description. */
    utypename = u_type_name(past_unit->type);
    if (past_unit->name) {
	tprintf(past_unitbuf, "%s %s", utypename, past_unit->name);
    } else if (past_unit->number > 0) {
	tprintf(past_unitbuf, "%d%s %s",
		past_unit->number, ordinal_suffix(past_unit->number),
		utypename);
    } else {
	strcat(past_unitbuf, utypename);
    }
    return past_unitbuf;
}

/* Given a unit and optional type u, summarize construction status
   and timing. */

void
construction_desc(char *buf, Unit *unit, int u)
{
    int est, u2;
    char ubuf[10], tmpbuf[100];
    Task *task;
    Unit *unit2;

    if (u != NONUTYPE) {
	est = est_completion_time(unit, u);
	if (est >= 0) {
	    sprintf(ubuf, "[%2d] ", est);
	} else {
	    strcpy(ubuf, " --  ");
	}
    } else {
	ubuf[0] = '\0';
    }
    name_or_number(unit, tmpbuf);
    sprintf(buf, "%s%s %s", ubuf, u_type_name(unit->type), tmpbuf);
    pad_out(buf, 25);
    if (unit->plan
	&& unit->plan->tasks) {
	task = unit->plan->tasks;
	if (task->type == TASK_BUILD) {
	    u2 = task->args[0];
	    tprintf(buf, " %s ", (is_unit_type(u2) ? u_type_name(u2) : "?"));
	    unit2 = find_unit(task->args[1]);
	    if (in_play(unit2) && unit2->type == u2) {
		tprintf(buf, "%d/%d done ", unit2->cp, u_cp(unit2->type));
	    }
	    tprintf(buf, "(%d of %d)", task->args[2] + 1, task->args[3]);
	} else if (task->type == TASK_DEVELOP) {
	    u2 = task->args[0];
	    if (is_unit_type(u2)) {
		tprintf(buf, " %s tech %d/%d",
			u_type_name(u2), unit->side->tech[u2], task->args[1]);
	    }
	}
    }
}

/* Given a unit and optional advance a, summarize research status
   and timing. */

void
research_desc(char *buf, Unit *unit, int a)
{
    char abuf[10], tmpbuf[100];

    if (a != NONATYPE) {
	if (u_can_research(unit->type)) {
	    sprintf(abuf, "[%2d] ", a_rp(a));
	} else {
	    strcpy(abuf, " --  ");
	}
    } else {
	abuf[0] = '\0';
    }
    name_or_number(unit, tmpbuf);
    sprintf(buf, "%s%s %s", abuf, u_type_name(unit->type), tmpbuf);
    pad_out(buf, 25);
}

void
researchible_desc(char *buf, Unit *unit, int a)
{
    char abuf[10];

    if (a != NONATYPE && unit != NULL) {
	if (u_advanced(unit->type)) {
	    sprintf(abuf, "[%2d] ", a_rp(a));
	} else {
	    strcpy(abuf, " --  ");
	}
    } else {
	abuf[0] = '\0';
    }
    sprintf(buf, "%s%s", abuf, a_type_name(a));
    pad_out(buf, 25);
}

/* This generates a textual description of a type's construction info,
   including estimated time for the given unit to build one, tooling &
   tech, plus number of that type in existence already. */

void
constructible_desc(char *buf, Side *side, int u, Unit *unit)
{
    char estbuf[20];
    char techbuf[50];
    char typenamebuf[50];
    int est, tp, num;

    if (unit != NULL) {
	est = est_completion_time(unit, u);
    	if (est >= 0) {
	    sprintf(estbuf, "[%2d] ", est);
	    if (uu_tp_to_build(unit->type, u) > 0) {
		tp = (unit->tooling ? unit->tooling[u] : 0);
		tprintf(estbuf, "(%2d) ", tp);
	    }
    	} else {
	    strcpy(estbuf, " --  ");
    	}
    } else {
	estbuf[0] = '\0';
    }
    if (u_tech_max(u) > 0) {
    	sprintf(techbuf, "[Tech %d/%d/%d] ",
		side->tech[u], u_tech_to_build(u), u_tech_max(u));
    } else {
	techbuf[0] = '\0';
    }
    strcpy(typenamebuf, u_type_name(u));
    /* If the single char for the type is different from the first character
       of its type name, mention the char. */
    if (!empty_string(u_uchar(u)) && (u_uchar(u))[0] != typenamebuf[0]) {
	tprintf(typenamebuf, "(%c)", (u_uchar(u))[0]);
    }
    sprintf(buf, "%s%s%-16.16s", estbuf, techbuf, typenamebuf);
    num = num_units_in_play(side, u);
    if (num > 0) {
	tprintf(buf, "  %3d", num);
    } else {
	strcat(buf, "     ");
    }
    num = num_units_incomplete(side, u);
    if (num > 0) {
	tprintf(buf, "(%d)", num);
    }
}

#if 0		/* Unused. */

void
historical_event_date_desc(HistEvent *hevt, char *buf)
{
    sprintf(buf, "%d: ", hevt->startdate);
}

#endif

int
find_event_type(Obj *sym)
{
    int i;

    for (i = 0; hevtdefns[i].name != NULL; ++i) {
	if (strcmp(c_string(sym), hevtdefns[i].name) == 0)
	  return i;
    }
    return -1;
}

/* (should abstract out evt arg -> unit/pastunit description code) */

void
historical_event_desc(Side *side, HistEvent *hevt, char *buf)
{
    int data0 = hevt->data[0];
    int data1 = hevt->data[1];
    int data2 = hevt->data[2];
    Obj *rest, *head, *pattern, *text;
    Unit *unit;
    PastUnit *pastunit, *pastunit2;
    Side *side2;
    
    for_all_list(g_event_narratives(), rest) {
	head = car(rest);
	if (consp(head)) {
	    pattern = car(head);
	    if (symbolp(pattern)
		&& find_event_type(pattern) == hevt->type) {
		text = cadr(head);
		if (stringp(text)) {
		    sprintf(buf, c_string(text));
		} else {
		    sprintlisp(buf, text, 50);
		}
		return;
	    } else if (consp(pattern)
		       && symbolp(car(pattern))
		       && pattern_matches_event(pattern, hevt)
		       ) {
		text = cadr(head);
		if (stringp(text)) {
		    sprintf(buf, c_string(text));
		} else {
		    event_desc_from_list(side, text, hevt, buf);
		}
		return;
	    }
	}
    }
    /* Generate a default description of the event. */
    switch (hevt->type) {
      case H_LOG_STARTED:
	sprintf(buf, "we started recording events");
	break;
      case H_LOG_ENDED:
	sprintf(buf, "we stopped recording events");
	break;
      case H_GAME_STARTED:
	sprintf(buf, "we started the game");
	break;
      case H_GAME_SAVED:
	sprintf(buf, "we saved the game");
	break;
      case H_GAME_RESTARTED:
	sprintf(buf, "we restarted the game");
	break;
      case H_GAME_ENDED:
	sprintf(buf, "we ended the game");
	break;
      case H_SIDE_JOINED:
      	side2 = side_n(data0);
	sprintf(buf, "%s joined the game",
		(side == side2 ? "you" : side_name(side2)));
	break;
      case H_SIDE_LOST:
      	side2 = side_n(data0);
	sprintf(buf, "%s lost!", (side == side2 ? "you" : side_name(side2)));
	/* Include an explanation of the cause, if there is one. */
	if (data1 == -1) {
	    tprintf(buf, " (resigned)");
	} else if (data1 == -2) {
	    tprintf(buf, " (self-unit died)");
	} else if (data1 > 0) {
	    tprintf(buf, " (scorekeeper %d)", data1);
	} else {
	    tprintf(buf, " (don't know why)");
	}
	break;
      case H_SIDE_WITHDREW:
      	side2 = side_n(data0);
	sprintf(buf, "%s withdrew!", (side == side2 ? "you" : side_name(side2)));
	break;
      case H_SIDE_WON:
      	side2 = side_n(data0);
	sprintf(buf, "%s won!", (side == side2 ? "you" : side_name(side2)));
	/* Include an explanation of the cause, if there is one. */
	if (data1 > 0) {
	    tprintf(buf, " (scorekeeper %d)", data1);
	} else {
	    tprintf(buf, " (don't know why)");
	}
	break;
      case H_UNIT_CREATED:
      	side2 = side_n(data0);
	sprintf(buf, "%s created ",
		(side == side2 ? "you" : side_name(side2)));
	unit = find_unit(data1);
	if (unit != NULL) {
	    strcat(buf, unit_handle(side, unit));
	} else {
	    pastunit = find_past_unit(data1);
	    if (pastunit != NULL) {
		strcat(buf, past_unit_handle(side, pastunit));
	    } else {
		tprintf(buf, "%d??", data1);
	    }
	}
	break;
      case H_UNIT_COMPLETED:
      	side2 = side_n(data0);
	sprintf(buf, "%s completed ",
		(side == side2 ? "you" : side_name(side2)));
	unit = find_unit(data1);
	if (unit != NULL) {
	    strcat(buf, unit_handle(side, unit));
	} else {
	    pastunit = find_past_unit(data1);
	    if (pastunit != NULL) {
		strcat(buf, past_unit_handle(side, pastunit));
	    } else {
		tprintf(buf, "%d??", data1);
	    }
	}
	break;
      case H_UNIT_DAMAGED:
	unit = find_unit_dead_or_alive(data0);
	if (unit != NULL) {
	    strcpy(buf, unit_handle(side, unit));
	} else {
 	    pastunit = find_past_unit(data0);
	    if (pastunit != NULL) {
		strcpy(buf, past_unit_handle(side, pastunit));
	    } else {
		sprintf(buf, "%d??", data0);
	    }
	}
	tprintf(buf, " damaged (%d -> %d hp)", data1, hevt->data[2]);
	break;
      case H_UNIT_CAPTURED:
	buf[0] = '\0';
	/* Note that the second optional value, if present, is the id
	   of the unit that did the capturing. */
	unit = find_unit_dead_or_alive(data1);
	if (unit != NULL) {
	    strcat(buf, unit_handle(side, unit));
	} else {
 	    pastunit = find_past_unit(data1);
	    if (pastunit != NULL) {
		strcat(buf, past_unit_handle(side, pastunit));
	    } else if (data1 == 0) {
		tprintf(buf, "somebody");
	    } else {
		tprintf(buf, "%d??", data1);
	    }
	}
 	tprintf(buf, " captured ");
 	/* Describe the unit that was captured. */
	unit = find_unit_dead_or_alive(data0);
	if (unit != NULL) {
	    strcat(buf, unit_handle(side, unit));
	} else {
 	    pastunit = find_past_unit(data0);
	    if (pastunit != NULL) {
		strcat(buf, past_unit_handle(side, pastunit));
	    } else {
		tprintf(buf, "%d??", data0);
	    }
	}
	break;
      case H_UNIT_SURRENDERED:
	buf[0] = '\0';
 	/* Describe the unit that surrendered. */
	unit = find_unit_dead_or_alive(data0);
	if (unit != NULL) {
	    strcat(buf, unit_handle(side, unit));
	} else {
 	    pastunit = find_past_unit(data0);
	    if (pastunit != NULL) {
		strcat(buf, past_unit_handle(side, pastunit));
	    } else {
		tprintf(buf, "%d??", data0);
	    }
	}
 	tprintf(buf, " surrendered to ");
	/* The second optional value, if present, is the id of the
	   unit that accepted the surrender. */
	unit = find_unit_dead_or_alive(data1);
	if (unit != NULL) {
	    strcat(buf, unit_handle(side, unit));
	} else {
 	    pastunit = find_past_unit(data1);
	    if (pastunit != NULL) {
		strcat(buf, past_unit_handle(side, pastunit));
	    } else if (data1 == 0) {
		tprintf(buf, "somebody");
	    } else {
		tprintf(buf, "%d??", data1);
	    }
	}
	break;
      case H_UNIT_ACQUIRED:
	buf[0] = '\0';
	if (data1 >= 0) {
	    side2 = side_n(data1);
	    strcat(buf, (side == side2 ? "you" : side_name(side2)));
	} else {
	    tprintf(buf, "somebody");
	}
 	tprintf(buf, " acquired ");
 	/* Describe the unit that was acquired. */
	unit = find_unit_dead_or_alive(data0);
	if (unit != NULL) {
	    strcat(buf, unit_handle(side, unit));
	} else {
 	    pastunit = find_past_unit(data0);
	    if (pastunit != NULL) {
		strcat(buf, past_unit_handle(side, pastunit));
	    } else {
		tprintf(buf, "%d??", data0);
	    }
	}
	break;
      case H_UNIT_REVOLTED:
	buf[0] = '\0';
 	/* Describe the unit that revolted. */
	unit = find_unit_dead_or_alive(data0);
	if (unit != NULL) {
	    strcat(buf, unit_handle(side, unit));
	} else {
 	    pastunit = find_past_unit(data0);
	    if (pastunit != NULL) {
		strcat(buf, past_unit_handle(side, pastunit));
	    } else {
		tprintf(buf, "%d??", data0);
	    }
	}
 	tprintf(buf, " revolted");
 	if (data1 >= 0) {
	    side2 = side_n(data1);
	    tprintf(buf, ", went over to %s", (side == side2 ? "you" : side_name(side2)));
	}
	break;
      case H_UNIT_KILLED:
      case H_UNIT_DIED_IN_ACCIDENT:
      case H_UNIT_DIED_FROM_TEMPERATURE:
      case H_UNIT_DIED_FROM_ATTRITION:
	/* Obviously, the unit mentioned here can only be a past unit. */
	pastunit = find_past_unit(data0);
	if (pastunit != NULL) {
	    sprintf(buf, "%s", past_unit_handle(side, pastunit));
	} else {
	    sprintf(buf, "%d??", data0);
	}
	if (hevt->type == H_UNIT_KILLED)
	  tprintf(buf, " was destroyed");
	else if (hevt->type == H_UNIT_DIED_IN_ACCIDENT)
	  tprintf(buf, " died in an accident");
	else if (hevt->type == H_UNIT_DIED_FROM_ATTRITION)
	  tprintf(buf, " died in damaging terrain");
	else
	  tprintf(buf, " died from excessive temperature");
	break;
      case H_UNIT_WRECKED:
      case H_UNIT_WRECKED_IN_ACCIDENT:
      case H_UNIT_WRECKED_FROM_ATTRITION:
	pastunit = find_past_unit(data0);
	if (pastunit != NULL) {
	    sprintf(buf, "%s", past_unit_handle(side, pastunit));
	} else {
	    sprintf(buf, "%d??", data0);
	}
	if (hevt->type == H_UNIT_WRECKED)
	  tprintf(buf, " was wrecked");
	else if (hevt->type == H_UNIT_WRECKED_FROM_ATTRITION)
	  tprintf(buf, " was wrecked in damaging terrain");
	else
	  tprintf(buf, " was wrecked in an accident");
	break;
      case H_UNIT_VANISHED:
	pastunit = find_past_unit(data0);
	if (pastunit != NULL) {
	    sprintf(buf, "%s", past_unit_handle(side, pastunit));
	} else {
	    sprintf(buf, "%d??", data0);
	}
	tprintf(buf, " vanished");
	break;
      case H_UNIT_DISBANDED:
	pastunit = find_past_unit(data0);
	if (pastunit != NULL) {
	    sprintf(buf, "%s", past_unit_handle(side, pastunit));
	} else {
	    sprintf(buf, "%d??", data0);
	}
	tprintf(buf, " was disbanded");
	break;
      case H_UNIT_GARRISONED:
	pastunit = find_past_unit(data0);
	if (pastunit != NULL) {
	    sprintf(buf, "%s", past_unit_handle(side, pastunit));
	} else {
	    sprintf(buf, "%d??", data0);
	}
	tprintf(buf, " was used to garrison ");
	unit = find_unit(data1);
	if (unit != NULL) {
	    strcat(buf, unit_handle(side, unit));
	} else {
	    pastunit2 = find_past_unit(data1);
	    if (pastunit2 != NULL) {
		strcat(buf, past_unit_handle(side, pastunit2));
	    } else {
		/* Should never happen, but don't choke if it does. */
		tprintf(buf, "?????");
	    }
	}
	break;
      case H_UNIT_STARVED:
	pastunit = find_past_unit(data0);
	if (pastunit != NULL) {
	    sprintf(buf, "%s", past_unit_handle(side, pastunit));
	} else {
	    sprintf(buf, "%d??", data0);
	}
	tprintf(buf, " starved to death");
	break;
      case H_UNIT_MERGED:
	pastunit = find_past_unit(data0);
	if (pastunit != NULL) {
	    sprintf(buf, "%s", past_unit_handle(side, pastunit));
	} else {
	    sprintf(buf, "%d??", data0);
	}
	tprintf(buf, " merged into another");
	break;
      case H_UNIT_LEFT_WORLD:
	pastunit = find_past_unit(data0);
	if (pastunit != NULL) {
	    sprintf(buf, "%s", past_unit_handle(side, pastunit));
	} else {
	    sprintf(buf, "%d??", data0);
	}
	tprintf(buf, " left the world");
	break;
      case H_UNIT_NAME_CHANGED:
	pastunit = find_past_unit(data0);
	if (pastunit != NULL) {
	    sprintf(buf, "%s", past_unit_handle(side, pastunit));
	} else {
	    sprintf(buf, "%d??", data0);
	}
	unit = find_unit(data1);
	if (unit != NULL) {
	    if (unit->name != NULL)
	      tprintf(buf, " changed name to \"%s\"", unit->name);
	    else
	      tprintf(buf, " became anonymous");
	} else {
	    pastunit2 = find_past_unit(data1);
	    if (pastunit2 != NULL) {
		if (pastunit2->name != NULL)
		  tprintf(buf, " changed name to \"%s\"", pastunit2->name);
		else
		  tprintf(buf, " became anonymous");
	    } else
	      tprintf(buf, " no name change???");
	}
	break;
      case H_UNIT_TYPE_CHANGED:
	pastunit = find_past_unit(data0);
	unit = find_unit(data1);
	if (pastunit != NULL) {
	    sprintf(buf, "%s", past_unit_handle(side, pastunit));
	} else if (unit != NULL) {
	    sprintf(buf, "%s", unit_handle(side, unit));
	}
	tprintf(buf, " changed into a %s", u_type_name(data2));
	break;
      default:
	/* Don't warn, will cause serious problems for windows that
	   display lists of events, but make sure the non-understood event
	   is obvious. */
	sprintf(buf, "?????????? \"%s\" ??????????",
		hevtdefns[hevt->type].name);
	break;
    }
}

int
pattern_matches_event(Obj *pattern, HistEvent *hevt)
{
    int data0, u;
    Obj *rest, *subpat;
    PastUnit *pastunit;

    if (find_event_type(car(pattern)) != hevt->type)
      return FALSE;
    data0 = hevt->data[0];
    for_all_list(cdr(pattern), rest) {
	subpat = car(rest);
	if (symbolp(subpat)) {
	    switch (hevt->type) {
	      case H_UNIT_STARVED:
	      case H_UNIT_VANISHED:
		u = utype_from_name(c_string(subpat));
		pastunit = find_past_unit(data0);
		if (pastunit != NULL && pastunit->type == u)
		  return TRUE;
		else
		  return FALSE;
		break;
	      default:
		return FALSE;
	    }
	} else {
	    /* (should warn of bad pattern syntax?) */
	    return FALSE;
	}
    }
    return TRUE;
}

void
event_desc_from_list(Side *side, Obj *lis, HistEvent *hevt, char *buf)
{
    int n;
    Obj *rest, *item;
    PastUnit *pastunit;

    buf[0] = '\0';
    for_all_list(lis, rest) {
	item = car(rest);
	if (stringp(item)) {
	    strcat(buf, c_string(item));
	} else if (numberp(item)) {
	    n = c_number(item);
	    if (between(0, n, 3)) {
		switch (hevt->type) {
		  case H_UNIT_STARVED:
		  case H_UNIT_VANISHED:
		    pastunit = find_past_unit(hevt->data[0]);
		    if (pastunit != NULL) {
			strcat(buf, past_unit_handle(side, pastunit));
		    } else {
			tprintf(buf, "%d?", hevt->data[0]);
		    }
		    break;
		  /* (should add other event types) */
		  default:
		    break;
		}
	    } else {
		tprintf(buf, " ??%d?? ", n);
	    }
	} else {
	    strcat(buf, " ????? ");
	}
    }
}

/* Return a string describing the action's result. */

const char *
action_result_desc(int rslt)
{
    const char *str;
    
    switch (rslt) {
      case A_ANY_OK:
	str = "OK";
	break;
      case A_ANY_DONE:
	str = "done";
	break;
      case A_ANY_CANNOT_DO:
	str = "can never do";
	break;
      case A_ANY_OCC_CANNOT_DO:
	str = "occupant cannot fight";
	break;
      case A_ANY_NO_ACP:
	str = "insufficient acp";
	break;
      case A_ANY_NO_MATERIAL:
	str = "insufficient material";
	break;
      case A_ANY_NO_AMMO:
	str = "insufficient ammo";
	break;
      case A_ANY_TOO_FAR:
	str = "too far";
	break;
      case A_ANY_TOO_NEAR:
	str = "too near";
	break;
      case A_MOVE_NO_MP:
	str = "insufficient mp";
	break;
      case A_MOVE_BLOCKING_ZOC:
	str = "blocking ZOC present";
	break;
      case A_MOVE_CANNOT_LEAVE_WORLD:
	str = "cannot leave world";
	break;
      case A_MOVE_DEST_FULL:
	str = "destination full";
	break;
      case A_CONSTRUCT_NO_TOOLING:
	str = "insufficient tooling to construct";
	break;
      case A_ATTACK_CANNOT_HIT:
	str = "attack cannot hit or damage target";
	break;
      case A_OVERRUN_CANNOT_HIT:
	str = "overrun cannot work";
	break;
      case A_OVERRUN_FAILED:
	str = "overrun failed";
	break;
      case A_OVERRUN_SUCCEEDED:
	str = "overrun succeeded";
	break;
      case A_FIRE_BLOCKED:
	str = "fire blocked by terrain";
	break;
      case A_FIRE_CANNOT_HIT:
	str = "fire cannot hit or damage target";
	break;
      case A_FIRE_INTO_OUTSIDE_WORLD:
	str = "cannot fire outside world";
	break;
      case A_CAPTURE_FAILED:
	str = "capture failed";
	break;
      case A_CAPTURE_SUCCEEDED:
	str = "capture succeeded";
	break;
      case A_ANY_ERROR:
	str = "misc error";
	break;
      default:
	if (between(0, rslt, NUMHEVTTYPES - 1))
	  str = hevtdefns[rslt].name;
	else
	  /* This should never happen. */
	  str = "???";
	break;
    }
    return str;
}

/* Explain the reason for failure to move into a cell. */

void
advance_failure_desc(char *buf, Unit *unit, HistEventType reason)
{
    /* impl_move_to has already returned a more detailed error 
    message, so we want to suppresss this one. */
    if (reason == A_MOVE_NO_PATH) {
	strcpy(buf, "");
	return;
    }
    strcpy(buf, unit_handle(unit->side, unit));
    if (reason == A_ANY_CANNOT_DO)
      strcat(buf, " can never do this!");
    else if (reason == A_ANY_NO_ACP)
      strcat(buf, " does not have enough ACP!");
    else if (reason == A_ANY_NO_MATERIAL)
      strcat(buf, " is out of some material!");
    else if (reason == A_ANY_NO_AMMO)
      strcat(buf, " is out of ammo!");
    else if (reason == A_MOVE_NO_MP)
      /* Player doesn't see mp calcs, so explain as ACP */
      strcat(buf, " does not have enough ACP!");
    else if (reason == A_MOVE_CANNOT_LEAVE_WORLD)
      strcat(buf, " can never leave the world!");
    else if (reason == A_MOVE_BLOCKING_ZOC)
      strcat(buf, " cannot enter a blocking ZOC!");
    else if (reason == A_MOVE_DEST_FULL)
      strcat(buf, " cannot fit into the destination!");
    else if (reason != H_UNDEFINED)
      sprintf(buf, " cannot act, reason is %s", action_result_desc(reason));
    else
      strcat(buf, " is unable to act, don't know why");
}

/* Generate a description of the borders and connections in and around
   a location. */

void
linear_desc(char *buf, int x, int y)
{
    int t, first = TRUE;

    buf[0] = '\0';
    if (any_aux_terrain_defined()) {
	for_all_terrain_types(t) {
	    if (t_is_border(t)
		&& aux_terrain_defined(t)
		&& any_borders_at(x, y, t)) {

#if 0 /* Confusing. */
		if (first) {
		    strcat(buf, " at ");
		    first = FALSE;
		} else {
		    strcat(buf, ",");
		}
		strcat(buf, t_type_name(t));
#endif

#if 0 /* takes up more space, but not very useful */
		for_all_directions(dir) {
		    if (border_at(x, y, dir, t)) {
			tprintf(buf, "/%s", dirnames[dir]);
		    }
		}
#endif

	    } else if (t_is_connection(t)
		       && aux_terrain_defined(t)
		       && any_connections_at(x, y, t)) {
		if (first) {
		    strcat(buf, " + ");
		    first = FALSE;
		} else {
		    strcat(buf, ",");
		}
		strcat(buf, t_type_name(t));
#if 0
		for_all_directions(dir) {
		    if (connection_at(x, y, dir, t)) {
			tprintf(buf, "/%s", dirnames[dir]);
		    }
		}
#endif
	    }
	}
    }
}

void
elevation_desc(char *buf, int x, int y)
{
    if (elevations_defined()) {
	sprintf(buf, "(Elev %d)", elev_at(x, y));
    }
}

const char *
feature_desc(Feature *feature, char *buf)
{
    int i, caps = FALSE;
    const char *str;

    if (feature == NULL)
      return NULL;
    if (feature->name) {
	/* Does the name need any substitutions done? */
	if (strchr(feature->name, '%')) {
	    i = 0;
	    for (str = feature->name; *str != '\0'; ++str) {
	    	if (*str == '%') {
		    /* Interpret substitution directives. */
		    switch (*(str + 1)) {
		      case 'T':
			caps = TRUE;
		      case 't':
			if (feature->feattype) {
			    buf[i] = '\0';
			    strcat(buf, feature->feattype);
			    if (caps)
			      capitalize(buf);
			    i = strlen(buf);
			}
			++str;
			break;
		      default:
			break;
		    }
	    	} else {
		    buf[i++] = *str;
	    	}
	    }
	    /* Close off the string. */
	    buf[i] = '\0';
	    return buf;
	} else {
	    /* Return the name alone. */
	    return feature->name;
	}
    } else {
	if (feature->feattype) {
	    strcpy(buf, "unnamed ");
	    strcat(buf, feature->feattype);
	    return buf;
	}
    }
    /* No description of the location is available. */
    return "anonymous feature";
}

/* Generate a string describing what is at the given location. */

const char *
feature_name_at(int x, int y)
{
    int fid = (features_defined() ? raw_feature_at(x, y) : 0);
    Feature *feature;

    if (fid == 0)
      return NULL;
    feature = find_feature(fid);
    if (feature != NULL) {
	if (featurebuf == NULL)
	  featurebuf = (char *)xmalloc(BUFSIZE);
	return feature_desc(feature, featurebuf);
    }
    /* No description of the location is available. */
    return NULL;
}

void
temperature_desc(char *buf, int x, int y)
{
    if (temperatures_defined()) {
	sprintf(buf, "(Temp %d)", temperature_at(x, y));
    }
}

#if 0
    int age, u;
    short view, prevview;
    Side *side2;

    /* Compose and display view history of this cell. */
    Dprintf("Drawing previous view info\n");
    age = side_view_age(side, curx, cury);
    prevview = side_prevview(side, curx, cury);
    if (age == 0) {
	if (prevview != view) {
	    if (prevview == EMPTY) {
		/* misleading if prevview was set during init. */
		sprintf(tmpbuf, "Up to date; had been empty.");
	    } else if (prevview == UNSEEN) {
		sprintf(tmpbuf, "Up to date; had been unexplored.");
	    } else {
		side2 = side_n(vside(prevview));
		u = vtype(prevview);
		if (side2 != side) {
		    sprintf(tmpbuf, "Up to date; had seen %s %s.",
			    (side2 == NULL ? "independent" :
			     side_name(side2)),
			    u_type_name(u));
		} else {
		    sprintf(tmpbuf,
			    "Up to date; had been occupied by your %s.",
			    u_type_name(u));
		}
	    }
	} else {
	    sprintf(tmpbuf, "Up to date.");
	}
    } else {
	if (prevview == EMPTY) {
	    sprintf(tmpbuf, "Was empty %d turns ago.", age);
	} else if (prevview == UNSEEN) {
	    sprintf(tmpbuf, "Terrain first seen %d turns ago.", age);
	} else {
	    side2 = side_n(vside(prevview));
	    u = vtype(prevview);
	    if (side2 != side) {
		sprintf(tmpbuf, "Saw %s %s, %d turns ago.",
			(side2 == NULL ? "independent" :
			 side_name(side2)),
			u_type_name(u), age);
	    } else {
		sprintf(tmpbuf, "Was occupied by your %s %d turns ago.",
			u_type_name(u), age);
	    }
	}
    }
#endif

void
size_desc(char *buf, Unit *unit, int label)
{
    buf[0] = '\0';
    if (!u_advanced(unit->type))
      return;
    tprintf(buf, "(%d)", unit->size);
}

void
hp_desc(char *buf, Unit *unit, int label)
{
    if (label) {
	sprintf(buf, "HP ");
    } else {
	buf[0] = '\0';
    }
    /* (print '-' or some such for zero hp case?) */
    if (unit->hp == u_hp(unit->type)) {
	tprintf(buf, "%d", unit->hp);
    } else {
	tprintf(buf, "%d/%d", unit->hp, u_hp(unit->type));
    } 
}

void
acp_desc(char *buf, Unit *unit, int label)
{
    int u = unit->type;

    if (!completed(unit)) {
	sprintf(buf, "%d/%d done", unit->cp, u_cp(u));
    } else if (unit->act && new_acp_for_turn(unit) > 0) {
    	if (label) {
    	    strcpy(buf, "ACP ");
    	} else {
	    buf[0] = '\0';
    	}
	if (unit->act->acp == unit->act->initacp) {
	    tprintf(buf, "%d", unit->act->acp);
	} else {
	    tprintf(buf, "%d/%d", unit->act->acp, unit->act->initacp);
	}
    } else {
	buf[0] = '\0';
    }
}

/* Describe a unit's current combat experience, if applicable. */

void
cxp_desc(char *buf, Unit *unit, int label)
{
    int cxpmax = u_cxp_max(unit->type);

    buf[0] = '\0';
    if (cxpmax == 0)
      return;
    if (label)
      strcat(buf, "  cXP ");
    if (unit->cxp == cxpmax) {
	tprintf(buf, "%d", unit->cxp);
    } else {
	tprintf(buf, "%d/%d", unit->cxp, cxpmax);
    } 
}

/* Describe a unit's current morale, if applicable. */

void
morale_desc(char *buf, Unit *unit, int label)
{
    int moralemax = u_morale_max(unit->type);

    buf[0] = '\0';
    if (moralemax == 0)
      return;
    if (label)
      strcat(buf, "  Mor ");
    if (unit->morale == moralemax) {
	tprintf(buf, "%d", unit->morale);
    } else {
	tprintf(buf, "%d/%d", unit->morale, moralemax);
    } 
}

/* Describe a unit's individual point value if it has one. */

void
point_value_desc(char *buf, Unit *unit, int label)
{
    buf[0] = '\0';
    if (unit_point_value(unit) < 0)
      return;
    if (label)
      strcat(buf, "  Value ");
    tprintf(buf, "%d", unit_point_value(unit));
}

/* Describe one "row" (group of 3) of a unit's supply status. */

int
supply_desc(char *buf, Unit *unit, int mrow)
{
    int u = unit->type, m, mm, tmprow;

    tmprow = 0;
    buf[0] = '\0';
    mm = 0;
    for_all_material_types(m) {
	if (um_storage_x(u, m) > 0) {
	    if (mm > 0 && mm % 3 == 0)
	      ++tmprow;
	    if (tmprow == mrow) {
		/* Don't print out storage space if it is set to arbitrarily large 
		(9999, 999 or 99) as in many games. */
	    	if (um_storage_x(u, m) == 9999
	    	    || um_storage_x(u, m) == 999
	    	    || um_storage_x(u, m) == 99) {
			tprintf(buf, "%s %d  ",
				m_type_name(m), unit->supply[m]);
		} else {
			tprintf(buf, "%s %d/%d  ",
				m_type_name(m), unit->supply[m], um_storage_x(u, m));
	    	}
	    }
	    ++mm;
	}
    }
    return (strlen(buf) > 0);
}

/* Describe a builder's current tooling. */

int
tooling_desc(char *buf, Unit *unit)
{
    int u2, num;

    buf[0] = '\0';
    if (unit->tooling == NULL)
      return FALSE;
    /* Use the number of nonzero tooling to govern the use of
       abbreviations. */
    num = 0;
    for_all_unit_types(u2)
      if (unit->tooling[u2] > 0)
	++num;
    if (num == 0) {
	strcat(buf, "No tooling");
	return TRUE;
    }
    strcat(buf, "Tooling: ");
    for_all_unit_types(u2) {
	if (unit->tooling[u2] > 0) {
	    tprintf(buf, "%s %d/%d  ",
		    (num > 2 ? shortest_unique_name(u2) : u_type_name(u2)),
		    unit->tooling[u2], uu_tp_to_build(unit->type, u2));
	}
    }
    return TRUE;
}

void
location_desc(char *buf, Side *side, Unit *unit, int u, int x, int y)
{
    int t = terrain_at(x, y);
    const char *featurename;

    if (unit != NULL && unit->transport != NULL) {
	sprintf(buf, "In %s", short_unit_handle(unit->transport));
    } else if (unit != NULL || u != NONUTYPE) {
	sprintf(buf, "In %s", t_type_name(t));
	linear_desc(buf + strlen(buf), x, y);
    } else if (terrain_visible(side, x, y)) {
	sprintf(buf, "Empty %s", t_type_name(t));
	linear_desc(buf + strlen(buf), x, y);
    } else {
	sprintf(buf, "Unknown");
    }
    if (terrain_visible(side, x, y)) {
	featurename = feature_name_at(x, y);
	if (!empty_string(featurename))
	  tprintf(buf, " (%s)", featurename);
	if (elevations_defined())
	  tprintf(buf, " (El %d)", elev_at(x, y));
	if (temperatures_defined())
	  tprintf(buf, " (T %d)", temperature_at(x, y));
	if (winds_defined()) {
	    int wforce = wind_force_at(x, y);

	    if (wforce == 0)
	      tprintf(buf, " (W calm)");
	    else
	      tprintf(buf, " (W f%d %s)", wforce, dirnames[wind_dir_at(x, y)]);
	}
	/* (should optionally list other local weather also) */
    }
    tprintf(buf, " at %d,%d", x, y);
}

/* Given a cell, describe where it is. */

void
destination_desc(char *buf, Side *side, Unit *unit, int x, int y, int z)
{
    int dir, x1, y1;
    Unit *unit2;

    if (!in_area(x, y)) {
	sprintf(buf, "?%d,%d?", x, y);
	return;
    }
    unit2 = unit_at(x, y);
    if (unit2 != NULL && unit2->side == side) {
	if (unit2 == unit) {
	    sprintf(buf, "self (%d,%d)", x, y);
	} else if (!empty_string(unit2->name)) {
	    sprintf(buf, "%s (%d,%d)", unit2->name, x, y);
	} else {
	    sprintf(buf, "%s (%d,%d)", u_type_name(unit2->type), x, y);
	}
	return;
    }
    for_all_directions(dir) {
	if (interior_point_in_dir(x, y, dir, &x1, &y1)) {
	    unit2 = unit_at(x1, y1);
	    /* (should look at stack) */
	    if (unit2 != NULL && unit2->side == side) {
		if (unit2 == unit) {
		    sprintf(buf, "%s (%d,%d)",
			    dirnames[opposite_dir(dir)], x, y);
		} else if (!empty_string(unit2->name)) {
		    sprintf(buf, "%s of %s (%d,%d)",
			    dirnames[opposite_dir(dir)], unit2->name, x, y);
		} else {
		    sprintf(buf, "%s of %s (%d,%d)",
			    dirnames[opposite_dir(dir)],
			    u_type_name(unit2->type), x, y);
		}
		return;
	    }
	}
    }
    /* This is the old reliable case. */
    sprintf(buf, "%d,%d", x, y);
    if (z != 0)
      tprintf(buf, ",%d", z);
}

void
latlong_desc(char *buf, int x, int y, int xf, int yf, int which)
{
    char minbuf[10];
    int rawlat, latdeg, latmin, rawlon, londeg, lonmin;

    buf[0] = '\0';
    if (world.circumference <= 0)
      return;
    xy_to_latlong(x, y, xf, yf, &rawlat, &rawlon);
    if (which & 2) {
	latdeg = abs(rawlat) / 60;
	latmin = abs(rawlat) % 60;
	minbuf[0] = '\0';
	if (latmin != 0)
	  sprintf(minbuf, "%dm", latmin);
	sprintf(buf, "%dd%s %c",
		latdeg, minbuf, (rawlat >= 0 ? 'N' : 'S'));
    }
    if (which & 1) {
	londeg = abs(rawlon) / 60;
	lonmin = abs(rawlon) % 60;
	minbuf[0] = '\0';
	if (lonmin != 0)
	  sprintf(minbuf, "%dm", lonmin);
	if (!empty_string(buf))
	  strcat(buf, " ");
	sprintf(buf + strlen(buf), "%dd%s %c",
		londeg, minbuf, (rawlon >= 0 ? 'E' : 'W'));
    }
}

static int *ohd_nums;
static int *ohd_incomplete;

void
others_here_desc(char *buf, Unit *unit)
{
    int u2, first = TRUE;
    Unit *unit2, *top;

    if (ohd_nums == NULL)
      ohd_nums = (int *) xmalloc(numutypes * sizeof(int));
    if (ohd_incomplete == NULL)
      ohd_incomplete = (int *) xmalloc(numutypes * sizeof(int));
    buf[0] = '\0';
    top = unit_at(unit->x, unit->y);
    if (top != NULL && top->nexthere != NULL) {
	for_all_unit_types(u2)
	  ohd_nums[u2] = ohd_incomplete[u2] = 0;
	for_all_stack(unit->x, unit->y, unit2)
	  if (completed(unit2))
	    ++ohd_nums[unit2->type];
	  else
	    ++ohd_incomplete[unit2->type];
	/* Don't count ourselves. */
	if (completed(unit))
	  --ohd_nums[unit->type];
	else
	  --ohd_incomplete[unit->type];
	for_all_unit_types(u2) {
	    if (ohd_nums[u2] > 0 || ohd_incomplete[u2] > 0) {
		if (first)
		  first = FALSE;
		else
		  strcat(buf, " ");
		if (ohd_nums[u2] > 0)
		  tprintf(buf, "%d", ohd_nums[u2]);
		if (ohd_incomplete[u2] > 0)
		  tprintf(buf, "(%d)", ohd_incomplete[u2]);
		strcat(buf, " ");
		strcat(buf, shortest_generic_name(u2));
	    }
	}
	strcat(buf, " here also");
    }
}

void
occupants_desc(char *buf, Unit *unit)
{
    int u2, first = TRUE;
    Unit *occ;

    if (ohd_nums == NULL)
      ohd_nums = (int *) xmalloc(numutypes * sizeof(int));
    if (ohd_incomplete == NULL)
      ohd_incomplete = (int *) xmalloc(numutypes * sizeof(int));
    buf[0] = '\0';
    if (unit->occupant != NULL) {
	strcat(buf, "Occs ");
	for_all_unit_types(u2)
	  ohd_nums[u2] = ohd_incomplete[u2] = 0;
	for_all_occupants(unit, occ)
	  if (completed(occ))
	    ++ohd_nums[occ->type];
	  else
	    ++ohd_incomplete[occ->type];
	for_all_unit_types(u2) {
	    if (ohd_nums[u2] > 0 || ohd_incomplete[u2] > 0) {
		if (first)
		  first = FALSE;
		else
		  strcat(buf, " ");
		if (ohd_nums[u2] > 0)
		  tprintf(buf, "%d", ohd_nums[u2]);
		if (ohd_incomplete[u2] > 0)
		  tprintf(buf, "(%d)", ohd_incomplete[u2]);
		strcat(buf, " ");
		strcat(buf, shortest_generic_name(u2));
	    }
	}
    }
}

void
oprole_desc(char *buf, Xconq::AI::OpRole *oprole)
{
    using namespace Xconq::AI;

    // Return, if no oprole.
    if (!oprole || (OR_NONE == oprole->type)) {
	buf[0] = 0;
	return;
    }
    // Mention that this is an oprole.
    strncat(buf, "Operational Role: ", BUFSIZE);
    // Mention which type of oprole.
    switch (oprole->type) {
	case OR_CONSTRUCTOR:
	    strncat(buf, "Constructor/Repairer", BUFSIZE - strlen(buf));
	    break;
	case OR_SHUTTLE:
	    strncat(buf, "Shuttle", BUFSIZE - strlen(buf));
	    break;
	default:
	    strncat(
		buf, "Unknown! (Weird, report this please.)", 
		BUFSIZE - strlen(buf));
	    break;
    }
}

/* Fill the given buffer with a verbal description of the unit's
   current plan. */

void
plan_desc(char *buf, Unit *unit)
{
    using namespace Xconq::AI;

    char goalbuf[BUFSIZE];
    int i;
    Plan *plan = unit->plan;
    Task *task = NULL;
    OpRole *oprole = NULL;

    if (plan == NULL) {
	buf[0] = '\0';
    	return;
    }
    oprole = find_oprole(unit->side, unit->id);
    if (oprole && (PLAN_PASSIVE == plan->type)) 
	oprole_desc(buf, oprole);
    else
	sprintf(buf, "Plan: %s", plantypenames[plan->type]);
    if (plan->waitingfortasks)
      strcat(buf, " Waiting");
    if (plan->asleep)
      strcat(buf, " Asleep");
    if (plan->reserve)
      strcat(buf, " Reserve");
    if (plan->delayed)
      strcat(buf, " Delay");
    /* The more usual case is to allow AI control of units, so we only
       mention it when it's off. */
    if (!plan->aicontrol)
      strcat(buf, " NoAI");
    if (plan->supply_is_low)
      strcat(buf, " SupplyLow");
    if (plan->waitingfortransport)
      strcat(buf, " WaitingForTransport");
    if (plan->maingoal) {
	strcat(buf, ". Goal: ");
	strcat(buf, goal_desc(goalbuf, plan->maingoal));
    }
    if (plan->formation) {
	strcat(buf, ". Formation: ");
	strcat(buf, goal_desc(goalbuf, plan->formation));
    }
    /* For tasks, just put out a count. */
    if (plan->tasks) {
	i = 0;
	for_all_tasks(plan, task)
	  ++i;
	tprintf(buf, ". %d task%s", i, (i == 1 ? "" : "s"));
    } 
    strcat(buf, ".");
}

/* Describe a task in a brief but intelligible way.  Note that the
   given side and/or unit may be NULL, such as when describing a task
   not yet assigned to a unit (standing orders for instance). */

void
task_desc(char *buf, Side *side, Unit *unit, Task *task)
{
    int i, slen, arg0, arg1, arg2, arg3, arg4, tp;
    const char *argtypes;
    char *sidedesc, *utypedesc;
    Unit *unit2;

    if (task == NULL) {
	buf[0] = '\0';
	return;
    }
    arg0 = task->args[0];
    arg1 = task->args[1];
    arg2 = task->args[2];
    arg3 = task->args[3];
    arg4 = task->args[4];
    sprintf(buf, "%s ", taskdefns[task->type].display_name);
    switch (task->type) {
      case TASK_BUILD:
	if (arg0 != 0) {
	    unit2 = find_unit(arg0);
	    if (unit2 != NULL) {
		tprintf(
		    buf, "%s: %d/%d done", medium_long_unit_handle(unit2), 
		    unit2->cp, u_cp(unit2->type));
	    }
	} 
#if (0)
	else if (is_unit_type(arg0)) {
	    tp = (unit->tooling ? unit->tooling[arg0] : 0);
	    if (tp < uu_tp_to_build(unit->type, arg0)) {
		tprintf(buf, " (tooling up: %d/%d)", tp, 
			uu_tp_to_build(unit->type, arg0));
	    }	
	}
	if (arg3 > 1) {
	    tprintf(buf, " (%d%s of %d)", arg2 + 1, ordinal_suffix(arg2 + 1), 
		    arg3);
	}
#endif
	break;
      case TASK_CAPTURE:
	unit2 = find_unit(arg0);
	tprintf(buf, "%s", medium_long_unit_handle(unit2));
	if (in_play(unit2))
	    tprintf(buf, " at (%d,%d)", unit2->x, unit2->y);
	break;
      case TASK_COLLECT:
	tprintf(buf, "%s around ", m_type_name(arg0));
	tprintf(buf, "%d,%d", arg1, arg2);
	break;
      case TASK_CONSTRUCT:
	tprintf(buf, "%d %s ", arg1, u_type_name(arg0));
	unit2 = find_unit(arg2);
	if (in_play(unit2))
	    tprintf(buf, "in %s", 
		    unit == unit2 ? "self" : medium_long_unit_handle(unit2));
	else
	    tprintf(buf, "at %d,%d", arg3, arg4);
	break;
      case TASK_DEVELOP:
	tprintf(buf, "tech for %s: %d/%d", u_type_name(arg0), 
		(side ? side->tech[arg0] : 0), arg1);
	break;
      case TASK_HIT_POSITION:
	tprintf(buf, "at %d,%d", arg0, arg1);
	break;
      case TASK_HIT_UNIT:
	unit2 = find_unit(arg0);
	tprintf(buf, "%s", medium_long_unit_handle(unit2));
	if (in_play(unit2))
	    tprintf(buf, " at (%d,%d)", unit2->x, unit2->y);
	break;
      case TASK_MOVE_DIR:
	tprintf(buf, "%s, %d times", dirnames[arg0], arg1);
	break;
      case TASK_MOVE_TO:
	if (unit != NULL && unit->x == arg0 && unit->y == arg1) {
	    tprintf(buf, "here");
	} else {
	    if (arg3 == 0) {
		/* do nothing */
	    } else if (arg3 == 1) {
		tprintf(buf, "adj ");
	    } else {
		tprintf(buf, "within %d of ", arg3);
	    }
	    tprintf(buf, "%d,%d", arg0, arg1);
	}
        break;
      case TASK_OCCUPY:
	unit2 = find_unit(arg0);
	if (unit2 != NULL) {
	    tprintf(buf, "%s ", medium_long_unit_handle(unit2));
	    if (!empty_string(unit2->name)) {
		tprintf(buf, "(%d,%d)", unit2->x, unit2->y);
	    } else {
	   	tprintf(buf, "at %d,%d", unit2->x, unit2->y);
	    }
	} else {
	    tprintf(buf, "unknown unit #%d", arg0);
	}
	break;
      case TASK_PICKUP:
	unit2 = find_unit(arg0);
	if (unit2 != NULL) {
	    tprintf(buf, "%s", medium_long_unit_handle(unit2));
	} else {
	    tprintf(buf, "unknown unit #%d", arg0);
	}
	break;
      case TASK_REPAIR:
	unit2 = find_unit(arg0);
	if (!unit2) 
	    tprintf(buf, "unknown unit #%d", arg0);
	else if (unit2->id == unit->id) 
	    tprintf(buf, "self");
	else
	    tprintf(buf, "%s", medium_long_unit_handle(unit2));
	break;
      case TASK_RESUPPLY:
	if (arg0 != NONMTYPE)
	  tprintf(buf, "%s", m_type_name(arg0));
	else
	  tprintf(buf, "all");
	if (arg1 != 0) {
	    unit2 = find_unit(arg1);
	    if (unit2 != NULL) {
		tprintf(buf, " at %s", short_unit_handle(unit2));
	    }
	}
	break;
      case TASK_SENTRY:
	/* (should display as calendar time) */
	tprintf(buf, "for %d turns", arg0);
	break;
      case TASK_PRODUCE:
	tprintf(buf, "%s (%d/%d)", mtypes[task->args[0]].name, task->args[2],
		task->args[1]);
	break;
      default:
	/* Default is just to dump out the raw data about the task. */
	tprintf(buf, "raw");
	argtypes = taskdefns[task->type].argtypes;
	slen = strlen(argtypes);
	for (i = 0; i < slen; ++i) {
	    tprintf(buf, "%c%d", (i == 0 ? ' ' : ','), task->args[i]);
	}
	break;
    }
    if (Debug) {
	/* Include the number of executions and retries. */
	tprintf(buf, " x %d", task->execnum);
	if (task->retrynum > 0) {
	    tprintf(buf, " fail %d", task->retrynum);
	}
    }
}

/* Describe an action in a brief but intelligible way. */

void
action_desc(char *buf, Action *action, Unit *unit)
{
    Unit *unit2 = NULL;
    int *args = NULL;
    int tv = UNSEEN;

    assert_error(buf, "Tried to write to a NULL buffer");
    assert_error(unit, "Tried to access a NULL unit");
    if ((action == NULL) || (action->type == ACTION_NONE)) {
	buf[0] = '\0';
	return;
    }
    args = action->args;
    /* Print the name of the action. */
    sprintf(buf, "%s ", actiondefns[(int) action->type].name);
    /* Print things specific to the type of action. */
    switch (action->type) {
      case ACTION_MOVE:
	if (in_area(args[0], args[1])) {
	    tv = terrain_view(unit->side, args[0], args[1]);
	    tprintf(buf, "to %s at (%d,%d)", 
		    (tv == UNSEEN) ? "unknown cell" 
				   : t_type_name(vterrain(tv)),
		    args[0], args[1]);
	    /* (TODO: Consider elevation.) */
	}
	else
	  tprintf(buf, "%s", "out of playing area");
	break;
      case ACTION_ENTER:
	/* (TODO: This should be rewritten to handle unit views instead of 
	    raw units.) */
	unit2 = find_unit(args[0]);
	if (!unit2)
	  tprintf(buf, "%s", "missing or out-of-play transport");
	else
	  tprintf(buf, "%s", unit_desig(unit2));
	break;
      case ACTION_ATTACK: case ACTION_FIRE_AT: case ACTION_CAPTURE:
	/* (TODO: This should be rewritten to handle unit views instead of 
	    raw units.) */
	unit2 = find_unit(args[0]);
	if (!unit2)
	  tprintf(buf, "%s", "missing or out-of-play defender");
	else
	  tprintf(buf, "%s %s at (%d,%d)", side_desig(unit2->side), 
		  u_type_name(unit2->type), unit2->x, unit2->y);
	break;
      case ACTION_OVERRUN: case ACTION_FIRE_INTO:
	if (in_area(args[0], args[1])) {
	    tv = terrain_view(unit->side, args[0], args[1]);
	    tprintf(buf, "%s at (%d,%d)", 
		    (tv == UNSEEN) ? "unknown cell" 
				   : t_type_name(vterrain(tv)),
		    args[0], args[1]);
	    /* (TODO: Consider elevation?) */
	}
	else
	  tprintf(buf, "%s", "a cell out of playing area");
	break;
      case ACTION_DETONATE:
	if (in_area(args[0], args[1])) {
	    tv = terrain_view(unit->side, args[0], args[1]);
	    tprintf(buf, "in %s at (%d,%d)", 
		    (tv == UNSEEN) ? "unknown cell" 
				   : t_type_name(vterrain(tv)),
		    args[0], args[1]);
	    /* (TODO: Consider elevation?) */
	}
	else
	  tprintf(buf, "%s", "in a cell out of playing area");
	break;
      case ACTION_PRODUCE:
	tprintf(buf, "%d %s", args[1], m_type_name(args[0]));
	break;
      case ACTION_EXTRACT:
	if (in_area(args[0], args[1])) {
	    tv = terrain_view(unit->side, args[0], args[1]);
	    tprintf(buf, "%d %s from %s at (%d,%d)", args[3], 
		    m_type_name(args[2]),  
		    (tv == UNSEEN) ? "unknown cell" 
				   : t_type_name(vterrain(tv)),
		    args[0], args[1]);
	    /* (TODO: Consider elevation?) */
	}
	else
	  tprintf(buf, "%d %s from a cell out of playing area", 
		  args[3], m_type_name(args[2]));
	break;
      case ACTION_TRANSFER:
	unit2 = find_unit(args[2]);
	if (!unit2)
	  tprintf(buf, "%s", "missing or out-of-play recipient");
	else
	  tprintf(buf, "%d %s to %s %s at (%d,%d)", 
		  args[1], m_type_name(args[0]), 
		  side_desig(unit2->side), 
		  u_type_name(unit2->type), unit2->x, unit2->y);
	break;
      case ACTION_DEVELOP:
	tprintf(buf, "%s", u_type_name(args[0]));
	break;
      case ACTION_TOOL_UP:
	tprintf(buf, "for constructing %s", u_type_name(args[0]));
	break;
      case ACTION_CREATE_IN:
	/* (TODO: This should be rewritten to handle unit views instead of 
	    raw units.) */
	unit2 = find_unit(args[1]);
	if (!unit2)
	  tprintf(buf, "%s", "missing or out-of-play womb unit");
	else
	  tprintf(buf, "%s %s at (%d,%d): %s", side_desig(unit2->side), 
		  u_type_name(unit2->type), unit2->x, unit2->y, 
		  u_type_name(args[0]));
	break;
      case ACTION_CREATE_AT:
	if (in_area(args[1], args[2])) {
	    tv = terrain_view(unit->side, args[1], args[2]);
	    tprintf(buf, "%s at (%d,%d): %s", 
		    (tv == UNSEEN) ? "unknown cell" 
				   : t_type_name(vterrain(tv)),
		    args[1], args[2], u_type_name(args[0]));
	    /* (TODO: Consider elevation?) */
	}
	else
	  tprintf(buf, "%s", "a cell out of playing area");
	break;
      case ACTION_BUILD: case ACTION_REPAIR:
	unit2 = find_unit(args[0]);
	if (!unit2)
	  tprintf(buf, "%s", "missing or out-of-play unit");
	else
	  tprintf(buf, "%s %s at (%d,%d)", side_desig(unit2->side), 
		  u_type_name(unit2->type), unit2->x, unit2->y);
	break;
      default: break;
    }
}


/* Describe a goal in a human-intelligible way. */

const char *
goal_desc(char *buf, Goal *goal)
{
    int numargs, i, arg;
    const char *argtypes;
    Unit *unit;
    
    if (goal == NULL)
      return "null goal";
    switch (goal->type) {
      case GOAL_KEEP_FORMATION:
	sprintf(buf, " %d,%d from %s (var %d)",
		goal->args[1], goal->args[2],
		unit_handle(NULL, find_unit(goal->args[0])),
		goal->args[3]);
	break;
      case GOAL_VICINITY_HELD:
	sprintf(buf, "Hold %dx%d area around %d,%d",
		goal->args[2], goal->args[3], goal->args[0], goal->args[1]);
	break;
      case GOAL_UNIT_OCCUPIED:
	unit = find_unit(goal->args[0]);
	sprintf(buf, "Hold %s ", medium_long_unit_handle(unit));
	    if (!empty_string(unit->name)) {
		tprintf(buf, "(%d,%d)", unit->x, unit->y);
	    } else {
	   	tprintf(buf, "at %d,%d", unit->x, unit->y);
	    }
	break;
	/* (should add more cases specific to common types of goals) */
      default:
	sprintf(buf, "%s%s", (goal->tf ? "" : "not "),
		goaldefns[goal->type].display_name);
	argtypes = goaldefns[goal->type].argtypes;
	numargs = strlen(argtypes);
	for (i = 0; i < numargs; ++i) {
	    arg = goal->args[i];
	    switch (argtypes[i]) {
	      case 'h':
		tprintf(buf, "%d", arg);
		break;
	      case 'm':
		if (is_material_type(arg))
		  tprintf(buf, " %s", m_type_name(arg));
		else
		  tprintf(buf, " m%d?", arg);
		break;
	      case 'S':
		tprintf(buf, " %s", short_side_title(side_n(arg)));
		break;
	      case 'u':
		if (is_unit_type(arg))
		  tprintf(buf, " %s", u_type_name(arg));
		else
		  tprintf(buf, " u%d?", arg);
		break;
	      case 'U':
		tprintf(buf, " %s", unit_handle(NULL, find_unit(arg)));
		break;
	      case 'w':
		tprintf(buf, " %dx", arg);
		break;
	      case 'x':
		tprintf(buf, " %d,", arg);
		break;
	      case 'y':
		tprintf(buf, "%d", arg);
		break;
	      default:
		tprintf(buf, " %d", arg);
		break;
	    }
	}
	break;
    }
    return buf;
}

/* Format a (real, not game) clock time into a standard form.  This
   routine will omit the hours part if it will be uninteresting. */

void
time_desc(char *buf, int seconds, int maxtime)
{
    int hour, minute, second;

    if (seconds >= 0) {
	hour = seconds / 3600;  
	minute = (seconds / 60) % 60;  
	second = seconds % 60;
    	if (between(1, maxtime, 3600) && hour == 0) {
	    sprintf(buf, "%.2d:%.2d", minute, second);
	} else {
	    sprintf(buf, "%.2d:%.2d:%.2d", hour, minute, second);
	}
    } else {
    	sprintf(buf, "??:??:??");
    }
}

#if 0		/* Unused. */

/* General-purpose routine to take an array of anonymous unit types and
   summarize what's in it, using numbers and unit chars. */

char *
summarize_units(char *buf, int *ucnts)
{
    char tmp[BUFSIZE];  /* should be bigger? */
    int u;

    buf[0] = '\0';
    for_all_unit_types(u) {
	if (ucnts[u] > 0) {
	    sprintf(tmp, " %d %s", ucnts[u], utype_name_n(u, 3));
	    strcat(buf, tmp);
	}
    }
    return buf;
}

#endif

static void notify_doctrine_1(Side *side, Doctrine *doctrine);

void
notify_doctrine(Side *side, char *spec)
{
    int u;
    const char *rest;
    char *arg, substr[BUFSIZE], outbuf[BUFSIZE];
    Doctrine *doctrine;

    if (!empty_string(spec))
      rest = get_next_arg(spec, substr, &arg);
    else
      arg = NULL;
    if ((doctrine = find_doctrine_by_name(arg)) != NULL) {
	/* Found a specific named doctrine. */
	/* (should say which of our own unit types use it) */
    } else if ((u = utype_from_name(arg)) != NONUTYPE) {
	doctrine = side->udoctrine[u];
    } else if (strcmp(arg, "default") == 0) {
	doctrine = side->default_doctrine;
	/* (should say which unit types use it) */
    } else {
	if (!empty_string(arg))
	  notify(side, "\"%s\" not recognized as doctrine name or unit type.",
		 arg);
	/* Note that although we list all doctrines, including those
	   belonging to other sides, because we don't know if the
	   other side is actually *using* any particular doctrine.
	   While this may seem like an information leak, if a side has
	   its own unique doctrine, curiously enough this corresponds
	   well to the real world; for instance, present-day nations'
	   militaries are all familiar with each other's doctrines. */
	outbuf[0] = '\0';
	for_all_doctrines(doctrine) {
	    if (!empty_string(doctrine->name))
	      tprintf(outbuf, " %s", doctrine->name);
	    else
	      tprintf(outbuf, " #%d", doctrine->id);
	    if (doctrine->next != NULL)
	      tprintf(outbuf, ",");
	}
	notify(side, "Doctrines available:%s", outbuf);
	for_all_doctrines(doctrine) {
	    notify_doctrine_1(side, doctrine);
	}
	return;
    }
    /* We now have a doctrine to display. */
    notify_doctrine_1(side, doctrine);
}

static void
notify_doctrine_1(Side *side, Doctrine *doctrine)
{
    int u, i, num, dflt;
    char abuf[BUFSIZE];

    if (!empty_string(doctrine->name))
      notify(side, "Doctrine '%s' (%s):",
	     doctrine->name, (doctrine->locked ? "fixed" : "adjustable"));
    else
      notify(side, "Doctrine #%d (%s):",
	     doctrine->id, (doctrine->locked ? "fixed" : "adjustable"));
    notify(side, "  Resupply at %d%% of storage", doctrine->resupply_percent);
    notify(side, "  Rearm at %d%% of storage", doctrine->rearm_percent);
    notify(side, "  Repair at %d%% of hp", doctrine->repair_percent);
    abuf[0] = '\0';
    i = 0;
    num = 0;
    dflt = doctrine->construction_run[0];
#if 0 /* should use histo code in write_table */
    for_all_unit_types(u) {
	if (dflt == doctrine->construction_run[u]) {
	    ++num;
	}
    }
#endif
    for_all_unit_types(u) {
	tprintf(abuf, "  %s %d",
		u_type_name(u), doctrine->construction_run[u]);
	if (i > 0 && (i++) % 4 == 0 && !empty_string(abuf)) {
	    notify(side, "  Construction run:%s", abuf);
	    abuf[0] = '\0';
	}
    }
    if (!empty_string(abuf))
      notify(side, "  Construction run:%s", abuf);
}

static int report_combat_special(Unit *unit1, Unit *unit2, const char *str);
int type_matches_symbol(Obj *sym, int u);

void
report_combat(Unit *atker, Unit *other, const char *str)
{
    int rslt;

    if (g_action_notices() != lispnil) {
	rslt = report_combat_special(atker, other, str);
	if (rslt)
	  return;
    }
    /* Default messages for each type of report. */
    if (strcmp(str, "destroy") == 0) {
	notify_combat(atker, other, (char *) "%s destroys %s!");
    } else if (strcmp(str, "destroy-occupant") == 0) {
	notify_combat(other, atker, (char *) " - and destroys occupant %s!");
    } else if (strcmp(str, "hit") == 0) {
	notify_combat(atker, other, (char *) "%s hits %s!");
    } else if (strcmp(str, "hit-occupant") == 0) {
	notify_combat(other, atker, (char *) " - and hits occupant %s!");
    } else if (strcmp(str, "miss-fire") == 0) {
	notify_combat(atker, other, (char *) "%s fires at %s.");
    } else if (strcmp(str, "miss-attack") == 0) {
	notify_combat(atker, other, (char *) "%s attacks %s.");
    } else if (strcmp(str, "miss-occupant") == 0) {
	/* (this case is too uninteresting to mention) */
    } else if (strcmp(str, "resist") == 0) {
	notify_combat(other, atker, (char *) "%s resists capture by %s!");
    } else if (strcmp(str, "resist/slaughter") == 0) {
	notify_combat(other, atker, (char *) "%s resists capture; %s slaughtered!");
    } else if (strcmp(str, "capture") == 0) {
	notify_combat(atker, other, (char *) "%s captures %s!");
    } else if (strcmp(str, "liberate") == 0) {
	notify_combat(atker, other, (char *) "%s liberates %s!");
    } else if (strcmp(str, "escape") == 0) {
	notify_combat(other, atker, (char *) "%s escapes!");
    } else if (strcmp(str, "retreat") == 0) {
	notify_combat(other, atker, (char *) "%s retreats!");
    } else if (strcmp(str, "detonate") == 0) {
	notify_combat(atker, other, (char *) "%s detonates!");
    } else {
	notify_combat(atker, other, str);
    }
}

static void
notify_combat(Unit *unit1, Unit *unit2, const char *str)
{
    char buf1[BUFSIZE], buf2[BUFSIZE];
    Side *side3;

    for_all_sides(side3) {
	if (active_display(side3)
	    && (side3->see_all
		|| (unit1 != NULL && side3 == unit1->side)
		|| (unit2 != NULL && side3 == unit2->side))) {
	    strcpy(buf1, unit_handle(side3, unit1));
	    strcpy(buf2, unit_handle(side3, unit2));
	    notify(side3, str, buf1, buf2);
	}
    }
}

static int
report_combat_special(Unit *unit1, Unit *unit2, const char *str)
{
    int found = FALSE;
    char abuf[BUFSIZE];
    Obj *rest, *head, *pat, *msgdesc;
    Side *side3;

    for_all_list(g_action_notices(), rest) {
	head = car(rest);
	if (!consp(head)) {
	    run_warning("Non-list in action-notices");
	    continue;
	}
	pat = car(head);
	if (symbolp(pat) && strcmp(c_string(pat), str) == 0) {
	    found = TRUE;
	    break;
	}
	if (consp(pat)
	    && symbolp(car(pat))
	    && strcmp(c_string(car(pat)), str) == 0
	    && pattern_matches_combat(cdr(pat), unit1, unit2)) {
	    found = TRUE;
	    break;
	}
    }
    /* If we have a match, do something with it. */
    if (found) {
	msgdesc = cadr(head);
	if (stringp(msgdesc)) {
	    strcpy(abuf, c_string(msgdesc));
	} else {
	    /* Notify all sides that could have seen the combat. */
	    for_all_sides(side3) {
		if (active_display(side3)
		    && (side3->see_all
			|| (unit1 != NULL && side3 == unit1->side)
			|| (unit2 != NULL && side3 == unit2->side))) {
		    combat_desc_from_list(side3, msgdesc, unit1, unit2, str,
					  abuf);
		    notify(side3, abuf);
		}
	    }
	}
    }
    return found;
}

static int
pattern_matches_combat(Obj *parms, Unit *unit, Unit *unit2)
{
    Obj *head;

    head = car(parms);
    if (!type_matches_symbol(head, unit->type))
      return FALSE;
    parms = cdr(parms);
    head = car(parms);
    if (!type_matches_symbol(head, unit2->type))
      return FALSE;
    return TRUE;
}

int
type_matches_symbol(Obj *sym, int u)
{
    const char *uname;
    Obj *val, *rest, *head;

    if (!symbolp(sym))
      return FALSE;
    uname = u_type_name(u);
    if (strcmp(c_string(sym), uname) == 0)
      return TRUE;
    if (match_keyword(sym, K_USTAR))
      return TRUE;
    if (boundp(sym)) {
	val = eval(sym);
	if (symbolp(val) && strcmp(c_string(val), uname) == 0)
	  return TRUE;
	if (utypep(val) && c_number(val) == u)
	  return TRUE;
	if (consp(val)) {
	    for_all_list(val, rest) {
		head = car(rest);
		if (symbolp(head) && strcmp(c_string(head), uname) == 0)
		  return TRUE;
		if (utypep(head) && c_number(head) == u)
		  return TRUE;
	    }
	}
    }
    return FALSE;
}

static void
combat_desc_from_list(Side *side, Obj *lis, Unit *unit, Unit *unit2,
		      const char *str, char *buf)
{
    int n;
    const char *symname;
    Obj *rest, *item;

    buf[0] = '\0';
    for_all_list(lis, rest) {
	item = car(rest);
	if (stringp(item)) {
	    strcat(buf, c_string(item));
	} else if (symbolp(item)) {
	    symname = c_string(item);
	    if (strcmp(symname, "actor") == 0) {
		strcat(buf, unit_handle(side, unit));
	    } else if (strcmp(symname, "actee") == 0) {
		strcat(buf, unit_handle(side, unit2));
	    } else {
		tprintf(buf, " ??%s?? ", symname);
	    }
	} else if (numberp(item)) {
	    n = c_number(item);
	    if (0 /* special processing */) {
	    } else {
		tprintf(buf, "%d", n);
	    }
	} else {
	    strcat(buf, " ????? ");
	}
    }
}

void
report_give(Side *side, Unit *unit, Unit *unit2, short *rslts)
{
    char buf[BUFSIZE];
    int m, something = FALSE;

    sprintf(buf, "%s gave", unit_handle(side, unit));
    for_all_material_types(m) {
	if (rslts[m] > 0) {
	    tprintf(buf, " %d %s", rslts[m], m_type_name(m));
	    something = TRUE;
	}
    }
    if (!something) {
	strcat(buf, " nothing");
    }
    if (unit2 != NULL) {
	tprintf(buf, " to %s", unit_handle(side, unit2));
    }
    notify(side, "%s.", buf);
}

void
report_take(Side *side, Unit *unit, int needed, short *rslts)
{
    char buf[BUFSIZE];
    int m, something = FALSE;

    if (!needed) {
	notify(side, "%s needed nothing.", unit_handle(side, unit));
	return;
    }
    buf[0] = '\0';
    for_all_material_types(m) {
	if (rslts[m] > 0) {
	    tprintf(buf, " %d %s", rslts[m], m_type_name(m));
	    something = TRUE;
	}
    }
    if (something) {
	notify(side, "%s got%s.", unit_handle(side, unit), buf);
    } else {
	notify(side, "%s got nothing.", unit_handle(side, unit));
    }
}

void
notify_all_of_resignation(Side *side, Side *side2)
{
    Side *side3;

    for_all_sides(side3) {
	if (side3 != side) {
	    notify(side3,
		   "%s %s giving up!",
		   short_side_title_with_adjective(side,
#ifdef RUDE
		   (char *)(flip_coin() ? "cowardly" : "wimpy")
#else
		   NULL
#endif /* RUDE */
		   ),
		   (char *)(short_side_title_plural_p(side) ? "are" : "is"));
	    if (side2 != NULL) {
		notify(side3, "... and donating everything to %s!",
		       short_side_title(side2));
	    }
	}
    }
}

/* Given a number, figure out what suffix should go with it. */

const char *
ordinal_suffix(int n)
{
    if (n % 100 == 11 || n % 100 == 12 || n % 100 == 13) {
	return "th";
    } else {
	switch (n % 10) {
	  case 1:   return "st";
	  case 2:   return "nd";
	  case 3:   return "rd";
	  default:  return "th";
	}
    }
}

/* Pluralize a word, attempting to be smart about various
   possibilities that don't have a different plural form (such as
   "Chinese" and "Swiss"). */

/* There should probably be a test for when to add "es" instead of "s". */

char *
plural_form(const char *word)
{
    char endch = ' ', nextend = ' ';
    int len;

    if (word == NULL) {
	run_warning("plural_form given NULL string");
	pluralbuf[0] = '\0';
	return pluralbuf;
    }
    len = strlen(word);
    if (len > 0)
      endch = word[len - 1];
    if (len > 1)
      nextend = word[len - 2];
    if (endch == 'h' || endch == 's' || (endch == 'e' && nextend == 's')) {
	sprintf(pluralbuf, "%s", word);
    } else {
	sprintf(pluralbuf, "%ss", word);
    }
    return pluralbuf;
}

/* Make the first letter of the buffer upper-case. */

char *
capitalize(char *buf)
{
    if (islower(buf[0]))
      buf[0] = toupper(buf[0]);
    return buf;
}

/* Make all letters in the buffer upper-case. */

char *
all_capitals(char *buf)
{
    int i = 0;

    while (buf[i] != '\0') {
    	if (islower(buf[i]))
	      buf[i] = toupper(buf[i]);
    	i++;
    }
    return buf;
}

/* Compose a readable form of the given date. */

const char *
absolute_date_string(int date)
{
    /* The first time we ask for a date, interpret the calendar. */
    if (calendar_type == cal_unknown)
      init_calendar();
    switch (calendar_type) {
      case cal_number:
	sprintf(datebuf, "%s%4d", turn_name, date);
	return datebuf;
      case cal_usual:
	return usual_date_string(date);
      default:
	case_panic("calendar type", calendar_type);
    }
    return "!?!";
}

/* Interpret the calendar definition. */

static void
init_calendar(void)
{
    Obj *cal, *caltype, *stepname, *steprest;

    cal = g_calendar();
    turn_name = "Turn";
    if (cal == lispnil) {
	/* The default is to have numbered turns named "Turn". */
	calendar_type = cal_number;
    } else if (consp(cal)) {
	caltype = car(cal);
	if (match_keyword(caltype, K_NUMBER)) {
	    calendar_type = cal_number;
	    if (stringp(cadr(cal)))
	      turn_name = c_string(cadr(cal));
	} else if (match_keyword(caltype, K_USUAL)) {
	    calendar_type = cal_usual;
	    stepname = cadr(cal);
	    if (symbolp(stepname)) {
		parse_date_step_range(cdr(cal));
	    } else if (consp(stepname)) {
		for_all_list(cdr(cal), steprest) {
		    parse_date_step_range(car(steprest));
		}
	    } else {
		init_warning("No name for date step type, substituting `day'");
		date_step_ranges[0].step_type = ds_day;
		date_step_ranges[0].step_size = 1;
		num_date_step_ranges = 1;
	    }
	}
    }
    if (calendar_type == cal_unknown)
      init_error("Bad calendar type");
    if (!empty_string(g_initial_date()))
      set_initial_date(g_initial_date());
}

static void
parse_date_step_range(Obj *form)
{
    int n;
    const char *stepnamestr;
    UsualDateStepType steptype;
    Obj *step;

    n = num_date_step_ranges;
    /* Peel off a turn range if supplied. */
    if (numberp(car(form))) {
	date_step_ranges[n].turn_start = c_number(car(form));
	form = cdr(form);
    }
    if (numberp(car(form))) {
	date_step_ranges[n].turn_end = c_number(car(form));
	form = cdr(form);
    }
    if (symbolp(car(form))) {
	stepnamestr = c_string(car(form));
	if (strcmp(stepnamestr, "second") == 0) {
	    steptype = ds_second;
	} else if (strcmp(stepnamestr, "minute") == 0) {
	    steptype = ds_minute;
	} else if (strcmp(stepnamestr, "hour") == 0) {
	    steptype = ds_hour;
	} else if (strcmp(stepnamestr, "day") == 0) {
	    steptype = ds_day;
	} else if (strcmp(stepnamestr, "week") == 0) {
	    steptype = ds_week;
	} else if (strcmp(stepnamestr, "month") == 0) {
	    steptype = ds_month;
	} else if (strcmp(stepnamestr, "season") == 0) {
	    steptype = ds_season;
	} else if (strcmp(stepnamestr, "year") == 0) {
	    steptype = ds_year;
	} else {
	    init_warning("\"%s\" not a known date step name", stepnamestr);
	    steptype = ds_day;
	}
    } else {
	/* (should warn) */
	steptype = ds_day;
    }
    date_step_ranges[n].step_type = steptype;
    /* Collect an optional multiple. */
    step = cadr(form);
    date_step_ranges[n].step_size = (numberp(step) ? c_number(step) : 1);
    ++num_date_step_ranges;
}

/* Given two dates, figure out how many turns encompassed by them. */

int
turns_between(const char *datestr1, const char *datestr2)
{
    int rslt, turn1, turn2;
    UsualDate date1, date2;

    if (calendar_type == cal_unknown)
      init_calendar();
    switch (calendar_type) {
      case cal_number:
	sscanf("%d", datestr1, &turn1);
	sscanf("%d", datestr2, &turn2);
	return (turn2 - turn1);
      case cal_usual:
	parse_usual_date(datestr1, 0, &date1);
	parse_usual_date(datestr2, 0, &date2);
	rslt = date2.year - date1.year;
	if (num_date_step_ranges == 1) {
	    if (date_step_ranges[0].step_type == ds_year)
	      return (rslt / date_step_ranges[0].step_size);
	    if (date_step_ranges[0].step_type < ds_year) {
		rslt = (12 * rslt) - (date2.month - date1.month);
	    }
	    if (date_step_ranges[0].step_type == ds_month)
	      return (rslt / date_step_ranges[0].step_size);
	    if (date_step_ranges[0].step_type < ds_month) {
		rslt = (30 * rslt) - (date2.day - date1.day);
	    }
	    if (date_step_ranges[0].step_type == ds_week)
	      return (((rslt + 6) / 7) / date_step_ranges[0].step_size);
	    if (date_step_ranges[0].step_type == ds_day)
	      return (rslt / date_step_ranges[0].step_size);
	    if (date_step_ranges[0].step_type < ds_day) {
		rslt = (24 * rslt) - (date2.hour - date1.hour);
	    }
	    if (date_step_ranges[0].step_type == ds_hour)
	      return (rslt / date_step_ranges[0].step_size);
	    return (rslt / date_step_ranges[0].step_size); /* semi-bogus */
	} else {
	    /* Too complicated for now, give up. */
	    return 1;
	}
      default:
	case_panic("calendar type", calendar_type);
	break;
    }
    return 1;  /* appease the compiler */
}

/* Given a date string, make it be the date of the first turn. */

void
set_initial_date(const char *str)
{
    if (calendar_type == cal_unknown)
      init_calendar();
    switch (calendar_type) {
      case cal_number:
	sscanf("%d", str, &turn_initial);
	break;
      case cal_usual:
	if (usual_initial == NULL)
	  usual_initial = (UsualDate *) xmalloc(sizeof(UsualDate));
	parse_usual_date(str, 0, usual_initial);
	break;
      default:
	case_panic("calendar type", calendar_type);
	break;
    }
}

/* Pick a date out of the given string. Note that this implementation
   does not detect extra garbage in the string, should fix someday. */

/* (should use strtol etc instead of sscanf) */

static void
parse_usual_date(const char *datestr, int range, UsualDate *udate)
{
    char aname[BUFSIZE];
    int i, cnt;
    
    udate->second = udate->minute = udate->hour = 0;
    udate->day = udate->month = udate->year = 0;
    aname[0] = '\0';
    if (!empty_string(datestr)) {
	/* Assume it's in a standard date format. */
	switch (date_step_ranges[0].step_type) {
	  case ds_second:
	    cnt = sscanf(datestr, "%d:%d:%d %d %s %d",
			 &(udate->second), &(udate->minute), &(udate->hour),
			 &(udate->day), aname, &(udate->year));
	    if (cnt != 6) {
		cnt = sscanf(datestr, "%d:%d:%d",
			     &(udate->second), &(udate->minute), &(udate->hour));
		if (cnt != 3)
		  goto bad_format;
		return;
	    }
	    --(udate->day);
	    break;
	  case ds_minute:
	    cnt = sscanf(datestr, "%d:%d %d %s %d",
			 &(udate->minute), &(udate->hour),
			 &(udate->day), aname, &(udate->year));
	    if (cnt != 5) {
		cnt = sscanf(datestr, "%d:%d",
			     &(udate->minute), &(udate->hour));
		if (cnt != 2)
		  goto bad_format;
		return;
	    }
	    --(udate->day);
	    break;
	  case ds_hour:
	    cnt = sscanf(datestr, "%d:00 %d %s %d",
			 &(udate->hour),
			 &(udate->day), aname, &(udate->year));
	    if (cnt != 4)
	      cnt = sscanf(datestr, "%d %d %s %d",
			   &(udate->hour),
			   &(udate->day), aname, &(udate->year));
	    if (cnt != 4)
	      goto bad_format;
	    --(udate->day);
	    break;
	  case ds_day:
	  case ds_week:
	    cnt = sscanf(datestr, "%d %s %d",
			 &(udate->day), aname, &(udate->year));
	    if (cnt != 3)
	      goto bad_format;
	    --(udate->day);
	    break;
	  case ds_month:
	    cnt = sscanf(datestr, "%s %d", aname, &(udate->year));
	    if (cnt != 2)
	      goto bad_format;
	    break;
	  case ds_season:
	    cnt = sscanf(datestr, "%s %d", aname, &(udate->year));
	    if (cnt != 2)
	      goto bad_format;
	    for (i = 0; i < 4; ++i) {
		if (strcmp(aname, seasons[i]) == 0) {
		    udate->month = i * 3;
		    return;
		}
	    }
	    init_warning("\"%s\" not a recognized season name", aname);
	    return;
	  case ds_year:
	    cnt = sscanf(datestr, "%d", &(udate->year));
	    if (cnt != 1)
	      goto bad_format;
	    return;
	  default:
	    init_warning("%d not an allowed date step type",
			 date_step_ranges[0].step_type);
	    break;
	}
	for (i = 0; i < 12; ++i) {
	    /* (should make case-insensitive) */
	    if (strcmp(aname, months[i]) == 0) {
		udate->month = i;
		return;
	    }
	}
	init_warning("\"%s\" not a recognized month name", aname);
    }
    return;
  bad_format:
    init_warning("\"%s\" is a badly formatted date", datestr);
}

/* Given a numeric date, convert it into something understandable. */

static int ever_mentioned_bc;

static const char *
usual_date_string(int date)
{
    int year = 0, season = 0, month = 0, day = 0;
    int hour = 0, second = 0, minute = 0;
    int i, r, range;

    /* The date, which is a turn number, should be 1 or more, but this
       routine may be called before the game really starts, so return
       something that will be distinctive if it's ever displayed. */
    if (date <= 0)
      return "pregame";
    /* First displayed date is normally turn 1; be zero-based for
       the benefit of calculation. */
    --date;
    /* Find the date step type and size. */
    for (range = 0; range < num_date_step_ranges; ++range) {
	if (between(date_step_ranges[range].turn_start,
		    date,
		    date_step_ranges[range].turn_end))
	  break;
    }
    /* If no matches, just use the last range in the array. */
    if (range >= num_date_step_ranges)
      range = num_date_step_ranges - 1;
    /* If multiples of basic step, convert to basic step by multiplying. */
    date *= date_step_ranges[range].step_size;
    if (usual_initial == NULL) {
	usual_initial = (UsualDate *) xmalloc(sizeof(UsualDate));
	if (!empty_string(g_initial_date()))
	  parse_usual_date(g_initial_date(), 0, usual_initial);
    }
    switch (date_step_ranges[range].step_type) {
      case ds_second:
	second = date % 60;
	minute = date / 60;
	sprintf(datebuf, "%d:%d", minute, second);
	/* (should add day/month/year if available?) */
	break;
      case ds_minute:
	minute = date % 60;
	hour = date / 60 + usual_initial->hour;
	sprintf(datebuf, "%d:%d", hour, minute);
	/* (should add day/month/year if available?) */
	break;
      case ds_hour:
	date += usual_initial->hour;
 	hour = date % 24;
 	date /= 24;
	date += usual_initial->day;
	for (i = 0; i < usual_initial->month; ++i)
	  date += monthdays[i];
	day = date % 365;
	month = 0;
	for (i = 0; i < 12; ++i) {
	    if (day < monthdays[i])
	      break;
	    day -= monthdays[i];
	    ++month;
	}
	++day;
	year = date / 365 + usual_initial->year;
	sprintf(datebuf, "%d:00 %2d %s %d",
		hour, day, months[month], ABS(year));
	break;
      case ds_week:
	/* Convert to days, then proceed as for days. */
	date *= 7;
	/* Fall through. */
      case ds_day:
	date += usual_initial->day;
	for (i = 0; i < usual_initial->month; ++i)
	  date += monthdays[i];
	day = date % 365;
	month = 0;
	for (i = 0; i < 12; ++i) {
	    if (day < monthdays[i])
	      break;
	    day -= monthdays[i];
	    ++month;
	}
	++day;
	year = date / 365 + usual_initial->year;
	sprintf(datebuf, "%2d %s %d", day, months[month], ABS(year));
	break;
      case ds_month:
	date += usual_initial->month;
    	month = date % 12;
	year = date / 12 + usual_initial->year;
	sprintf(datebuf, "%s %d", months[month], ABS(year));
	break;
      case ds_season:
     	season = date % 4;
	year = date / 4 + usual_initial->year;
	sprintf(datebuf, "%s %d", seasons[season], ABS(year));
	break;
      case ds_year:
	year = usual_initial->year;
	/* (should do something similar for other step types) */
	for (r = 0; r < range; ++r)
	  year += ((date_step_ranges[r].turn_end
		    - date_step_ranges[r].turn_start)
		   * date_step_ranges[r].step_size);
	year += (date - (date_step_ranges[range].turn_start
			 * date_step_ranges[range].step_size));
	sprintf(datebuf, "%d", ABS(year));
	break;
      default:
	sprintf(datebuf, "%d is unknown date step type",
		date_step_ranges[0].step_type);
	break;
    }
    if (year < 0) {
	strcat(datebuf, " BC");
	ever_mentioned_bc = TRUE;
    } else if (ever_mentioned_bc) {
	/* If any date was displayed as "BC", use "AD" with all positive
	   year numbers. */
	strcat(datebuf, " AD");
    }
    return datebuf;
}

/* Show some overall numbers on performance of a side. */

void
write_side_results(FILE *fp, Side *side)
{
    int i;

    if (side == NULL) {
	fprintf(fp, "Results for game as a whole:\n\n");
    } else {
	fprintf(fp, "Results for %s%s",
		short_side_title(side),
		(side_won(side) ? " (WINNER)" :
		 (side_lost(side) ? " (LOSER)" :
		  "")));
	for (i = 0; i < numscores; ++i) {
	    fprintf(fp, " %d", side->scores[i]);
	}
	fprintf(fp, ", played by %s:\n\n",
		long_player_title(spbuf, side->player, NULL));
    }
}

/* Display what is essentially a double-column bookkeeping of unit gains
   and losses. */

void
write_unit_record(FILE *fp, Side *side)
{
    int u, gainreason, lossreason, totgain, totloss, val;

    fprintf(fp, "Unit Record (gains and losses by cause and unit type)\n");
    fprintf(fp, " Unit Type ");
    for (gainreason = 0; gainreason < num_gain_reasons; ++gainreason) {
	fprintf(fp, " %3s", gain_reason_names[gainreason]);
    }
    fprintf(fp, " Gain |");
    for (lossreason = 0; lossreason < num_loss_reasons; ++lossreason) {
	fprintf(fp, " %3s", loss_reason_names[lossreason]);
    }
    fprintf(fp, " Loss |");
    fprintf(fp, " Total\n");
    for_all_unit_types(u) {
	if (u_possible[u]) {
	    totgain = 0;
	    fprintf(fp, " %9s ", utype_name_n(u, 9));
	    for (gainreason = 0; gainreason < num_gain_reasons; ++gainreason) {
		val = gain_count(side, u, gainreason);
		if (val > 0) {
		    fprintf(fp, " %3d", val);
		    totgain += val;
		} else {
		    fprintf(fp, "    ");
		}
	    }
	    fprintf(fp, "  %3d |", totgain);
	    totloss = 0;
	    for (lossreason = 0; lossreason < num_loss_reasons; ++lossreason) {
		val = loss_count(side, u, lossreason);
		if (val > 0) {
		    fprintf(fp, " %3d", val);
		    totloss += val;
		} else {
		    fprintf(fp, "    ");
		}
	    }
	    fprintf(fp, "  %3d |", totloss);
	    fprintf(fp, "  %3d\n", totgain - totloss);
	}
    }
    fprintf(fp, "\n");
}

static int
gain_count(Side *side, int u, int r)
{
    int sum;

    if (side != NULL)
      return side_gain_count(side, u, r);
    sum = 0;
    for_all_sides(side) {
	sum += side_gain_count(side, u, r);
    }
    return sum;
}

static int
loss_count(Side *side, int u, int r)
{
    int sum;

    if (side != NULL)
      return side_loss_count(side, u, r);
    sum = 0;
    for_all_sides(side) {
	sum += side_loss_count(side, u, r);
    }
    return sum;
}

/* Nearly-raw combat statistics; hard to interpret, but they provide
   a useful check against subjective evaluation of performance. */

void
write_combat_results(FILE *fp, Side *side)
{
    int a, d, atk;

    fprintf(fp,
	    "Unit Combat Results (average damage over # attacks against enemy, by type)\n");
    fprintf(fp, " A  D->");
    for_all_unit_types(d) {
	if (u_possible[d]) {
	    fprintf(fp, " %4s ", utype_name_n(d, 4));
	}
    }
    fprintf(fp, "\n");
    for_all_unit_types(a) {
	if (u_possible[a]) {
	    fprintf(fp, " %4s ", utype_name_n(a, 4));
	    for_all_unit_types(d) {
		if (u_possible[d]) {
		    atk = atkstats(side, a, d);
		    if (atk > 0) {
			fprintf(fp, " %5.2f",
				((float) hitstats(side, a, d)) / atk);
		    } else {
			fprintf(fp, "      ");
		    }
		}
	    }
	    fprintf(fp, "\n     ");
	    for_all_unit_types(d) {
		if (u_possible[d]) {
		    atk = atkstats(side, a, d);
		    if (atk > 0) {
			fprintf(fp, " %4d ", atk);
		    } else {
			fprintf(fp, "      ");
		    }
		}
	    }
	    fprintf(fp, "\n");
	}
    }
    fprintf(fp, "\n");
}

static int
atkstats(Side *side, int a, int d)
{
    int sum;

    if (side != NULL)
      return side_atkstats(side, a, d);
    sum = 0;
    for_all_sides(side) {
	sum += side_atkstats(side, a, d);
    }
    return sum;
}

static int
hitstats(Side *side, int a, int d)
{
    int sum;

    if (side != NULL)
      return side_hitstats(side, a, d);
    sum = 0;
    for_all_sides(side) {
	sum += side_hitstats(side, a, d);
    }
    return sum;
}

/* The following code formats a list of types that are missing images. */

void
record_missing_image(int typtyp, const char *str)
{
    if (missinglist == NULL) {
	missinglist = (char *)xmalloc(BUFSIZE);
	missinglist[0] = '\0';
    }
    ++missing[typtyp];
    /* Add the name of the image-less type, but only if one of
       the first few. */
    if (between(1, totlisted, NUMTOLIST))
      strcat(missinglist, ",");
    if (totlisted < NUMTOLIST) {
	strcat(missinglist, str);
    } else if (totlisted == NUMTOLIST) {
	strcat(missinglist, "...");
    }
    ++totlisted;
}

/* Return true if any images could not be found, and provide some
   helpful info into the supplied buffer. */

int
missing_images(char *buf)
{
    if (missinglist == NULL)
      return FALSE;
    buf[0] = '\0';
    if (missing[UTYP] > 0)
      tprintf(buf, " %d unit images", missing[UTYP]);
    if (missing[TTYP] > 0)
      tprintf(buf, " %d terrain images", missing[TTYP]);
    if (missing[3] > 0)
      tprintf(buf, " %d emblems", missing[3]);
    tprintf(buf, " - %s", missinglist);
    return TRUE;
}

/* format a number up to 32 bits with 3 digits of precision. */

char
*format_number(char *buf, int value) 
{
    const int thousand = 1000;
    const int million = thousand * thousand;
    char *spbuf = buf;
    
    if(value < 0) {
	value = -value;
	*spbuf++ = '-';
    }
    *spbuf = 0;
    if (value < 10 * thousand) {
    	sprintf(spbuf, "%d", value);
    } else if (value < 100 * thousand) {
    	sprintf(spbuf,"%4.1fk", ((double) value) / thousand);
    } else if (value < million) {
    	sprintf(spbuf,"%dK", value / thousand);
    } else if (value < 10 * million) {
    	sprintf(spbuf,"%4.2fM", ((double) value) / million);
    } else if(value < 100 * million) {
    	sprintf(spbuf,"%4.1fM", ((double) value) / million);
    } else if(value < 1000 * million) {
    	sprintf(spbuf,"%dM", value / million);
    } else {
    	sprintf(spbuf,"%4.2G",((double) value) / (1000 * million));
    }
    return buf;
}
