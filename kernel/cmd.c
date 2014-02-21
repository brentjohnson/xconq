/* Generic commands.
   Copyright (C) 1998-2000 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/* The commands in this file are the same for all interfaces.  They
   typically consist of direct operations on units, or complicated
   command-line-type commands. */

#include "conq.h"
#include "kpublic.h"
#include "imf.h"
#include "ui.h"

int autofinish_start;
int autofinish_count;

static void notify_relationships(Side *side);
static int parse_advance_spec(char *str);
static int do_one_auto(Side *side, Unit *unit);
static int do_one_clear_plan(Side *side, Unit *unit);
static int do_one_delay(Side *side, Unit *unit);
static int do_one_detonate(Side *side, Unit *unit);
static int do_one_disband(Side *side, Unit *unit);
static int do_one_disembark(Side *side, Unit *unit);
static int do_one_embark(Side *side, Unit *unit);
static int do_one_give(Side *side, Unit *unit);
static int do_one_idle(Side *side, Unit *unit);
static int do_one_reserve(Side *side, Unit *unit);
static int do_one_return(Side *side, Unit *unit);
static int do_one_sleep(Side *side, Unit *unit);
static int do_one_take(Side *side, Unit *unit);
static int do_one_wake(Side *side, Unit *unit);
static int do_one_wake_all(Side *side, Unit *unit);

/* The command table is an array of all the commands. */

typedef struct cmdtab {
    char fchar;			/* character to match against */
    char *name;			/* Full name of command */
    void (*fn)(Side *side);	/* Pointer to command's function */
    char *help;                 /* short documentation string */
} CmdTab;

#define C(c) ((c)-0x40)

#undef DEF_CMD
#define DEF_CMD(LETTER,NAME,FN,HELP) { LETTER, NAME, FN, HELP },

/* Define the table of commands. */

CmdTab commands[] = {

#include "cmd.def"

  { 0, NULL, /* NULL, */ NULL, NULL }
};

/* This is the actual key typed, for use if several keyboard commands
   share a single function. */

char tmpkey;

/* If the command was typed as a command-line sort of thing, then this
   global will point to whatever came after the name of the
   command. */

char *cmdargstr;

/* Help nodes that list all the commands, both single-char and long. */

HelpNode *key_commands_help_node;

HelpNode *long_commands_help_node;

HelpNode *map_help_node;

/* Search in command table and execute function if found, complaining if
   the command is not recognized. */

void
execute_command(Side *side, int ch)
{
    CmdTab *cmd;
    void (*fn)(Side *side);

    /* The long-name commands all have a char of 0, so we must ensure
       nonzero characters coming in here. */
    if (ch == '\0')
      run_error("Character '\\0' in execute_command");
    cmdargstr = NULL;
    for (cmd = commands; cmd->name != NULL; ++cmd) {
	if (ch == cmd->fchar) {
	    fn = cmd->fn;
	    if (fn == NULL) {
		run_warning("no command function for %s (0x%x)?",
			    cmd->name, ch);
		return;
	    }
	    tmpkey = ch;
	    (*fn)(side);
	    /* Whatever might have happened, we *did* find the command. */
	    return;
	}
    }
    cmd_error(side, "unrecognized command key '%c'", ch);
}

/* Given a string, find and interpret a long-name command in it. */

void
execute_long_command(Side *side, char *cmdstr)
{
    int prefix;
    char *cmdname;
    CmdTab *cmd;
    void (*fn)(Side *side);

    /* (should do something with prefix, which is presently ignored) */
    parse_long_name_command(cmdstr, &prefix, &cmdname, &cmdargstr,
			    copy_string(cmdstr));
    if (empty_string(cmdname)) {
	/* Treat this is a deliberate cancellation, not an error. */
	notify(side, "No command.");
	return;
    }
    for (cmd = commands; cmd->name != NULL; ++cmd) {
	if (strcmp(cmdname, cmd->name) == 0) {
	    fn = cmd->fn;
	    if (fn == NULL) {
		run_warning("no command function for %s?", cmd->name);
		return;
	    }
	    tmpkey = cmd->fchar;
	    (*fn)(side);
	    /* Whatever might have happened, we *did* find the command. */
	    return;
	}
    }
    cmd_error(side, "unrecognized command name \"%s\"", cmdname);
}

/* Collect a command name and an argument string from the given
   string, discarding excess whitespace. */

void
parse_long_name_command(char *cmdstr, int *prefixp, char **namep, char **argp,
			char *buf)
{
    int j, prefixarg = -1;
    char *cmdname, *cmdarg;

    if (empty_string(cmdstr)) {
	*namep = *argp = NULL;
	return;
    }
    strcpy(buf, cmdstr);
    /* Look for the first nonwhite char, make it start of command name. */
    cmdname = buf;
    while ((*cmdname == ' ' || *cmdname == '\t') && *cmdname != '\0')
      ++cmdname;
    /* If this is a digit, then it's a prefix arg; extract it. */
    if (isdigit(*cmdname)) {
	prefixarg = *cmdname - '0';
	++cmdname;
	while (isdigit(*cmdname)) {
	    prefixarg = prefixarg * 10 + (*cmdname - '0');
	    ++cmdname;
	}
	/* Skip over any space between prefix arg and cmdname. */
	while ((*cmdname == ' ' || *cmdname == '\t') && *cmdname != '\0')
	  ++cmdname;
    }
    /* Scan over command name, which is delimited by whitespace or end
       of line. */
    for (j = 0; cmdname[j] != ' ' && cmdname[j] != '\t' && cmdname[j] != '\n' && cmdname[j] != '\0'; ++j)
      ;
    /* If there's more than just the command name, extract an argument. */
    if (cmdname[j] != '\0') {
	cmdarg = cmdname + j + 1;
	while ((*cmdarg == ' ' || *cmdname == '\t' || *cmdname == '\n') && *cmdarg != '\0')
	  ++cmdarg;
	if (*cmdarg == '\0')
	  cmdarg = NULL;
    } else {
	cmdarg = NULL;
    }
    /* Terminate the command name. */
    cmdname[j] = '\0';
    /* Remove trailing whitespace from the argument. */
    if (!empty_string(cmdarg)) {
	for (j = strlen(cmdarg) - 1; j >= 0 && (cmdarg[j] == ' ' || cmdarg[j] == '\t' || cmdarg[j] == '\n'); --j)
	  ;
	cmdarg[j + 1] = '\0';
    }
    DGprintf("Command is \"%s\", argument is \"%s\", prefix is %d\n",
	     (cmdname != NULL ? cmdname : "<null>"),
	     (cmdarg != NULL ? cmdarg : "<null>"),
	     prefixarg);
    *prefixp = prefixarg;
    *namep = cmdname;
    *argp = cmdarg;
}

/* Describe all the single-key commands for help. */

void
describe_key_commands(int arg, char *key, TextBuffer *buf)
{
    CmdTab *cmd;

    for (cmd = commands; cmd->name != NULL; ++cmd) {
	describe_command(cmd->fchar, cmd->name, cmd->help, TRUE, buf);
    }
}

/* Describe all the long-name commands for help. */

void
describe_long_commands(int arg, char *key, TextBuffer *buf)
{
    CmdTab *cmd;

    for (cmd = commands; cmd->name != NULL; ++cmd) {
	describe_command (cmd->fchar, cmd->name, cmd->help, FALSE, buf);
    }
}

static int
apply_to_all_selected_units(Side *side, int (*fn)(Side *side, Unit *unit))
{
    int i, rslt, numcould = 0, numfail = 0;
    Unit *unit;
    UnitVector *selvec;

    if (fn == NULL)
      /* (should be a fatal error?) */
      return 0;
    /* Scan all the selected units. Note that we assume that the list
       (unit vector) doesn't change while we're scanning, which means
       that we should be checking each unit, in case a command caused
       later units in the list to disappear (for instance a
       detonation). */
    selvec = get_selected_units(side);
    for (i = 0; i < selvec->numunits; ++i) {
	unit = unit_in_vector(selvec, i);
	if (in_play(unit) && side_controls_unit(side, unit)) {
	    ++numcould;
	    rslt = (*fn)(side, unit);
	    if (!rslt)
	      ++numfail;
	}
    }
    /* Return true if everything went OK, false if the function failed
       on some units. */
    return ((numcould > 0) ? (numfail == 0) : TRUE);
}

/* Generic command functions.  These are the command functions that
   behave identically for all interfaces. */

#define DURING_GAME_ONLY(side)  \
  if (endofgame) {  \
    cmd_error((side), "Cannot do after game is over!");  \
    return;  \
  }

void
do_add_player(Side *side)
{
    DURING_GAME_ONLY(side);
    net_request_additional_side(cmdargstr);
}

#if 0

void
do_agreement_draft(Side *side)
{
    char *title, titlebuf[20];
    Agreement *ag;

    DURING_GAME_ONLY(side);
    /* (should have a net_create_agreement) */
    ag = create_agreement(0);
    if (!empty_string(cmdargstr)) {
	title = cmdargstr;
    } else {
	sprintf(titlebuf, "#%d", ag->id);
	title = copy_string(titlebuf);
    }
    ag->name = title;
    /* The creating side is by definition one of the drafters. */
    ag->drafters = add_side_to_set(side, ag->drafters);
}

void
do_agreement_drafter(Side *side)
{
    DURING_GAME_ONLY(side);
}

void
do_agreement_propose(Side *side)
{
    DURING_GAME_ONLY(side);
}

void
do_agreement_proposer(Side *side)
{
    DURING_GAME_ONLY(side);
}

void
do_agreement_sign(Side *side)
{
    DURING_GAME_ONLY(side);
}

void
do_agreement_signer(Side *side)
{
    DURING_GAME_ONLY(side);
}

void
do_agreement_term(Side *side)
{
    int agid, agterm;
    char *arg, *rest, *term;
    Agreement *ag;

    DURING_GAME_ONLY(side);
    rest = get_next_arg(cmdargstr, tmpbuf, &arg);
    if (empty_string(arg)) {
	cmd_error(side, "no agreement id");
	return;
    }
    agid = strtol(arg, NULL, 10);
    agterm = 0;
    if (isdigit(*rest)) {
	rest = get_next_arg(rest, tmpbuf, &arg);
	agterm = strtol(arg, NULL, 10);
    }
    term = copy_string(rest);
    ag = find_agreement(agid);
    if (ag == NULL) {
	cmd_error(side, "no agreement #%d", agid);
	return;
    }
    /* (should be kernel function) */
    ag->terms = cons(new_string(term), ag->terms);
}

#endif

/* Set which AI is to run the side's play. */

void
do_ai_side(Side *side)
{
    char *arg, *rest, *aitypename = NULL;

    DURING_GAME_ONLY(side);
    /* Look at the optional command argument, extract options and/or
       AI type name from it. */
    if (!empty_string(cmdargstr)) {
	rest = get_next_arg(cmdargstr, tmpbuf, &arg);
	if (strcmp(arg, "help") == 0
	    || strcmp(arg, "?") == 0) {
	    aitypename = next_ai_type_name(NULL);
	    while (aitypename != NULL) {
		notify(side, " %s - %s",
		       aitypename, ai_type_help(find_ai_type(aitypename)));
		aitypename = next_ai_type_name(aitypename);
	    }
	    return;
	} else if (strcmp(arg, "+") == 0) {
	    /* Note that this flag is used by local AIs only, so don't
	       need to synchronize kernels here. */
	    side->ai_may_resign = TRUE;
	    notify(side, "AI may decide to draw or resign.");
	    return;
	} else if (strcmp(arg, "-") == 0) {
	    side->ai_may_resign = FALSE;
	    notify(side, "AI may only recommend whether to draw or resign.");
	    return;
	}
	aitypename = arg;
    }
    if (empty_string(aitypename)) {
    	/* Toggle AI between mplayer and NULL. */
	/* (should toggle between "preferred AI" for the side) */
    	if (side->player->aitypename == NULL) {
	    aitypename = "mplayer";
	} else {
	    aitypename = NULL;
    	}
    } else if (strcmp(aitypename, "none") == 0) {
	/* Special case to handle command "AI None" in Side menu. */
	aitypename = NULL;
    }
    /* Feed back to user.  Note that we say "want to have" because we
       haven't gotten feedback from remote programs yet. */
    if (!empty_string(aitypename)) {
	notify(side, "Want to have AI %s running this side.",
	       aitypename);
    } else {
	notify(side, "Want to have no AI running this side.");
    }
    net_set_side_ai(side, aitypename);
}

void
do_auto(Side *side)
{
    DURING_GAME_ONLY(side);
    apply_to_all_selected_units(side, do_one_auto);
}

static int
do_one_auto(Side *side, Unit *unit)
{
    int newval;

    if (unit->plan == NULL)
      return FALSE;
    if (side->prefixarg < 0)
      newval = !unit->plan->aicontrol;
    else if (side->prefixarg == 0)
      newval = FALSE;
    else
      newval = TRUE;
    net_set_unit_ai_control(side, unit, newval, FALSE);
    net_force_replan(unit);
    return TRUE;
}

void
do_c_rate(Side *side)
{
    int m = NONMTYPE, m1, m2, n, tot, pct;
    char *str, *reststr;
    char tbuf[BUFSIZE];

    DURING_GAME_ONLY(side);
    /* For now, assume only one m. */
    for_all_material_types(m1) {
	for_all_material_types(m2) {
	    if (mm_conversion(m1, m2) > 0) {
		m = m1;
		break;
	    }
	}
    }
    if (m == NONMTYPE)
      return;
    if (side->c_rates == NULL)
      side->c_rates = (short *) xmalloc(nummtypes * sizeof(short));
    if (!empty_string(cmdargstr)) {
	str = cmdargstr;
	for_all_material_types(m2) {
	    if (mm_conversion(m, m2) > 0) {
		n = strtol(str, &reststr, 10);
		str = reststr;
		/* (should use a net_ routine) */
		side->c_rates[m2] = n;
	    }
	}
    }
    tot = 0;
    for_all_material_types(m2) {
	if (mm_conversion(m, m2) > 0) {
	    tot += side->c_rates[m2];
	}
    }
    tbuf[0] = '\0';
    for_all_material_types(m2) {
	if (mm_conversion(m, m2) > 0) {
	    pct = (tot > 0 ? ((side->c_rates[m2] * 100) / tot) : 0);
	    sprintf(tbuf+strlen(tbuf), "  %s %d%%", m_type_name(m2), pct);
	}
    }
    notify(side, "%s conversion rates: %s", m_type_name(m), tbuf);
}

void
do_clear_plan(Side *side)
{
    DURING_GAME_ONLY(side);
    apply_to_all_selected_units(side, do_one_clear_plan);
}

static int
do_one_clear_plan(Side *side, Unit *unit)
{
    if (unit->plan == NULL)
      return FALSE;
    net_set_unit_plan_type(side, unit, PLAN_NONE);
    return TRUE;
}

void
do_delay(Side *side)
{
    DURING_GAME_ONLY(side);
    apply_to_all_selected_units(side, do_one_delay);
}

static int
do_one_delay(Side *side, Unit *unit)
{
    if (unit->plan == NULL)
      return FALSE;
    net_delay_unit(unit, TRUE);
    return TRUE;
#if 0 /* add this case from tkcmd.c? */
	unit = find_next_awake_mover(side, map->curunit);
	if (unit != map->curunit) {
	    set_current_unit(map, unit);
	} else {
	    cmd_error(side, "No next awake mover found.");
	}
#endif
}

void
do_detonate(Side *side)
{
    DURING_GAME_ONLY(side);
    /* (should add Mac version's choice of where to detonate?) */
    apply_to_all_selected_units(side, do_one_detonate);
}

static int
do_one_detonate(Side *side, Unit *unit)
{
    int rslt;

    rslt = check_detonate_action(unit, unit, unit->x, unit->y, unit->z);
    if (valid(rslt)) {
	net_prep_detonate_action(unit, unit, unit->x, unit->y, unit->z);
	return TRUE;
    } else {
	notify(side, "can't detonate for some reason?");
	return FALSE;
    }
}

/* Get rid of a unit. */

void
do_disband(Side *side)
{
    DURING_GAME_ONLY(side);
    apply_to_all_selected_units(side, do_one_disband);
}

static int
do_one_disband(Side *side, Unit *unit)
{
    /* (should have more feedback?) */
    net_disband_unit(side, unit);
    return TRUE;
}

void
do_disembark(Side *side)
{
    DURING_GAME_ONLY(side);
    apply_to_all_selected_units(side, do_one_disembark);
}

static int
do_one_disembark(Side *side, Unit *unit)
{
    Unit *transport;

    transport = unit->transport;
    if (transport == NULL) {
	cmd_error(side, "Not in a transport");
	return FALSE;
    }
    if (!in_play(transport)) {
	cmd_error(side, "Transport is nonsensical?");
	return FALSE;
    }
    /* Try moving into the transport's transport, if there is one. */
    if (transport->transport != NULL
        && can_occupy(unit, transport->transport)) {
	net_prep_enter_action(unit, unit, transport->transport);
	/* (should be able to set up task if can't do action immediately) */
	return TRUE;
    }
    /* Try moving into the open in the cell. We don't test for
    type_survives_in_cell here, so it is possible to deliberately
    jump overboard. */
    if (type_can_occupy_cell(unit->type, unit->x, unit->y)) {
	net_prep_move_action(unit, unit, unit->x, unit->y, unit->z);
	/* (should be able to set up task if can't do action immediately) */
	return TRUE;
    }
    cmd_error(side, "Can't disembark here!");
    return FALSE;
}

void
do_distrust(Side *side)
{
    Side *side2;

    DURING_GAME_ONLY(side);
    if (empty_string(cmdargstr)) {
	notify_relationships(side);
	return;
    }
    side2 = parse_side_spec(cmdargstr);
    if (side2 == NULL) {
	cmd_error(side, "No side matching \"%s\"", cmdargstr);
	return;
    }
    if (side2 == side) {
	cmd_error(side, "We're not confused about trusting ourselves!");
	return;
    }
    net_set_trust(side, side2, 0);
}

void
notify_relationships(Side *side)
{
    char *buf, buf1[1000], buf2[1000], buf3[1000], buf4[1000];
    Side *side2;

    /* List the status of all other sides wrt us. */
    buf1[0] = buf2[0] = buf3[0] = buf4[0] = '\0';
    for_all_sides(side2) {
	if (side2 != side) {
	    /* Choose which of four possible buckets the side falls into. */
	    if (trusted_side(side, side2)) {
		if (trusted_side(side2, side)) {
		    buf = buf1;
		} else {
		    buf = buf2;
		}
	    } else {
		if (trusted_side(side2, side)) {
		    buf = buf3;
		} else {
		    buf = buf4;
		}
	    }
	    /* Add the side's title to the chosen bucket. */
	    if (strlen(buf) > 0)
	      strcat(buf, ", ");
	    strcat(buf, short_side_title(side2));
	}
    }
    /* Describe each bucket. */
    if (strlen(buf1) > 0)
      notify(side, "We trust them, and vice versa: %s.", buf1);
    if (strlen(buf2) > 0)
      notify(side, "We trust them, but they don't trust us: %s.", buf2);
    if (strlen(buf3) > 0)
      notify(side, "They trust us, but we don't trust them: %s.", buf3);
    if (strlen(buf4) > 0)
      notify(side, "We don't trust them, and vice versa: %s.", buf4);
}

void
do_doctrine(Side *side)
{
    DURING_GAME_ONLY(side);
    if (!empty_string(cmdargstr)) {
	if (strcmp(cmdargstr, "set") == 0) {
	    notify(side, "Set what?");
	} else if (strncmp(cmdargstr, "set ", 4) == 0) {
	    net_set_doctrine(side, cmdargstr + 4);
	} else if (strcmp(cmdargstr, "help") == 0
		   || strcmp(cmdargstr, "?") == 0) {
	    notify(side, "doctrine set <doct-name> <property> <value>");
	    notify(side, "\"doctrine\" alone to see current settings");
	}
	return;
    }
    notify_doctrine(side, cmdargstr);
}

void
do_down(Side *side)
{
    cmd_error(side, "not yet implemented");
}

void
do_draw_willingness(Side *side)
{
    DURING_GAME_ONLY(side);
    if (side->prefixarg < 0)
      side->prefixarg = 1;
    net_set_willing_to_draw(side, (side->prefixarg ? 1 : 0));
}

void
do_embark(Side *side)
{
    DURING_GAME_ONLY(side);
    apply_to_all_selected_units(side, do_one_embark);
}

static int
do_one_embark(Side *side, Unit *unit)
{
    Unit *transport;

    transport = embarkation_unit(unit);
    if (transport != NULL) {
	net_prep_enter_action(unit, unit, transport);
	return TRUE;
    }
    cmd_error(side, "Nothing for %s to enter!", unit_handle(side, unit));
    return FALSE;
}

/* Command to end the side's activity for the turn. */

void
do_end_turn(Side *side)
{
    DURING_GAME_ONLY(side);
    net_finish_turn(side);
}

void
do_force_global_replan(Side *side)
{
    Unit *unit;
    
    DURING_GAME_ONLY(side);
    for_all_side_units(side, unit) {
	if (in_play(unit) && unit->plan != NULL) {
	    net_force_replan(unit);
	}
    }
}

void
do_give(Side *side)
{
    DURING_GAME_ONLY(side);
    if (nummtypes == 0) {
	cmd_error(side, "No materials in this game!");
	return;
    }
    apply_to_all_selected_units(side, do_one_give);
}

static short *gt_amts;
static short *gt_rslts;

static int
do_one_give(Side *side, Unit *unit)
{
    short m;
    Unit *unit2;

    if (gt_amts == NULL)
      gt_amts = (short *) xmalloc(nummtypes * sizeof(short));
    if (gt_rslts == NULL)
      gt_rslts = (short *) xmalloc(nummtypes * sizeof(short));

    for_all_material_types(m)
      gt_amts[m] = side->prefixarg;
    unit2 = give_supplies(unit, gt_amts, gt_rslts);
    report_give(side, unit, unit2, gt_rslts);
    return TRUE;
}

/* Tell the unit to sit around for a given number of turns. */

void
do_idle(Side *side)
{
    DURING_GAME_ONLY(side);
    apply_to_all_selected_units(side, do_one_idle);
}

static int
do_one_idle(Side *side, Unit *unit)
{
    if (side->prefixarg < 0)
      side->prefixarg = 9999;
    net_set_sentry_task(unit, side->prefixarg);
    return TRUE;
}

/* Release a controlled side to act independently. */

void
do_release(Side *side)
{
    Side *side2;

    DURING_GAME_ONLY(side);
    if (empty_string(cmdargstr)) {
	cmd_error(side, "No side given to release!");
	return;
    }
    side2 = parse_side_spec(cmdargstr);
    if (side2 == NULL) {
	cmd_error(side, "Can't get a side from \"%s\"!", cmdargstr);
	return;
    }
    if (side2 == side) {
	cmd_error(side, "Can't release ourselves!?");
	return;
    }
    if (side2->controlled_by != side) {
	cmd_error(side, "You don't control %s!", short_side_title(side2));
	return;
    }
    /* Modify the controllee's bit, not the controller's. */
    net_set_controlled_by(side2, side, FALSE);
}

/* Set a side's current research topic. */

void
do_research(Side *side)
{
    int a;

    DURING_GAME_ONLY(side);
    if (numatypes == 0) {
	cmd_error(side, "No research in this game.");
	return;
    }
    if (!empty_string(cmdargstr)) {
	if (!g_side_can_research()) {
	    cmd_error(side, "Side cannot do research.");
	    return;
	}
	if (strcmp(cmdargstr, "nothing") == 0) {
	    notify(side, "Your wise men will rest now.");
	    net_set_side_research_topic(side, NONATYPE);
	    return;
	}
	a = parse_advance_spec(cmdargstr);
	if (a == -1) {
	    cmd_error(side, "Not a valid advance: \"%s\"", cmdargstr);
	    return;
	} else if (a == -2) {
	    cmd_error(side, "Ambiguous: \"%s\"", cmdargstr);
	    return;
	}
	notify(side, "Your wise men will search for the secret of %s.",
	       a_type_name(a));
	net_set_side_research_topic(side, a);
	return;
    } else {
	int i = 0;
        char abuf[BUFSIZE];

	if (g_side_can_research()) {
	    a = side->research_topic;
	    if (a == NOADVANCE) {
		notify(side, "Your wise men are waiting for a research area.");
	    } else if (a == NONATYPE) {
		notify(side, "Your wise men are resting.");
	    } else {
		notify(side,
		       "Your wise men are %d/%d of the way to achieving %s.",
		       side->advance[a], a_rp(a), a_type_name(a));
	    }
	}
	/* List the available advances. */
	/* (should be a separate function perhaps) */
	strcpy(abuf, "Next advances: ");
	for_all_advance_types(a) {
	    if (side_can_research(side, a)) {
		if (i > 0)
		  strcat(abuf, ", ");
		strcat(abuf, a_type_name(a));
		++i;
		if (i == 4) {
		    notify(side, "%s", abuf);
		    abuf[0] = '\0';
		    i = 0;
		}
	    }
	}
	if (i > 0)
	  notify(side, "%s", abuf);
    }
}

/* Given a string, find an advance whose name at least partially matches. */

int
parse_advance_spec(char *str)
{
    int a, rslt = -1;

    for_all_advance_types(a) {
	if (strstr(a_type_name(a), str)) {
	    if (rslt >= 0) {
		return -2;
	    }
	    rslt = a;
	}
    }
    return rslt;
}

/* Make the selected units sleep just for the remainder of the current
   turn. */

void
do_reserve(Side *side)
{
    DURING_GAME_ONLY(side);
    apply_to_all_selected_units(side, do_one_reserve);
}

static int
do_one_reserve(Side *side, Unit *unit)
{
    net_set_unit_reserve(side, unit, TRUE, FALSE);
    return TRUE;
}

/* Set up tasks to resupply the selected units. */

/* (should warn if task is likely to fail) */

void
do_return(Side *side)
{
    DURING_GAME_ONLY(side);
    apply_to_all_selected_units(side, do_one_return);
}

static int
do_one_return(Side *side, Unit *unit)
{
    /* (should doublecheck range and error out if no chance) */
    net_set_resupply_task(unit, NONMTYPE);
    return TRUE;
}

/* Auto-finish turns, for the given number of turns. */

void
do_run(Side *side)
{
    int turns;

    DURING_GAME_ONLY(side);
    if (!empty_string(cmdargstr))
      turns = atoi(cmdargstr);
    else
      turns = side->prefixarg;
    if (turns < 0)
      turns = 0;
    notify(side, "Running free for %d turn%s.",
	   turns, (turns == 1 ? "" : "s"));
    if (turns > 0) {
	net_set_autofinish(side, TRUE);
	autofinish_start = g_turn();
	autofinish_count = turns;
    }
}

/* Set the rate at which AIs move their units. */

void
do_set_rate(Side *side)
{
    int slow, fast;
    char *reststr;

    DURING_GAME_ONLY(side);
    if (!empty_string(cmdargstr)) {
	slow = strtol(cmdargstr, &reststr, 10);
	fast = strtol(reststr, &reststr, 10);
	set_play_rate(slow, fast);
    }
}

/* Put a unit to sleep, which means it is inactive each turn until it
   is woken up, either by some alarm going off (such as low supply) or
   by explicit player action. */

void
do_sleep(Side *side)
{
    DURING_GAME_ONLY(side);
    apply_to_all_selected_units(side, do_one_sleep);
}

static int
do_one_sleep(Side *side, Unit *unit)
{
    net_set_unit_asleep(side, unit, TRUE, FALSE);
    return TRUE;
}

/* Submit to (become controlled by) the specified side. */
 
void
do_submit(Side *side)
{
    Side *side2;

    DURING_GAME_ONLY(side);
    if (side->controlled_by != NULL) {
	cmd_error(side, "You are already controlled by %s!",
		  short_side_title(side->controlled_by));
    }
    if (empty_string(cmdargstr)) {
	cmd_error(side, "No side given to submit to!");
	return;
    }
    side2 = parse_side_spec(cmdargstr);
    if (side2 == NULL) {
	cmd_error(side, "Can't get a side from \"%s\"!", cmdargstr);
	return;
    }
    if (side2 == side) {
	cmd_error(side, "Can't submit to ourselves!?");
	return;
    }
    net_set_controlled_by(side, side2, TRUE);
    /* (should draw emblem inverted underneath controller's emblem) */
}

/* Take supplies from transport. */

void
do_take(Side *side)
{
    DURING_GAME_ONLY(side);
    if (nummtypes == 0) {
	cmd_error(side, "No materials in this game!");
	return;
    }
    apply_to_all_selected_units(side, do_one_take);
}

static int
do_one_take(Side *side, Unit *unit)
{
    short m, needed;

    if (gt_amts == NULL)
      gt_amts = (short *) xmalloc(nummtypes * sizeof(short));
    if (gt_rslts == NULL)
      gt_rslts = (short *) xmalloc(nummtypes * sizeof(short));

    for_all_material_types(m)
      gt_amts[m] = side->prefixarg;
    needed = take_supplies(unit, gt_amts, gt_rslts);
    report_take(side, unit, needed, gt_rslts);
    return TRUE;
}

void
do_trust(Side *side)
{
    Side *side2;

    DURING_GAME_ONLY(side);
    if (empty_string(cmdargstr)) {
	notify_relationships(side);
	return;
    }
    side2 = parse_side_spec(cmdargstr);
    if (side2 == NULL) {
	cmd_error(side, "No side matching \"%s\"", cmdargstr);
	return;
    }
    if (side2 == side) {
	cmd_error(side, "We're not confused about trusting ourselves!");
	return;
    }
    net_set_trust(side, side2, 1);
}

void
do_wake(Side *side)
{
    DURING_GAME_ONLY(side);
    apply_to_all_selected_units(side, do_one_wake);
}

static int
do_one_wake(Side *side, Unit *unit)
{
    net_wake_unit(side, unit, FALSE);
    /* If an argument was given, apply to all "top-level" units within
       the radius specified by the argument. */
    if (side->prefixarg >= 0)
      net_wake_area(side, unit->x, unit->y, side->prefixarg, FALSE);
    return 1;
}

void
do_wake_all(Side *side)
{
    DURING_GAME_ONLY(side);
    apply_to_all_selected_units(side, do_one_wake_all);
}

static int
do_one_wake_all(Side *side, Unit *unit)
{
    net_wake_unit(side, unit, TRUE);
    /* If an argument was given, apply to all units and occs within
       the radius specified by the argument. */
    if (side->prefixarg >= 0)
      net_wake_area(side, unit->x, unit->y, side->prefixarg, TRUE);
    return 1;
}

void
do_warning_log(Side *side)
{
    warnings_logged = !warnings_logged;
    if (warnings_logged)
      notify_all("Warnings now being logged in \"Xconq.Warnings\".\n");
    else
      notify_all("Warnings will not be logged.\n");
}

#ifdef DESIGNERS

void
do_gdl(Side *side)
{
    /* (should check designer status?) */
    if (!empty_string(cmdargstr))
      interp_form(NULL, read_form_from_string(cmdargstr, NULL, NULL, NULL));
    /* (should broadcast to all?) */
}

#endif /* DESIGNERS */

#ifdef DEBUGGING

/* Debugging-related commands. */

/* General debugging toggles. */

void
do_debug(Side *side)
{
#ifndef Debug
    toggle_debugging(&Debug);
#endif
    notify_all("Debugging: %d", Debug);
    update_everything();
}

void
do_debugg(Side *side)
{
#ifndef DebugG
    toggle_debugging(&DebugG);
#endif
    notify_all("Graphics debugging: %d", DebugG);
    update_everything();
}

void
do_debugm(Side *side)
{
#ifndef DebugM
    toggle_debugging(&DebugM);
#endif
    notify_all("Machine play/AI debugging: %d", DebugM);
    update_everything();
}

#endif /* DEBUGGING */
