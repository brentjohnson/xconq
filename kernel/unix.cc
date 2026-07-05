/* Unix-specific code for Xconq kernel.
   Copyright (C) 1987-1989, 1991-1997, 1999 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/* Unix interface stuff.  Do NOT attempt to use this file in a non-Unix
   system, not even an ANSI one! */

/* Also note that this file does not include all Xconq .h files, since
   it may be used with auxiliary programs. */

#include "config.h"
#include "misc.h"
#include "dir.h"
#include "lisp.h"
#include "module.h"
#include "system.h"

#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <pwd.h>
#include <errno.h>
#include <fcntl.h>

/* (should not include these anymore?) */
#include <sys/file.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>

/* Default names for places in Unix. */

#ifndef XCONQDATA
#define XCONQDATA ".."
#endif

#ifndef XCONQLIB
#define XCONQLIB "lib"
#endif

#ifndef XCONQIMAGES
#define XCONQIMAGES "images"
#endif

#ifndef XCONQSCORES
#define XCONQSCORES ""
#endif

#ifndef XCONQ_SRCDIR
#define XCONQ_SRCDIR ""
#endif

#if 1 /* POSIX */
static void stop_handler(int sig);
static void crash_handler(int sig);
static void hup_handler(int sig);
#else /* BSD etc? */
static void stop_handler(int sig, int code, struct sigcontext *scp, char *addr);
static void crash_handler(int sig, int code, struct sigcontext *scp, char *addr);
static void hup_handler(int sig, int code, struct sigcontext *scp, char *addr);
#endif

static int is_directory(const char *path);

static char *score_file_pathname(const char *name);

/* Is the given path a directory? */
static int 
is_directory(const char *path)
{
    struct stat fsent;

    memset(&fsent, 0, sizeof(struct stat));
    if (-1 != stat((const char *)path, &fsent)) {
	if (S_ISDIR(fsent.st_mode))
	  return TRUE;
    }
    return FALSE;
}

char *
default_library_pathname(void)
{
    char *name = NULL;
    int i = 0;
    int found = FALSE;
    const char *abspaths [] = {
	"/usr/share/xconq/lib", "/usr/local/share/xconq/lib", 
	"/opt/share/xconq/lib", "/opt/local/share/xconq/lib", 
	"/usr/local/xconq/lib", "/opt/local/xconq/lib", 
	"/usr/local/xconq/share/lib", "/opt/local/xconq/share/lib", NULL
    };
    const char *relpaths [] = {
	"../share/xconq/lib", "xconq/lib", "../../xconq/lib", NULL
    };

    name = (char *)xmalloc(strlen(XCONQDATA) + 1 + strlen(XCONQLIB) + 1);
    make_pathname(XCONQDATA, XCONQLIB, NULL, name);
    if (is_directory(name))
      found = TRUE;
    else
      free(name);
    /* Not installed (yet): fall back to the source checkout this binary
       was built from, so a freshly built, uninstalled binary still finds
       its data. */
    if (!found && !empty_string(XCONQ_SRCDIR)) {
	name = (char *)xmalloc(strlen(XCONQ_SRCDIR) + 1 + strlen(XCONQLIB) + 1);
	make_pathname(XCONQ_SRCDIR, XCONQLIB, NULL, name);
	if (is_directory(name))
	  found = TRUE;
	else
	  free(name);
    }
    for (i = 0; abspaths[i] && !found; ++i) {
	if (is_directory(abspaths[i])) {
	    name = copy_string(abspaths[i]);
	    found = TRUE;
	}
    }
    for (i = 0; relpaths[i] && !found; ++i) {
	if (is_directory(relpaths[i])) {
	    name = copy_string(relpaths[i]);
	    found = TRUE;
	}
    }
    if (!found)
      name = copy_string(".");
    return name;
}

char *
default_images_pathname(char *libpath)
{
    char *name = NULL;
    int i = 0;
    int found = FALSE;
    const char *abspaths [] = {
	"/usr/share/xconq/images", "/usr/local/share/xconq/images", 
	"/opt/share/xconq/images", "/opt/local/share/xconq/images", 
	"/usr/local/xconq/images", "/opt/local/xconq/images", 
	"/usr/local/xconq/share/images", "/opt/local/xconq/share/images", NULL
    };
    const char *relpaths [] = {
	"../share/xconq/images", "xconq/images", "../../xconq/images", NULL
    };

    if (libpath == NULL)
      libpath = default_library_pathname();
    name = (char *)xmalloc(strlen(libpath) + 4 + strlen(XCONQIMAGES) + 1);
    strcpy(name, libpath);
    strcat(name, "/../");
    strcat(name, XCONQIMAGES);
    if (is_directory(name))
      found = TRUE;
    else
      free(name);
    for (i = 0; abspaths[i] && !found; ++i) {
	if (is_directory(abspaths[i])) {
	    name = copy_string(abspaths[i]);
	    found = TRUE;
	}
    }
    for (i = 0; relpaths[i] && !found; ++i) {
	if (is_directory(relpaths[i])) {
	    name = copy_string(relpaths[i]);
	    found = TRUE;
	}
    }
    if (!found)
      name = copy_string(".");
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
    const char *fullname;
    
    fullname = game_filename(NULL, fname);
    write_entire_game_state(fullname);
}

static char *homedir;
static char *confdir;
static char *olddir;
static int olddir_checked;
static int olddir_note_given;

/* $HOME, falling back to the password-file entry, and finally ".". */
static const char *
user_home(void)
{
    char *str;
    struct passwd *pwd;

    if ((str = getenv("HOME")) != NULL)
      return str;
    if ((pwd = getpwuid(getuid())) != NULL)
      return pwd->pw_dir;
    return ".";
}

/* The pre-XDG per-user directory (~/.xconq), if it still exists. Returns
   NULL once XCONQHOME is set (which overrides everything, legacy included)
   or if no such directory was ever created.

   Callers must only consult this from within a fallback branch, i.e.
   after the new XDG-based lookup has already failed -- the first non-NULL
   return prints a one-time note, so calling it unconditionally would be
   misleading. */
const char *
legacy_homedir(void)
{
    char *cand;

    if (olddir_checked)
      return olddir;
    olddir_checked = TRUE;
    if (getenv("XCONQHOME") != NULL)
      return NULL;
    cand = (char *)xmalloc(strlen(user_home()) + 20);
    sprintf(cand, "%s/.xconq", user_home());
    if (is_directory(cand)) {
	olddir = cand;
	if (!olddir_note_given) {
	    fprintf(stderr,
		    "xconq: falling back to legacy directory \"%s\" "
		    "(move your files to the new XDG locations to silence "
		    "this message)\n",
		    olddir);
	    olddir_note_given = TRUE;
	}
    } else {
	free(cand);
    }
    return olddir;
}

/* Create PATH and any missing parent directories, all mode 0700 (like
   "mkdir -p"). PATH is modified in place and restored before returning. */
static void
mkdir_p(char *path)
{
    char *slash;

    for (slash = path + 1; *slash; ++slash) {
	if (*slash == '/') {
	    *slash = '\0';
	    if (access(path, F_OK) != 0)
	      mkdir(path, 0700);
	    *slash = '/';
	}
    }
    if (access(path, F_OK) != 0)
      mkdir(path, 0700);
}

/* Build "<base>/xconq", creating it and any missing parents (mode 0700)
   if it does not exist, where <base> is $ENVVAR or else
   $HOME/FALLBACK_REL. */
static char *
xdg_xconq_dir(const char *envvar, const char *fallback_rel)
{
    const char *envval = getenv(envvar);
    char *base, *dir;

    if (envval != NULL && *envval) {
	base = copy_string(envval);
    } else {
	base = (char *)xmalloc(strlen(user_home()) + strlen(fallback_rel) + 2);
	sprintf(base, "%s/%s", user_home(), fallback_rel);
    }
    dir = (char *)xmalloc(strlen(base) + 8);
    sprintf(dir, "%s/xconq", base);
    free(base);
    if (access(dir, F_OK) != 0) {
	mkdir_p(dir);
	/* (should warn of problems) */
    }
    return dir;
}

/* Directory for per-user data: saved games, checkpoints, scores. */

char *
game_homedir(void)
{
    char *str;

    if (homedir != NULL)
      return homedir;
    if ((str = getenv("XCONQHOME")) != NULL) {
	homedir = copy_string(str);
	/* Try to ensure that the directory exists. */
	if (access(homedir, F_OK) != 0) {
	    mkdir(homedir, 0755);
	    /* (should warn of problems) */
	}
    } else {
	homedir = xdg_xconq_dir("XDG_DATA_HOME", ".local/share");
    }
    return homedir;
}

/* Directory for per-user config: preferences. */

char *
game_confdir(void)
{
    char *str;

    if (confdir != NULL)
      return confdir;
    if ((str = getenv("XCONQHOME")) != NULL) {
	confdir = copy_string(str);
	if (access(confdir, F_OK) != 0) {
	    mkdir(confdir, 0755);
	}
    } else {
	confdir = xdg_xconq_dir("XDG_CONFIG_HOME", ".config");
    }
    return confdir;
}

static char *
game_filename_in(const char *namevar, const char *defaultname, const char *dir)
{
    char *str;

    if (namevar && (str = getenv(namevar)) != NULL && *str) {
	return copy_string(str);
    }
    str = (char *)xmalloc(strlen(dir) + 1 + strlen(defaultname) + 1);
    strcpy(str, dir);
    strcat(str, "/");
    strcat(str, defaultname);
    return str;
}

char *
game_filename(const char *namevar, const char* defaultname)
{
    return game_filename_in(namevar, defaultname, game_homedir());
}

char *
game_conf_filename(const char *namevar, const char* defaultname)
{
    return game_filename_in(namevar, defaultname, game_confdir());
}

/* This wrapper replaces fopen everywhere in the kernel. On the mac
   side it does unix-to-mac linefeed conversion if necessary. Here it
   does absolutely nothing. */

FILE *
open_file(const char *filename, const char *mode)
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
    return open_file(score_file_pathname(name), "r");
}

FILE *
open_scorefile_for_writing(const char *name)
{
    return open_file(score_file_pathname(name), "a");
}

void
close_scorefile_for_writing(FILE *fp)
{
    fclose(fp);
}

/* Given the name of a scorefile, return a complete path to it,
   accounting for env vars etc. Scores are per-user by default (a
   "scores" subdirectory of the data dir); XCONQ_SCORES (runtime) or the
   compiled-in XCONQSCORES (set via -DXCONQ_SCORES_DIR, for shared
   installs) can override that. */

static char *
score_file_pathname(const char *name)
{
    static char *scorenamebuf;
    char *scoredir;

    /* (Note that this wires in the name on the first call, so cannot
       be called with different names.  We could make this smarter, but
       no point to it right now.) */
    if (scorenamebuf == NULL) {
	const char *envval = getenv("XCONQ_SCORES");

	if (!empty_string(envval)) {
	    scoredir = copy_string(envval);
	} else if (!empty_string(XCONQSCORES) && is_directory(XCONQSCORES)) {
	    scoredir = copy_string(XCONQSCORES);
	} else {
	    char *home = game_homedir();

	    scoredir = (char *)xmalloc(strlen(home) + 8);
	    sprintf(scoredir, "%s/scores", home);
	    if (access(scoredir, F_OK) != 0) {
		mkdir(scoredir, 0700);
		/* (should warn of problems) */
	    }
	}
	scorenamebuf = (char *)xmalloc(strlen(scoredir) + 1 + strlen(name) + 1);
	sprintf(scorenamebuf, "%s/%s", scoredir, name);
	free(scoredir);
    }
    return scorenamebuf;
}

/* Take directory path PATH, filename NAME, and extension EXTN and
   make a pathname along the lines of PATH/NAME.EXTN.
   Put the result in PATHBUF, which must point to storage of at
   least BUFSIZE bytes. */
/* Future direction: would like to change the interface to return a
   freshly malloc'd array rather than all this BUFSIZE baloney. */
void
make_pathname(const char *path, const char *name, const char *extn, char *pathbuf)
{
    int pathbuf_size = 1;
    strcpy(pathbuf, "");
    if (!empty_string(path)) {
	pathbuf_size += strlen(path) + 1;
	if (pathbuf_size > BUFSIZE) {
	    pathbuf[0] = '\0';
	    return;
	}
	strcat(pathbuf, path);
	strcat(pathbuf, "/");
    }
    pathbuf_size += strlen(name);
    if (pathbuf_size > BUFSIZE) {
	pathbuf[0] = '\0';
	return;
    }
    strcat(pathbuf, name);
    /* Don't add a second identical extension, but do add if extension
       is different (in case we want "foo.12" -> "foo.12.g" for instance) */
    if (strrchr(name, '.')
	&& extn
	&& strcmp((char *) strrchr(name, '.') + 1, extn) == 0)
      return;
    if (!empty_string(extn)) {
	pathbuf_size += strlen(extn) + 1;
	if (pathbuf_size > BUFSIZE) {
	    pathbuf[0] = '\0';
	    return;
	}
	strcat(pathbuf, ".");
	strcat(pathbuf, extn);
    }
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
#if 1
stop_handler(int sig)
#else
stop_handler(int sig, int code, struct sigcontext *scp, char *addr)
#endif
{
    /* (should try to tell other programs of departure?) */
    close_displays();
    exit(1);
}

/* This routine attempts to save the state before dying. */

static void
#if 1
crash_handler(int sig)
#else
crash_handler(int sig, int code, struct sigcontext *scp, char *addr)
#endif
{
    static int already_been_here = FALSE;

    if (!already_been_here) {
	already_been_here = TRUE;
	close_displays();  
	printf("Fatal error encountered. Signal %d\n", sig);
#if 0
	printf(" code %d\n", code);
#endif
	printf("\n");
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
#if 1
hup_handler(int sig)
#else
hup_handler(int sig, int code, struct sigcontext *scp, char *addr)
#endif
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

const char *
error_save_filename(void)
{
    /* No need to cache name, will only get requested once. */
    return game_filename("XCONQERRORFILE", ERRORFILE);
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
    signal(SIGHUP, hup_handler);
    signal(SIGSEGV, crash_handler);
    signal(SIGFPE, crash_handler);
    signal(SIGILL, crash_handler);
    signal(SIGINT, crash_handler);
    signal(SIGQUIT, crash_handler);
    signal(SIGTERM, crash_handler);
    /* The following signals may not be available everywhere. */
#ifdef SIGBUS
    signal(SIGBUS, crash_handler);
#endif /* SIGBUS */
#ifdef SIGSYS
    signal(SIGSYS, crash_handler);
#endif /* SIGSYS */
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
