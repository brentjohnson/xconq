/* Definitions of system-specific things for Xconq.
   Copyright (C) 1995, 1996, 1997, 1999, 2000 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/*! \file kernel/system.h
 * \brief Definitions of system-specific things for Xconq.
 * \note this assumes module.h is already included.
 */

/* (Note that limits.h or hlimits.h may define PATH_LEN_MAX.) */
/* (1024 bytes should be fairly safe on most systems) */
#define XCQ_PATH_LEN_MAX 1024

extern char *default_library_pathname(void);
extern char *default_images_pathname(char *libpath);
extern char *news_filename(void);
extern char *game_homedir(void);
extern char *game_filename(char *namevar, char* defaultname);
extern FILE *open_library_file(char *filename);
extern FILE *open_scorefile_for_reading(char *name);
extern FILE *open_scorefile_for_writing(char *name);
extern void close_scorefile_for_writing(FILE *fp);
extern void make_pathname(char *path, char *name, char *extn, char *pathbuf);
#ifdef WIN32
extern void extract_dirname(char **dirname, char *pathname);
#endif
extern int remove_file(char *fname);
extern char *error_save_filename(void);
extern void init_signal_handlers(void);
extern int n_seconds_elapsed(int n);
extern int n_ms_elapsed(int n);
extern void record_ms(void);

extern int open_remote_connection(char *methodname, int willhost);
extern void low_send(int rid, char *buf);
extern int low_receive(int *rid, char *buf, int maxchars, int timeout);
extern void close_remote_connection(int rid);

