// Xconq
// Unix-specific Filesystem Functions

// $Id: fs.cc,v 1.2 2006/04/07 01:33:25 eric_mcdonald Exp $

/*//////////////////////////// COPYRIGHT NOTICE ////////////////////////////////

  Copyright (C) 1987-1989   Stanley T. Shebs
  Copyright (C) 1991-1997   Stanley T. Shebs
  Copyright (C) 1999        Stanley T. Shebs
  Copyright (C) 2006	    Eric A. McDonald

//////////////////////////////////// LICENSE ///////////////////////////////////

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

//////////////////////////////////////////////////////////////////////////////*/

/*! \file
    \brief Unix-specific filesystem functions.
    \ingroup grp_util
*/

/* Unix interface stuff.  Do NOT attempt to use this file in a non-Unix
   system, not even an ANSI one! */

/* Also note that this file does not include all Xconq .h files, since
   it may be used with auxiliary programs. */

extern "C" {
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
}
#include <cerrno>

#include "util/constants.h"
#include "util/memory.h"
#include "util/string.h"
#include "util/callbacks.h"
#include "util/fs.h"

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
#define XCONQSCORES "scores"
#endif

NAMESPACE_XCONQ_BEGIN
NAMESPACE_UTILITY_BEGIN

uid_t games_uid;

static int is_directory(const char *path);

static char *score_file_pathname(char *name);

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
    char *abspaths [] = {
	"/usr/share/xconq/lib", "/usr/local/share/xconq/lib", 
	"/opt/share/xconq/lib", "/opt/local/share/xconq/lib", 
	"/usr/local/xconq/lib", "/opt/local/xconq/lib", 
	"/usr/local/xconq/share/lib", "/opt/local/xconq/share/lib", NULL
    };
    char *relpaths [] = {
	"../share/xconq/lib", "xconq/lib", "../../xconq/lib", NULL
    };

    name = (char *)xmalloc(strlen(XCONQDATA) + 1 + strlen(XCONQLIB) + 1);
    make_pathname(XCONQDATA, XCONQLIB, NULL, name);
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
default_images_pathname(char *libpath)
{
    char *name = NULL;
    int i = 0;
    int found = FALSE;
    char *abspaths [] = {
	"/usr/share/xconq/images", "/usr/local/share/xconq/images", 
	"/opt/share/xconq/images", "/opt/local/share/xconq/images", 
	"/usr/local/xconq/images", "/opt/local/xconq/images", 
	"/usr/local/xconq/share/images", "/opt/local/xconq/share/images", NULL
    };
    char *relpaths [] = {
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
save_game(char *fname)
{
    char *fullname;
    
    fullname = game_filename(NULL, fname);
    write_entire_game_state(fullname);
}

static char *homedir;

char *
game_homedir(void)
{
    char *str;
    struct passwd *pwd;

    if (homedir != NULL)
      return homedir;
    if ((str = getenv("XCONQHOME")) != NULL) {
	homedir = copy_string(str);
    } else if ((str = getenv("HOME")) != NULL) {
	homedir = (char *)xmalloc(strlen(str) + 20);
	strcpy(homedir, str);
	strcat(homedir, "/.xconq");
    } else if ((pwd = getpwuid(getuid())) != NULL) {
	homedir = (char *)xmalloc(strlen(pwd->pw_dir) + 20);
	strcpy(homedir, pwd->pw_dir);
	strcat(homedir, "/.xconq");
    } else {
	homedir = ".";
    }
    /* Try to ensure that the directory exists. */
    if (access(homedir, F_OK) != 0) {
	mkdir(homedir, 0755);
	/* (should warn of problems) */
    }
    return homedir;
}

char *
game_filename(char *namevar, char* defaultname)
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
open_library_file(char *filename)
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
open_scorefile_for_reading(char *name)
{
    FILE *fp;

    fp = open_file(score_file_pathname(name), "r");
    return fp;
}

FILE *
open_scorefile_for_writing(char *name)
{
    FILE *fp;

    /* The scorefile is only writable by the owner of the Xconq
       executable, but we normally run as the user, so switch over
       before writing. */
    setuid(games_uid);
    fp = open_file(score_file_pathname(name), "a");
    return fp;
}

void
close_scorefile_for_writing(FILE *fp)
{
    fclose(fp);
    /* Reset the uid back to the user who started the game. */
    setuid(getuid());
}

/* Given the name of a scorefile, return a complete path to it,
   accounting for env vars etc. */

static char *
score_file_pathname(char *name)
{
    static char *scorenamebuf;
    char *scorepath = NULL, *gamehomepath = NULL, *dirpathbuf = NULL;
    int i = 0;
    int found = FALSE, da_scorepath = FALSE;
    char *abspaths [] = {
	"/var/lib/games/xconq", "/var/lib/xconq", 
	"/var/lib/games/xconq/scores", "/var/lib/xconq/scores", 
	"/usr/share/xconq/scores", NULL
    };
    char *ghrelpaths [] = {
	"scores", "../xconq", "../xconq/scores", "../Xconq", 
	"../Xconq/scores", ".", NULL
    };

    /* (Note that this wires in the name on the first call, so cannot
       be called with different names.  We could make this smarter, but
       no point to it right now.) */
    if (scorenamebuf == NULL) {
	/* See if the environment provides a scores directory. */
	scorepath = getenv("XCONQ_SCORES");
	/* If the environment does not provide a scores directory, then 
	   try using the compiled in one. */
	if (empty_string(scorepath))
	  scorepath = XCONQSCORES;
	if (is_directory(scorepath))
	  found = TRUE;
	/* If the compiled in scores directory is not present, then start 
	   looking elsewhere. */
	/* Absolute paths. */
	for (i = 0; abspaths[i] && !found; ++i) {
	    if (is_directory(abspaths[i])) {
		scorepath = abspaths[i];
		found = TRUE;
	    }
	}
	/* Paths relative to the game home dir. */
	gamehomepath = game_homedir();
	for (i = 0; ghrelpaths[i] && !found; ++i) {
	    dirpathbuf = (char *)xmalloc(strlen(gamehomepath) + 1 + 
					 strlen(ghrelpaths[i]) + 10);
	    make_pathname(gamehomepath, ghrelpaths[i], NULL, dirpathbuf);
	    if (is_directory(dirpathbuf)) {
		scorepath = copy_string(dirpathbuf);
		da_scorepath = TRUE;
		found = TRUE;
	    }
	    free(dirpathbuf);
	}
	/* If we can find nowhere suitable, then try to create a new 
	   scores directory. If that fails, then just give the caller 
	   something, and return. */
	if (!found) {
	    dirpathbuf = (char *)xmalloc(strlen(gamehomepath) + 1 + 16);
	    make_pathname(gamehomepath, "scores", NULL, dirpathbuf);
	    if (!is_directory(dirpathbuf) && (ENOENT == errno)) {
		if (access(homedir, X_OK|W_OK))
		  mkdir(dirpathbuf, 0755);
		scorepath = copy_string(dirpathbuf);
		da_scorepath = TRUE;
	    }
	    else {
		scorepath = "scores";
	    }
	    free(dirpathbuf);
	}
	/* Make the full path. */
	scorenamebuf = (char *)xmalloc(strlen(scorepath) + 1 + strlen(name) + 
				       10);
	make_pathname(scorepath, name, NULL, scorenamebuf);
	if (da_scorepath)
	  free(scorepath);
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
make_pathname(char *path, char *name, char *extn, char *pathbuf)
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

char *
error_save_filename(void)
{
    /* No need to cache name, will only get requested once. */
    return game_filename("XCONQERRORFILE", ERRORFILE);
}

NAMESPACE_UTILITY_END
NAMESPACE_XCONQ_END

