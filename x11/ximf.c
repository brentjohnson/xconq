/* X11-specific functions for image families in Xconq.
   Copyright (C) 1992-1998 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "config.h"
#include "misc.h"
#include "lisp.h"
#include "module.h"
#include "system.h"
#include "imf.h"

#ifdef __cplusplus
extern "C" {
#endif

#if (!defined (__CYGWIN32__) && !defined(__MINGW32__))
#include <X11/Xos.h>
#endif
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>

#ifdef __cplusplus
}
#endif

#include "ximf.h"

int tmp_valid;

Display *tmp_display;

Window tmp_root_window;

static Pixmap x11_load_bitmap(Display *dpy, Window rootwin,
			      const char *name, char *ext, int *w, int *h);
static void x11_make_color_pixmap(Display *dpy, Window rootwin, Image *img);

X11Image *
init_x11_image(Image *img)
{
    X11Image *ximg;

    ximg = (X11Image *) xmalloc(sizeof(X11Image));
    /* Point to the generic image. */
    ximg->generic = img;
    ximg->mono = None;
    ximg->mask = None;
    ximg->colr = None;
    ximg->colpix = NULL;
    return ximg;
}

X11Image *
get_x11_image(Image *img)
{
    X11Image *ximg;

    if (img->hook)
      return (X11Image *) img->hook;
    ximg = init_x11_image(img);
    img->hook = (char *) ximg;
    return ximg;
}

/* This tries to fill in the given image family by looking for and loading
   standard X11 bitmap files. */

ImageFamily *
x11_load_imf(ImageFamily *imf)
{
    int w, h;
    Pixmap pic;
    Image *img;
    Display *dpy = NULL;
    Window rootwin = None;
    X11ImageFamily *ximf;
    X11Image *ximg;

    /* If no imf or no name, don't even try. */
    if (imf == NULL || imf->name == NULL)
      return NULL;
    if (strcmp(imf->name, "none") == 0)
      return imf;
    if (tmp_valid) {
	if (imf->hook == NULL) {
	    /* Make a copy of the imf that will be specific to the
	       current display and root window. */
	    imf = clone_imf(imf);
	    imf->hook = (char *)xmalloc(sizeof(X11ImageFamily));
	    ximf = (X11ImageFamily *) imf->hook;
	    ximf->dpy = tmp_display;
	    ximf->rootwin = tmp_root_window;
	}
    }
    ximf = (X11ImageFamily *) imf->hook;
    if (ximf) {
	dpy = ximf->dpy;
	rootwin = ximf->rootwin;
    }
    /* Grab at plausibly-named bitmaps. */
    pic = x11_load_bitmap(dpy, rootwin, imf->name, (char *) "b", &w, &h);
    if (pic != None) {
	img = get_img(imf, w, h);
	ximg = get_x11_image(img);
	ximg->mono = pic;
    }
    pic = x11_load_bitmap(dpy, rootwin, imf->name, (char *) "m", &w, &h);
    if (pic != None) {
	img = get_img(imf, w, h);
	ximg = get_x11_image(img);
	ximg->mask = pic;
    }
    pic = x11_load_bitmap(dpy, rootwin, imf->name, (char *) "8.b", &w, &h);
    if (pic != None) {
	img = get_img(imf, w, h);
	ximg = get_x11_image(img);
	ximg->mono = pic;
    }
    pic = x11_load_bitmap(dpy, rootwin, imf->name, (char *) "8.m", &w, &h);
    if (pic != None) {
	img = get_img(imf, w, h);
	ximg = get_x11_image(img);
	ximg->mask = pic;
    }
    pic = x11_load_bitmap(dpy, rootwin, imf->name, (char *) "32.b", &w, &h);
    if (pic != None) {
	img = get_img(imf, w, h);
	ximg = get_x11_image(img);
	ximg->mono = pic;
    }
    pic = x11_load_bitmap(dpy, rootwin, imf->name, (char *) "32.m", &w, &h);
    if (pic != None) {
	img = get_img(imf, w, h);
	ximg = get_x11_image(img);
	ximg->mask = pic;
    }
    return imf;
}

/* Try to load a bitmap of the given name, looking in both the current dir
   and the library dir. */

static Pixmap
x11_load_bitmap(Display *dpy, Window rootwin, const char *name, char *ext,
                int *wp, int *hp)
{
    int hotx, hoty;
    unsigned int w, h;
    Pixmap rslt;
    static char sbuf[BUFSIZE];

    if (ext != NULL) {
	make_pathname(NULL, name, ext, sbuf);
	if (XReadBitmapFile(dpy, rootwin, sbuf,
			    &w, &h, &rslt, &hotx, &hoty) == BitmapSuccess) {
	    DGprintf("Loaded bitmap \"%s\"\n", sbuf);
	    *wp = w;  *hp = h;
	    return rslt;
	}
    }
    return None;
}

/* if (force): prefer data over rawdata; always re-create pixmaps */

static void x11_interp_image (ImageFamily *imf, Image *img, int force);
static void x11_interp_image_1 (ImageFamily *imf, Image *img,
				Image *subimg, int subi, int force);

ImageFamily *
x11_interp_imf(ImageFamily *imf, Image *img, int force)
{
    X11ImageFamily *ximf;

    if (tmp_valid) {
	if (imf->hook == NULL) {
	    /* Make a copy of the imf that will be specific to the
	       current display and root window. */
	    imf = clone_imf(imf);
	    imf->hook = (char *)xmalloc(sizeof(X11ImageFamily));
	    ximf = (X11ImageFamily *) imf->hook;
	    ximf->dpy = tmp_display;
	    ximf->rootwin = tmp_root_window;
	}
    }
    if (img == NULL) {
	for_all_images(imf, img) {
	    x11_interp_image(imf, img, force);
	}
    } else {
	x11_interp_image(imf, img, force);
    }
    return imf;
}

/* Basically a duplicate of tk_interp_image.  Would be nice to have
   a better way of separating out the generic from the non-generic. */
static void
x11_interp_image(ImageFamily *imf, Image *img, int force)
{
    int subi;

    /* Iterate through any subimages. */
    if (img->isborder && img->subimages != NULL) {
	for (subi = 0; subi < 16; ++subi) {
	    x11_interp_image_1(imf, img, img->subimages[subi], subi, force);
	}
    } else if (img->isconnection && img->subimages != NULL) {
	for (subi = 0; subi < 64; ++subi) {
	    x11_interp_image_1(imf, img, img->subimages[subi], subi, force);
	}
    } else if (img->istransition && img->subimages != NULL) {
	for (subi = 0; subi < 4 * 4; ++subi) {
	    x11_interp_image_1(imf, img, img->subimages[subi], subi, force);
	}
    } else if (img->numsubimages > 0 && img->subimages != NULL) {
	for (subi = 0; subi < img->numsubimages; ++subi) {
	    x11_interp_image_1(imf, img, img->subimages[subi], subi, force);
	}
    } else {
	x11_interp_image_1(imf, img, img, 0, force);
    }
}

static void
x11_interp_image_1(ImageFamily *imf, Image *img, Image *subimg, int subi, int force)
{
    int w, h, numbytes;
    int dummy, red, grn, blu, screen;
    X11ImageFamily *ximf;
    X11Image *ximg;
    Display *dpy;
    Window rootwin;
    Colormap cmap;
    XColor col;

    w = img->w;  h = img->h;
    ximg = get_x11_image(subimg);
    ximf = (X11ImageFamily *) imf->hook;
    dpy = ximf->dpy;
    rootwin = ximf->rootwin;
    screen = DefaultScreen(dpy);
    cmap = XDefaultColormap(dpy, screen);

    /* A 1x1 image is just a color - make it into a solid color. */
    if (w == 1 && h == 1 && img->palette != lispnil) {
	/* force this? */
	img->numcolors = 1;
	parse_lisp_palette_entry(car(img->palette), &dummy,
				 &red, &grn, &blu);
	col.red = red;  col.green = grn;  col.blue = blu;
	col.flags = DoRed | DoGreen | DoBlue;
	ximg->colpix = (Pixel *) xmalloc(sizeof(Pixel));
	if (XAllocColor(dpy, cmap, &col)) {
	    ximg->colpix[0] = col.pixel;
	} else {
	    init_warning("Cannot alloc color #%2.2x%2.2x%2.2x, will leave black",
			 red, grn, blu);
	    ximg->colpix[0] = XBlackPixel(dpy, screen);
	}
    }

    /* Put the data in "raw" form before trying to turn it into
       X11-specific objects (this part should be made generic,
       in imf.c, rather than duplicated here and tkimf.c).  */
    if (subimg->colrdata != lispnil
	&& (subimg->rawcolrdata == NULL || force)) {
	numbytes = h * computed_rowbytes(w, subimg->pixelsize);
	subimg->rawcolrdata = (char *)xmalloc(numbytes);
	interp_bytes(subimg->colrdata, numbytes, subimg->rawcolrdata, 0);
    }
    if (subimg->rawcolrdata == NULL && img->filedata != lispnil) {
	make_image_from_file_image(imf, img, subimg, subi);
    }
    make_raw_mono_data(subimg, force);
    if (subimg->maskdata != lispnil
	&& (subimg->rawmaskdata == NULL || force)) {
	numbytes = h * computed_rowbytes(w, 1);
	subimg->rawmaskdata = (char *)xmalloc(numbytes);
	interp_bytes(subimg->maskdata, numbytes, subimg->rawmaskdata, 0);
    }
    /* At this point our data is known to be in "raw" form, ready to
       turn into X11-specific objects. */
    if (subimg->rawmonodata && (ximg->mono == None || force)) {
	numbytes = h * computed_rowbytes(w, 1);
	reverse_bit_endianness(subimg->rawmonodata, numbytes);
	ximg->mono =
	  XCreateBitmapFromData(dpy, rootwin, subimg->rawmonodata, w, h);
	reverse_bit_endianness(subimg->rawmonodata, numbytes);
    }
    if (subimg->rawmaskdata && (ximg->mask == None || force)) {
	numbytes = h * computed_rowbytes(w, 1);
	reverse_bit_endianness(subimg->rawmaskdata, numbytes);
	ximg->mask =
	  XCreateBitmapFromData(dpy, rootwin, subimg->rawmaskdata, w, h);
	reverse_bit_endianness(subimg->rawmaskdata, numbytes);
    }
    if (subimg->rawcolrdata && (ximg->colr == None || force)) {
	x11_make_color_pixmap(dpy, rootwin, subimg);
    }
}

/* X11 bitmaps are always in little-endian bit order, while IMF images
   are always big-endian in bit order, so we must reverse the bits
   in each byte individually. */

void
reverse_bit_endianness(char *rawdata, int numbytes)
{
    int i, j, byte, byte2;

    for (i = 0; i < numbytes; ++i) {
	byte = rawdata[i];
	byte2 = 0;
	for (j = 0; j < 8; ++j) {
	    byte2 = (byte2 << 1) | (byte & 1);
	    byte >>= 1;
	}
	rawdata[i] = byte2;
    }
}

static void
x11_make_color_pixmap(Display *dpy, Window rootwin, Image *img)
{
    int ipal[4][256];
    int r, ri, rc, depth, c, ln, screen, rsize, rmask;
    XColor col;
    char *rp;
    Obj *pal;
    Colormap cmap;
    GC bgc, cgc[256];
    X11Image *ximg = (X11Image *) img->hook;

    if (ximg == NULL || img->rawcolrdata == NULL)
      return;
    /* Can't make color pixmaps if we don't have any colors. */
    if (img->palette == lispnil && img->rawpalette == NULL)
      return;
    if (img->rawpalette == NULL) {
	/* Parse the Lispified palette. */
        /* (should allocate and store directly instead of using ipal) */
	c = 0;
	for (pal = img->palette; pal != lispnil; pal = cdr(pal)) {
	    parse_lisp_palette_entry(car(pal), &ipal[0][c],
				     &ipal[1][c], &ipal[2][c], &ipal[3][c]);
	    c++;
	}
	img->numcolors = c;
	if (c == 0)
	  return;
	/* store palette */
	img->rawpalette = (int *) xmalloc(img->numcolors * 4 * sizeof(int));
	for (c = 0; c < img->numcolors; c++) {
	    for (ln = 0; ln < 4; ln++) {
		img->rawpalette[4 * c + ln] = ipal[ln][c];
	    }
	}
    }
    screen = DefaultScreen(dpy);
    cmap = XDefaultColormap(dpy, screen);
    depth = DefaultDepth(dpy, screen);
    if (img->numcolors <= 0) {
	run_warning("No colors?");
	return;
    }
    /* Allocate colors. */
    ximg->colpix = (Pixel *) xmalloc(img->numcolors * sizeof(Pixel));
    for (c = 0; c < img->numcolors; c++) {
	col.red   = img->rawpalette[4 * c + 1];
	col.green = img->rawpalette[4 * c + 2];
	col.blue  = img->rawpalette[4 * c + 3];
	col.flags = DoRed | DoGreen | DoBlue;
	if (XAllocColor(dpy, cmap, &col)) {
	    ximg->colpix[c] = col.pixel;
	} else {
	    init_warning("Cannot alloc color #%2.2x%2.2x%2.2x, will leave black",
			 img->rawpalette[4 * c + 1],
			 img->rawpalette[4 * c + 2],
			 img->rawpalette[4 * c + 3]);
	    ximg->colpix[c] = XBlackPixel(dpy,screen);
	}
    }
    ximg->colr = XCreatePixmap(dpy, rootwin, img->w, img->h, depth);
    /* (should freak out if we can't make pixmaps?) */
    if (ximg->colr == None)
      return;
    /* Create the GCs, one for each color. */
    bgc = XCreateGC(dpy, ximg->colr, 0, NULL);
    XSetForeground(dpy, bgc, XBlackPixel(dpy, screen));
    for (c = 0; c < 256; c++) {
	cgc[c] = bgc;
    }
    for (c = 0; c < img->numcolors; c++) {
	cgc[img->rawpalette[4*c]] = XCreateGC(dpy, ximg->colr, 0, NULL);
	XSetForeground(dpy, cgc[img->rawpalette[4*c]], ximg->colpix[c]);
    }
    /* Draw the image by plotting each point using the set of GCs
       from above. */
    rsize = img->pixelsize;
    rmask = (1 << img->pixelsize) - 1;
    rp = img->rawcolrdata;
    for (r = 0; r < img->h; r++) {
	ri = 8 - img->pixelsize;
	for (c = 0; c < img->w; c++) {
	    /* imf decoding stuff */
	    rc = ((int) (*rp >> ri)) & rmask;
	    if (ri) {
		ri -= img->pixelsize;
	    } else {
		ri = 8 - img->pixelsize;
		rp++;
	    }
	    XDrawPoint(dpy, ximg->colr, cgc[rc], c, r);
	}
	if ((img->pixelsize * img->w) % 8) {
	    rp++;
	}
    }
    /* Clean up all the GCs. */
    XFreeGC(dpy, bgc);
    for (c = 0; c < img->numcolors; c++) {
	XFreeGC(dpy, cgc[img->rawpalette[4*c]]);
    }
}

void
make_generic_image_data(ImageFamily *imf)
{
    /* (should write impl?) */
}
