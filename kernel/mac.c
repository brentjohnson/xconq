/* Mac-specific code for Xconq kernel.
   Copyright (C) 1992-1997, 1999, 2002 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/* The code in this file is Mac-specific, but not necessarily specific
   to any particular app; it should be usable with utilities and with
   apps that don't have fancy interface. */

#include "config.h"
#include "misc.h"
#include "dir.h"
#include "lisp.h"
#include "module.h"
#include "system.h"

#ifndef c2p
#define c2p(STR,PBUF) \
  strcpy(((char *) PBUF) + 1, STR);  \
  PBUF[0] = strlen(STR);
#endif

#ifndef p2c
#define p2c(PSTR,BUF)  \
  strncpy(BUF, ((char *) (PSTR) + 1), PSTR[0]);  \
  BUF[PSTR[0]] = '\0';
#endif

extern CursHandle sendcursor;
extern CursHandle receivecursor;
extern CursHandle current_cursor;

/* The HFS volume that the program started with. */

short initial_vrefnum = -1;
long initial_dirid = -1;

static char *news_fname;
static char *homedir;

/* We normally keep the library and image folders in the same folder
   as the app. */

#ifndef XCONQLIB
#define XCONQLIB ":lib"
#endif

#ifndef XCONQIMAGES
#define XCONQIMAGES ":images"
#endif

#ifndef XCONQSAVE
#define XCONQSAVE ":save"
#endif

/* Set use_mac_charcodes to true if we want strmgetc to do on-the-fly 
conversion to high ascii mac char codes. This is needed by the Mac PPC 
interface, but NOT by MacTcl. The SDL interface currently lacks font 
support for high ascii chars, so the question is moot for MacSDL. */

extern int use_mac_charcodes;
extern int convert_mac_charcodes(int ch);

/* Handle some non-english characters whose coding differ between unix and
mac. This is required for the Swedish name generator to return non-garbage
on the mac. Note 1: The signs that look like < and , are not the normal < and ,.
Note 2: Typecasting ch to char is necessary here, but it must remain an int to 
handle EOF correctly on unix. */

int
convert_mac_charcodes(int ch)
{
	if (use_mac_charcodes) {

	    if ((char) ch == 'Å')
	        ch = '';
	    if ((char) ch == 'Ä')
	        ch = '€';
	    if ((char) ch == 'Ö')
	        ch = '…';
	    if ((char) ch == 'Ü')
	        ch = '†';
	    if ((char) ch == 'Ë')
	        ch = 'è';
	    if ((char) ch == 'É')
	        ch = 'ƒ';
	    if ((char) ch == 'È')
	        ch = 'é';
	    if ((char) ch == 'Ñ')
	        ch = '„';
	    if ((char) ch == 'Ç')
	        ch = '‚';
	    if ((char) ch == 'Æ')
	        ch = '®';
	    if ((char) ch == 'Ø')
	        ch = '¯';

	    if ((char) ch == 'å')
	        ch = 'Œ';
	    if ((char) ch == 'ä')
	        ch = 'Š';
	    if ((char) ch == 'ö')
	        ch = 'š';
	    if ((char) ch == 'Ü')
	        ch = 'Ÿ';
	    if ((char) ch == 'ë')
	        ch = '‘';
	    if ((char) ch == 'é')
	        ch = 'Ž';
	    if ((char) ch == 'è')
	        ch = '';
	    if ((char) ch == 'ñ')
	        ch = '–';
	    if ((char) ch == 'ç')
	        ch = '';
	    if ((char) ch == 'æ')
	        ch = '¾';
	    if ((char) ch == 'ø')
	        ch = '¿';
	}
	return ch;
}

char *
default_library_pathname()
{
    return XCONQLIB;
}

char *
default_images_pathname(char *libpath)
{
    return XCONQIMAGES;
}

char *
news_filename()
{
    if (news_fname == NULL) {
	  news_fname = NEWSFILE;
    }
    return news_fname;
}

void
save_game(const char *name)
{
	Str255 tmpstr;
	short	curvrefnum;
	long	curdirid;

	HGetVol(NULL, &curvrefnum, &curdirid);
	c2p(game_homedir(), tmpstr);
	HSetVol(NULL, initial_vrefnum, initial_dirid);
	HSetVol(tmpstr, NULL, NULL);
	write_entire_game_state(name);
	HSetVol(NULL, curvrefnum, curdirid);
}

char *
game_homedir(void)
{
	Str255 tmpstr;
	char tmpbuf[255];
	long dirid;
	short	curvrefnum;
	long	curdirid;
	OSErr err;

	HGetVol(NULL, &curvrefnum, &curdirid);
	HSetVol(NULL, initial_vrefnum, initial_dirid);
	sprintf(tmpbuf, XCONQSAVE);
	c2p(tmpbuf, tmpstr);
	err = HSetVol(tmpstr, NULL, NULL);
	if (err != noErr) {
		DirCreate(curvrefnum, curdirid, tmpstr, &dirid);
	}
	/* If we are done loading the game we return to the initial
	volume and directory so that file saves work correctly. */ 
	if (mainmodule && mainmodule->loaded) {
		HSetVol(NULL, initial_vrefnum, initial_dirid);
	/* If we are still loading modules and images we need to
	restore the current directory in case we are running the
	game from a non-standard location. */
	} else {
		HSetVol(NULL, curvrefnum, curdirid);
	}
	homedir = copy_string(tmpbuf);
	return homedir;
}

char *
game_filename(const char *namevar, const char* defaultname)
{
    char *str, *home;

    if (namevar && (str = getenv(namevar)) != NULL && *str) {
	return copy_string(str);
    }
    home = game_homedir();
    str = (char *)xmalloc(strlen(home) + 1 + strlen(defaultname) + 1);
    strcpy(str, home);
    strcat(str, ":");
    strcat(str, defaultname);
    return str;
}

/* The purpose of this wrapper for fopen is to handle cases where the 
mac is opening a unix file and automatic linefeed conversion therefore
should be disabled. */

FILE *
open_file(char *filename, char *mode)
{
	FILE	*fp;
	FSSpec fsspec;
	Str255 pname;
	FInfo finfo;
	short	curvrefnum;
	long	curdirid;
	int 	mac_linefeeds = 0;
	int 	unix_linefeeds = 0;
	int	ch;
	
	/* First svae the current volume and directory in case we
	are running the game from a non-standard location. */
	HGetVol(NULL, &curvrefnum, &curdirid);
	c2p(filename, pname);
	/* Force debug output and warnings to the top directory. */ 
	if (strcmp(filename, "Xconq.DebugOut") == 0
	    || strcmp(filename, "Xconq-Master.DebugOut") == 0
	    || strcmp(filename, "Xconq-Client.DebugOut") == 0
	    || strcmp(filename, "Xconq.Warnings") == 0) {
		HSetVol(NULL, initial_vrefnum, initial_dirid);
	}
	/* Writing or Appending: always mac format. */	
	if (strcmp(mode, "w") == 0) {
		fp = fopen(filename, "w");
		FSMakeFSSpec(0, 0, pname, &fsspec);
		FSpGetFInfo(&fsspec, &finfo);
		(&finfo)->fdType = 'TEXT';
		if (strcmp(filename, "Xconq.DebugOut") == 0
		    || strcmp(filename, "Xconq-Master.DebugOut") == 0
		    || strcmp(filename, "Xconq-Client.DebugOut") == 0
		    || strcmp(filename, "Xconq.Warnings") == 0) {
			(&finfo)->fdCreator = 'ttxt';
		} else {
			(&finfo)->fdCreator = 'XCNQ';
		}
 		FSpSetFInfo(&fsspec, &finfo);
		HSetVol(NULL, curvrefnum, curdirid);
		return fp;
	}		
	if (strcmp(mode, "a") == 0) {
		fp = fopen(filename, "a");
		FSMakeFSSpec(0, 0, pname, &fsspec);
		FSpGetFInfo(&fsspec, &finfo);
		(&finfo)->fdType = 'TEXT';
		if (strcmp(filename, "Xconq.DebugOut") == 0
		    || strcmp(filename, "Xconq-Master.DebugOut") == 0
		    || strcmp(filename, "Xconq-Client.DebugOut") == 0
		    || strcmp(filename, "Xconq.Warnings") == 0) {
			(&finfo)->fdCreator = 'ttxt';
		} else {
			(&finfo)->fdCreator = 'XCNQ';
		}
 		FSpSetFInfo(&fsspec, &finfo);
		HSetVol(NULL, curvrefnum, curdirid);
		return fp;
	}		
	/* Reading: first open in binary mode (no conversion). */
	fp = fopen(filename, "rb");
	/* Look in xconq start directory if we did not find a file. */
	if (fp == NULL) {
		HSetVol(NULL, initial_vrefnum, initial_dirid);
		fp = fopen(filename, "rb");
	}
	/* Give up if we found nothing there either. */
	if (fp == NULL) {
		HSetVol(NULL, curvrefnum, curdirid);
		return fp;
	}
	/* Binary (image) files don't need conversion. */
	if (strcmp(mode, "rb") == 0) {
		HSetVol(NULL, curvrefnum, curdirid);
		return fp;
	}
	/* Text file. Count the number of linefeeds of each kind. */
	while ((ch = getc(fp)) != EOF) {
		if (ch == '\r')
			++mac_linefeeds;
		if (ch == '\n')
			++unix_linefeeds;
	}
	/* Close the file (also needed in unix case to reset file pointer). */
	fclose(fp);
	/* Assume that a mac file has more mac than unix linefeeds. */
	if (mac_linefeeds > unix_linefeeds) {
		/* Reopen mac file in text mode (with conversion). */
		fp = fopen(filename, "r");
		/* Look in xconq start directory if we did not find a file. */
		if (fp == NULL) {
			HSetVol(NULL, initial_vrefnum, initial_dirid);
			fp = fopen(filename, "r");
		}
	} else	{
		/* Reopen unix file in binary mode (no conversion). */
		fp = fopen(filename, "rb");
		/* Look in xconq start directory if we did not find a file. */
		if (fp == NULL) {
			HSetVol(NULL, initial_vrefnum, initial_dirid);
			fp = fopen(filename, "rb");
		}
	}
	HSetVol(NULL, curvrefnum, curdirid);
	return fp;		
}

FILE *
open_library_file(const char *filename)
{
     char fullnamebuf[BUFSIZE];
    LibraryPath *p;
    FILE *fp;
	
    /* Now try to open the file. */
    fp = open_file(filename, "r");
    if (fp != NULL)
      return fp;
    /* Generate library pathname. */
    for_all_library_paths(p) {
	make_pathname(p->path, filename, NULL, fullnamebuf);
	/* Now try to open the file. */
 	fp = open_file(fullnamebuf, "r");
	if (fp != NULL)
	  return fp;
    }
    return NULL;
}

FILE *
open_scorefile_for_reading(const char *name)
{
    short curvrefnum;
    long curdirid;
    Str255 tmpstr;
    FILE *fp;
	
    HGetVol(NULL, &curvrefnum, &curdirid);
    HSetVol(NULL, initial_vrefnum, initial_dirid);
    c2p(game_homedir(), tmpstr);
    HSetVol(tmpstr, NULL, NULL);
    fp = open_file(name, "r");
    HSetVol(NULL, curvrefnum, curdirid);
    return fp;
}

FILE *
open_scorefile_for_writing(const char *name)
{
    Str255 pname, tmpstr;
    char buf[255];
    FSSpec fsspec;
    FInfo finfo;
    short curvrefnum;
    long curdirid;
    FILE *fp;
	
    HGetVol(NULL, &curvrefnum, &curdirid);
    HSetVol(NULL, initial_vrefnum, initial_dirid);
    c2p(game_homedir(), tmpstr);
    HSetVol(tmpstr, NULL, NULL);
    /* Now try to open the file. */
    sprintf(buf, "%s", name);
    fp = open_file(buf, "a");
    /* Set the creator to ttxt, overriding what open_file just did. */
    c2p(name, pname);
    FSMakeFSSpec(0, 0, pname, &fsspec);
    FSpGetFInfo(&fsspec, &finfo);
    (&finfo)->fdCreator = 'ttxt';
    FSpSetFInfo(&fsspec, &finfo);
    HSetVol(NULL, curvrefnum, curdirid);
    return fp;
}

/* Close scorefile after having written to it; nothing special to do on Macs. */

void
close_scorefile_for_writing(FILE *fp)
{
    fclose(fp);
}

void
make_pathname(const char *path, const char *name, const char *extn, char *pathbuf)
{
    strcpy(pathbuf, "");
    if (!empty_string(path)) {
	strcat(pathbuf, path);
	strcat(pathbuf, ":");
    }
    strcat(pathbuf, name);
    /* Don't add a second identical extension, but do add if extension
       is different (in case we want "foo.12" -> "foo.12.g" for instance) */
    if (strrchr(name, '.')
	&& extn
	&& strcmp(strrchr(name, '.') + 1, extn) == 0)
      return;
    if (!empty_string(extn)) {
	strcat(pathbuf, ".");
	strcat(pathbuf, extn);
    }
}

/* Remove a given file. */

int
remove_file(char *fname)
{
    Str255 pname;
    FSSpec fsspec;
    OSErr err;
    
    c2p(fname, pname);
    err = FSMakeFSSpec(0, 0, pname, &fsspec);
    if (err == noErr) {
	err = FSpDelete(&fsspec);
    }
    return err;
}

char *
error_save_filename(void)
{
    /* No need to cache name, will only get requested once. */
    return game_filename("XCONQERRORFILE", ERRORFILE);
}

void
init_signal_handlers()
{
}

int last_ticks = 0;

int
n_seconds_elapsed(int n)
{
    int ticks = TickCount();

    if (((ticks - last_ticks) / 60) > n) {
	last_ticks = ticks;
    	return TRUE;
    } else {
	return FALSE;
    }
}

int last_ticks_for_ms = 0;

int
n_ms_elapsed(int n)
{
    return (((TickCount() - last_ticks_for_ms) * 16) > n);
}

void
record_ms()
{
    last_ticks_for_ms = TickCount();
}

/* These four functions need to be defined as dummys if we have not
     included socket.c in the project. To compile socket.c with the GUSI 
     libraries, do the following:

1. Define GUSI in your precompiled header.
2. Add kernel/socket.c to your project.
3. Add these config and library files:
	GUSIConfig_OTINET.cp
	GUSI_Core.PPC.lib
	OpenTransportLib
	OpenTptInternetLib
	OpenTransportAppPPC.o
	OpenTptInetOOC.o
	ThreadsLib
	PLStringFuncsPPC.lib
*/

#ifndef GUSI

int
open_remote_connection(char *methodname, int willhost)
{
	return FALSE;
}

void
close_remote_connection(int rid)
{
}

void
low_send(int id, const char *buf)
{
}

int
low_receive(int *idp, char *buf, int maxchars, int timeout)
{
	return 0;
}

#endif  /* GUSI_SOURCE */


