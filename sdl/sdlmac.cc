/* Main program for running Xconq under SDL on the Mac.
   Copyright (C) 2001 Hans Ronne.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "conq.h"
#include "kpublic.h"
#include "cmdline.h"
#include "sdlconq.h"
#include "sdlpreconq.h"

extern void interpret_variants(void);
extern int launch_game(void);

/* Tells strmgetc in lisp.c to do on-the-fly conversion to high
ascii mac char codes. This does not affect the map font (font.bmp)
which lacks high ascii characters, but the console output is more
correct if conversion is on. */

int use_mac_charcodes = TRUE;

int using_sdl;

extern short initial_vrefnum;
extern long initial_dirid;

#include <SegLoad.h>	/* ExitToShell. */

#if __profile__
#include <Profiler.h>
#endif

#define XconqSignature 'XCNQ'

#ifndef p2c
#define p2c(PSTR,BUF)  \
  strncpy(BUF, ((char *) (PSTR) + 1), PSTR[0]);  \
  BUF[PSTR[0]] = '\0';
#endif

WindowPtr sdl_console = NULL;
WindowPtr sdl_window = NULL;

static void init_ae(void);
static void start_game(void);
static void do_ae_open_document(const AppleEvent *message);
static void open_game(Str255 name, int vrefnum);
static OSStatus MissingParameterCheck(const AppleEvent *inputEvent);
static pascal OSErr DoOpenApp(const AppleEvent *inputEvent, AppleEvent *outputEvent, long handlerRefCon);
static pascal OSErr DoQuitApp(const AppleEvent *inputEvent, AppleEvent *outputEvent, long handlerRefCon);
static pascal OSErr DoOpenDocument(const AppleEvent *inputEvent, AppleEvent *outputEvent, long handlerRefCon);

/* Note: holding down the command key on launch brings up the
command line dialog for SDL apps on the mac. In can be used both 
to feed xconq a command line and to pick the video driver. The line
arguments and settings are saved in a System:Preferences file that
is used also when the command line dialog is not invoked. */

int
main(int argc, char *argv[])
{
	EventRecord event;
	Boolean gotevent;

	/* Init various things. */
	MaxApplZone();
	InitGraf(&qd.thePort);
	FlushEvents(everyEvent, 0);
	InitWindows();
	init_ae();

	/* Capture the current vrefnum. */
	HGetVol(NULL, &initial_vrefnum, &initial_dirid);

	/* Init profiling if asked to. */
#if __profile__
	ProfilerInit(collectDetailed, bestTimeBase, 1250, 36);
	ProfilerSetStatus(false);
#endif

	/* Put the game in a defined state. */
	clear_game_modules();
	init_data_structures();
	init_library_path(NULL);

	/* Parse the command line if used. */
	parse_command_line(argc, argv, general_options);
	parse_command_line(argc, argv, variant_options);
	parse_command_line(argc, argv, player_options);
	parse_command_line(argc, argv, leftover_options);

	while (1) {
		/* Find the high level event that launched xconq. */
		gotevent = WaitNextEvent(everyEvent, &event, 0L, 0);
		if (gotevent && (&event)->what == kHighLevelEvent) {
			    AEProcessAppleEvent(&event);	
		}
	}
}

void
start_game()
{
	start_game_load_stage();
	load_all_modules();
	check_game_validity();
	if (my_rid > 0 && my_rid == master_rid) {
	    broadcast_game_module();
	}
	interpret_variants();
	set_variants_from_options();
	start_player_pre_setup_stage();
	set_players_from_options();
	start_player_setup_stage();
	initial_ui_init();
	launch_game();
	
	/* Hide the SIOUX console behind the map, set global 
	pointers to both windows, and redraw the map. Hide
	the menu bar in fullscreen mode. */
	sdl_console = FrontWindow();
	/* If we don't have a console this has no effect since
	the main window then is the only app window. */
	SendBehind(FrontWindow(), 0);
	/* Get a pointer to the main window. */
	sdl_window = FrontWindow();
	if (fullscreen)
	    HideMenuBar();
	redraw_screen(sscreen);

#if __profile__
	ProfilerSetStatus(true);
#endif

	ui_mainloop();
}

static OSStatus	
MissingParameterCheck(const AppleEvent *inputEvent)
{
	OSStatus	anErr;
	AEKeyword	missingKeyword;
	DescType	ignoredActualType;
	Size		ignoredActualSize;
	
	anErr = AEGetAttributePtr(inputEvent, keyMissedKeywordAttr, typeWildCard,
						&ignoredActualType, (Ptr) &missingKeyword, 
						sizeof(AEKeyword), &ignoredActualSize);
	if (anErr == noErr) {
		anErr = errAEParamMissed;
	} else {
		if (anErr == errAEDescNotFound)
			anErr = noErr;			
	} return anErr;	
}

static pascal OSErr 
DoOpenApp(const AppleEvent *inputEvent, AppleEvent *outputEvent, long handlerRefCon)
{
#pragma unused (outputEvent, handlerRefCon)

	start_game();
	return(MissingParameterCheck(inputEvent));
}

static pascal OSErr	
DoQuitApp(const AppleEvent *inputEvent, AppleEvent *outputEvent, long handlerRefCon)
{
#pragma unused (outputEvent, handlerRefCon)

	mac_exit(true);
	return(MissingParameterCheck(inputEvent));
}

static pascal OSErr	
DoOpenDocument(const AppleEvent *inputEvent, AppleEvent *outputEvent, long handlerRefCon)
{
#pragma unused (outputEvent, handlerRefCon)

	do_ae_open_document(inputEvent);
	return(MissingParameterCheck(inputEvent));
}

/* Minimalistic Apple Events handling. */

void
init_ae()
{
	AEInstallEventHandler(kCoreEventClass, kAEOpenApplication,
					NewAEEventHandlerProc(DoOpenApp), 0L, false);
	AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments,
					NewAEEventHandlerProc(DoOpenDocument), 0L, false);
	AEInstallEventHandler(kCoreEventClass, kAEPrintDocuments,
					NewAEEventHandlerProc(DoQuitApp), 0L, false);
	AEInstallEventHandler(kCoreEventClass, kAEQuitApplication,
					NewAEEventHandlerProc(DoQuitApp), 0L, false);

}

/* We never want to open more than one document. */

void
do_ae_open_document(const AppleEvent *message)
{
	AEKeyword ignoredKeyWord;
	DescType ignoredType;
	Size ignoredSize;
	AEDesc theDesc;
	FSSpec fsspec;
	short vrefnum;

	AEGetParamDesc(message, keyDirectObject, typeAEList, &theDesc);
	AEGetNthPtr(&theDesc, 1, typeFSS, &ignoredKeyWord, &ignoredType,
					   (Ptr) &fsspec, sizeof(fsspec), &ignoredSize);
	OpenWD((&fsspec)->vRefNum, (&fsspec)->parID, XconqSignature, &vrefnum);
	open_game((&fsspec)->name, vrefnum);
}

void
open_game(Str255 name, int vrefnum)
{
	char modulename[256];

	SetVol(nil, vrefnum);
	clear_game_modules();
	/* Build a module with the given filename. */
	mainmodule = get_game_module(NULL);
	p2c(name, modulename);
	mainmodule->filename = copy_string(modulename);
	start_game();
}

void
mac_exit(int status)
{

#if __profile__
	/* Dump any unsaved profiler output. */
	ProfilerDump("\pXconq.Profile");
	ProfilerTerm();
#endif
	/* Restore the menu bar. */
	if (fullscreen)
	    ShowMenuBar();
	ExitToShell();
}

/* These three functions are not defined in the SDL sources,
so they must be provided here. */

Player *
add_default_player()
{
	Player *player = add_player();
	
	player->displayname = "MacSDL";
	return player;
}

void
make_default_player_spec(void)
{
	default_player_spec = "MacSDL";
}
