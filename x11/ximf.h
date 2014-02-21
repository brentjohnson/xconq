/* Definitions of image families for the X11 interface to Xconq.
   Copyright (C) 1992-1995, 1998, 1999 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/* Structure for X11-specific data. */

/* Note that this structure has an implicit display and therefore player,
   because pixmaps and fonts always have a particular associated display. */

#ifdef __cplusplus
extern "C" {
#endif

#include <X11/Intrinsic.h>

#ifdef __cplusplus
}
#endif

typedef struct a_x11_image_family {
    Display *dpy;		/* Display of family's pixmaps */
    Window rootwin;		/* Root window of family's pixmaps */
} X11ImageFamily;

typedef struct a_x11_image {
    Image *generic;		/* pointer to generic image */
    Pixmap mono;
    Pixmap colr;
    Pixmap mask;
    Pixel *colpix;
} X11Image;

extern X11Image *init_x11_image(Image *img);
extern X11Image *get_x11_image(Image *img);
extern ImageFamily *x11_load_imf(ImageFamily *imf);
extern ImageFamily *x11_interp_imf(ImageFamily *imf, Image *img, int force);
extern void reverse_bit_endianness(char *rawdata, int numbytes);
