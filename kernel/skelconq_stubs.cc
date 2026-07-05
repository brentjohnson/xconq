/* Minimal "interface" callback stubs shared by skelconq and the test/unit
   unittests binary. See skelconq_stubs.h for why this is a separate file.
   This is straight code motion out of skelconq.cc (originally by Stanley T.
   Shebs) plus the tolerate_warnings knob described there; no behavior
   changed for skelconq itself.

   Copyright (C) 1991-1997, 1999-2000 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "conq.h"
#include "kpublic.h"
#include "print.h"
#include "skelconq_stubs.h"

extern Side* dside;

Side *defaultside = NULL;

time_t skelturnstart;

int freerunturns = 0;

int numcellupdatesperturn = 0;

int numusefulcellupdatesperturn = 0;

int linemiddle = FALSE;

int tolerate_warnings = FALSE;

unsigned int warningcount = 0;

void
run_ui_idler(void)
{}

Player *
add_default_player(void)
{
    Player *dflt = add_player();

    dflt->displayname = "stdio";
    Dprintf("Added the default player %s\n", player_desig(dflt));
    return dflt;
}

void
make_default_player_spec(void)
{
    default_player_spec = copy_string("stdio");
}

/* Create a user interface, but leave it turned off. */

void
init_ui(Side *side)
{
    if (side_wants_display(side)) {
        side->ui = (UI *) xmalloc(sizeof(UI));
	/* Display should not become active yet. */
	side->ui->active = FALSE;
	defaultside = side;
	/* Do this so game doesn't run out of control. */
	net_set_autofinish(side, FALSE);
	DGprintf("Created a UI for %s\n", side_desig(side));
    } else {
	side->ui = NULL;
    }
}

/* This tests whether the side has a display and if it is in use. */

int
active_display(Side *side)
{
    return (side && side_has_display(side) && side->ui->active);
}

/* Shut down displays - should be done before any sort of exit. */

void
close_displays(void)
{
    Side *side;

    for_all_sides(side) {
	if (active_display(side)) {
	    side->ui->active = FALSE;
	    printf("Display \"%s\" closed.\n", side->player->displayname);
	}
    }
}

void
cmd_error(Side *side, const char *fmt, ...)
{
    char tmpnbuf[BUFSIZE];
    va_list ap;

    if (!empty_string(fmt)) {

	va_start(ap, fmt);
	vsnprintf(tmpnbuf, sizeof tmpnbuf, fmt, ap);
	va_end(ap);

	low_notify(dside, tmpnbuf);
    }
}

void
low_notify(Side *side, char *str)
{
    printf("To %s: %s\n", side_desig(side), str);
}

void
update_area_display(Side *side)
{
}

extern int suppress_update_cell_display;

/* (should count # of calls to inactive and non-displayed side...) */

void
update_cell_display(Side *side, int x, int y, int flags)
{
    ++numcellupdatesperturn;
    if (active_display(side) && !suppress_update_cell_display) {
	++numusefulcellupdatesperturn;
	DGprintf("Update %s: view of %d,%d 0x%x\n",
		 side_desig(side), x, y, flags);
    }
}

/* This hook updates any display of the current turn/date, and any
   other global info, as needed. */

void
update_turn_display(Side *side, int rightnow)
{
    long secs;
    time_t xxx;

    if (active_display(side)) {
	time(&xxx);
	secs = idifftime(xxx, skelturnstart);
	printf("Update %s: turn %d (%ld seconds since last)",
	       side_desig(side), g_turn(), secs);
	DGprintf("%s", (rightnow ? " (now)" : ""));
	printf("\n");
	printf("%d cell updates, %d useful\n",
	       numcellupdatesperturn, numusefulcellupdatesperturn);
	numcellupdatesperturn = numusefulcellupdatesperturn = 0;
	skelturnstart = xxx;
	--freerunturns;
	if (freerunturns == 0) {
	    Side *side2;

	    for_all_sides(side2) {
	        if (active_display(side2))
		  net_set_autofinish(side2, FALSE);
	    }
	}
	if (rightnow)
	  fflush(stdout);
    }
}

void
update_action_display(Side *side, int rightnow)
{
    if (active_display(side) && DebugG) {
	printf("Update %s: ready to act\n", side_desig(side));
    }
}

void
update_action_result_display(Side *side, Unit *unit, int rslt, int rightnow)
{
    if (active_display(side) && DebugG) {
	printf("Update %s: %s action result is %s\n",
	       side_desig(side), unit_desig(unit), hevtdefns[rslt].name);
    }
}

/* This is for animation of fire-at actions. */

void
update_fire_at_display(Side *side, Unit *unit, Unit *unit2, int m, int rightnow)
{
    if (active_display(side) && DebugG) {
	printf("Update %s: %s fire at %s\n",
	       side_desig(side), unit_desig(unit), unit_desig(unit2));
    }
}

/* This is for animation of fire-into actions. */

void
update_fire_into_display(Side *side, Unit *unit, int x, int y, int z, int m,
			 int rightnow)
{
    if (active_display(side) && DebugG) {
	printf("Update %s: %s fire into %d,%d\n",
	       side_desig(side), unit_desig(unit), x, y);
    }
}

void
update_event_display(Side *side, HistEvent *hevt, int rightnow)
{
    if (active_display(side)) {
	switch (hevt->type) {
	  case H_SIDE_LOST:
	    printf("%s lost!\n", side_desig(side_n(hevt->data[0])));
	    break;
	  case H_SIDE_WON:
	    printf("%s won!\n",  side_desig(side_n(hevt->data[0])));
	    break;
	  default:
	    DGprintf("Update %s: event %s %d\n",
		     side_desig(side), hevtdefns[hevt->type].name,
		     hevt->data[0]);
	}
    }
}

void
update_all_progress_displays(const char *str, int s)
{
    if (DebugG) {
	printf("Update all progress displays\n");
    }
}

/* This hook should update the side's view of the given side, no matter
   who it belongs to. */

void
update_side_display(Side *side, Side *side2, int rightnow)
{
    char *side2desc = copy_string(side_desig(side2));

    if (active_display(side) && DebugG) {
	printf("Update %s: side %s%s\n",
	       side_desig(side), side2desc, (rightnow ? " (now)" : ""));
    }
}

void
update_research_display(Side *side)
{
    /* There is no display side here; pick research for the side the
       kernel is asking about. */
    if (numatypes > 0
	&& side != NULL
	&& g_side_can_research()
	&& side->research_topic == NOADVANCE) {
	/* Should eventually pop up a side research dialog instead. */
	auto_pick_side_research(side);
    }
}

/* This hook should update the side's view of the given unit, no matter
   who it belongs to. */

void
update_unit_display(Side *side, Unit *unit, int rightnow)
{
    if (active_display(side) && DebugG) {
	printf("Update %s: unit %s%s\n",
	       side_desig(side), unit_desig(unit), (rightnow ? " (now)" : ""));
    }
}

void
update_unit_acp_display(Side *side, Unit *unit, int rightnow)
{
    if (active_display(side) && DebugG) {
	printf("Update %s: unit %s acp%s\n",
	       side_desig(side), unit_desig(unit), (rightnow ? " (now)" : ""));
    }
}

/* This hook updates any realtime clock displays.  If the game does not
   have any realtime constraints, this will never be called. */

void
update_clock_display(Side *side, int rightnow)
{
    if (active_display(side) && DebugG) {
	printf("Update %s: %d secs this turn, %d total\n",
	       side_desig(side), side->turntimeused, side->totaltimeused);
	/* also display total game clock */
    }
}

void
update_message_display(Side *side, Side *sender, const char *str, int rightnow)
{
    if (active_display(side) && DebugG) {
	printf("Update %s: side %d sends \"%s\"%s\n",
	       side_desig(side), side_number(sender), str, (rightnow ? " (now)" : ""));
    }
}

void
update_everything(void)
{
    printf("Update everything!\n");
}

void
action_point(Side *side, int x, int y)
{
}

/* This reports progress in reading GDL files. */

void
announce_read_progress(void)
{
}

/* This is used for initialization steps that take a long time. */

void
announce_lengthy_process(const char *msg)
{
    printf("%s; ", msg);
    fflush(stdout);
    linemiddle = TRUE;
}

/* Update the progress, expressing it as a percentage done. */

void
announce_progress(int percentdone)
{
    printf(" %d%%", percentdone);
    fflush(stdout);
    linemiddle = TRUE;
}

/* Announce the end of the lengthy process. */

void
finish_lengthy_process(void)
{
    printf(" done.\n");
    linemiddle = FALSE;
}

int
schedule_movie(Side *side, const char *movie, ...)
{
    return FALSE;
}

void
play_movies(SideMask sidemask)
{
}

void
flush_display_buffers(Side *side)
{
    if (active_display(side) && DebugG) {
	printf("To %s: flush display buffers\n", side_desig(side));
    }
}

/* An init error needs to have the command re-run. */

void
low_init_error(const char *str)
{
    if (linemiddle)
      printf("\n");
    fprintf(stderr, "Error: %s.\n", str);
    fflush(stderr);
}

static void
report_warning(void)
{
    /* Ideally this would get passed back to the test framework which could
       record it as FAIL or ERROR, but for now just make sure it doesn't
       score as PASS. */
    ++warningcount;
    if (!tolerate_warnings)
      exit(EXIT_FAILURE);
}

/* A warning just gets displayed, no other action is taken. */

void
low_init_warning(char *str)
{
    if (linemiddle)
      printf("\n");
    fprintf(stdout, "Warning: %s.\n", str);
    fflush(stdout);

    report_warning();
}

/* A run error is fatal. */

void
low_run_error(char *str)
{
    if (linemiddle)
      fprintf(stderr, "\n");
    fprintf(stderr, "Error: %s.\n", str);
    fflush(stderr);
    exit(1);
}

/* Runtime warnings are for when it's important to bug the players,
   usually a problem with Xconq or a game design. */

void
low_run_warning(char *str)
{
    if (linemiddle)
      printf("\n");
    fprintf(stdout, "Warning: %s.\n", str);
    fflush(stdout);
    report_warning();
}

void
print_form(Obj *form)
{
    print_form_and_value(stdout, form);
}

void
end_printing_forms(void)
{
}

/* Note that the fake declaration below will cause problems if imf.h
   is included. */

#if (0)
void make_generic_image_data(char *imf);
void
make_generic_image_data(char *imf)
{}
#else
#include "imf.h"
void make_generic_image_data(ImageFamily *imf);
void
make_generic_image_data(ImageFamily *imf)
{}
#endif

void
unit_research_dialog(Unit *unit)
{
    auto_pick_unit_research(unit);
}

void
add_remote_locally(int rid, const char *str)
{
}

void
send_chat(int rid, char *str)
{
    printf("From %d: received chat \"%s\"\n", rid, str);
}

/* Dummy function needed in run_turn_start. */

void
place_legends(Side *side)
{
}
