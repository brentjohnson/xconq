/* Main program for running Xconq under tcltk on the Macintosh.
   Copyright (C) 2002 Hans Ronne.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "conq.h"
#include "kpublic.h"
#include "cmdline.h"
#include "tcltk/tkconq.h"

#if __profile__
#include <Profiler.h>
#endif

#ifndef p2c
#define p2c(PSTR,BUF)  \
  strncpy(BUF, ((char *) (PSTR) + 1), PSTR[0]);  \
  BUF[PSTR[0]] = '\0';
#endif

/* Tells strmgetc in lisp.c to do on-the-fly conversion to high
ascii mac char codes. This is not needed by MacTcl. */

int use_mac_charcodes = FALSE;

int blinking_curunit = TRUE;

extern int use_stdio;
extern int use_clip_mask;

extern short initial_vrefnum;
extern long initial_dirid;

extern int launch_game(void);
extern void ui_mainloop(void);
extern void initial_ui_init(void);

static int MacintoshInit _ANSI_ARGS_((void));
static void init_ae(void);
static Boolean missed_any_parameters(const AppleEvent *message);
static pascal OSErr do_ae_open_application(const AppleEvent *message, AppleEvent *reply, long refcon);
static pascal OSErr do_ae_open_documents(const AppleEvent *message, AppleEvent *reply, long refcon);
static pascal OSErr do_ae_print_documents(const AppleEvent *message, AppleEvent *reply, long refcon);
static pascal OSErr do_ae_quit_application(const AppleEvent *message, AppleEvent *reply, long refcon);
static int open_game_from_fsspec(FSSpec *fsspec);
static void mac_exit(int status);


int
main(int argc, char *argv[])
{
    if (MacintoshInit()  != TCL_OK) {
	Tcl_Exit(1);
    }
    /* Capture the current vrefnum. */
    HGetVol(NULL, &initial_vrefnum, &initial_dirid);

	/* Init profiling if asked to. */
#if __profile__
	ProfilerInit(collectDetailed, bestTimeBase, 1250, 36);
	ProfilerSetStatus(true);
#endif

    init_ae();

    use_stdio = FALSE;
    
    /* Tweak behavior of tcl/tk image handling. */
    use_clip_mask = TRUE;

    /* Put the game in a defined state. */
    clear_game_modules();
    init_data_structures();
    init_library_path(NULL);

    initial_ui_init();
    /* interp is defined only after initial_ui_init. */
    TkMacInitMenus(interp);

#ifdef MACOSX
	/* Copied from macconq.c. Not yet enabled (or 
	tested) since MACOSX is false here. */
    if (argc > 1) {
	parse_command_line(argc, argv, general_options);
	parse_command_line(argc, argv, variant_options);
	parse_command_line(argc, argv, player_options);
	parse_command_line(argc, argv, leftover_options);

	/* We're probably typing in a command line, use stdout. */
	printf("\n              Welcome to X11 Xconq version %s\n\n", version_string());
	printf("%s", license_string());
	print_any_news();

	start_game_load_stage();
	load_all_modules();
	check_game_validity();
	if (my_rid > 0 && my_rid == master_rid) {
		broadcast_game_module();
	}
	set_variants_from_options();
	start_player_pre_setup_stage();
	set_players_from_options();
	start_player_setup_stage();
	launch_game();
    }
#endif

    /* Needed under the OSX Classic environment. */
    popup_game_dialog();

    /* Go into the main play loop. */
    ui_mainloop();

    /* Humor the compiler. */
    return 0;
}

static int
MacintoshInit()
{
    MaxApplZone();
    MoreMasters();
    MoreMasters();
    MoreMasters();
    MoreMasters();

    tcl_macQdPtr = &qd;

     if (TkMacHaveAppearance()) {
         RegisterAppearanceClient();
     }

    InitGraf(&tcl_macQdPtr->thePort);
    InitFonts();

    if (TkMacHaveAppearance() >= 0x110) {
	InitFloatingWindows();
    } else {
	InitWindows();
    }

    InitMenus();
    InitDialogs((long) NULL);		
    InitCursor();

    FlushEvents(everyEvent, 0);
    SetEventMask(everyEvent);
    Tcl_MacSetEventProc(TkMacConvertEvent);

    return TCL_OK;
}

/* Minimalistic Apple Events handling. */

void
init_ae()
{
	AEInstallEventHandler(kCoreEventClass, kAEOpenApplication,
					NewAEEventHandlerProc(do_ae_open_application), 0L, false);
	AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments,
					NewAEEventHandlerProc(do_ae_open_documents), 0L, false);
	AEInstallEventHandler(kCoreEventClass, kAEPrintDocuments,
					NewAEEventHandlerProc(do_ae_print_documents), 0L, false);
	AEInstallEventHandler(kCoreEventClass, kAEQuitApplication,
					NewAEEventHandlerProc(do_ae_quit_application), 0L, false);

}

/* Used to check for any unread required parameters. Returns true if we
   missed at least one. */

Boolean
missed_any_parameters(const AppleEvent *message)
{
	OSErr err;
	DescType ignoredActualType;
	AEKeyword missedKeyword;
	Size ignoredActualSize;
	EventRecord event;

	err = AEGetAttributePtr(message, keyMissedKeywordAttr, typeKeyword, &ignoredActualType,
						    (Ptr) &missedKeyword, sizeof(missedKeyword), &ignoredActualSize);
	/* No error means that we found some unused parameters. */
	if (err == noErr) {
		event.message = *(long *) &ignoredActualType;
		event.where = *(Point *) &missedKeyword;
		err = errAEEventNotHandled;
	}
	/* errAEDescNotFound means that there are no more parameters.  If we get
	   an error code other than that, flag it. */
	return (err != errAEDescNotFound);
}

static pascal OSErr
do_ae_open_application(const AppleEvent *message, AppleEvent *reply, long refcon)
{
#pragma unused (message, refcon)
	OSErr err;

	popup_game_dialog();
	AEPutParamPtr(reply, keyErrorNumber, typeShortInteger, (Ptr) &err, sizeof(short));
	return err;
}

/* Called when we receive an AppleEvent with an ID of "kAEOpenDocuments".
   This routine gets the direct parameter, parses it up into little FSSpecs,
   and opens the first indicated file.  It also shows the technique to be used in
   determining if you are doing everything the AppleEvent record is telling
   you.  Parameters can be divided up into two groups: required and optional.
   Before executing an event, you must make sure that you've read all the
   required events.  This is done by making an "any more?" call to the
   AppleEvent manager. */

static pascal OSErr
do_ae_open_documents(const AppleEvent *message, AppleEvent *reply, long refcon)
{
#pragma unused (refcon)

	OSErr err, err2;
	AEDesc theDesc;
	FSSpec fsspec;
	short loop;
	long numFilesToOpen;
	AEKeyword ignoredKeyWord;
	DescType ignoredType;
	Size ignoredSize;

	theDesc.dataHandle = nil;

	err = AEGetParamDesc(message, keyDirectObject, typeAEList, &theDesc);
	if (err)
	  return err;
	if (!missed_any_parameters(message)) {
		/* Got all the parameters we need.  Now, go through the direct object,
		   see what type it is, and parse it up. */
		err = AECountItems(&theDesc, &numFilesToOpen);
		if (!err) {
			/* We have numFilesToOpen that need opening, as either a window
			   or to be printed.  Go to it... */
			for (loop = 1; ((loop <= numFilesToOpen) && (!err)); ++loop) {
				err = AEGetNthPtr(&theDesc, loop, typeFSS, &ignoredKeyWord, &ignoredType,
								  (Ptr) &fsspec, sizeof(fsspec), &ignoredSize);
				if (err)
				  break;
				if (open_game_from_fsspec(&fsspec))
				  break;
			}
		}
	}
	err2 = AEDisposeDesc(&theDesc);
	err = (err ? err : err2);
	AEPutParamPtr(reply, keyErrorNumber, typeShortInteger, (Ptr) &err, sizeof(short));
	return err;
}

static pascal OSErr
do_ae_print_documents(const AppleEvent *message, AppleEvent *reply, long refcon)
{
	OSErr err;

	AEPutParamPtr(reply, keyErrorNumber, typeShortInteger, (Ptr) &err, sizeof(short));
	return err;
}

static pascal OSErr
do_ae_quit_application(const AppleEvent *message, AppleEvent *reply, long refcon)
{
	OSErr err = noErr;

	mac_exit(TRUE);
	AEPutParamPtr(reply, keyErrorNumber, typeShortInteger, (Ptr) &err, sizeof(short));
	return noErr;
}

/* Open a game file using an FSSpec, which will typically come from an AppleEvent. */

int
open_game_from_fsspec(FSSpec *fsspec)
{
	char modulename[256];
	Module *module;

	/* Set the current volume/dir to be where the file is, so path-less
	   fopen() in module loading below will find the file.  Note that this
	   won't work for loading multiple modules from multiple non-library
	   locations, but this dialog can only load one file at a time anyway. */
	HSetVol(NULL, fsspec->vRefNum, fsspec->parID);
	clear_game_modules();
	/* Build a module with the given filename. */
	module = get_game_module(NULL);
	p2c(fsspec->name, modulename);
	module->filename = copy_string(modulename);
	/* Load the module. */
	mainmodule = module;
	load_game_module(module, TRUE);
	if (!module->loaded) {
		init_warning("Could not load a module from %s", module->filename);
		return FALSE;
	}
	/* If the loaded module has bugs, we will get alerts here. */
	check_game_validity();
	/* Let the master broadcast_game_module. */
    	if (my_rid > 0 && my_rid == master_rid) {
		broadcast_game_module();	
	}
	/* Only solo players and the master are allowed to do this. */
	if (my_rid == 0 || (my_rid > 0 && my_rid == master_rid)) {
   	 	start_variant_setup_stage();
	    	eval_tcl_cmd("popup_variants_dialog");
	}
	return TRUE;
}

void
mac_exit(int status)
{

#if __profile__
	/* Dump any unsaved profiler output. */
	ProfilerDump("\pXconq.Profile");
	ProfilerTerm();
#endif
	ExitToShell();
}

/* These three functions are not defined in the SDL sources,
so they must be provided here. */

Player *
add_default_player()
{
	Player *player = add_player();
	
	player->displayname = "MacTCL";
	return player;
}

void
make_default_player_spec(void)
{
	default_player_spec = "MacTCL";
}

