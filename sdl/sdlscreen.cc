/* Screen display code for the SDL interface to Xconq.
   Copyright (C) 2000, 2001 Stanley T. Shebs.
   Copyright (C) 2004 Eric A. McDonald.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/* The code in this file manages screens, which are single windows
   that include a map and possibly some panels.  The typical interface
   consists of just one screen. */

#include "sdlpreconq.h"
#include "conq.h"
#include "sdlconq.h"

/* (Hack. Access the 'at_turn_start' variable in 'run.c'.) */
extern int at_turn_start;

extern void set_panel_sizes(Screen *screen);
extern void update_unit_info(Screen *screen);
extern /*static*/ void xform(Map *map, int x, int y, int *sxp, int *syp);
extern void update_mouseover(Screen *screen);

extern Panel *create_panel(Screen *screen, int paneltype, int w, int h,
			   Panel *outer);

extern SDLButton *create_button(Panel *panel, int x, int y, int w, int h);
extern void draw_button(Screen *screen, Panel *panel, SDLButton *button);

static void set_panel_position(Panel *panel, int x, int y);
static void draw_mouseover_panel(Screen *screen, Panel *panel);
static void draw_unit_info(Screen *screen, Panel *panel);
static void draw_side_research_info(Screen *screen, Panel *panel);
static void draw_bottom_panel(Screen *screen, Panel *panel);
static void draw_world_map(Screen *screen, Panel *panel);

static int calculate_minimap_view_power(Panel *panel);
static int calculate_minimap_width(Panel *panel);
static int calculate_minimap_height(Panel *panel);
static int calculate_minimap_sx(Panel *panel);
static int calculate_minimap_sy(Panel *panel);

static int calculate_unit_action_panel_width(Panel *panel);
static int calculate_unit_action_panel_height(Panel *panel);

int G_uact_button_offset = 0;
static void hide_unit_action_panel_fn(SDLButton *button);
static void prev_uact_button_fn(SDLButton *button);
static void next_uact_button_fn(SDLButton *button);

/* Reset the screen input fields. */

void
reset_screen_input(Screen *screen)
{
    assert_error(screen, "Attempted to manipulate a NULL screen");
    screen->prefixarg = -1;
    screen->inptype = -1;
    screen->inpch = 0;
}

/* Calculate the optimal view power of the minimap. */

static int
calculate_minimap_view_power(Panel *panel)
{
    int viewpow = 0;

    assert_error(panel, "Attempted to access a NULL panel");
    for (viewpow = NUMPOWERS - 1; viewpow > 0; --viewpow) {
	if (((area.width * hws[viewpow]) <= MINIMAP_WIDTH_DEFAULT)
	    && ((area.height * hcs[viewpow]) < (panel->h - 20)))
	  return viewpow;
    }
    return viewpow;
}

/* Calculate the viewport width of the minimap. */

static int
calculate_minimap_width(Panel *panel)
{
    assert_error(panel, "Attempted to access a NULL panel");
    return min(area.width * hws[calculate_minimap_view_power(panel)],
	       MINIMAP_WIDTH_DEFAULT);
}

/* Calculate the viewport height of the minimap. */

static int
calculate_minimap_height(Panel *panel)
{
    assert_error(panel, "Attempted to access a NULL panel");
    return min(area.height * hcs[calculate_minimap_view_power(panel)], 
	       panel->h - (2 * MINIMAP_BORDER_SIZE));
}

/* Calculate the screen X position of the left edge of the minimap. */

static int
calculate_minimap_sx(Panel *panel)
{
    assert_error(panel, "Attempted to access a NULL panel");
    return panel->x + panel->w - (2 * MINIMAP_BORDER_SIZE) - 
	   MINIMAP_WIDTH_DEFAULT + 
	   (MINIMAP_WIDTH_DEFAULT - sscreen->minimap->main_vp->pxw) / 2;
#if (0)
    return panel->x + panel->w - 16 - 200 + 
	   (200 - sscreen->minimap->main_vp->pxw) / 2;
#endif
}

/* Calculate the screen Y position of the top edge of the minimap. */

static int
calculate_minimap_sy(Panel *panel)
{
    assert_error(panel, "Attempted to access a NULL panel");
    return panel->y + MINIMAP_BORDER_SIZE + 
	   (panel->h - calculate_minimap_height(panel)) / 2;
#if (0)
    return panel->y + 10 + (panel->h - 20 - sscreen->minimap->main_vp->pxh) / 2;
#endif
}

/* Calculate the available width for laying out unit action buttons. */

static int
calculate_unit_action_panel_width(Panel *panel)
{
    assert_error(panel, "Attempted to access a NULL panel");
    switch (panel->type) {
      case bottom_panel:
	return sscreen->minimap->sx - panel->x;
      default: break;
    }
    return 0;
}

/* Calculate the available height for laying out unit action buttons. */

static int
calculate_unit_action_panel_height(Panel *panel)
{
    assert_error(panel, "Attempted to access a NULL panel");
    switch (panel->type) {
      case bottom_panel:
	return 56;
      default: break;
    }
    return 0;
}

/* Alpha blend a filled rectangle into a surface. */

int
alpha_blend_fill_rect(SDL_Surface *surf, SDL_Rect *rect, 
		      Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    SDL_Surface *asurf = NULL;
    Uint32 rmask = 0, gmask = 0, bmask = 0, amask = 0;
    SDL_Rect rect2;

    /* Setup the masks for the temp surface. */
#if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif
    /* Create the temp surface. */
    asurf = SDL_CreateRGBSurface(SDL_SWSURFACE, rect->w, rect->h, 32,
				 rmask, gmask, bmask, amask);
    if (!asurf) {
	run_warning("Failed to allocate an alpha blending surface");
	return FALSE;
    }
    /* Turn on alpha blending on the temp surface. */
    SDL_SetAlpha(asurf, SDL_SRCALPHA | SDL_RLEACCEL, SDL_ALPHA_OPAQUE);
    /* Dump the fill rect (including alpha data) into the temp surface.
       (Note that alpha belnding is not performed with 'SDL_FillRect'.) */
    rect2.x = 0;  rect2.y = 0;  rect2.w = rect->w;  rect2.h = rect->h;
    SDL_FillRect(asurf, &rect2, SDL_MapRGBA(asurf->format, r, g, b, a));
    /* Blit the temp surface into the main surface. */
    SDL_BlitSurface(asurf, NULL, surf, rect);
    /* Free up the temp surface. */
    SDL_FreeSurface(asurf);
    return TRUE;
}

/* Initialize the screen. */

void
open_screen(void)
{
    int x, y;
    Screen *screen;
    Panel *mouseover;

    /* Note that SDL doesn't presently support multiple windows, but
       it might in the future, so we make screen objects. */
    screen = (Screen *) xmalloc(sizeof(Screen));
    ui->screens = screen;
    ui->curscreen = screen;
    /* Also assign it to a global. */
    sscreen = screen;
    /* Allocate our composition buffer. */
    screen->surf = SDL_AllocSurface(SDL_HWSURFACE, mscreen->w, mscreen->h, 32,
				    0x00ff0000, 0x0000ff00, 0x000000ff, 0);
    /* Set up a particular panel layout, at least for now. */
    screen->panels = create_panel(screen, bottom_panel,
				  screen->surf->w, 130, NULL);
    screen->panels->label = "Bottom";
    mouseover = create_panel(screen, mouseover_panel,
			     screen->surf->w, 20, screen->panels);
#if 0 /* works, but hard to read */
    mouseover->overlay = TRUE;
#endif
    mouseover->label = "Mouseover";
    /* Set up the screen's main map. */
    screen->map = create_map(FALSE);
    set_panel_sizes(screen);
    set_view_power(screen->map->main_vp, 5);
    pick_a_focus(dside, &x, &y);
    set_view_focus(screen->map->main_vp, x, y);
    center_on_focus(screen->map->main_vp);
    /* Link the map back to the screen. */
    screen->map->screen = screen;
    /* Initialize various screen properties. */
    screen->mode = move_mode;
    screen->autoselect = TRUE;
    screen->tmp_mode = no_tmp_mode;
    screen->scroll_mode[0] = no_scroll_mode;
    screen->prefixarg = -1;
    screen->inptype = NONUTYPE;
    screen->uvec = (short *) xmalloc(numutypes * sizeof(short));
    screen->ustr = (char *) xmalloc((numutypes + 1) * sizeof(char));
    screen->tvec = (short *) xmalloc(numttypes * sizeof(short));
    screen->tstr = (char *) xmalloc((numttypes + 1) * sizeof(char));
    screen->mouseover_panel_locked = FALSE;
    screen->unit_action_panel_visible = FALSE;
    screen->side_research_panel_visible = FALSE;
    screen->active = TRUE;
    redraw_screen(screen);
}

Panel *
create_panel(Screen *screen, int paneltype, int w, int h, Panel *outer)
{
    Panel *panel;
    Map *mini;
    int miniwidth = 0, miniheight = 0;

    panel = (Panel *) xmalloc(sizeof(Panel));
    panel->type = paneltype;
    panel->w = panel->full_w = w;  panel->h = panel->full_h = h;
    panel->out = outer;
    if (outer != NULL)
      outer->in = panel;
    panel->screen = screen;
    /* Now do things specific to particular panel types. */
    switch (panel->type) {
      case bottom_panel:
	/* Create world map. */
	if (screen->minimap == NULL) {
	    mini = create_map(TRUE);
	    /* Link the map back to the screen. */
	    mini->screen = screen;
	    set_view_power(mini->main_vp, calculate_minimap_view_power(panel));
	    miniwidth = calculate_minimap_width(panel);
	    miniheight = calculate_minimap_height(panel);
	    set_view_size(mini->main_vp, miniwidth, miniheight);
	    mini->main_vp->draw_names = FALSE;
	    mini->main_vp->draw_feature_names = FALSE;
	    /* Kind of a hack... */
	    screen->minimap = mini;
	}
	/* (TODO: Create scrolling text region.) */
      default: break;
    } /* switch panel type */
    /* Setup any visible buttons. */
    reset_panel_buttons(panel);
    /* Return pointer to created panel. */
    return panel;
}

/* Set the buttons for a panel. */
/* (TODO: Lock display during this operation so that we don't fire off 
    any event handlers while the state of a panel is in flux.) */

void
reset_panel_buttons(Panel *panel)
{
    int panelw = 0, panelh = 0, numrows = 0, numcols = 0, numbuttons = 0;
    int i = 0, j = 0;

    assert_error(panel, "Attempted to manipulate a NULL panel");
    /* Clear out the old buttons, if any. */
    for (i = 0; i < panel->numbuttons; ++i) {
	free(panel->buttons[i]);
	panel->buttons[i] = NULL;
    }
    panel->numbuttons = 0;
    /* Set the new buttons. */
    switch (panel->type) {
      case mouseover_panel: break;
      case bottom_panel:
	panelw = calculate_unit_action_panel_width(panel);
	panelh = calculate_unit_action_panel_height(panel);
	numcols = (panelw - (2 * UNIT_ACTION_PANEL_BORDER_SIZE)) / 52;
	numrows = panelh / 52;
	numbuttons = numrows * numcols;
	if ((numbuttons < 4) 
	    || !(panel->screen->unit_action_panel_visible
		 || panel->screen->side_research_panel_visible)){
	    panel->numbuttons = 0;
	    break;
	}
	/* Create a collection of buttons that will be used for
	   various unit tasks. */
	/* The row of buttons competes with the minimap for the panel width. 
	   An offset of 8 pixels is considered from the left margin. */
	for (i = 0; i < numcols; ++i) {
	    for (j = 0; j < numrows; ++j)
	      create_button(panel, 8 + (i * 52), (panel->h - 56) + (j * 52), 
			    48, 48);
	}
	panel->numbuttons = numbuttons;
	break;
      default: break;
    }
}

void
set_panel_sizes(Screen *screen)
{
    int cav_x, cav_y, cav_w, cav_h;
    int map_cav_x, map_cav_y, map_cav_w, map_cav_h;
    Panel* panel;

    /* Start with a cavity the size of the whole screen, and work
       inwards. */
    cav_x = cav_y = 0;
    cav_w = screen->surf->w;
    cav_h = screen->surf->h;
    map_cav_x = cav_x;  map_cav_y = cav_y;
    map_cav_w = cav_w;  map_cav_h = cav_h;
    for (panel = screen->panels; panel != NULL; panel = panel->in) {
	switch (panel->type) {
	  case mouseover_panel:
	    panel->w = cav_w;
	    /* Leave panel height fixed. */
	    break;
	  case bottom_panel:
	    panel->w = cav_w;
	    /* Leave panel height fixed. */
	    break;
	  default: break;
	}
	G_uact_button_offset = 0;
	set_panel_position(panel, cav_x, cav_y + cav_h - panel->h);
	cav_x = cav_x;  cav_y = cav_y;
	cav_w = cav_w;  cav_h -= panel->h;
	if (!panel->overlay) {
	    map_cav_x = cav_x;  map_cav_y = cav_y;
	    map_cav_w = cav_w;  map_cav_h = cav_h;
	}
    }
    /* Give the remaining space to the main map. */
    set_view_size(screen->map->main_vp, map_cav_w, map_cav_h);
}

/* Set the panel to be at the given x,y, and possibly update the
   location of things within the panel. */

static void
set_panel_position(Panel *panel, int x, int y)
{
    panel->x = x;  panel->y = y;
    switch (panel->type) {
      case bottom_panel:
#if (0)
	sscreen->minimap->sx = panel->x + panel->w - 16 - 200;
	sscreen->minimap->sx += (200 - sscreen->minimap->main_vp->pxw) / 2;
	sscreen->minimap->sy = panel->y + 10;
	sscreen->minimap->sy += 
	    (panel->h - 20 - sscreen->minimap->main_vp->pxh) / 2;
#endif
	sscreen->minimap->sx = calculate_minimap_sx(panel);
	sscreen->minimap->sy = calculate_minimap_sy(panel);
	break;
      default: break;
    }
    reset_panel_buttons(panel);
}

int last_redraw;

void
redraw_screen(Screen *screen)
{
    Panel *panel;

    /* Get the mouseover buffer updated. */
    update_mouseover(screen);
    /* Don't bother with any previously-scheduled updates. */
    screen->num_updates = 0;
    /* Update the whole map display. */
    add_update(screen, 0, 0,
	       screen->map->main_vp->pxw, screen->map->main_vp->pxh);
    /* Update anything associated with unit info. */
    update_unit_info(screen);
    /* Update all of the panels. */
    for (panel = screen->panels; panel != NULL; panel = panel->in) {
	add_update(screen, panel->x, panel->y, panel->w, panel->h);
    }
    update_screen(screen, FALSE);
}

void
add_update(Screen *screen, int sx, int sy, int sw, int sh)
{
    /* If the rect is entirely outside the screen, we can skip it. */
    if (sx > mscreen->w || sy > mscreen->h)
      return;
    /* If we've accumulated too many updates, dump them all out. */
    if (screen->num_updates >= 1000)
      update_screen(screen, TRUE);
    /* It is important to clip the update rects to the screen since
       there is a bug in SDL which will cause UpdateRects to crash if
       an update rect extends outside the screen. */
    sx = max(0, sx);  sy = max(0, sy);
    sw = min(mscreen->w - sx, sw);  sh = min(mscreen->h - sy, sh);
    screen->updates[screen->num_updates].x = sx;
    screen->updates[screen->num_updates].y = sy;
    screen->updates[screen->num_updates].w = sw;
    screen->updates[screen->num_updates].h = sh;
    ++(screen->num_updates);
}

void
update_screen(Screen *screen, int rightnow)
{
    int i, now;
    Panel *panel;
    Uint32 color;
    SDL_Rect rect, rect2, dummyrect;

    /* Don't try to redraw too many times in one second. */
    now = SDL_GetTicks();
    if (!rightnow && (now - last_redraw) < 20)
      return;
    for (i = 0; i < screen->num_updates; ++i) {
	rect = screen->updates[i];
	color = SDL_MapRGB(screen->surf->format, 0, 0, 0);
	SDL_FillRect(screen->surf, &rect, color);
	memcpy(screen->map->vp, screen->map->main_vp, sizeof(VP));
	/* Shrink the viewport down to the update region only. */
	set_view_size(screen->map->vp, 
		      screen->updates[i].w, screen->updates[i].h);
	set_view_position(screen->map->vp,
			  screen->map->vp->sx + screen->updates[i].x,
			  screen->map->vp->sy + screen->updates[i].y);
	draw_map(screen->map);
	/* If a panel coincides with an update rect, 
	   then update entire panel. */ 
	for (panel = screen->panels; panel != NULL; panel = panel->in) {
	    rect2.x = panel->x;  rect2.y = panel->y;
	    rect2.w = panel->w;  rect2.h = panel->h;
	    /* (TODO: Make this more efficient.) */
	    if (SDLIntersectRect(&rect, &rect2, &dummyrect)) 
	      draw_panel(screen, panel);
	}
#if 0
	if (screen->cursor && use_cursors) {
	    rect.x = screen->cursorx;  rect.y = screen->cursory;
	    rect.w = screen->cursor->surf->w;  rect.h = screen->cursor->surf->h;
	    SDL_BlitSurface(screen->cursor->surf, NULL, screen->surf, &rect);
	}
#endif
	rect = screen->updates[i];
#if (0) /* Enable to see redraws. */
	draw_rect(screen->surf, rect.x, rect.y, rect.w, rect.h,
		  random_color(screen->surf));
#endif
	SDL_BlitSurface(screen->surf, &rect, mscreen, &rect);
    } /* For all update rectangles. */
    /* Draw cursor onto display buffer after everything else. */
    if (screen->cursor && use_cursors) {
	rect.x = screen->cursorx - screen->cursor->hotx; 
	rect.y = screen->cursory - screen->cursor->hoty;
	rect.w = screen->cursor->surf->w;  rect.h = screen->cursor->surf->h;
	SDL_BlitSurface(screen->cursor->surf, NULL, mscreen, &rect);
    }
    /* NOTE: Do we need to add cursor to list of update rects? */
    /* SDL_Flip only works if SDL_DOUBLEBUF is set. If not, it is
       better to call SDL_UpdateRects for the update rects instead,
       since SDL_Flip will force a very slow update of the entire
       screen. */
    if (mscreen->flags & SDL_DOUBLEBUF) {
	SDL_Flip(mscreen);
    } else {
	SDL_UpdateRects(mscreen, screen->num_updates, 
		        (SDL_Rect *) screen->updates); 
    }
    last_redraw = SDL_GetTicks();
    /* All updates are done, so clear the count. */
    screen->num_updates = 0;
}

void
draw_panel(Screen *screen, Panel *panel)
{
    int i;
    Uint32 color;
    SDL_Rect rect;

    if (panel->h > 10) {
	if (!panel->overlay) {
	    color = SDL_MapRGB(screen->surf->format, 0, 40, 100);
	    rect.x = panel->x;  rect.y = panel->y;
	    rect.w = panel->w;  rect.h = panel->h;
	    SDL_FillRect(screen->surf, &rect, color);
	}
	color = SDL_MapRGB(screen->surf->format, 100, 140, 200);
#if (0) /* to see redraws */
	color = random_color(screen->surf);
#endif
	draw_rect(screen->surf, rect.x, rect.y, rect.w, rect.h, color);
	switch (panel->type) {
	  case mouseover_panel:
	    draw_mouseover_panel(screen, panel);
	    break;
	  case bottom_panel:
	    draw_bottom_panel(screen, panel);
	    break;
	}
	for (i = 0; i < panel->numbuttons; ++i) {
	    draw_button(screen, panel, panel->buttons[i]);
	}
    }
    /* Always draw the expand/contract button. */
    color = SDL_MapRGB(screen->surf->format, 200, 100, 100);
    rect.x = panel->x + panel->w - 10;  rect.y = panel->y + panel->h - 10;
    rect.w = 9;  rect.h = 9;
    SDL_FillRect(screen->surf, &rect, color);
}

static void
draw_mouseover_panel(Screen *screen, Panel *panel)
{
    extern char *mouseover_buf;

    /* Make sure there is something to draw. */
    screen_oneliner(screen);
    draw_string(screen->surf, panel->x + 5, panel->y + 5, mouseover_buf);
}

static void
draw_bottom_panel(Screen *screen, Panel *panel)
{
    if (!(screen->modalhandler))
      draw_unit_info(screen, panel);
    if (screen->side_research_panel_visible)
      draw_side_research_info(screen, panel);
    draw_world_map(screen, panel);
}

/* Draw side research info in appropriate panel. */

static void
draw_side_research_info(Screen *screen, Panel *panel)
{
    char infobuf[BUFSIZE];
    int topicscount = 0, a = NONATYPE;
    SDL_Surface *surf;
    int sx0 = 0, sy0 = 0;
    int margx = BOTTOM_PANEL_X_MARGIN, margy = BOTTOM_PANEL_Y_MARGIN;

    surf = screen->surf;
    sx0 = panel->x;  sy0 = panel->y;
    for_all_advance_types(a) {
	if (side_can_research(dside, a))
	  ++topicscount;
    }
    snprintf(infobuf, BUFSIZE, 
	     "Your side has %d topics available to research.", topicscount);
    draw_string(surf, sx0 + margx, sy0 + margy, infobuf);
}

/* Draw unit info in appropriate panel. */

static void
draw_unit_info(Screen *screen, Panel *panel)
{
    char infobuf[BUFSIZE];
    int u, mrow, x = -1, y = -1, sx0, sy0;
    int margx = BOTTOM_PANEL_X_MARGIN, margy = BOTTOM_PANEL_Y_MARGIN;
    int lineh = 16, col2 = 200;
    Unit *unit = screen->curunit;
    Task *task;
    ImageFamily *uimf, *eimf;
    SDL_Surface *surf;

    if (!in_play(unit))
      return;
    /* (should do this as a separate "subpanel") */
    surf = screen->surf;
    sx0 = panel->x;  sy0 = panel->y;
    /* Update the image displayed. */
    uimf = unit->imf;
    eimf = ui->eimages[unit->side->id];
#if 0
    eval_tcl_cmd("update_unit_picture %d \"%s\" \"%s\"",
		 screenn,
		 (uimf ? uimf->name : "(no)"),
		 (eimf ? eimf->name : "(no)"));
#endif
    /* Say which unit this is. */
    strcpy(infobuf, unit_handle(dside, unit));
    capitalize(infobuf);
    draw_string(surf, sx0 + margx, sy0 + margy, infobuf);
    u = unit->type;
    /* Say which unit this is. */
    strcpy(infobuf, unit_handle(dside, unit));
    capitalize(infobuf);
    x = unit->x;  y = unit->y;
    location_desc(infobuf, dside, unit, u, x, y);
    draw_string(surf, sx0 + margx, sy0 + margy + 1 * lineh, infobuf);
    /* Very briefly list the numbers and types of the occupants. */
    occupants_desc(infobuf, unit);
    draw_string(surf, sx0 + margx, sy0 + margy + 2 * lineh, infobuf);
    /* Display the "important" parameters. */
    infobuf[0] = '\0';
    /* Size is most important, but only applies to advanced units. */
    if (u_advanced(u)) {
	size_desc(tmpbuf, unit, TRUE);
	strcat(infobuf, tmpbuf);
	strcat(infobuf, "   ");
    }
    /* (should say something about parts?) */
    hp_desc(tmpbuf, unit, TRUE);
    strcat(infobuf, tmpbuf);
    strcat(infobuf, "   ");
    acp_desc(tmpbuf, unit, TRUE);
    strcat(infobuf, tmpbuf);
    cxp_desc(tmpbuf, unit, TRUE);
    strcat(infobuf, tmpbuf);
    morale_desc(tmpbuf, unit, TRUE);
    strcat(infobuf, tmpbuf);
    point_value_desc(tmpbuf, unit, TRUE);
    strcat(infobuf, tmpbuf);
    /* Crude hack, should be replaced with something better */
    if (supply_system_in_use()) 
      sprintf(&infobuf[strlen(infobuf)],
	      "   Supply in flow=%d%% connectedness=%d%%",
	      supply_inflow(unit), supply_connectedness(unit));
    draw_string(surf, sx0 + margx + col2, sy0 + margy, infobuf);
    /* List other stack members here. */
    others_here_desc(infobuf, unit);
    draw_string(surf, sx0 + margx + col2, sy0 + margy + 1 * lineh, infobuf);
    /* Describe the state of all the supplies. */
    for (mrow = 0; mrow < 2; ++mrow) {
	supply_desc(infobuf, unit, mrow);
	draw_string(surf, sx0 + margx + col2, sy0 + margy + (2 + mrow) * lineh, infobuf);
    }
    /* Share the last line between additional supply and tooling. */
    if (!supply_desc(infobuf, unit, 2))
      tooling_desc(infobuf, unit);
    draw_string(surf, sx0 + margx + col2, sy0 + margy + 4 * lineh, infobuf);
    /* Describe the current plan and task agenda. */
    plan_desc(infobuf, unit);
    draw_string(surf, sx0 + margx, sy0 + margy + 3 * lineh, infobuf);
    infobuf[0] = '\0';
    task = NULL;
    if (unit->plan)
      task = unit->plan->tasks;
    if (task) {
	task_desc(infobuf, unit->side, unit, task);
	task = task->next;
	if (task) {
	    strcat(infobuf, "; then ");
	    task_desc(infobuf+strlen(infobuf), unit->side, unit, task);
	}
    }
    draw_string(surf, sx0 + margx, sy0 + margy + 4 * lineh, infobuf);
}

/* (NOTE: Does this have to be 10000 for some of the more "prolific" 
    game developers? :-) */
struct a_protobutton G_protobuttons[1000];

int G_num_protobuttons;

/* Hide unit action button panel. */

static void
hide_unit_action_panel_fn(SDLButton *button)
{
    assert_error(button, "Attempted to access a NULL button");
    sscreen->unit_action_panel_visible = FALSE;
    update_mouseover(sscreen);
    update_unit_info(sscreen);
    update_screen(sscreen, TRUE);
}

/* Load previous set of unit action buttons. */

static void
prev_uact_button_fn(SDLButton *button)
{
    Panel *panel = NULL;

    assert_error(button, "Attempted to access a NULL button");
    panel = button->panel;
    assert_error(button->panel, "Attempted to access a NULL panel");
    G_uact_button_offset -= (button->panel->numbuttons - 2);
    update_mouseover(sscreen);
    update_unit_info(sscreen);
    update_screen(sscreen, TRUE);
}

/* Load next set of unit action buttons. */

static void
next_uact_button_fn(SDLButton *button)
{
    Panel *panel = NULL;

    assert_error(button, "Attempted to access a NULL button");
    panel = button->panel;
    assert_error(panel, "Attempted to access a NULL panel");
    G_uact_button_offset += (panel->numbuttons - 2);
    update_mouseover(sscreen);
    update_unit_info(sscreen);
    update_screen(sscreen, TRUE);
}

/* Update the unit info screen. This include the drawing of any buttons 
   related to the current unit's function. */

void
update_unit_info(Screen *screen)
{
    int i, j, numbuttons = 0, uvecsz = 0;
    Uint32 color;
    Panel *panel;
    SDLButton *button;
    struct a_protobutton *protobutton;
    Unit *unit = NULL;
    Side *side = NULL;
    Unit **units = NULL;

    /* Return if the unit info panel is not visible. */
    if (!(screen->unit_action_panel_visible)) 
      return;
    /* Look for panels that display unit info. */
    for (panel = screen->panels; panel; panel = panel->in) {
	switch (panel->type) {
	  case bottom_panel:
	    /* Change the state of all the buttons. */
	    /* Reset all buttons. */
	    reset_panel_buttons(panel);
	    /* First calculate the set of possible buttons. */
	    G_num_protobuttons = 0;
	    unit = screen->curunit;
	    /* Don't try anything if the unit is not active. */
	    if (!is_active(unit)) {
		add_update(screen, panel->x, panel->y, panel->w, panel->h);
		break;
	    }
	    /* (TODO: Handle vector of selected units. If the selected units 
		vector is empty, then use the current unit as is done now.) */
	    units = &unit;
	    uvecsz = 1;
	    if (unit) {
		side = unit->side;
		add_delay_button(screen, units, uvecsz);
		/* (TODO: Implement Undelay button. Maybe....) */
		add_skip_button(screen, units, uvecsz);
		add_sleep_button(screen, units, uvecsz);
		add_wake_button(screen, units, uvecsz);
		/* (TODO: Implement Cancel Plan button, if in 
		    survey mode.) */
		/* (TODO: Implement Disembark button.) */
		/* (TODO: Implement Formation button.) */
		/* (TODO: Implement Leave Formation button.) */
		/* (TODO: Implement Take Materials From button.) */
		/* (TODO: List materials that can be collected.) */
		/* (TODO: Implement Detonate button.) */
		/* (TODO: Implement Alter Terrain button.) */
		/* (TODO: List any stagnant construction that can be 
		    resumed.) */
		add_construct_buttons(screen, units, uvecsz);
		add_research_buttons(screen, units, uvecsz);
		add_develop_buttons(screen, units, uvecsz);
		add_change_type_buttons(screen, units, uvecsz);
		/* (TODO: Implement Give Materials To button.) */
		/* (TODO: List any manual repairs that can be made.) */
		/* (TODO: List any construction that can be helped.) */
		/* (TODO: Implement Disband button.) */
		/* (TODO: Implement Give Unit button.) */
	    }
	    /* Retain the button offset unless it's too big. */
	    if (G_num_protobuttons <= G_uact_button_offset)
	      G_uact_button_offset = 0;
	    if (G_num_protobuttons && panel->numbuttons) 
	      numbuttons = min(G_num_protobuttons, panel->numbuttons - 1);
	    else
	      numbuttons = 0;
	    /* Assign protobuttons to buttons, possibly with scrolling
	       buttons at either end. */
	    j = G_uact_button_offset;
	    for (i = 0; i < numbuttons; ++i) {
		button = panel->buttons[i];
		button->visible = FALSE;
		if ((i == 0) && (G_uact_button_offset > 0)) {
		    button->visible = TRUE;
		    button->draw_bg = TRUE;
		    color = SDL_MapRGB(screen->surf->format, 120, 150, 150);
		    button->bg = color;
		    button->label = "Prev";
		    button->picture = NULL;
		    button->pic_surface = NULL;
		    button->click_fn = prev_uact_button_fn;
		} else if ((i == numbuttons - 1)
			   && ((G_num_protobuttons - G_uact_button_offset) > 
				numbuttons)) {
		    button->visible = TRUE;
		    button->draw_bg = TRUE;
		    color = SDL_MapRGB(screen->surf->format, 120, 150, 150);
		    button->bg = color;
		    button->label = "Next";
		    button->picture = NULL;
		    button->pic_surface = NULL;
		    button->click_fn = next_uact_button_fn;
		} else if (j < G_num_protobuttons) {
		    protobutton = &(G_protobuttons[j]);
		    button->visible = TRUE;
		    button->data = protobutton->data;
		    button->draw_bg = TRUE;
		    color = SDL_MapRGB(screen->surf->format, 120, 150, 200);
		    button->bg = color;
		    button->label = protobutton->label;
		    button->picture = protobutton->picture;
		    button->pic_surface = NULL;
		    button->help = protobutton->help;
		    button->click_fn = protobutton->click_fn;
		    ++j;
		}
	    } /* for all buttons */
	    /* Append a "Last" button to the end of the visible buttons list. */
	    if (numbuttons) {
		if ((G_num_protobuttons - G_uact_button_offset + 2) 
		    < numbuttons)
		  button = panel->buttons[G_num_protobuttons - 
					  G_uact_button_offset + 1];
		else
		  button = panel->buttons[numbuttons];
		button->visible = TRUE;
		button->draw_bg = TRUE;
		color = SDL_MapRGB(screen->surf->format, 120, 150, 150);
		button->bg = color;
		button->picture = NULL;
		button->pic_surface = NULL;
		button->label = "Hide";
		button->help = "Hide Unit Actions Panel";
		button->click_fn = hide_unit_action_panel_fn;
	    }
	    /* Update the whole panel. */
	    add_update(screen, panel->x, panel->y, panel->w, panel->h);
	    break;
	}
    } /* for all panels */
}

static void
draw_world_map(Screen *screen, Panel *panel)
{
    Uint32 color;
    SDL_Rect rect;

    color = SDL_MapRGB(screen->surf->format, 0, 0, 0);
#if (0)  /* Use this to see more of map outline */
    color = random_color(screen->surf);
#endif
    rect.x = panel->x + panel->w - 16 - 200;  rect.y = panel->y + 10;
    rect.w = 200;  rect.h = 130 - 20;
    SDL_FillRect(screen->surf, &rect, color);
#if (0)  /* Use this to see how often redraws are happening */
    color = random_color(screen->surf);
    draw_rect(screen->surf, rect.x, rect.y, rect.w, rect.h, color);
#endif
    memcpy(screen->minimap->vp, screen->minimap->main_vp, sizeof(VP));
    draw_map(screen->minimap);
}

void
update_cell(Screen *screen, int x, int y)
{
    int namelength = 1, cells = 1, sx, sy, sw, sh, i;
    Unit *unit;
    VP *vp;

    if (!in_area(x, y))
      return;
    if (screen == NULL)
      return;
    vp = screen->map->vp;

    /* Find max length of a unit name (skip at 4 x 4 or below). */
    if (vp->draw_names && vp->power > 2) {
	if (vp->power == 3)
		namelength = 11;
	if (vp->power == 4)
		namelength = 6;
	if (vp->power == 5)
		namelength = 4;
	if (vp->power == 6)
		namelength = 3;
    }
    /* Skip cells that are not inside the area. */
    if (!area.xwrap) {
	for (i = 1; i <= namelength;  i++) {
		if (!in_area(x + i, y)) {
			namelength = i;
			break;
		}
	}
    }	
    /* Then check if an erasing row really is needed. */
    if (namelength > 1) {
	if (vp->show_all) {
		for_all_units(unit) {
			/* Only check named units. */
			if (!unit->name)
				continue;
			/* First check if the unit is located at (x, y).*/
			if (is_located_at(unit, x, y) 
			/* Also check if it just left (x, y). Catches unit that died!*/
			    || was_located_at(unit, x, y)) {
				/* Make an erasing row if either is true. */
				cells = namelength;
				break;
			}
		}
	} else {
		/* We can't figure out if a unit view got erased in this update, so 
		we always do the erasing row. */ 
		cells = namelength;
	}
    }
    xform(screen->map, x, y, &sx, &sy);
    sh = vp->hh;
    sw = vp->hw * cells;
#if (0)
    /* Even though big icons fit within one hex, the selection box
    and health bar does not. */
    if (big_unit_images) {
	sx -= 3;
	sy -= 4;
	sw += 6;
	sh += 7; 
    }
#endif
    /* Unit images may extend up into the hex row above. */
    if (vp->isometric) {
	sy -= vp->hh / 2;
	sh += vp->hh / 2;
    }
    /* Clip the result to avoid misaligned updates at the edges. */
    sx = limitn(0, sx, vp->totsw);
    sy = limitn(0, sy, vp->totsh);
    sw = limitn(0, sw, vp->totsw - sx);
    sh = limitn(0, sh, vp->totsh -sy);

    add_update(screen, sx, sy, vp->hw, vp->hh);
    update_screen(screen, FALSE);
}

/* Buttons. */

SDLButton *
create_button(Panel *panel, int x, int y, int w, int h)
{
    SDLButton *newbutton;

    newbutton = (SDLButton *) xmalloc(sizeof(SDLButton));
    newbutton->x = x;  newbutton->y = y;
    newbutton->w = w;  newbutton->h = h;
    panel->buttons[panel->numbuttons++] = newbutton;
    newbutton->panel = panel;
    return newbutton;
}

void
draw_button(Screen *screen, Panel *panel, SDLButton *button)
{
    Uint32 color;
    SDL_Rect rect;

    if (!button->visible)
      return;
    rect.x = panel->x + button->x;  rect.y = panel->y + button->y;
    rect.w = button->w;  rect.h = button->h;
    if (button->draw_bg)
      SDL_FillRect(screen->surf, &rect, button->bg);
    color = SDL_MapRGB(screen->surf->format, 100, 140, 200);
#if (0) /* to see redraws */
    color = random_color(screen->surf);
#endif
    draw_rect(screen->surf, rect.x, rect.y, rect.w, rect.h, color);
    if (button->picture) {
	if (button->pic_surface == NULL) {
	    Image *img;
#if (0)
	    img = best_image(button->picture, button->w, button->h);
#endif
	    /* (HACK: Changes to 'best_image' force some images to be 
		scaled up which should not be scaled up.) */
	    /* (TODO: Fix. Perhaps query IMF to see which images are already 
		available to determine if scaling is even needed, and if 
		not, just select the "natural" image.) */
	    img = best_image(button->picture, 32, 32);
	    if (img != NULL) {
		if (sdl_image(img))
		  button->pic_surface = sdl_image(img);
	    }
	}
	if (button->pic_surface != NULL) {
	    /* Center the button image. */
	    rect.x += (rect.w - button->pic_surface->w) / 2;
	    rect.y += (rect.h - button->pic_surface->h) / 2;
	    SDL_BlitSurface(button->pic_surface, NULL, screen->surf, &rect);
	    return;
	}
    }
    if (!empty_string(button->label))
      draw_string(screen->surf, rect.x, rect.y, button->label);
}

/*** Lower-level graphics and utilities. ***/

Uint32
random_color(SDL_Surface *surf)
{
    return SDL_MapRGB(surf->format, xrandom(255), xrandom(255), xrandom(255));
}

/* Draw a rectangle inside the bounds defined by the arguments. */

void
draw_rect(SDL_Surface *surf, int sx, int sy, int sw, int sh, Uint32 col)
{
    SDL_Rect rect;

    rect.x = sx;  rect.y = sy;
    rect.w = sw;  rect.h = 1;
    SDL_FillRect(surf, &rect, col);
    rect.x = sx;  rect.y = sy;
    rect.w = 1;  rect.h = sh;
    SDL_FillRect(surf, &rect, col);
    rect.x = sx;  rect.y = sy + sh - 1;
    rect.w = sw;  rect.h = 1;
    SDL_FillRect(surf, &rect, col);
    rect.x = sx + sw - 1;  rect.y = sy;
    rect.w = 1;  rect.h = sh;
    SDL_FillRect(surf, &rect, col);
}

void
draw_line(SDL_Surface *surf, int sx1, int sy1, int sx2, int sy2, Uint32 col)
{
    SDL_Rect rect;

    if (sy1 == sy2) {
	rect.x = min(sx1, sx2);  rect.y = sy1;
	rect.w = max(sx1, sx2) - rect.x;  rect.h = 1;
	SDL_FillRect(surf, &rect, col);
    } else if (sx1 == sx2) {
	rect.x = sx1;  rect.y = min(sy1, sy2);
	rect.w = 1;  rect.h = max(sy1, sy2) - rect.y;
	SDL_FillRect(surf, &rect, col);
    } else {
	/* general case not implemented */
    }
}

/* Display a string in the small font. */

void
draw_string(SDL_Surface *surf, int x0, int y0, const char *str)
{
    int i, n = 0, x, y;
    SDL_Rect rect1, rect2;
    int numlf = 0;

    for (i = 0; i < strlen(str); ++i) {
	if ('\n' == str[i]) {
	    ++numlf;	/* Line feed. */
	    n = 0;	/* Carriage return. */
	    continue;
	}
	if ('\t' == str[i]) {
	    n += 4;	/* Horizontal tab. */
	    continue;
	}
	x = (str[i] - 32) % 16;
	y = (str[i] - 32) / 16;
	rect1.x = x * 14 + 7;  rect1.y = y * 14;
	rect1.w = 7;  rect1.h = 15;
	rect2.x = x0 + 7 * n;  rect2.y = y0 + (numlf * 16);
	rect2.w = 7;  rect2.h = 15;
	SDL_BlitSurface(small_font, &rect1, surf, &rect2);
	++n;
    }
}

/* Lifted from SDL verbatim. */
/*
 * A function to calculate the intersection of two rectangles:
 * return true if the rectangles intersect, false otherwise
 */

int
SDLIntersectRect(SDL_Rect *A, SDL_Rect *B, SDL_Rect *intersection)
{
	int Amin, Amax, Bmin, Bmax;

	/* Horizontal intersection */
	Amin = A->x;
	Amax = Amin + A->w;
	Bmin = B->x;
	Bmax = Bmin + B->w;
	if(Bmin > Amin)
	  Amin = Bmin;
	intersection->x = Amin;
	if(Bmax < Amax)
	  Amax = Bmax;
	intersection->w = Amax - Amin > 0 ? Amax - Amin : 0;

	/* Vertical intersection */
	Amin = A->y;
	Amax = Amin + A->h;
	Bmin = B->y;
	Bmax = Bmin + B->h;
	if(Bmin > Amin)
	  Amin = Bmin;
	intersection->y = Amin;
	if(Bmax < Amax)
	  Amax = Bmax;
	intersection->h = Amax - Amin > 0 ? Amax - Amin : 0;

	return (intersection->w && intersection->h);
}
