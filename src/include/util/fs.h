// Xconq
// Filesystem-related Utility Functions

// $Id: fs.h,v 1.1 2006/04/07 02:13:35 eric_mcdonald Exp $

/*//////////////////////////// COPYRIGHT NOTICE ////////////////////////////////

  Copyright (C) 1987-1989   Stanley T. Shebs
  Copyright (C) 1991-1997   Stanley T. Shebs
  Copyright (C) 1999	    Stanley T. Shebs
  Copyright (C) 2004-2006   Eric A. McDonald

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
    \brief Filesystem-related utility functions and constants.
    \ingroup grp_util

    \todo Replace 'LibraryPath' with 'list<char *>'.

    \todo Replace 'for_all_library_paths' either with a STL iteration, 
	  or else a custom inline function.

    \todo Figure out what to do with 'write_entire_game_state' and 
	  'save_game' since they really don't belong this low.

    \todo Try to get rid of Win32-specific 'extract_dirname'.

    Defines constant and function macros for the project.\n
    Declares utility functions for the project.
*/

#ifndef XCONQ_FS_H
#define XCONQ_FS_H

#include "util/base.h"

// Default names for special files.

#ifndef NEWSFILE
//! News file.
#ifdef WIN32
#define NEWSFILE "XconqNews.txt"
#else
#define NEWSFILE "XconqNews"
#endif
#endif

#ifndef ERRORFILE
//! Error save file.
#define ERRORFILE "ErrorSave.xcq"
#endif

NAMESPACE_XCONQ_BEGIN
NAMESPACE_UTILITY_BEGIN

// Filesystem Paths

//! Make platform-specific filesystem pathname.
extern void make_pathname(char *path, char *name, char *extn, char *pathbuf);

//! Make platform-specific filesystem pathname for a game file.
extern char *game_filename(char *namevar, char* defaultname);

//! Make platform-specific filesystem pathname for an error save file.
extern char *error_save_filename(void);

//! Get default library pathname.
extern char *default_library_pathname(void);

//! Make default images pathname.
extern char *default_images_pathname(char *libpath);

//! Get news filename.
extern char *news_filename(void);

//! Get directory to save games to.
extern char *game_homedir(void);

//! Open library file.
extern FILE *open_library_file(char *filename);

//! Open score file for reading.
extern FILE *open_scorefile_for_reading(char *name);

//! Open score file for writing.
extern FILE *open_scorefile_for_writing(char *name);

//! Close score file.
extern void close_scorefile_for_writing(FILE *fp);

#ifdef WIN32
//! On Win32, we may need to special extraction of dirnames.
extern void extract_dirname(char **dirname, char *pathname);
#endif

//! Delete a file.
extern int remove_file(char *fname);

// Filesystem Path Chain

//! Node of filesystem path chain.
struct LibraryPath {
    //! Filesystem path.
    char *path;
    //! Next node.
    LibraryPath *next;
};

//! List of paths to Xconq game libraries.
extern LibraryPath *xconq_libs;

//! Pointer to last LibraryPath object loaded by user.
extern LibraryPath *last_user_xconq_lib;

//! Iterate library paths.
#define for_all_library_paths(p)  \
  for (p = xconq_libs; p != NULL; p = p->next)

//! Wrapper for opening files on various platforms.
extern FILE *open_file(char *filename, char *mode);

//! Save a game with proper filename.
/*! For coordinated saving of network games. */
extern void save_game(char *fname);

NAMESPACE_UTILITY_END
NAMESPACE_XCONQ_END

#endif // ifndef XCONQ_FS_H
