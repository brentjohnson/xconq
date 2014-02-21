/* Main program for running imfapp under tcltk on the Macintosh.
   Copyright (C) 2003 Hans Ronne.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#ifdef __cplusplus
extern "C" {
#endif

/* For tcltk 8.4 source compatibility. */
#define USE_NON_CONST

#include <tclMacCommonPch.h>
#include <tclMac.h>			/* Includes tcl.h. */
#include <tkMac.h>			/* Includes tk.h. */

#ifdef __cplusplus
}
#endif

#include "config.h"
#include "misc.h"
#include "lisp.h"
#include "imf.h"

#define XconqSignature 'XCNQ'

extern Tcl_Interp *interp;

char spbuf[BUFSIZE];

char readerrbuf[1000];

/* Tells strmgetc in lisp.c to do on-the-fly conversion to high
ascii mac char codes. This is not needed by MacTcl. */

int use_mac_charcodes = FALSE;

extern int ui_init(void);
extern void imfapp_main(void);

static int MacintoshInit _ANSI_ARGS_((void));
static void init_ae(void);
static void start_game(void);
static void mac_exit(int status);
static void do_ae_open_document(const AppleEvent *message);
static void open_game(Str255 name, int vrefnum);
static OSStatus MissingParameterCheck(const AppleEvent *inputEvent);
static pascal OSErr DoOpenApp(const AppleEvent *inputEvent, AppleEvent *outputEvent, long handlerRefCon);
static pascal OSErr DoQuitApp(const AppleEvent *inputEvent, AppleEvent *outputEvent, long handlerRefCon);
static pascal OSErr DoOpenDocument(const AppleEvent *inputEvent, AppleEvent *outputEvent, long handlerRefCon);

int
main(int argc, char *argv[])
{
    if (MacintoshInit()  != TCL_OK) {
	Tcl_Exit(1);
    }
    init_ae();
    ui_init();
    init_lisp();

    /* Has to be done here since interp is defined only after
    initial_ui_init. */
    TkMacInitAppleEvents(interp);

    imfapp_main();
    /* Humor the compiler. */
    return 0;
}

void
start_game()
{
	imfapp_main();
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

	mac_exit(TRUE);
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
	SetVol(nil, vrefnum);
	imfapp_main();
}

void
mac_exit(int status)
{
	ExitToShell();
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

/* Need these so we can link in Lisp reader code. */

void
announce_read_progress(void)
{
}

void  
syntax_error(Obj *x, char *msg)
{
    sprintlisp(readerrbuf, x, BUFSIZE);
    init_warning("syntax error in `%s' - %s", readerrbuf, msg);
}

void
low_init_error(char *str)
{
    fprintf(stderr, "Error: %s.\n", str);
    fflush(stderr);
}

/* A warning just gets displayed, no other action is taken. */

void
low_init_warning(char *str)
{
    fprintf(stdout, "Warning: %s.\n", str);
    fflush(stdout);
}

/* A run error is fatal. */

void
low_run_error(char *str)
{
    fprintf(stderr, "Error: %s.\n", str);
    fflush(stderr);
    exit(1);
}

void
low_run_warning(char *str)
{
    fprintf(stdout, "Warning: %s.\n", str);
    fflush(stdout);
}

/* Make the table so keyword lookup works. */

struct a_key {
    char *name;
} keywordtable[] = {

#undef  DEF_KWD
#define DEF_KWD(NAME,code)  { NAME },

#include "keyword.def"

    { NULL }
};


char *
keyword_name(enum keywords k)
{
    return keywordtable[k].name;
}

void
init_predefined_symbols(void)
{
}
