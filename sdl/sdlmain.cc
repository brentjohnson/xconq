/* Toplevel code for the SDL interface to Xconq.
   Copyright (C) 2000, 2001 Stanley T. Shebs.
   Copyright (C) 2004 Eric A. McDonald.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "sdlpreconq.h"
#include "conq.h"
#include "kpublic.h"
#include "sdlconq.h"
extern void set_panel_sizes(Screen *screen);
#include "cmdline.h"

extern void interpret_variants(void);
extern int launch_game(void);

/* Fullscreen mode interferes with debugging under X11 
   (but not under MacOS), so leave it off by default. */

/* We now set it to FALSE also for the Mac since fullscreen 
mode is always forced when using the Dsp driver. */

#ifdef UNIX
int fullscreen = FALSE;
#else
int fullscreen = FALSE;
#endif

/* The mac does not use SDL_APPMOUSEFOCUS. Test for SDL_APPACTIVE
   instead, which on the mac tells us if the SDL window is in the
   front or in the background. */

#ifdef MAC
#undef SDL_APPMOUSEFOCUS
#define SDL_APPMOUSEFOCUS SDL_APPINPUTFOCUS
#endif

/* Default size of window in non-fullscreen mode. */

int mainw = 800;
int mainh = 600;

/* This is the main display surface. */

SDL_Surface *mscreen;

struct a_real_ui *ui;

extern int ok_to_exit;

extern int research_popped_up;

/* This is the number of pixels around the edge of the screen that will
   result in autoscrolling if the cursor is positioned within that
   distance from the edge. */

int autoscroll_width = 16;

/* Delay in ms between the detection of being in the autoscroll
   and the start of actual scrolling. */

int autoscroll_delay = 800;

/* Storage for values of user preferences. */

VP default_vp;
int default_draw_terrain_images;
int default_draw_transitions;
char *default_font_family;
int default_font_size;

int use_stdio;

/* This flag is true while an runtime error or warning dialog is up;
   it is used to suppress attempts to draw, because they may have been
   the cause of the dialog popping up. */
  
int error_popped_up;

/* True if any variants are available. */

int any_variants;

/* These are true when a standard variant is available to be chosen. */

int vary_world_size;
int vary_real_time;

/* Tcl widgets are numbered 0-15 for checkboxes, -1 for world size
   dialog, -2 for real time dialog.  These map to and from variant
   indexes, which are defined by the order in which they're listed
   in module->variants. */

#define MAXCHECKBOXES 16

int numcheckboxes = MAXCHECKBOXES;

/* Indexes into the array of the main module's variants. */

int checkboxpos[MAXCHECKBOXES];
int worldsizepos = -1;
int realtimepos = -1;

/* Reverse lookup for tcltk widgets -> variants. */

int varrev[100];

char *default_cursor_name;

SDLCursor *generic_cursor;

SDL_Surface *small_font;

Screen *sscreen;

int use_cursors;

extern int using_sdl;

/* Declarations of local functions. */

static int interp_sym(SDLKey sym);
static void auto_scroll_map(Screen *screen, int dx, int dy);

static void update_side_progress_display(Side *side, Side *side2);
static void update_side_score_display(Side *side, Side *side2);

static void help_unit_type(Screen *screen);
static void help_terrain_type(Screen *screen);

static void init_redraws(void);

#if 0
static void ui_update_state(void);
#endif

static void update_variant_setting(int which);
static void update_assignment(int n);

static void ui_idle(void);

static void handle_event(SDL_Event *evt);
static void interp_key(int key);
static void update_mouseover_x(Screen *screen, int rawx, int rawy);
static void handle_mouse_down(Screen *screen, int sx, int sy, int button);
static void handle_mouse_down_in_panel(Screen *screen, Panel *panel,
				       int sx, int sy, int button);

extern void update_mouseover(Screen *screen);

static void move_look(Screen *screen, int sx, int sy);
static void move_the_selected_unit(Screen *screen, Unit *unit, int sx, int sy);

static void sdl_run_game(void);

static void init_all_displays(void);

static void side_research_fn(Screen *screen, int cancelled);

/* Do the most basic setup for the user interface.  This part is
   limited to that which can be done before a game has been loaded. */

void
initial_ui_init(void)
{
    int rslt;
    LibraryPath *path;
#ifndef MAC
    SDL_Rect **modes;
#endif
	 /* Hack to signal main in xconq.c that we dont want a new game dialog. */
	 using_sdl = TRUE;

    rslt = SDL_Init(SDL_INIT_VIDEO);
    if (rslt < 0)
      init_error("could not open SDL display");

    SDL_WM_SetCaption("Xconq", "Xconq");

    if (!empty_string(getenv("SDL_VIDEODRIVER"))
#ifdef UNIX
	/* The XFree86 fbcon and dga drivers run in fullscreen mode. */
	&& (strcmp(getenv("SDL_VIDEODRIVER"), "fbcon") == 0
	    || strcmp(getenv("SDL_VIDEODRIVER"), "dga") == 0)
#endif
#ifdef MAC
	/* The Mac DrawSprocket driver also runs in fullscreen mode. */
	&& strcmp(getenv("SDL_VIDEODRIVER"), "DSp") == 0
#endif
	)
      fullscreen = TRUE; 

    /* Get the available fullscreen modes if asked to. */
    if (fullscreen) {

#ifndef MAC
	/* Does not work on the mac. Should figure out why. */
	modes = SDL_ListModes(NULL,
			      SDL_FULLSCREEN | SDL_HWSURFACE | SDL_DOUBLEBUF);
	/* Use the largest screen available. */
	if (modes > (SDL_Rect **)0) {
	    mainw = modes[0]->w;  mainh = modes[0]->h;
	}
#else
	/* On the mac, we use qd.thePort instead. */
	/* (which won't work on OS X tho.) */
	mainw = qd.thePort->portRect.right;
	mainh = qd.thePort->portRect.bottom;
#endif

    }
    
    /* Use fullscreen mode if asked to, and hardware acceleration if
       available.
    
       Note 1: Under MacOS, hardware acceleration is only available in
       fullscreen mode, and double buffering only with the Dsp
       driver. Both the SDL_DOUBLEBUF and SDL_HWSURFACE flags are
       therefore always 0 in non-fullscreen mode, even if we try to
       set them. In fullscreen mode, the SDL_HWSURFACE flag (and it
       only) is set to 1 if we try to set either SDL_DOUBLEBUF or
       SDL_HWSURFACE for the Toolbox driver. However, surfaces that
       are created with SDL_AllocSurface will not have the
       SDL_HWSURFACE flag set, even if we ask for it and it is set for
       the screen.
    
       With the DSp driver (which only works in fullscreen mode) it is
       possible to set both SDL_HWSURFACE and SDL_DOUBLEBUF. Sometimes
       (depending on available VRAM?) only the SDL_DOUBLEBUF flag is
       set. This is important since surfaces that are created with
       SDL_AllocSurface can be allocated in VRAM (by setting the
       SDL_HWSURFACE flag) only if BOTH SDL_DOUBLEBUF and
       SDL_HWSURFACE are set for the screen.
    
       Note 2: Under Linux, the x11 video driver does not allow us to
       set either the SDL_HWSURFACE or the SDL_DOUBLEBUF flag to 1. It
       is, however, possible to use fullscreen mode without
       acceleration by setting SDL_FULLSCREEN to 1.  The dga driver
       always runs in fullscreen mode, so SDL_FULLSCREEN is set to 1
       irrespective of what we specify below. Hardware acceleration
       can then be turned on by setting SDL_DOUBLEBUF and
       SDL_HWSURFACE to 1. This does not help the poor performance
       under Linux much, though. Should figure out why. */
  
    mscreen = SDL_SetVideoMode(mainw, mainh, 32,
			       SDL_HWSURFACE
			       | SDL_DOUBLEBUF
			       | (fullscreen ? SDL_FULLSCREEN : SDL_RESIZABLE));

    imf_interp_hook = sdl_interp_imf;
    /* No special family loader needed, at least at present. */

    default_cursor_name = g_generic_cursor();
    generic_cursor = get_cursor(default_cursor_name, 0, 0);
    if (generic_cursor != NULL) {
#ifndef MAC      
	SDL_ShowCursor(0);
#endif
	use_cursors = TRUE;
    }

    for_all_library_paths(path) {
	make_pathname(path->path, "font.bmp", "", spbuf);
	small_font = SDL_LoadBMP(spbuf);
	if (small_font)
	  break;
    }
    if (small_font == NULL)
      init_error("could not open SDL small_font");

    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

    SDL_SetColorKey(small_font, SDL_SRCCOLORKEY|SDL_RLEACCEL,
		    SDL_MapRGB(small_font->format, 255, 0, 255));

    SDL_EnableUNICODE(1);
    
    update_variant_callback = update_variant_setting;
    update_assignment_callback = update_assignment;

    ui = (struct a_real_ui *) xmalloc(sizeof(struct a_real_ui));

#if 0
    Tcl_DoWhenIdle(check_network, (ClientData) 0);
    Tcl_CreateTimerHandler(100, check_network, (ClientData) 0);
#endif
}

/* Autoscroll if the mouse is near the edge of a map. */

int
point_in_map(Map *map, int rawx, int rawy)
{
    if (map == NULL)
      return FALSE;
    rawx -= map->sx;  rawy -= map->sy;
    if (!between(0, rawx, map->main_vp->pxw))
      return FALSE;
    if (!between(0, rawy, map->main_vp->pxh))
      return FALSE;
    return TRUE;
}

void
autoscroll(Screen *screen, int rawx, int rawy)
{
    int which, xdelta, ydelta;
    enum mapmode prevscrollmode;
    VP *vp;

    if (!use_cursors)
      return;
    if (point_in_map(screen->map, rawx, rawy)) {
	vp = screen->map->main_vp;
	rawx -= screen->map->sx;  rawy -= screen->map->sy;
	which = 0;
    } else if (point_in_map(screen->minimap, rawx, rawy)) {
	vp = screen->minimap->main_vp;
	rawx -= screen->minimap->sx;  rawy -= screen->minimap->sy;
	which = 1;
    } else {
	screen->scroll_mode[0] = no_scroll_mode;
	screen->scroll_mode[1] = no_scroll_mode;
	set_tool_cursor(screen);
	return;
    }
    prevscrollmode = screen->scroll_mode[which];
    screen->scroll_mode[which] = no_scroll_mode;
    xdelta = ydelta = 0;
    /* Note that rawx,rawy may be validly 0,0 here, so avoid that case
       - means that autoscrolling won't work on the exact edge of the
       screen, but not likely to be a problem in practice. */
    if (rawx > 0 && rawx < autoscroll_width && vp->sx > vp->sxmin) {
	screen->scroll_mode[which] = scroll_left_mode;
	xdelta = -1;
    } else if (rawx > (vp->pxw - autoscroll_width) && vp->sx < vp->sxmax) {
	screen->scroll_mode[which] = scroll_right_mode;
	xdelta = 1;
    }
    if (rawy > 0 && rawy < autoscroll_width && vp->sy > vp->symin) {
	screen->scroll_mode[which] = scroll_up_mode;
	ydelta = -1;
    } else if (rawy > (vp->pxh - autoscroll_width) && vp->sy < vp->symax) {
	screen->scroll_mode[which] = scroll_down_mode;
	ydelta = 1;
    }
    if (xdelta != 0 || ydelta != 0) {
	if (screen->autoscroll_delaying[which]) {
	    if (n_ms_elapsed(autoscroll_delay)) {
		int nsx, nsy;

		nsx = vp->sx + 8 * xdelta;  nsy = vp->sy + 8 * ydelta;
		if (which == 0)
		  set_view_position(screen->map->main_vp, nsx, nsy);
		else if (which == 1)
		  set_view_position(screen->minimap->main_vp, nsx, nsy);
		if (which == 0)
		  focus_on_center(screen->map->main_vp);
		redraw_screen(screen);
	    }
	} else {
	    record_ms();
	    screen->autoscroll_delaying[which] = TRUE;
	}
    } else {
	screen->autoscroll_delaying[which] = FALSE;
    }
    /* Make the cursor change appearance immediately, even while
       delaying the autoscroll. */
    if (screen->scroll_mode[which] != prevscrollmode)
      set_tool_cursor(screen);
}

void
set_current_unit(Screen *screen, Unit *unit)
{
    Unit *oldunit = screen->curunit;

    if (unit == oldunit)
      return;
    if (unit == NULL
	|| (in_play(unit) && side_controls_unit(dside, unit))
	|| endofgame) {
	screen->curunit = unit;
	screen->curunit_id = (unit ? unit->id : 0);
    }
    /* Make sure the unit is actually visible on-screen. */
    if (unit != NULL) {
	put_on_screen(screen->map, unit->x, unit->y);
	screen->scrolled_to_unit = TRUE;
    }
    /* (should only do this if screen not scrolled) */
    if (oldunit)
      update_cell(screen, oldunit->x, oldunit->y);
    if (screen->curunit)
      update_cell(screen, screen->curunit->x, screen->curunit->y);
    if (!(screen->side_research_panel_visible))
      screen->unit_action_panel_visible = TRUE;
    G_uact_button_offset = 0;
    update_unit_info(screen);
    update_screen(screen, TRUE);
}

void
ui_mainloop(void)
{
    SDL_Event evt;
    Uint32 now, next, last_idle = SDL_GetTicks();
    int eventcount = 0;

    while (1) {
	/* Process any pending events. */
	if (SDL_PollEvent(&evt) != 0) {
	    handle_event(&evt);
	    ++eventcount;
	    continue;
	}
	/* Take a quick break to cut CPU usage. */
	/* (TODO: Calibrate delay. We don't want to be slacking on systems 
	    that are having trouble keeping up as it is. In those cases we 
	    actually do want to use CPU as much as possible.) */
	if (eventcount) {
	    eventcount = 0;
	    SDL_Delay(10);
	}
	/* Run the game. */
	sdl_run_game();

	if (sscreen->active)
	  autoscroll(sscreen, sscreen->cursorx, sscreen->cursory);

	ui_idle();

	now = SDL_GetTicks();
	next = 40 - (now - last_idle);
	if (next > 0) {
#if 1
	    SDL_Delay(1);
#endif
	    last_idle = SDL_GetTicks();
	}
    }
}

/* Mandatory Definition: 'run_ui_idler'. Used by various parts of the 
   built-in AI code to make sure that the UI is responsive. */

void
run_ui_idler(void)
{
    SDL_Event evt;

    while (SDL_PollEvent(&evt) != 0) { 
	handle_event(&evt);
	ui_idle();
    }
}

/* The purpose of this function is to test for SDL mouse focus on the
   mac and turn the system cursor on or off. This has to be done
   explicitly since no SDL_ACTIVEEVENT is generated on the mac when we
   move the cursor in or out of the SDL window (unlike unix where the
   SDL_APPMOUSEFOCUS flag is set). */

#ifdef MAC

int
handle_macos_cursor(void)
{
	WindowPtr win;
	Point mouse;
	Rect rect;

	if (SDL_GetAppState() & SDL_APPMOUSEFOCUS) {
		win = FrontWindow();
		if (win && win == sdl_window) {
			GetMouse(&mouse);
			LocalToGlobal(&mouse);
			rect = (*((WindowPeek) win)->contRgn)->rgnBBox;
			/* The mouse is in the map. */
			if (PtInRect(mouse, &rect)) {
				SDL_ShowCursor(0);
				use_cursors = TRUE;
				return FALSE;
			}		
		}
	}
	SDL_ShowCursor(1);
	use_cursors = FALSE;
	if (!fullscreen)
	    update_cursor(sscreen);
	return TRUE;
}

#endif

static void
handle_event(SDL_Event *evt)
{
    int key, x, y;

    if (sscreen == NULL)
      return;

    switch (evt->type) {

      case SDL_VIDEORESIZE:
#ifdef MAC
	/* We resized the SIOUX console, not the main window. */
	if (FrontWindow() && FrontWindow() != sdl_window) {
	    redraw_screen(sscreen);
	    break;
	}
#endif
	/* Get the new window size, but only allow it to be so small. */
	mainw = max(evt->resize.w, 232);
	mainh = max(evt->resize.h, 232);
	/* Change the video mode. */
	SDL_FreeSurface(mscreen);
 	mscreen = SDL_SetVideoMode(mainw, mainh, 32, SDL_RESIZABLE);
	/* Reallocate the composition surface. */
	SDL_FreeSurface(sscreen->surf);
	sscreen->surf =
	  SDL_AllocSurface(SDL_SWSURFACE, mainw, mainh, 32,
			   0x00ff0000, 0x0000ff00, 0x000000ff, 0);
	/* Redraw the resized sscreen. */
	set_panel_sizes(sscreen);
	redraw_screen(sscreen);
	break;

      case SDL_ACTIVEEVENT:
#if 0
	printf("active %d %d %d\n", evt->active.type, evt->active.gain, evt->active.state);
#endif
	if (evt->active.state & SDL_APPMOUSEFOCUS) {
	    sscreen->active = evt->active.gain;
	    use_cursors = sscreen->active;
	    if (use_cursors) {
		SDL_ShowCursor(0);
		/* Update the cursor position. */
		SDL_GetMouseState(&x, &y);
		sscreen->cursorx = x;  sscreen->cursory = y;
		/* Restore the default cursor mode. */
		sscreen->tmp_mode = no_tmp_mode;
		sscreen->scroll_mode[0] = no_scroll_mode;
		sscreen->scroll_mode[1] = no_scroll_mode;
		set_tool_cursor(sscreen);
	    } else {
		SDL_ShowCursor(1);
	    }
	    update_cursor(sscreen);
	}
	break;

      case SDL_KEYDOWN:
        /* First check for special keys using SDL virtual keysymbols. */
        if (interp_sym(evt->key.keysym.sym))
          break; 
	key = evt->key.keysym.unicode & 0x7f;
	/* "Keystrokes" with a unicode of 0 are modifiers like shift
	   and ctrl, ignore them. */
	if (key == 0)
	  break;
	interp_key(key);
	break;

      case SDL_KEYUP:
	break;

      case SDL_MOUSEMOTION:
#ifdef MAC
	if (handle_macos_cursor())
	  break;
#endif
	if (evt->motion.x != sscreen->cursorx
	    || evt->motion.y != sscreen->cursory) {
	    /* Redraw at the old cursor location. */
	    add_update(sscreen,
		       sscreen->cursorx - sscreen->cursor->hotx,
		       sscreen->cursory - sscreen->cursor->hoty,
		       sscreen->cursor->surf->w, sscreen->cursor->surf->h);
	    /* Mouseover panels may change. */
	    update_mouseover(sscreen);
	}
	sscreen->cursorx = evt->motion.x;  sscreen->cursory = evt->motion.y;
	update_mouseover_x(sscreen, sscreen->cursorx, sscreen->cursory);
	update_screen(sscreen, FALSE);
	break;

      case SDL_MOUSEBUTTONDOWN:
	handle_mouse_down(sscreen, evt->button.x, evt->button.y,
			  evt->button.button);
	break;

      case SDL_MOUSEBUTTONUP:
	break;

      case SDL_QUIT:
	if (dside)
	  do_quit(dside);
	else
	  exit_xconq();
	break;

      default:
	break;
    }
}

/* Handle scrolling and other numeric keypad commands. */ 

static int
interp_sym(SDLKey sym)
{
    if (sym == SDLK_KP8) {
	auto_scroll_map(sscreen, 0, -4);
	return TRUE;
    }
    if (sym == SDLK_KP2) {
	auto_scroll_map(sscreen, 0, 4);
	return TRUE;
    }
    if (sym == SDLK_KP4) {
	auto_scroll_map(sscreen, -4, 0);
	return TRUE;
    }
    if (sym == SDLK_KP6) {
	auto_scroll_map(sscreen, 4, 0);
	return TRUE;
    }
    if (sym == SDLK_KP7) {
	auto_scroll_map(sscreen, -4, -4);
	return TRUE;
    }
    if (sym == SDLK_KP3) {
	auto_scroll_map(sscreen, 4, 4);
	return TRUE;
    }
    if (sym == SDLK_KP1) {
	auto_scroll_map(sscreen, -4, 4);
	return TRUE;
    }
    if (sym == SDLK_KP9) {
	auto_scroll_map(sscreen, 4, -4);
	return TRUE;
    }
    if (sym == SDLK_KP_PLUS) {
	do_zoom_in(dside);
	return TRUE;
    }
    if (sym == SDLK_KP_MINUS) {
	do_zoom_out(dside);
	return TRUE;
    }
    if (sym == SDLK_KP_MULTIPLY) {
	do_set_view_angle(dside);
	return TRUE;
    }
    if (sym == SDLK_KP5) {
	do_recenter(dside);
	return TRUE;
    }

#ifdef MAC
    /* Use the escape key to toggle between the screen and the SIOUX
       console on the mac. */
    if (sym == SDLK_ESCAPE) {
         if (FrontWindow() == sdl_window) {
	     sscreen->active = FALSE;
	     use_cursors = FALSE;
	     SelectWindow(sdl_console);
	     SDL_ShowCursor(1);
	} else if (FrontWindow() == sdl_console) {
	    SDL_ShowCursor(0);
	    sscreen->active = TRUE;
	    use_cursors = TRUE;
	    SelectWindow(sdl_window);
	    redraw_screen(sscreen);
	}	
	return FALSE;
    }
#endif

    return FALSE;
}

/* Scroll the screen automatically until the key is released. */

void
auto_scroll_map(Screen *screen, int dx, int dy)
{
    while (SDL_PeepEvents(NULL, 1, SDL_PEEKEVENT, SDL_KEYUPMASK) == 0) {
	set_view_position(screen->map->main_vp,
			  screen->map->main_vp->sx + dx, 
			  screen->map->main_vp->sy + dy);
	focus_on_center(screen->map->main_vp);
	redraw_screen(screen);
	SDL_Delay(1);
	SDL_PumpEvents();
    }
}

static void
interp_key(int key)
{
    int x, y, cancelled;
    Screen *screen;
    void (*fn)(Screen *screenx, int cancelledx);

#if 0
    printf("key '%c'\n", key);
#endif
    screen = sscreen;
    screen->inpch = key;
    screen->inpsx = screen->cursorx;  screen->inpsy = screen->cursory;
    screen->inpx = screen->inpy = -1;
    if (nearest_cell(screen->map->main_vp, screen->inpsx, screen->inpsy,
		     &x, &y, NULL, NULL)) {
	screen->inpx = x;  screen->inpy = y;
    }
    DGprintf("key %d %d -> %d %d\n",
	     screen->inpsx, screen->inpsy, screen->inpx, screen->inpy);
    /* Call the modal handler if defined, giving it screen and cancel
       flag. */
    if (screen->modalhandler) {
	fn = screen->modalhandler;
	cancelled = (screen->inpch == ESCAPE_CHAR);
	/* Remove the handler - will restore itself if needed. */
	screen->modalhandler = NULL;
	(*fn)(screen, cancelled);
	if (cancelled) {
	    notify(dside, "Cancelled.");
	    reset_screen_input(screen);
	}
    } else if (isdigit(screen->inpch)) {
	/* Digits are assumed to be part of a command prefix, such as
	   a repetition count. */
	if (screen->prefixarg < 0) {
	    screen->prefixarg = 0;
	} else {
	    screen->prefixarg *= 10;
	}
	screen->prefixarg += (screen->inpch - '0');
	/* Construction run legnths may be modified. */
	if (screen->unit_action_panel_visible) {
	    update_unit_info(screen);
	    update_screen(screen, TRUE);
	}
    } else if ((ESCAPE_CHAR == screen->inpch) 
	       && screen->unit_action_panel_visible) {
	screen->prefixarg = -1;
	/* Construction run legnths may be modified. */
	update_unit_info(screen);
	redraw_screen(screen);
    } else {
	/* In any other situation, the character is a single-letter
	   command. */
	dside->prefixarg = screen->prefixarg;
	ui->beepcount = 0;
	execute_command(dside, screen->inpch);
    }
    /* Clear the command char, so menu selects and other issuers of
       commands aren't confused with keystroke commands. */
    screen->inpch = '\0';
    /* Reset the prefix arg unless there is still more modal input
       to be read or the action buttons are available. */
    if ((screen->modalhandler == NULL) && !(screen->unit_action_panel_visible))
      screen->prefixarg = -1;
}

static void
update_mouseover_x(Screen *screen, int rawx, int rawy)
{
    int x, y, u;
    enum mapmode prevtmpmode;
    Unit *unit, *unit2;
    UnitView *uview;
    Side *side2;
    VP *vp;

    DGprintf("over screen %x %d %d\n", screen, rawx, rawy);
    if (point_in_map(screen->map, rawx, rawy)) {
	vp = screen->map->main_vp;
	prevtmpmode = screen->tmp_mode;
	screen->tmp_mode = no_tmp_mode;
	unit = screen->curunit;
	/* Modify the cursor depending on the type of unit it is over. */
	if (unit != NULL && !is_designer(dside)) {
	    nearest_cell(vp, rawx, rawy, &x, &y, NULL, NULL);
	    nearest_unit_view(dside, vp, rawx, rawy, &uview);
	    if (in_area(x, y)) {
		if (vp->show_all) {
		    nearest_unit(dside, vp, rawx, rawy, &unit2);
		    if (unit2 != NULL) {
			if (unit_could_attack(unit, unit2->type, unit2->side,
					      x, y))
			  screen->tmp_mode = attack_mode;
		    }
		} else if (uview != NULL) {
		    u = uview->type;
		    side2 = side_n(uview->siden);
		    if (unit_could_attack(unit, u, side2, x, y))
		      screen->tmp_mode = attack_mode;
		}
	    }
	}
	if (screen->tmp_mode != prevtmpmode)
	  set_tool_cursor(screen);
    }
    autoscroll(screen, rawx, rawy);
    screen->last_rawx[0] = rawx;  screen->last_rawy[0] = rawy;
}

extern int s_nearest_cell(Screen *screen, int sx, int sy, int *xp, int *yp);

int
s_nearest_cell(Screen *screen, int sx, int sy, int *xp, int *yp)
{
    sx -= screen->map->sx;  sy -= screen->map->sy;
    return nearest_cell(screen->map->main_vp, sx, sy, xp, yp, NULL, NULL);
}

static void
handle_mouse_down(Screen *screen, int sx, int sy, int button)
{
    int ax, ay;
    void (*fn)(Screen *screenx, int cancelledx);
    Panel *panel;

#ifdef MAC
    WindowPtr win;
    Point mouse;
	
    if (SDL_GetAppState() & SDL_APPMOUSEFOCUS) {
	GetMouse(&mouse);
	LocalToGlobal(&mouse);
	FindWindow(mouse, &win);
	if (win && win == sdl_window) {
	    SDL_ShowCursor(0);
	    sscreen->active = TRUE;
	    use_cursors = TRUE;
	} else {
	    SDL_ShowCursor(1);
	    sscreen->active = FALSE;
	    use_cursors = FALSE;
	}
    }
#endif
    if (screen == NULL) {
	beep();
	return;
    }
    for (panel = screen->panels; panel != NULL; panel = panel->in) {
	if (between(panel->x, sx, panel->x + panel->w)
	    && between(panel->y, sy, panel->y + panel->h)) {
	    handle_mouse_down_in_panel(screen, panel, sx, sy, button);
	    return;
	}
    }
    /* (this can be tricked by overlap with offscreen areas) */
    if (!s_nearest_cell(screen, sx, sy, &ax, &ay)) {
	beep();
	return;
    }
    nearest_unit(dside, screen->map->main_vp, sx, sy, &(screen->inpunit));
    screen->inpx = ax;  screen->inpy = ay;
    /* Assume that last place clicked is a reasonable focus. */
    if (inside_area(ax, ay))
      set_view_focus(screen->map->main_vp, ax, ay);
    if (screen->modalhandler) {
	fn = screen->modalhandler;
	screen->modalhandler = NULL;
	(*fn)(screen, 0);
	return;
    }
    switch (screen->mode) {
      case survey_mode:
	if (button == 1) {
	    move_look(screen, sx, sy);
	} else if (button == 3) {
	    if (screen->curunit && side_controls_unit(dside, screen->curunit)) {
		move_the_selected_unit(screen, screen->curunit, sx, sy);
	    } else {
		beep();
	    }
	}
	break;
      case move_mode:
	/* Both left and right buttons do the same thing in this mode. */
	if (screen->curunit && side_controls_unit(dside, screen->curunit)) {
	    move_the_selected_unit(screen, screen->curunit, sx, sy);
	} else {
	    beep();
	}
	break;
     default:
	/* error eventually */
	break;
    }
}

static void
move_look(Screen *screen, int sx, int sy)
{
    int nx, ny;
    Unit *unit;

    if (s_nearest_cell(screen, sx, sy, &nx, &ny)) {
	if (inside_area(nx, ny)) {
	    nearest_unit(dside, screen->map->main_vp, sx, sy, &unit);
	    if (unit != NULL
		&& !(side_controls_unit(dside, unit) || endofgame))
	      unit = NULL;
	    set_current_unit(screen, unit);
	} else {
	    beep();
	}
    }
}

static void
move_the_selected_unit(Screen *screen, Unit *unit, int sx, int sy)
{
    int x, y, rslt;
    HistEventType reason;
    Unit *other = NULL;
    char failbuf[BUFSIZE];

    s_nearest_cell(screen, sx, sy, &x, &y);
    nearest_unit(dside, screen->map->main_vp, sx, sy, &other);
    rslt = advance_into_cell(dside, unit, x, y, other, &reason);
    if (!rslt) {
	advance_failure_desc(failbuf, unit, reason);
	notify(dside, "%s", failbuf);
	beep();
    }
}

static void
handle_mouse_down_in_panel(Screen *screen, Panel *panel, int sx, int sy,
			   int mousebutton)
{
    int i;
    SDLButton *button;

    /* Make the mouse location be panel-relative. */
    sx -= panel->x;  sy -= panel->y;
    if (between(panel->w - 10, sx, panel->w)
	&& between(panel->h - 10, sy, panel->h)) {
	if (panel->h > 10)
	  panel->h = 10;
	else
	  panel->h = panel->full_h;
	set_panel_sizes(screen);
	redraw_screen(screen);
	return;
    }
    /* See if we clicked on any buttons in the panel. */
    for (i = 0; i < panel->numbuttons; ++i) {
	button = panel->buttons[i];
	if (between(button->x, sx, button->x + button->w)
	    && between(button->y, sy, button->y + button->h)) {
	    if (button->click_fn)
	      (*(button->click_fn))(button);
	    draw_button(screen, panel, button);
	    return;
	}
    }
    switch (panel->type) {
      case bottom_panel:
	if (point_in_map(screen->minimap, sx + panel->x, sy + panel->y)) {
	    int x, y;
	    if (nearest_cell(screen->minimap->main_vp, 
			     sx + panel->x - screen->minimap->sx,
			     sy + panel->y - screen->minimap->sy,
			     &x, &y, NULL, NULL)) {
		recenter(screen->map, x, y);
	    }
	}
	break;
    }
}

char *mouseover_buf;

void
update_mouseover(Screen *screen)
{
    Panel *panel;

    for (panel = screen->panels; panel != NULL; panel = panel->in) {
	if (panel->type == mouseover_panel) {
	    if (screen_oneliner(screen))
	      add_update(screen, panel->x, panel->y, panel->w, panel->h);
	}
    }
}

/* Compute the current contents of the mouseover buf.  Return true if
   it changed, false if not. */

int
screen_oneliner(Screen *screen)
{
    int i, sx, sy;
    Panel *panel;
    SDLButton *button;

    if (!(screen->modalhandler))
      screen->mouseover_panel_locked = FALSE;
    if (screen->modalhandler && screen->mouseover_panel_locked)
      return TRUE;
    if (mouseover_buf == NULL)
      mouseover_buf = (char *)xmalloc(BUFSIZE);
    /* Make tmpbuf has something reasonable to begin with. */
    tmpbuf[0] = '\0';
    sx = screen->cursorx;  sy = screen->cursory;
    if (point_in_map(screen->map, sx, sy)) {
	oneliner(dside, screen->map->main_vp, sx, sy);
    } else if (point_in_map(screen->minimap, sx, sy)) {
	oneliner(dside, screen->minimap->main_vp, sx, sy);
    } else {
	for (panel = screen->panels; panel != NULL; panel = panel->in) {
	    if (between(panel->x, sx, panel->x + panel->w)
		&& between(panel->y, sy, panel->y + panel->h)) {
		if (panel->label)
		  strcpy(tmpbuf, panel->label);
		sx -= panel->x;  sy -= panel->y;
		for (i = 0; i < panel->numbuttons; ++i) {
		    button = panel->buttons[i];
		    if (between(button->x, sx, button->x + button->w)
			&& between(button->y, sy, button->y + button->h)) {
			if (button->help)
			  strcpy(tmpbuf, button->help);
			else if (button->label)
			  strcpy(tmpbuf, button->label);
			break;
		    }
		}
		sx += panel->x;  sy += panel->y;
	    }
	}
    }
    if (strcmp(mouseover_buf, tmpbuf) == 0)
      return FALSE;
    strcpy(mouseover_buf, tmpbuf);
    return TRUE;
}

void
update_cursor(Screen *screen)
{
    add_update(screen,
	       screen->cursorx - screen->cursor->hotx,
	       screen->cursory - screen->cursor->hoty,
	       screen->cursor->surf->w, screen->cursor->surf->h);
    update_screen(screen, TRUE);
}

static void
sdl_run_game(void)
{
    int maxactions, rslt = 0, interval;
    int cs1, cs2;

    maxactions = 10;

    /* Check for any input from remotes. */
    if (my_rid > 0) {
	flush_outgoing_queue();
	receive_data(0, MAXPACKETS);
	++rslt;
    }
    cs1 = cs2 = 0;
    if (my_rid > 0 && my_rid != master_rid)
      cs1 = game_checksum();
    run_local_ai(1, 20);
    if (my_rid > 0 && my_rid != master_rid)
      cs2 = game_checksum();
    if (cs1 != cs2)
      run_warning("Checksum %d -> %d after run_local_ai\n", cs1, cs2);

    /* Run the kernel itself. */
    rslt += net_run_game(maxactions);
    
#if 0
    cs1 = cs2 = 0;
    if (my_rid > 0 && my_rid != master_rid)
      cs1 = game_checksum();
    run_local_ai(2, 20);
    if (my_rid > 0 && my_rid != master_rid)
      cs2 = game_checksum();
    if (cs1 != cs2)
      run_warning("Checksum %d -> %d after run_local_ai\n", cs1, cs2);
#endif

    /* Check for any input from remotes. */
    if (my_rid > 0) {
	flush_outgoing_queue();
	receive_data(0, MAXPACKETS);
	++rslt;
    }
    /* Set up to call it again in a little while. */
    /* If things are happening, call again quickly, for responsiveness. */
    interval = 10;
    /* If nothing seems to be happening right now, do a few times/sec,
       just in case. */
    if (rslt == 0)
      interval = 250;
}

static void
ui_idle(void)
{
    Screen *screen;
    Unit *unit, *unit2;

    for_all_screens(screen) {
	unit = screen->curunit;
	if (screen->autoselect) {
	    if (in_play(unit)
		&& unit->id == screen->curunit_id
		&& side_controls_unit(dside, unit)
		&& has_acp_left(unit)
		&& (unit->plan ? !unit->plan->asleep : TRUE)
		&& (unit->plan ? !unit->plan->reserve : TRUE)
		&& (unit->plan ? !unit->plan->delayed : TRUE)
		) {
		if (!in_middle(screen->map, unit->x, unit->y)
		    && !screen->scrolled_to_unit) {
		    put_on_screen(screen->map, unit->x, unit->y);
		    screen->scrolled_to_unit = TRUE;
		}
	    } else {
		unit2 = autonext_unit_inbox(dside, unit, screen->map->main_vp);
		if (unit2
		    && unit2->plan
		    && !unit2->plan->asleep
		    && !unit2->plan->reserve
		    && !unit2->plan->delayed
		    && unit2->plan->waitingfortasks) {
		    /* Use this unit */
		} else {
		    if (!in_play(unit)
			|| unit->id != screen->curunit_id
			|| !side_controls_unit(dside, unit))
		      unit = NULL;
		    unit2 = autonext_unit(dside, unit);
		}
		if (unit2 != unit)
		  screen->scrolled_to_unit = FALSE;
		if (unit2 != NULL)
		  set_current_unit(screen, unit2);
	    }
	} else {
	    /* Even when not auto-selecting, the selected unit may pass
	       out of our control. */
	    if (!in_play(unit)
		|| unit->id != screen->curunit_id
		|| !(side_controls_unit(dside, unit) || endofgame))
	      unit = NULL;
	    set_current_unit(screen, unit);
	}
    }
}

#if 0
enum setup_stage last_stage = initial_stage;

void
check_network(ClientData cldata)
{
    extern int quitter;

    if (hosting || my_rid > 0) {
	flush_outgoing_queue();
	receive_data(0, MAXPACKETS);
	if (my_rid > 0
	    && my_rid != master_rid
	    && current_stage != last_stage) {
	    if (current_stage == game_load_stage) {
		eval_tcl_cmd("popup_game_dialog");
	    } else if (current_stage == variant_setup_stage) {
		eval_tcl_cmd("popup_variants_dialog");
	    } else if (current_stage == player_setup_stage) {
		eval_tcl_cmd("set_variants");
	    } else if (current_stage == game_ready_stage) {
		eval_tcl_cmd("set_players");
	    }
	    last_stage = current_stage;
	}
	if (quitter != 0) {
	    close_remote_connection(quitter);
	    eval_tcl_cmd("insert_chat_string %d %d \"has quit\"",
			 my_rid, quitter);
	    --numremotes;
	    quitter = 0;
	}
    }
    /* (should detect when networking no longer possible, not resched calls) */
#if 0
    Tcl_DoWhenIdle(check_network, (ClientData) 0);
#endif
    Tcl_CreateTimerHandler(100, check_network, (ClientData) 0);
}
#endif

/* All update_xxx_display callbacks are here. */

void
update_area_display(Side *side)
{
    Screen *screen;

    if (!active_display(side))
      return;
    for_all_screens(screen)
      redraw_screen(screen);
}

extern int suppress_update_cell_display;

/* Draw an individual detailed hex, as a row of one, on all screens. */

void
update_cell_display(Side *side, int x, int y, int flags)
{
    int dir, x1, y1;
    Screen *screen;
    VP *vp;

    if (!active_display(side) || suppress_update_cell_display)
      return;
    for_all_screens(screen) {
	vp = screen->map->vp;
	if (!((flags & UPDATE_ALWAYS)
		|| ((flags & UPDATE_COVER) && vp->draw_cover)
		|| ((flags & UPDATE_TEMP) && vp->draw_temperature)
		|| ((flags & UPDATE_CLOUDS) && vp->draw_clouds)
		|| ((flags & UPDATE_WINDS) && vp->draw_winds)
	)) {
		continue;
	}
	update_cell(screen, x, y);
	if (flags & UPDATE_ADJ) {
	    vp = screen->map->main_vp;
	    if ((side->terrview != NULL && vp->hw > 10)
		|| vp->draw_people
		|| vp->draw_control
		|| vp->draw_feature_boundaries
		|| (vp->draw_elevations && numdesigners > 0)
		) {
		for_all_directions(dir) {
		    if (point_in_dir(x, y, dir, &x1, &y1)
			/* A totally unseen adjacent cell will have
			   nothing worth redrawing. */
			&& (terrain_view(dside, x1, y1) != UNSEEN
			    || numdesigners > 0)) {
			update_cell(screen, x1, y1);
		    }
		}
	    }
	}
	update_screen(screen, TRUE);
    }
}

/* The kernel calls this to update info about the given side. */

void
update_side_display(Side *side, Side *side2, int rightnow)
{
    int m;
    char sidebuf[BUFSIZE];
    char icontitle [BUFSIZE], windowtitle [BUFSIZE];
    int ibufleft = BUFSIZE, wbufleft = BUFSIZE;

    if (!active_display(side))
      return;
    if (side2 == NULL)
      return;
    /* Prepare icon and window captions for side with display. */
    if (side == side2) {
	windowtitle[0] = 0;
	strcpy(icontitle, "Xconq - ");
	ibufleft -= strlen(icontitle);
	if (mainmodule->title) {
	    strncat(icontitle, mainmodule->title, ibufleft);
	    strcpy(windowtitle, icontitle);
	    strncat(windowtitle, " - ", wbufleft);
	    ibufleft = BUFSIZE - strlen(icontitle);
	    wbufleft = BUFSIZE - strlen(windowtitle);
	}
	strncat(windowtitle, short_side_title(side2), wbufleft);
	wbufleft = BUFSIZE - strlen(windowtitle);
	strncat(windowtitle, " - ", wbufleft);
	wbufleft = BUFSIZE - strlen(windowtitle);
	snprintf(sidebuf, BUFSIZE, "Turn %d", g_turn());
	strncat(windowtitle, sidebuf, wbufleft);
	SDL_WM_SetCaption(windowtitle, icontitle);
    }
    /* Build up and write the textual description of the side. */
    sidebuf[0] = '\0';
    if (is_designer(side2))
      strcat(sidebuf, "(designer)");
    strcat(sidebuf, short_side_title(side2));
    if (side2->willingtodraw) {
	strcat(sidebuf, "[draw]");
    }
    if (side2->player) {
	strcat(sidebuf, "(");
	simple_player_title(sidebuf+strlen(sidebuf), side2->player);
	strcat(sidebuf, ")");
    }
#if 0
    eval_tcl_cmd("update_game_side_info %d {%s} %d %d %d",
		 side2->id, sidebuf, side2->everingame, side2->ingame,
		 side2->status);
#endif
    update_side_progress_display(side, side2);
    update_side_score_display(side, side2);
    for_all_material_types(m) {
	if (m_treasury(m)) {
#if 0
	    eval_tcl_cmd("update_side_treasury %d %d %d",
			 side2->id, mtype_indexes[m], side2->treasury[m]);
#endif
	}
    }
}

static void side_research_fn(Screen *screen, int cancelled);
static void prev_srsch_button_fn(SDLButton *button);
static void next_srsch_button_fn(SDLButton *button);
static void side_research_button_fn(SDLButton *button);
static void side_auto_research_button_fn(SDLButton *button);
static int add_side_research_buttons(Panel *panel, Side *side);

int G_srsch_button_offset;

/* Modal handler for side research. */

static void
side_research_fn(Screen *screen, int cancelled)
{
    if (cancelled)
      return;
    if (NOADVANCE == screen->inptype) {
	screen->modalhandler = side_research_fn;
	run_ui_idler();
	SDL_Delay(10);
    }
    else {
	screen->modalhandler = NULL;
	G_srsch_button_offset = 0;
	net_set_side_research_topic(dside, screen->inptype);
	reset_screen_input(screen);
	screen->unit_action_panel_visible = TRUE;
	screen->side_research_panel_visible = FALSE;
	update_unit_info(screen);
	update_screen(screen, TRUE);
    }
}

/* Load previous set of side research buttons. */

static void
prev_srsch_button_fn(SDLButton *button)
{
    Panel *panel = NULL;

    assert_error(button, "Attempted to access a NULL button");
    panel = button->panel;
    assert_error(button->panel, "Attempted to access a NULL panel");
    G_srsch_button_offset -= (button->panel->numbuttons - 2);
    add_side_research_buttons(panel, dside);
    add_update(sscreen, panel->x, panel->y, panel->w, panel->h);
    update_mouseover(sscreen);
    update_screen(sscreen, TRUE);
}

/* Load next set of side research buttons. */

static void
next_srsch_button_fn(SDLButton *button)
{
    Panel *panel = NULL;

    assert_error(button, "Attempted to access a NULL button");
    panel = button->panel;
    assert_error(button->panel, "Attempted to access a NULL panel");
    G_srsch_button_offset += (button->panel->numbuttons - 2);
    add_side_research_buttons(panel, dside);
    add_update(sscreen, panel->x, panel->y, panel->w, panel->h);
    update_mouseover(sscreen);
    update_screen(sscreen, TRUE);
}

/* Click handler for side research button. */

static void
side_research_button_fn(SDLButton *button)
{
    assert_error(button, "Attempted to access a NULL button");
    sscreen->inptype = button->data;
    side_research_fn(sscreen, FALSE);
}

/* Click handler for side auto research button. */

static void
side_auto_research_button_fn(SDLButton *button)
{
    assert_error(button, "Attempted to access a NULL button");
    auto_pick_side_research(dside);
    sscreen->inptype = dside->research_topic;
    side_research_fn(sscreen, FALSE);
}

/* Add side research buttons to the screen. */

static int
add_side_research_buttons(Panel *panel, Side *side)
{
    int a = NONATYPE, numbuttons = 0, i = 0, j = 0;
    struct a_protobutton *protobutton = NULL;
    SDLButton *button = NULL;
    Uint32 color = 0;
    Screen *screen = NULL;

    assert_error(panel, "Attempted to modify a NULL panel");
    assert_error(side, "Attempted to access a NULL side");
    screen = panel->screen;
    G_num_protobuttons = 0;
    /* Reset buttons. */
    reset_panel_buttons(panel);
    /* Load possible advances into protobuttons. */
    for_all_advance_types(a) {
	if (side_can_research(side, a)) {
	    protobutton = &(G_protobuttons[G_num_protobuttons++]);
	    memset(protobutton, 0, sizeof(struct a_protobutton));
	    protobutton->data = a;
	    protobutton->label = "R&D";
	    protobutton->picture = NULL;
	    sprintf(protobutton->help, "Research %s", a_type_name(a));
	    protobutton->click_fn = side_research_button_fn;
	}	
    } /* for all advances */
    /* Load an auto research button. */
    if (G_num_protobuttons) {
	protobutton = &(G_protobuttons[G_num_protobuttons++]);
	memset(protobutton, 0, sizeof(struct a_protobutton));
	protobutton->label = "Auto\nR&D";
	protobutton->picture = NULL;
	sprintf(protobutton->help, "Automatically Choose Research Topic");
	protobutton->click_fn = side_auto_research_button_fn;
    }
    /* Retain the button offset unless it's too big. */
    if (G_num_protobuttons <= G_srsch_button_offset)
      G_srsch_button_offset = 0;
    if (G_num_protobuttons && panel->numbuttons)
      numbuttons = min(G_num_protobuttons, panel->numbuttons - 1);
    else
      numbuttons = 0;
    /* Assign protobuttons to buttons, possibly with scrolling
       buttons at either end. */
    j = G_srsch_button_offset;
    for (i = 0; i < numbuttons; ++i) {
	button = panel->buttons[i];
	button->visible = FALSE;
	if ((i == 0) && (G_srsch_button_offset > 0)) {
	    button->visible = TRUE;
	    button->draw_bg = TRUE;
	    color = SDL_MapRGB(screen->surf->format, 120, 150, 150);
	    button->bg = color;
	    button->label = "Prev";
	    button->picture = NULL;
	    button->pic_surface = NULL;
	    button->click_fn = prev_srsch_button_fn;
	} 
	else if ((i == numbuttons - 1)
		 && ((G_num_protobuttons - G_srsch_button_offset) >
		      numbuttons)) {
	    button->visible = TRUE;
	    button->draw_bg = TRUE;
	    color = SDL_MapRGB(screen->surf->format, 120, 150, 150);
	    button->bg = color;
	    button->label = "Next";
	    button->picture = NULL;
	    button->pic_surface = NULL;
	    button->click_fn = next_srsch_button_fn;
	}
	else if (j < G_num_protobuttons) {
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
	if ((G_num_protobuttons - G_srsch_button_offset + 2)
	    < numbuttons)
	  button = panel->buttons[G_num_protobuttons -
				  G_srsch_button_offset + 1];
	else
	  button = panel->buttons[numbuttons];
	button->visible = TRUE;
	button->draw_bg = TRUE;
	color = SDL_MapRGB(screen->surf->format, 120, 150, 150);
	button->bg = color;
	button->picture = NULL;
	button->pic_surface = NULL;
	button->data = NONATYPE;
	button->label = "Skip\nR&D";
	button->help = "Skip Research";
	button->click_fn = side_research_button_fn;
    }
    return numbuttons;
}

/* Update the side research display. */

void
update_research_display(Side *side)
{
    Screen *screen = NULL;
    Panel *panel = NULL;
    int a = NONATYPE;
    int numbuttons = 0;
    /* SDL_Event evt; */

    assert_error(side, "Attempted to manipulate a NULL side");
    if (is_advance_type(side->research_topic))
      return;
    for (panel = sscreen->panels; panel; panel = panel->in) {
	if (panel->type == bottom_panel)
	  break;
    }
    if (!panel)
      return;
    screen = panel->screen;
    if ((side == dside) && !side_has_ai(side) && !(side->autoresearch)) {
	screen->unit_action_panel_visible = FALSE;
	screen->side_research_panel_visible = TRUE;
	numbuttons = add_side_research_buttons(panel, side);
	/* If there was space to setup side research buttons and there were 
	   advances to be bound to them, then do it. */
	if (numbuttons) {
	    screen->inptype = NOADVANCE;
	    screen->modalhandler = side_research_fn;
	    draw_panel(screen, panel);
	    add_update(screen, panel->x, panel->y, panel->w, panel->h);
	    update_screen(screen, TRUE);
	    return;
	}
	/* We may have 0 buttons, if the panel is resized to be too small, 
	   or if the side is out of advances to research. */
	else {
	    screen->modalhandler = NULL;
	    screen->unit_action_panel_visible = TRUE;
	    screen->side_research_panel_visible = FALSE;
	}
#if (0)
	/* Fire off an artificial mouseover to make sure that the mouseover 
	   panel, etc... gets updated. */
	memset(&evt, 0, sizeof(SDL_Event));
	evt.motion.type = SDL_MOUSEMOTION;
	evt.motion.x = (screen->cursorx - screen->cursor->hotx);
	evt.motion.y = (screen->cursory - screen->cursor->hoty);
	SDL_PushEvent(&evt);
#endif
    }
    auto_pick_side_research(side);
    a = side->research_topic;
    net_set_side_research_topic(side, a);
    a = side->research_goal;
    net_set_side_research_goal(side, a);
}

/* For sides that are active in the game, update their progress display. */

static void
update_side_progress_display(Side *side, Side *side2)
{
    int totacp, percentleft, percentresv, activ;
    extern int curpriority;

    /* No progress display if no game action for this side. */
    if (!side2->ingame || endofgame)
      return;
    activ = FALSE;
    totacp = percentleft = percentresv = 0;
    totacp = side_initacp(side2);
    if (totacp > 0) {
	percentleft = (100 * side_acp(side2)) / totacp;
	percentleft = limitn(0, percentleft, 100);
	percentresv = (100 * side_acp_reserved(side2)) / totacp;
	percentresv = limitn(0, percentresv, percentleft);
    }
    activ = (!g_use_side_priority() || curpriority == side2->priority);
#if 0
    eval_tcl_cmd("update_side_progress %d %d %d %d %d",
		 side2->id, activ, percentleft, percentresv,
		 side2->finishedturn);
#endif
}

static void
update_side_score_display(Side *side, Side *side2)
{
    int i;
    char *scoredesc;
    Scorekeeper *sk;

    if (keeping_score() && side2->everingame) {
	i = 0;
	for_all_scorekeepers(sk) {
	    scoredesc = side_score_desc(spbuf, side2, sk);
#if 0
	    eval_tcl_cmd("update_game_side_score score%d_%d \"%s\"",
			 i, side2->id, scoredesc);
#endif
	    ++i;
	}
    }
}

/* The kernel calls this to update info about the given unit. */

void
update_unit_display(Side *side, Unit *unit, int rightnow)
{
    Screen *screen;

    if (!active_display(side))
      return;
    if (unit == NULL)
      return;
    if (inside_area(unit->x, unit->y)) {
	update_cell_display(side, unit->x, unit->y, UPDATE_ALWAYS);
    }
    for_all_screens(screen) {
	if (unit == screen->curunit) {
	    update_unit_info(screen);
	}
    }
    if (unit->side != NULL) {
	update_side_progress_display(side, unit->side);
	update_side_score_display(side, unit->side);
    }
}

void
update_unit_acp_display(Side *side, Unit *unit, int rightnow)
{
    /* There isn't really much to skip over from what regular unit
       display does, so just call it. */
    update_unit_display(side, unit, rightnow);
}

void
update_action_result_display(Side *side, Unit *unit, int rslt, int rightnow)
{
    Screen *screen;

    if (!active_display(side))
      return;

    for_all_screens(screen) {
	if (unit == screen->curunit)
	  screen->scrolled_to_unit = FALSE;
    }
}

/* The kernel calls this to update the global game state. */

void
update_turn_display(Side *side, int rightnow)
{
    Screen *screen;
    Unit *unit;
    static int told_outcome = FALSE;

    if (!active_display(side))
      return;

    for_all_screens(screen) {
	if ((unit = screen->curunit) != NULL) {
	    if (inside_area(unit->x, unit->y)) {
		update_cell_display(side, unit->x, unit->y, UPDATE_ALWAYS);
	    }
	    update_unit_info(screen);
	}
    }
    if (endofgame && !told_outcome && side == dside) {
	/* Force every screen into a see-all/survey mode. */
	for_all_screens(screen) {
	    set_show_all(screen, TRUE);
	    redraw_screen(screen);
	    if (screen->mode != survey_mode) {
		do_survey(side);
	    }
	}
#if 0
	if (side_won(dside)) {
	    eval_tcl_cmd("popup_game_over_dialog won");
	} else if (side_lost(dside)) {
	    eval_tcl_cmd("popup_game_over_dialog lost");
	} else {
	    eval_tcl_cmd("popup_game_over_dialog over");
	}
#endif
	told_outcome = TRUE;
    }
}

void
update_action_display(Side *side, int rightnow)
{
    Screen *screen;
    Unit *unit;

    if (!active_display(side))
      return;

    for_all_screens(screen) {
	if ((unit = screen->curunit) != NULL) {
	    if (inside_area(unit->x, unit->y)) {
		update_cell_display(side, unit->x, unit->y, UPDATE_ALWAYS);
	    }
	    update_unit_info(screen);
	}
    }
}

void
update_event_display(Side *side, HistEvent *hevt, int rightnow)
{
}

void
update_fire_at_display(Side *side, Unit *unit, Unit *unit2, int m, int now)
{
    Screen *screen;

    if (!active_display(side))
      return;
    for_all_screens(screen) {
#if 0
	draw_fire_line(screen, unit, unit2, 0, 0);
#endif
    }
}

/* This is for animation of fire-into actions. */

void
update_fire_into_display(Side *side, Unit *unit, int x, int y, int z, int m,
			 int rightnow)
{
    Screen *screen;

    if (!active_display(side))
      return;
    for_all_screens(screen) {
#if 0
	draw_fire_line(screen, unit, NULL, x, y);
#endif
    }
}

/* Updates to clock need to be sure that display changes immediately. */

void
update_clock_display(Side *side, int rightnow)
{
}

void
update_message_display(Side *side, Side *sender, char *str, int rightnow)
{
    if (!active_display(side))
      return;

    notify(side, "From %s: \"%s\"",
	   (sender != NULL ? short_side_title(sender) : "<anon>"), str);
}

void
update_all_progress_displays(char *str, int s)
{
}

void
action_point(Side *side, int x, int y)
{
    Screen *screen;

    if (!active_display(side))
      return;
    if (!inside_area(x, y))
      return;

    for_all_screens(screen) {
	if (screen->follow_action && !in_middle(screen->map, x, y)) {
	    put_on_screen(screen->map, x, y);
	    screen->scrolled_to_unit = TRUE;
	}
    }
    put_on_screen(NULL, x, y);
}

void
flush_display_buffers(Side *side)
{
}

void
update_everything()
{
}

/* This is called by generic Unix crash-handling code, no need for
   us to do anything special. */

void
close_displays()
{
}

int
schedule_movie(Side *side, char *movie, ...)
{
#if 0
    va_list ap;
    int i;
    enum movie_type itype;

    if (!active_display(side))
      return FALSE;
    if (side->ui->numscheduled >= 10)
      return FALSE;
    memset(&(side->ui->movies[side->ui->numscheduled]), 0, sizeof(struct a_movie));
    side->ui->movies[side->ui->numscheduled].type = movie;
    itype = movie_null;
    if (strcmp(movie, "miss") == 0)
      itype = movie_miss;
    else if (strcmp(movie, "hit") == 0)
      itype = movie_hit;
    else if (strcmp(movie, "hit-short") == 0)
      itype = movie_hit_short;
    else if (strcmp(movie, "death") == 0)
      itype = movie_death;
    else if (strcmp(movie, "nuke") == 0)
      itype = movie_nuke;
    else if (strcmp(movie, "sound") == 0)
      itype = movie_sound;
    else if (strcmp(movie, "flash") == 0)
      itype = movie_flash;
    else
      /* Do nothing. */
      return FALSE;
    side->ui->movies[side->ui->numscheduled].itype = itype;
    va_start(ap, movie);
    for (i = 0; i < 5; ++i)
      side->ui->movies[side->ui->numscheduled].args[i] = va_arg(ap, int);
    va_end(ap);
    ++side->ui->numscheduled;
#endif
    return TRUE;
}

void
play_movies(SideMask sidemask)
{
#if 0
    int j, unitid, btype, dur, x, y;
    Screen *screen;
    Unit *unit;

    if (!active_display(dside))
      return;
    if (side_in_set(dside, sidemask)) {
	for (j = 0; j < dside->ui->numscheduled; ++j) {
	    btype = -1;
	    dur = 400;
	    switch (dside->ui->movies[j].itype) {
	      case movie_null:
		break;
	      case movie_miss:
		if (btype < 0)
		  btype = 0;
	      case movie_hit:
	      case movie_hit_short:
		if (btype < 0)
		  btype = 1;
		if (dside->ui->movies[j].itype == movie_hit_short)
		  dur = 100;
	      case movie_death:
		if (btype < 0)
		  btype = 2;
		unitid = dside->ui->movies[j].args[0];
		unit = find_unit(unitid);
		if (unit == NULL || !in_area(unit->x, unit->y))
		  continue;
		for_all_screens(screen) {
		    draw_unit_blast(screen, unit, btype, dur);
		}
		break;
	      case movie_nuke:
		x = dside->ui->movies[j].args[0];
		y = dside->ui->movies[j].args[1];
		for_all_screens(screen) {
		    draw_cell_blast(screen, x, y, 3, 800);
		}
		break;
	      case movie_flash:
		/* Flashing does the whole screen. */
		for_all_screens(screen) {
		    draw_cell_blast(screen, 0, 0, 10, 100);
		}
		break;
	      default:
		break;
	    }
	}
	dside->ui->numscheduled = 0;
    }
#endif
}

/* Beep the beeper! */

void
beep(void)
{
    printf("Beep!\n");
}

void
low_notify(Side *side, char *str)
{
    printf("To %d: %s\n", side->id, str);
}

extern void popup_game_dialog(void);

void
popup_game_dialog(void)
{
    draw_rect(mscreen, 100, 100, 300, 300, random_color(sscreen->surf));
}

void
init_redraws(void)
{
    Side *side2;
    Screen *screen;

    if (dside->ui != NULL) {
	/* The moment of truth - up to now output has been suppressed. */
	dside->ui->active = TRUE;
    }

    for_all_sides(side2) {
	update_side_display(dside, side2, TRUE);
    }
    update_turn_display(dside, TRUE);

    for_all_screens(screen)
      set_tool_cursor(screen);
}

#if 0
static void
ui_update_state(void)
{
    Obj *state = lispnil;

    push_binding(&state, intern_symbol("default-power"),
		 new_number(default_vp.power));
    push_binding(&state, intern_symbol("default-draw-grid"),
		 new_number(default_vp.draw_grid));
    push_binding(&state, intern_symbol("default-draw-coverage"),
		 new_number(default_vp.draw_cover));
    push_binding(&state, intern_symbol("default-draw-elevations"),
		 new_number(default_vp.draw_elevations));
    push_binding(&state, intern_symbol("default-draw-lighting"),
		 new_number(default_vp.draw_lighting));
    push_binding(&state, intern_symbol("default-draw-people"),
		 new_number(default_vp.draw_people));
    push_binding(&state, intern_symbol("default-draw-control"),
		 new_number(default_vp.draw_control));
    push_binding(&state, intern_symbol("default-draw-temperature"),
		 new_number(default_vp.draw_temperature));
    push_binding(&state, intern_symbol("default-draw-winds"),
		 new_number(default_vp.draw_winds));
    push_binding(&state, intern_symbol("default-draw-clouds"),
		 new_number(default_vp.draw_clouds));
    push_binding(&state, intern_symbol("default-draw-unit-names"),
		 new_number(default_vp.draw_names));
    push_binding(&state, intern_symbol("default-draw-feature-names"),
		 new_number(default_vp.draw_feature_names));
    push_binding(&state, intern_symbol("default-draw-feature-boundaries"),
		 new_number(default_vp.draw_feature_boundaries));
    push_binding(&state, intern_symbol("default-draw-meridians"),
		 new_number(default_vp.draw_meridians));
    push_binding(&state, intern_symbol("default-meridian-interval"),
		 new_number(default_vp.meridian_interval));
    push_binding(&state, intern_symbol("default-draw-ai"),
		 new_number(default_vp.draw_ai));
    push_binding(&state, intern_symbol("default-draw-terrain-images"),
		 new_number(default_draw_terrain_images));
    push_binding(&state, intern_symbol("default-draw-transitions"),
		 new_number(default_draw_transitions));
    push_binding(&state, intern_symbol("default-font-family"),
		 new_string(default_font_family));
    push_binding(&state, intern_symbol("default-font-size"),
		 new_number(default_font_size));
    push_binding(&state, intern_symbol("checkpoint-interval"),
		 new_number(checkpoint_interval));

    dside->uidata = replace_at_key(dside->uidata, "unix", state);
}
#endif

static void interp_unix_ui_data(Obj *uispec);

void
get_preferences(void)
{
    int startlineno = 0, endlineno = 0;
    Obj *uispec;
    FILE *fp;
#ifdef UNIX
    struct stat statbuf;
#endif

#ifdef UNIX
    if (!stat(preferences_filename(), &statbuf)) {
#endif
    if ((fp = fopen(preferences_filename(), "r")) != NULL) {
	uispec = read_form(fp, &startlineno, &endlineno);
	interp_unix_ui_data(uispec);
	fclose(fp);
    }
#ifdef UNIX
    }
    else {
        if ((fp = fopen(old_preferences_filename(), "r")) != NULL) {
            uispec = read_form(fp, &startlineno, &endlineno);
            interp_unix_ui_data(uispec);
            fclose(fp);
        }
    }
#endif
}

/* Given a list of preference specifications, decipher them and set
   appropriate C variables, also pass into tcl preferences code. */

static void
interp_unix_ui_data(Obj *uispec)
{
    int numval;
    char *strval = NULL;
    char *name;
    Obj *rest, *bdg;

    for_all_list(uispec, rest) {
	bdg = car(rest);
	if (!consp(bdg)) {
	    /* Don't complain out loud normally, confusing to users
	       because preferences are under Xconq and not user
	       control. */
	    Dprintf("Syntax error in preference binding?\n");
	    continue;
	}
	if (symbolp(car(bdg))) {
	    name = c_string(car(bdg));
	    strval = NULL;
	    numval = 0;
	    if (numberp(cadr(bdg))) {
		numval = c_number(cadr(bdg));
	    } else if (stringp(cadr(bdg))) {
		strval = c_string(cadr(bdg));
	    } else {
		Dprintf("Preference property `%s' not a number or string, setting to zero\n",
			name);
	    }
	    if (strcmp(name, "default-power") == 0) {
		default_vp.power = numval;
	    } else if (strcmp(name, "default-draw-grid") == 0) {
		default_vp.draw_grid = numval;
	    } else if (strcmp(name, "default-draw-coverage") == 0) {
		default_vp.draw_cover = numval;
	    } else if (strcmp(name, "default-draw-elevations") == 0) {
		default_vp.draw_cover = numval;
	    } else if (strcmp(name, "default-draw-lighting") == 0) {
		default_vp.draw_lighting = numval;
	    } else if (strcmp(name, "default-draw-people") == 0) {
		default_vp.draw_people = numval;
	    } else if (strcmp(name, "default-draw-control") == 0) {
		default_vp.draw_control = numval;
	    } else if (strcmp(name, "default-draw-temperature") == 0) {
		default_vp.draw_temperature = numval;
	    } else if (strcmp(name, "default-draw-winds") == 0) {
		default_vp.draw_winds = numval;
	    } else if (strcmp(name, "default-draw-clouds") == 0) {
		default_vp.draw_clouds = numval;
	    } else if (strcmp(name, "default-draw-unit-names") == 0) {
		default_vp.draw_names = numval;
	    } else if (strcmp(name, "default-draw-feature-names") == 0) {
		default_vp.draw_feature_names = numval;
	    } else if (strcmp(name, "default-draw-feature-boundaries") == 0) {
		default_vp.draw_feature_boundaries = numval;
	    } else if (strcmp(name, "default-draw-meridians") == 0) {
		default_vp.draw_meridians = numval;
	    } else if (strcmp(name, "default-meridian-interval") == 0) {
		default_vp.meridian_interval = numval;
	    } else if (strcmp(name, "default-draw-ai") == 0) {
		default_vp.draw_ai = numval;
	    } else if (strcmp(name, "default-draw-terrain-images") == 0) {
		default_draw_terrain_images = numval;
	    } else if (strcmp(name, "default-draw-transitions") == 0) {
		default_draw_transitions = numval;
	    } else if (strcmp(name, "default-font-family") == 0) {
		default_font_family = copy_string(strval);
	    } else if (strcmp(name, "default-font-size") == 0) {
		default_font_size = numval;
	    } else if (strcmp(name, "checkpoint-interval") == 0) {
		checkpoint_interval = numval;
	    } else {
		/* Note unrecognized properties, but don't bother the user. */
		Dprintf("Preference binding `%s' unrecognized\n", name);
	    }
	} else {
	    /* As with above comment. */
	    Dprintf("Syntax error in preference binding head?\n");
	}
    }
}

void
popup_help(Side *side, HelpNode *node)
{
}

HelpNode *cur_help_node;

#define NODESTACKSIZE 50

HelpNode **node_stack;

int node_stack_pos;

#if 0
static void describe_screen(int arg, char *key, TextBuffer *buf);

static void
describe_screen(int arg, char *key, TextBuffer *buf)
{
    tbcat(buf, "** In MOVE mode (crosshair cursor):\n");
    tbcat(buf, "The next unit that can do anything will be selected ");
    tbcat(buf, "automatically.  It is indicated by a moving marquee.\n");
    tbcat(buf, "Left- or right-click anywhere in the screen to move the selected unit there.\n");
    tbcat(buf, "When all units have moved, the turn ends and the next ");
    tbcat(buf, "turn starts immediately.\n");
    tbcat(buf, "\n");
    tbcat(buf, "** In SURVEY mode (magnifying glass cursor):\n");
    tbcat(buf, "Left-click on any of your units to make it the selected one.\n");
    tbcat(buf, "To move it, right-click anywhere in the screen to move there; ");
    tbcat(buf, "or use the 'm'ove command, or 'z' to switch ");
    tbcat(buf, "to move mode.\n");
    tbcat(buf, "The turn ends only when you do <ret> or pick the menu item ");
    tbcat(buf, "Side->End This Turn.\n");
    tbcat(buf, "\n");
    tbcat(buf, "** In both modes:\n");
    tbcat(buf, "If the destination is far away, the unit will take as many ");
    tbcat(buf, "turns as it needs to get there.\n");
    tbcat(buf, "If the unit is adjacent to an enemy unit, and you click ");
    tbcat(buf, "on the enemy, your unit will attack.\n");
    tbcat(buf, "\n");
    tbcat(buf, "Nearly all keyboard and menu commands work on only ");
    tbcat(buf, "the selected unit.\n");
    tbcat(buf, "\n");
}
#endif

/* Go through all the game's variants and set up appropriate tcl
   variables. */

#if 0
static void interpret_checkbox(Variant *var, int which, int n);
#endif

void
interpret_variants(void)
{
#if 0
    int i;
    char *vartypename;
    Variant *var;

    any_variants = FALSE;
    vary_world_size = vary_real_time = FALSE;
    /* Set default behavior for all the checkboxes. */
    for (i = 0; i < MAXCHECKBOXES; ++i) {
	eval_tcl_cmd("set varianttext(%d) Unused", i);
	eval_tcl_cmd("set variantstate(%d) disabled", i);
	eval_tcl_cmd("set varianthelp(%d) nothing", i);
    }
    if (mainmodule == NULL || mainmodule->variants == NULL)
      return;
    /* The first six checkboxes are assigned to common types of
       variants. */
    numcheckboxes = 6;
    for (i = 0; mainmodule->variants[i].id != lispnil; ++i) {
	var = &(mainmodule->variants[i]);
	any_variants = TRUE;
	vartypename = c_string(var->id);
	switch (keyword_code(vartypename)) {
	  case K_WORLD_SEEN:
	    interpret_checkbox(var, i, 0);
	    break;
	  case K_SEE_ALL:
	    interpret_checkbox(var, i, 1);
	    break;
	  case K_SEQUENTIAL:
	    interpret_checkbox(var, i, 2);
	    break;
	  case K_PEOPLE:
	    interpret_checkbox(var, i, 3);
	    break;
	  case K_ECONOMY:
	    interpret_checkbox(var, i, 4);
	    break;
	  case K_SUPPLY:
	    interpret_checkbox(var, i, 5);
	    break;
	  case K_WORLD_SIZE:
	    /* If the area is already set up, it's too late. */
	    /* (this seems wrong?) */
	    if (area.width > 0 || area.height > 0)
	      break;
	    vary_world_size = TRUE;
	    worldsizepos = i;
	    varrev[i] = -1;
	    break;
	  case K_REAL_TIME:
	    vary_real_time = TRUE;
	    realtimepos = i;
	    varrev[i] = -2;
	    break;
	  default:
	    if (numcheckboxes < MAXCHECKBOXES)
	      interpret_checkbox(var, i, numcheckboxes++);
	    else
	      init_warning("too many variants, can't set all of them");
	    break;
	}
	update_variant_setting(i);
    }
    /* Set flags that enable/disable subdialogs. */
    eval_tcl_cmd("set vary_world_size %d", vary_world_size);
    eval_tcl_cmd("set vary_real_time %d", vary_real_time);
#endif
}

#if 0
static void
interpret_checkbox(Variant *var, int which, int n)
{
    eval_tcl_cmd("set variantstate(%d) active", n);
    eval_tcl_cmd("set varianttext(%d) \"%s\"", n, var->name);
    eval_tcl_cmd("set varianthelp(%d) \"%s\"", n, var->help);
    checkboxpos[n] = which;
    varrev[which] = n;
}
#endif

static void
init_all_displays(void)
{
    int numdisplays;
    Side *side;

    numdisplays = 0;
    for_all_sides(side) {
	if (side_has_display(side)) {
	    if (side_has_local_display(side))
	      init_display();
	    ++numdisplays;
	}
    }
    if (numdisplays == 0) {
	if (use_stdio)
	  fprintf(stderr, "Must have at least one display to start.\n");
	exit(0);
    }
    open_screen();
}

/* Prompt for a type of a unit from player, maybe only allowing some
   types to be accepted.  Also allow specification of no unit type.
   We do this by scanning the vector, building a string of chars and a
   vector of unit types, so as to be able to screen back when done. */

int
ask_unit_type(Screen *screen, char *prompt, int *possibles,
	      void (*handler)(Screen *screen, int cancelled))
{
    int u, numtypes = 0;

    for_all_unit_types(u) {
	if (possibles == NULL || possibles[u]) {
	    screen->uvec[numtypes] = u;
	    screen->ustr[numtypes] = unitchars[u];
	    ++numtypes;
	} else {
	}
    }
    screen->ustr[numtypes] = '\0';
    if (numtypes > 1) {
	printf("ask_unit_type %s [%s]\n", prompt, screen->ustr);
	screen->modalhandler = handler;
    }
    return numtypes;
}

/* Do something with the char or unit type that the player entered. */

int
grok_unit_type(Screen *screen, int *typep)
{
    int i;

    *typep = NONUTYPE;
    if (screen->inptype != NONUTYPE) {
	/* Collect the type saved from a mouse click or other input. */
	*typep = screen->inptype;
	/* Reset so doesn't affect subsequent unit type queries. */
	screen->inptype = NONUTYPE;
    } else if (screen->inpch != '\0') {
	if (screen->inpch == '?') {
	    help_unit_type(screen);
	    return FALSE;
	}
	i = iindex(screen->inpch, screen->ustr);
	if (i >= 0) {
	    *typep = screen->uvec[i];
	} else {
	    notify(dside, "Must type a unit type char from the list, or <esc>");
	    return FALSE;
	}
    } else {
	notify(dside, "weird");
	return FALSE;
    }
    /* Make the unit type string be empty. */
    screen->ustr[0] = '\0';
    return TRUE;
}

void
cancel_unit_type(Screen *screen)
{
}

static void
help_unit_type(Screen *screen)
{
    int i;
    char helpbuf[BUFSIZE];

    helpbuf[0] = '\0';
    for (i = 0; screen->ustr[i] != '\0'; ++i) {
	/* Put out several types on each line. */
	if (i % 4 == 0) {
	    if (i > 0) {
		notify(dside, "%s", helpbuf);
	    }
	    /* Indent each line a bit (also avoids notify's
	       auto-capitalization). */
	    strcpy(helpbuf, "  ");
	}
	tprintf(helpbuf, "%c %s, ", screen->ustr[i], u_type_name(screen->uvec[i]));
    }
    /* Add an extra helpful comment, then dump any leftovers. */
    tprintf(helpbuf, "? for this help info"); 
    notify(dside, "%s", helpbuf);
}

int
ask_terrain_type(Screen *screen, char *prompt, int *possibles,
		 void (*handler)(Screen *screen, int cancelled))
{
    int numtypes = 0, t;

    for_all_terrain_types(t) {
	if (possibles == NULL || possibles[t]) {
	    screen->tvec[numtypes] = t;
	    screen->tstr[numtypes] = terrchars[t];
	    ++numtypes;
	}
    }
    screen->tstr[numtypes] = '\0';
    if (numtypes > 1) {
	printf("ask_terrain_type %s [%s]\n", prompt, screen->tstr);
	screen->modalhandler = handler;
    }
    return numtypes;
}

/* Do something with the char or terrain type that the player entered. */

int
grok_terrain_type(Screen *screen, int *typep)
{
    int i;

    *typep = NONTTYPE;
    if (screen->inpch == '?') {
	help_terrain_type(screen);
	return FALSE;
    }
    i = iindex(screen->inpch, screen->tstr);
    if (i >= 0) {
	*typep = screen->tvec[i];
	return TRUE;
    } else {
	notify(dside, "Must type a terrain type char or <esc>");
	return FALSE;
    }
}

static void
help_terrain_type(Screen *screen)
{
    int i;
    char helpbuf[BUFSIZE];

    for (i = 0; screen->tstr[i] != '\0'; ++i) {
	/* Put out several types on each line. */
	if (i % 4 == 0) {
	    if (i > 0) {
		notify(dside, "%s", helpbuf);
	    }
	    /* Indent each line a bit (also avoids confusion due to
	       notify's capitalization). */
	    strcpy(helpbuf, "  ");
	}
	tprintf(helpbuf, "%c %s, ", screen->tstr[i], t_type_name(screen->tvec[i]));
    }
    /* Add an extra helpful comment, then dump any leftovers. */
    tprintf(helpbuf, "? for this help info"); 
    notify(dside, "%s", helpbuf);
}

/* User is asked to pick a position on screen.  This will iterate until the
   space bar designates the final position. */

/* (should change the cursor temporarily) */

void
ask_position(Screen *screen, char *prompt,
	     void (*handler)(Screen *screen, int cancel))
{
    if (mouseover_buf == NULL)
      mouseover_buf = (char *)xmalloc(BUFSIZE);
#if (0)
    notify(dside, "%s [click to set]", prompt);
#endif
    snprintf(mouseover_buf, BUFSIZE, "%s [click to set]", prompt);
    screen->answer[0] = '\0';
    screen->mouseover_panel_locked = TRUE;
    screen->modalhandler = handler;
    update_mouseover(screen);
    update_screen(screen, TRUE);
}

int
grok_position(Screen *screen, int *xp, int *yp, Unit **unitp)
{
    if (in_area(screen->inpx, screen->inpy)) {
	*xp = screen->inpx;  *yp = screen->inpy;
	if (unitp != NULL)
	  *unitp = screen->inpunit;
	return TRUE;
    } else {
	/* Make any possible usage attempts fail. */
	*xp = *yp = -1;
	if (unitp != NULL)
	  *unitp = NULL;
	return FALSE;
    }
}

/* Prompt for a yes/no answer with a settable default. */

void
ask_bool(Screen *screen, char *question, int dflt,
	 void (*handler)(Screen *screen, int cancelled))
{
    if (mouseover_buf == NULL)
      mouseover_buf = (char *)xmalloc(BUFSIZE);
    snprintf(mouseover_buf, BUFSIZE, "%s %c", question, (dflt ? 'y' : 'n'));
    screen->answer[0] = '\0';
    screen->tmpint = dflt;
    screen->mouseover_panel_locked = TRUE;
    screen->modalhandler = handler;
    update_mouseover(screen);
    update_screen(screen, TRUE);
}

/* Figure out what the answer actually is, keeping the default in mind. */

int
grok_bool(Screen *screen)
{
    int dflt = screen->tmpint;
    char ch = screen->inpch;

    if (dflt ? (lowercase(ch) == 'n') : (lowercase(ch) == 'y'))
      dflt = !dflt;
    return dflt;
}

/* Read a string from the prompt window.  Deletion is allowed, and a
   text cursor (an underscore) is displayed. */

void
ask_string(Screen *screen, char *prompt, char *dflt,
	   void (*handler)(Screen *screen, int cancelled))
{
    if (mouseover_buf == NULL)
      mouseover_buf = (char *)xmalloc(BUFSIZE);
    /* Default must be non-NULL. */
    if (dflt == NULL)
      dflt = "";
    sprintf(screen->answer, "%s", dflt);
    snprintf(mouseover_buf, BUFSIZE, "%s %s", prompt, screen->answer);
    screen->mouseover_panel_locked = TRUE;
    screen->modalhandler = handler;
    update_mouseover(screen);
    update_screen(screen, TRUE);
}

/* Dig a character from the input and add it into the string.
   Keep returning FALSE until we get something, then make a copy
   of the result string and return TRUE. */

int
grok_string(Screen *screen, char **strp)
{
    char ch = screen->inpch;
    int len;

    /* Note that we can't use '?' for help here, might be a part of
       the string to be returned. */
    if (ch == '\r' || ch == '\n') {
	*strp = copy_string(screen->answer);
	return TRUE;
    } else {
	len = strlen(screen->answer);
	if (ch == BACKSPACE_CHAR || ch == DELETE_CHAR) {
	    if (len > 0)
	      --len;
	} else {
	    screen->answer[len++] = ch;
	}
	screen->answer[len] = '\0';
	return FALSE;
    }
}

void
ask_side(Screen *screen, char *prompt, Side *dfltside,
	 void (*handler)(Screen *screen, int cancelled))
{
    char *dfltstr;

    if (mouseover_buf == NULL)
      mouseover_buf = (char *)xmalloc(BUFSIZE);
    dfltstr = (char *)(dfltside == NULL ? "nobody" : side_name(dfltside));
    strcpy(screen->answer, dfltstr);
    snprintf(mouseover_buf, BUFSIZE, "%s %s", prompt, screen->answer);
    screen->mouseover_panel_locked = TRUE;
    screen->modalhandler = handler;
    update_mouseover(screen);
    update_screen(screen, TRUE);
}

int
grok_side(Screen *screen, Side **side2p)
{
    char ch = screen->inpch;
    int len, sidenum;
    Side *side3;

    *side2p = NULL;
    if (ch == '?') {
	notify(dside, "  Type in the name or number of a side,");
	notify(dside, "  or \"nobody\" to answer with no side.");
	/* A flag to suppress complaints... */
	*side2p = dside;
	return FALSE;
    } else if (ch == '\r' || ch == '\n') {
	if (empty_string(screen->answer)
	    || strcmp(screen->answer, "nobody") == 0) {
#if 0 /* experimental */
	    *side2p = indepside;
#endif
	    return TRUE;
	}
	if (isdigit(*(screen->answer))) {
	    sidenum = strtol(screen->answer, NULL, 10);
	    side3 = side_n(sidenum);
	    if (side3) {
		*side2p = side3;
		return TRUE;
	    }
	    beep();
	    notify(dside, "\"%s\" is not a valid side number", screen->answer);
	    return FALSE;
	}
	for_all_sides(side3) {
	    if ((!empty_string(side3->name)
		 && strstr(side3->name, screen->answer))
		|| (!empty_string(side3->adjective)
		    && strstr(side3->adjective, screen->answer))
		|| (!empty_string(side3->pluralnoun)
		    && strstr(side3->pluralnoun, screen->answer))
		|| (!empty_string(side3->noun)
		    && strstr(side3->noun, screen->answer))) {
		*side2p = side3;
		return TRUE;
	    }
	}
	beep();
	notify(dside, "\"%s\" is not recognized as a side name", screen->answer);
	return FALSE;
    } else {
	len = strlen(screen->answer);
	if (ch == BACKSPACE_CHAR || ch == DELETE_CHAR) {
	    if (len > 0)
	      --len;
	} else {
	    screen->answer[len++] = ch;
	}
	screen->answer[len] = '\0';
	return FALSE;
    }
}

void
add_remote_locally(int rid, char *str)
{
#if 0
    eval_tcl_cmd("set master_rid %d", master_rid);
    eval_tcl_cmd("add_program %d %d {%s}", my_rid, rid, str);
#endif
}

void
send_chat(int rid, char *str)
{
}

/* Given a set of numbers that represent the current desired value of
   a given variant, update the tcl values for that variant (which has
   the side effect of updating the displayed widgets). */

static void
update_variant_setting(int which)
{
#if 0
    int rev = varrev[which];
    Variant *var = &(mainmodule->variants[which]);

    if (rev == -1) {
	eval_tcl_cmd("set new_width %d", var->newvalues[0]);
	eval_tcl_cmd("set new_height %d", var->newvalues[1]);
	eval_tcl_cmd("set new_circumference %d", var->newvalues[2]);
    } else if (rev == -2) {
	eval_tcl_cmd("set new_time_for_game %d", var->newvalues[0]);
	eval_tcl_cmd("set new_time_per_side %d", var->newvalues[1]);
	eval_tcl_cmd("set new_time_per_turn %d", var->newvalues[2]);
    } else {
	eval_tcl_cmd("set variantvalue(%d) %d", rev, var->newvalues[0]);
    }
#endif
}

/* Update the display of the given side/player assignment. */

static void
update_assignment(int n)
{
}

/* Reading is usually pretty fast, so don't do anything special here. */

void
announce_read_progress(void)
{
}

int announced = FALSE;

char *announcemsg = NULL;

/* Announce the start of a time-consuming computation. */

void
announce_lengthy_process(char *msg)
{
    n_seconds_elapsed(0);
    announcemsg = copy_string(msg);
    if (announcemsg && use_stdio) {
	printf("%s;", announcemsg);
	free(announcemsg);
	announcemsg = NULL;
	fflush(stdout);
	announced = TRUE;
    }
}

/* Announce the making of progress on the computation. */

void
announce_progress(int percentdone)
{
    if (n_seconds_elapsed(2) && use_stdio) {
	printf(" %d%%,", percentdone);
	fflush(stdout);
	announced = TRUE;
    }
}

/* Announce the end of the time-consuming computation. */

void
finish_lengthy_process(void)
{
    if (announced && use_stdio) {
	printf(" done.\n");
	announced = FALSE;
    }
}

/* An init error needs to have the command re-run. */

void
low_init_error(char *str)
{
    if (use_stdio) {
	fprintf(stderr, "Error: %s.\n", str);
	fflush(stderr);
    } else {
    }
    exit(1);
}

/* A warning just gets displayed, no other action is taken. */

void
low_init_warning(char *str)
{
    if (use_stdio) {
	fprintf(stderr, "Warning: %s.\n", str);
	fflush(stderr);
    } else {
    }
}

/* Run errors are fatal, so we try to save state and get out. */

void
low_run_error(char *str)
{
    if (use_stdio) {
	fprintf(stderr, "Error: %s.\n", str);
	fflush(stderr);
	fprintf(stderr, "Saving the game...");
	close_displays();
	write_entire_game_state(saved_game_filename());
	fprintf(stderr, " done.\n");
	exit(1);
    }
    error_popped_up = TRUE;
}

/* Runtime warnings are for when it's important to bug the players,
   usually a problem with Xconq or a game design. */

void
low_run_warning(char *str)
{
    /* Dump to console, just for the record. */
    if (use_stdio) {
	fprintf(stderr, "Warning: %s.\n", str);
	fflush(stderr);
    }
    error_popped_up = TRUE;
}

/* This should be called before any sort of normal exit. */

void
exit_xconq(void)
{
    if (numremotes > 1 && !ok_to_exit) {
	/* Let the master know we're getting out. */
	send_quit();
    }
    close_displays();
    /* Toggle on the OS cursor. */
    SDL_ShowCursor(1);
    SDL_Quit();
    exit(0);
}

/* Run all the steps that bring a game up once all the players and
   sides have been decided on. */

int
launch_game(void)
{
    Player *player;

    /* Give all unassigned players to the master program. This code, taken
       from macinit.c, finally solved the ai problem in unix network games. */
    if (numremotes > 0) {
	for_all_players(player) {
	    if (player->rid == 0)
		player->rid = master_rid;
	}
    }
    /* Tell all the other programs it's time to launch too. */
    start_game_ready_stage();
    /* (should resync RNG just in case?) */
    /* Do the time-consuming part of setup calculations. */
    calculate_globals();
    run_synth_methods();
    final_init();
    assign_players_to_sides();
    place_legends(dside);
    /* Get the displays set up, but don't draw anything yet. */
    init_all_displays();
    /* Now bring up the init data on each display. */
    init_redraws();
    /* Set up the signal handlers. */
    init_signal_handlers();
    /* (should notify players of all the game options in effect) */
    notify_instructions();
    /* Do first set of turn calcs, but let the host lead the way. */
    if (my_rid == 0 || (my_rid > 0 && my_rid == master_rid)) {
	net_run_game(0);
    }
    return TRUE;
}

#if (0) /* Might be able to recycle this for side research. */

/* Run modal handler for unit research. */

void
unit_research_dialog(Unit *unit)
{
#if (0)
    auto_pick_unit_research(unit);
#endif
#if (0)
    sscreen->unit_action_panel_visible = TRUE;
    set_current_unit(sscreen, unit);
    sscreen->inptype = NONATYPE;
    update_unit_info(sscreen);
    sscreen->modalhandler = unit_research_fn;
    redraw_screen(sscreen);
    /* Do not return until a choice has been made. */
    while (sscreen->modalhandler) {
	run_ui_idler();
	SDL_Delay(10);	/* Keep CPU usage from getting too heavy. */
    }
    reset_screen_input(sscreen);
#endif
}

#endif

void
unit_research_dialog(Unit *unit) {}

void
print_form(Obj *form)
{
    print_form_and_value(stdout, form);
}

void
end_printing_forms(void)
{
}

void
place_legends(Side *side)
{
    int nf = numfeatures;

    if (!features_defined() || nf <= 0)
      return;
    if (side != dside)
      return;
    if (ui->legends == NULL)
      ui->legends = (Legend *) xmalloc((nf + 1) * sizeof(Legend));
    place_feature_legends(ui->legends, nf, side, 0, 1);
}

/* Choose the cursor to display.  If the screen is in a special
   interaction mode, use the cursor for that. */

void
set_tool_cursor(Screen *screen)
{
    SDLCursor *cursor;

    if (!use_cursors)
      return;

    cursor = ui->cursors[screen->mode];
    if (screen->tmp_mode != no_tmp_mode)
      cursor = ui->cursors[screen->tmp_mode];
    /* Autoscroll overrides other modes. */
    if (screen->scroll_mode[0] != no_scroll_mode)
      cursor = ui->cursors[screen->scroll_mode[0]];
    if (screen->scroll_mode[1] != no_scroll_mode)
      cursor = ui->cursors[screen->scroll_mode[1]];
    if (cursor == NULL)
      cursor = generic_cursor;
    screen->cursor = cursor;
}

void
set_show_all(Screen *screen, int flag)
{
    assert_error(screen && screen->map && screen->map->main_vp, 
		 "Attempted to access an incomplete screen definition");
    screen->map->main_vp->show_all = flag;
}
