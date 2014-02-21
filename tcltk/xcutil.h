/* Definitions used in X utility programs.
   Copyright (C) 1993-1995, 1997, 1999 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

extern void close_displays(void);
extern int write_entire_game_state(char *fname);
extern int keyword_code(char *str);
extern Image *read_xbm_file(char *filename, ImageFamily *imf,
			    readimf_hook hook);
extern Image *read_xpm_file(char *filename, ImageFamily *imf,
			    readimf_hook hook);
extern void parse_xpm_colors(char *name, int *r, int *g, int *b); 
extern void write_xpm_file(FILE *fp, char *name, Image *img);
extern void write_x11_bitmaps(ImageFamily *imf, int mkfiles);
extern void write_xbm_file(FILE *fp, char *name, int cols, int rows,
			   char *data);
extern char *find_imf_name(char *rawname);
extern int read_any_file(char *filename, readimf_hook hook);
extern void reverse_rawdata(ImageFamily *imf);
