/* Definitions for printing in Xconq.
   Copyright (C) 1994, 1996, 1999 Stanley T. Shebs and Massimo Campostrini.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/* Parameters to control view printing. */

typedef struct a_print_parameters {
    /* binary flags */
    int corner_coord;
    int terrain_dither;
    int terrain_double;
    int features;
    int cell_summary;
    int cm;  /* otherwise inches */
    /* integer flags */
    int names;
    /* lengths */
    double cell_size;
    double cell_grid_width;
    double border_width;
    double connection_width;
    double page_width;
    double page_height;
    double top_margin;
    double bottom_margin;
    double left_margin;
    double right_margin;
    /* gray levels */
    double terrain_gray;
    double enemy_gray;
} PrintParameters;

/* Prototypes. */

extern void init_ps_print(PrintParameters *pp);
extern void dump_ps_view(Side *side, PrintParameters *pp, const char *filename);
