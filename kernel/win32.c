/* Win32-specific code for Xconq kernel.
   Copyright (C) 1999 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/* Win32 interface stuff.  Do NOT attempt to use this file in a
   non-Windows system! */

/* Also note that this file does not include all Xconq .h files, since
   it may be used with auxiliary programs. */

#include "config.h"
#include "misc.h"
#include "dir.h"
#include "lisp.h"
#include "module.h"
#include "system.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <errno.h>
#include <fcntl.h>
#include <signal.h>

#if (defined (__CYGWIN32__) || defined (__MINGW32__)) 
#include <unistd.h>
#endif

#ifdef __CYGWIN32__
#include <limits.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <pwd.h>
void cygwin_conv_to_full_win32_path(const char *path, char *win32_path);
#endif

#ifdef _MSC_VER
#include <io.h>
#include <windows.h>
#endif

#ifdef __cplusplus
}
#endif

#define F_OK 0

/* Default names for places. */

/* (should change default passed in by compiler instead) */
#undef XCONQDATA

#ifndef XCONQDATA
/* The Windows app should now live in the top directory. */
#define XCONQDATA "."
#endif

#ifndef XCONQLIB
#define XCONQLIB "lib"
#endif

#ifndef XCONQIMAGES
#define XCONQIMAGES "images"
#endif

#ifdef __CYGWIN32__
/* used in xconq.c and xtconq.c */
uid_t games_uid;
#endif

void extract_dirname (char **dirname, char *pathname);

static void stop_handler(int sig);
static void crash_handler(int sig);
static void hup_handler(int sig);

char *xcq_program_name = NULL;
FILE *xcq_fstdout = NULL;

char *
default_library_pathname(void)
{
    char *name = NULL;
    char *tmpname = NULL;    

    if (xcq_program_name) {
        extract_dirname(&tmpname, xcq_program_name);
        name = (char *)xmalloc(strlen(tmpname) + 1 + strlen(XCONQLIB) + 1);
        strcpy(name, tmpname);
        free(tmpname);
    }
    else {
        name = (char *)xmalloc(strlen(XCONQDATA) + 1 + strlen(XCONQLIB) + 1);
        strcpy(name, XCONQDATA);
    }
    strcat(name, "/");
    strcat(name, XCONQLIB);
    return name;
}

char *
default_images_pathname(char *libpath)
{
    char *name;

    if (libpath == NULL)
      libpath = default_library_pathname();
    name = (char *)xmalloc(strlen(libpath) + 4 + strlen(XCONQIMAGES) + 1);
    strcpy(name, libpath);
    strcat(name, "/../");
    strcat(name, XCONQIMAGES);
    return name;
}

char *
news_filename(void)
{
    /* (should search in library list) */
    make_pathname(xconq_libs->path, NEWSFILE, "", spbuf);
    return spbuf;
}

void
save_game(const char *fname)
{
    char *name;

    name = copy_string(game_filename("", fname));
    write_entire_game_state(name);
}

static char *homedir;

char *
game_homedir(void)
{
    char *str;
#ifdef __CYGWIN32__
    struct passwd *pwd;
    char *homedirconv = NULL;
    char *pc = NULL;
#endif

    if (homedir != NULL)
      return homedir;
    if ((str = getenv("XCONQ_HOME")) != NULL) {
	homedir = copy_string(str);
    } else if ((str = getenv("HOME")) != NULL) {
	homedir = (char *) xmalloc(strlen(str) + 20);
	strcpy(homedir, str);
	strcat(homedir, "/xconq");
#ifdef __CYGWIN32__
    } else if ((pwd = getpwuid(getuid())) != NULL) {
	homedir = (char *) xmalloc(strlen(pwd->pw_dir) + 20);
	strcpy(homedir, pwd->pw_dir);
	strcat(homedir, "/xconq");
#endif
    } else {
	homedir = "./save";
    }

    /* Convert to something that can actually be used by native Win32 
       Tcl/Tk and perhaps others as well.
    */
#ifdef __CYGWIN32__
    homedirconv = (char *) xmalloc(PATH_MAX + 1);
    cygwin_conv_to_full_win32_path(homedir, homedirconv);
    /*
    for (pc = homedirconv; *pc; pc++)
      if (*pc == '\\')
        *pc = '/';
    */
    free(homedir);
    homedir = homedirconv;
#endif

    /* Try to ensure that the directory exists. */
    if (access(homedir, F_OK) != 0) {
#ifdef __CYGWIN32__
	mkdir(homedir, 0755);
#elif defined (_MSC_VER) || defined (__MWERKS__)
	CreateDirectory(homedir, NULL);
#else
	mkdir(homedir);
#endif
	/* (should warn of problems) */
    }
    return homedir;
}
 
char *
game_filename(const char *namevar, const char *defaultname)
{
    char *str, *home;

    if (namevar && (str = getenv(namevar)) != NULL && *str) {
	return copy_string(str);
    }
    home = game_homedir();
    str = (char *)xmalloc(strlen(home) + 1 + strlen(defaultname) + 1);
    strcpy(str, home);
    strcat(str, "/");
    strcat(str, defaultname);
    return str;
}

/* This wrapper replaces fopen everywhere in the kernel. On the mac
   side it does unix-to-mac linefeed conversion if necessary. Here it
   does absolutely nothing. */

FILE *
open_file(char *filename, char *mode)
{
	return fopen(filename, mode);
}

FILE *
open_library_file(const char *filename)
{
    char fullnamebuf[BUFSIZE];
    LibraryPath *p;
    FILE *fp = NULL;

    fp = open_file(filename, "r");
    if (fp != NULL) {
	return fp;
    }
    for_all_library_paths(p) {
	/* Generate library pathname. */
	make_pathname(p->path, filename, NULL, fullnamebuf);
	fp = open_file(fullnamebuf, "r");
	if (fp != NULL) {
	    return fp;
	}
    }
    return NULL;
}

FILE *
open_scorefile_for_reading(const char *name)
{
    FILE *fp;

    fp = open_file(game_filename(NULL, name), "r");
    return fp;
}

FILE *
open_scorefile_for_writing(const char *name)
{
    FILE *fp;

    fp = open_file(game_filename(NULL, name), "a");
    return fp;
}

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
	strcat(pathbuf, "/");
    }
    strcat(pathbuf, name);
    /* Don't add a second identical extension, but do add if extension
       is different (in case we want "foo.12" -> "foo.12.g" for instance) */
    if (strrchr(name, '.')
	&& extn
	&& strcmp((char *) strrchr(name, '.') + 1, extn) == 0)
      return;
    if (!empty_string(extn)) {
	strcat(pathbuf, ".");
	strcat(pathbuf, extn);
    }
}

/* Get dirname of a given file. */

void
extract_dirname (char **dirname, char *pathname)
{
    char *pdelim = NULL;

    *dirname = copy_string(pathname);
    pdelim = strrchr(*dirname, '/');
    if (!pdelim)
        pdelim = strrchr(*dirname, '\\');
    else
        *pdelim = 0;
    if (!pdelim)
        **dirname = 0;
    else
        *pdelim = 0;  
}

/* Remove a given file. */

int
remove_file(char *fname)
{
    unlink(fname);
    /* (should return real outcome) */
    return TRUE;
}

/* Default behavior on explicit kill. */

static void
stop_handler(int sig)
{
    close_displays();
    exit(1);
}

/* This routine attempts to save the state before dying. */

static void
crash_handler(int sig)
{
    static int already_been_here = FALSE;

    if (!already_been_here) {
	already_been_here = TRUE;
	close_displays();  
	printf("Fatal error encountered. Signal %d\n", sig);
	write_entire_game_state(error_save_filename());
    }
#ifdef __cplusplus
    throw "snafu";
#else
    abort();
#endif
}

/* Accidental disconnection saves state. */

static void
hup_handler(int sig)
{
    static int already_been_here = FALSE;

    if (!already_been_here) {
	already_been_here = TRUE;
	close_displays();
	printf("Somebody was disconnected, saving the game.\n");
	write_entire_game_state(error_save_filename());
    }
#ifdef __cplusplus
    throw "snafu";
#else
    abort();
#endif
}

void
init_signal_handlers(void)
{
    signal(SIGINT, stop_handler);
    if (0 /* don't accidently quit */ && !Debug) {
	signal(SIGINT, SIG_IGN);
    } else {
	signal(SIGINT, SIG_DFL);
/*	signal(SIGINT, crash_handler);  */
    }
    signal(SIGSEGV, crash_handler);
    signal(SIGFPE, crash_handler);
    signal(SIGILL, crash_handler);
    signal(SIGINT, crash_handler);
    signal(SIGTERM, crash_handler);
    /* The following signals may not be available everywhere. */
#ifdef SIGHUP
    signal(SIGHUP, hup_handler);
#endif
#ifdef SIGQUIT
    signal(SIGQUIT, crash_handler);
#endif
#ifdef SIGBUS
    signal(SIGBUS, crash_handler);
#endif
#ifdef SIGSYS
    signal(SIGSYS, crash_handler);
#endif
}

char *
error_save_filename(void)
{
    /* No need to cache name, will only get requested once. */
    return game_filename("XCONQERRORFILE", ERRORFILE);
}

struct timeval reallasttime = { 0, 0 };

struct timeval realcurtime;

int
n_seconds_elapsed(int n)
{
    gettimeofday(&realcurtime, NULL);
    if (realcurtime.tv_sec > (reallasttime.tv_sec + (n - 1))) {
	reallasttime = realcurtime;
	return TRUE;
    } else {
	return FALSE;
    }
}

/* Returns true if n milliseconds have passed since the time was recorded
   via record_ms(). */

struct timeval reallastmstime = { 0, 0 };

int
n_ms_elapsed(int n)
{
    int interval;
    struct timeval tmprealtime;

    gettimeofday(&tmprealtime, NULL);
    interval =
      (tmprealtime.tv_sec - reallastmstime.tv_sec) * 1000
	+ (tmprealtime.tv_usec - reallastmstime.tv_usec) / 1000;
    return (interval > n);
}

/* Record the current time of day. */

void
record_ms(void)
{
    gettimeofday(&reallastmstime, NULL);
}
