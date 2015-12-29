/* Tk-specific functions for image families in Xconq.
   Copyright (C) 1998, 1999 Stanley T. Shebs.

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

#ifdef MAC
#include <tclMacCommonPch.h>
#include <tclMac.h>			/* Includes tcl.h. */
#include <tkMac.h>			/* Includes tk.h. */
#else
#include <tcl.h>
#include <tk.h>
#endif /* MAC */

/* These must be included AFTER tcl.h and tk.h. */
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#ifdef __cplusplus
}
#endif

#include "tkimf.h"

/* Lifted from tk sources - XPutImage is not available to Windows port. */
#ifdef UNIX
#define TkPutImage(colors, ncolors, display, pixels, gc, image, destx, desty, srcx, srcy, width, height) \
	XPutImage(display, pixels, gc, image, destx, desty, srcx, \
	srcy, width, height);
#else
extern void TkPutImage(unsigned long *colors, int ncolors,
		       Display* display, Drawable d, GC gc, XImage* image,
		       int src_x, int src_y, int dest_x, int dest_y,
		       unsigned int width, unsigned int height);
#endif

extern Tcl_Interp *interp;

int tmp_valid;

Tk_Window tmp_root_window;

static Pixmap tk_load_bitmap(Tk_Window rootwin, char *name, char *ext,
			     int *w, int *h);
static void tk_interp_image(ImageFamily *imf, Image *img, int force);
static void tk_interp_image_1(ImageFamily *imf, Image *img, Image *subimg,
			      int subi, int force);
static void tk_make_color_pixmap(Tk_Window rootwin, ImageFamily *imf,
				 Image *img);
static void tk_make_mono_pixmap(Tk_Window rootwin, ImageFamily *imf,
				Image *img);
static void tk_make_mask_pixmap(Tk_Window rootwin, ImageFamily *imf,
				Image *img);

/* Stash for cloned image families. */

static ImageFamily *tkimages[MAXIMAGEFAMILIES];

int numtkimages = 0;

TkImage *
init_tk_image(Image *img)
{
    TkImage *tkimg;

    tkimg = (TkImage *) xmalloc(sizeof(TkImage));
    /* Point to the generic image. */
    tkimg->generic = img;
    tkimg->mono = None;
    tkimg->mask = None;
    tkimg->colr = None;
    tkimg->solid = NULL;
    return tkimg;
}

TkImage *
get_tk_image(Image *img)
{
    TkImage *tkimg;

    if (img->hook)
      return (TkImage *) img->hook;
    tkimg = init_tk_image(img);
    img->hook = (char *) tkimg;
    return tkimg;
}

/* This tries to fill in the given image family by looking for and loading
   standard X11 bitmap files. */

ImageFamily *
tk_load_imf(ImageFamily *imf)
{
    int w, h;
    Pixmap pic;
    Image *img;
    Tk_Window rootwin = None;
    TkImageFamily *tkimf;
    TkImage *tkimg;

    /* If no imf or no name, don't even try. */
    if (imf == NULL || imf->name == NULL)
      return NULL;
    if (strcmp(imf->name, "none") == 0)
      return imf;
    if (tmp_valid) {
	if (imf->hook == NULL) {
	    imf->hook = (char *)xmalloc(sizeof(TkImageFamily));
	    tkimf = (TkImageFamily *) imf->hook;
	    tkimf->rootwin = tmp_root_window;
	    /* Record the cloned family for later lookup. */
	    tkimages[numtkimages++] = imf;
	}
    }
    tkimf = (TkImageFamily *) imf->hook;
    if (tkimf) {
	rootwin = tkimf->rootwin;
    }
    /* Grab at plausibly-named bitmaps. */
    pic = tk_load_bitmap(rootwin, imf->name, "b", &w, &h);
    if (pic != None) {
	img = get_img(imf, w, h);
	tkimg = get_tk_image(img);
	tkimg->mono = pic;
    }
    pic = tk_load_bitmap(rootwin, imf->name, "m", &w, &h);
    if (pic != None) {
	img = get_img(imf, w, h);
	tkimg = get_tk_image(img);
	tkimg->mask = pic;
    }
    pic = tk_load_bitmap(rootwin, imf->name, "8.b", &w, &h);
    if (pic != None) {
	img = get_img(imf, w, h);
	tkimg = get_tk_image(img);
	tkimg->mono = pic;
    }
    pic = tk_load_bitmap(rootwin, imf->name, "8.m", &w, &h);
    if (pic != None) {
	img = get_img(imf, w, h);
	tkimg = get_tk_image(img);
	tkimg->mask = pic;
    }
    pic = tk_load_bitmap(rootwin, imf->name, "32.b", &w, &h);
    if (pic != None) {
	img = get_img(imf, w, h);
	tkimg = get_tk_image(img);
	tkimg->mono = pic;
    }
    pic = tk_load_bitmap(rootwin, imf->name, "32.m", &w, &h);
    if (pic != None) {
	img = get_img(imf, w, h);
	tkimg = get_tk_image(img);
	tkimg->mask = pic;
    }
    return imf;
}

/* Try to load a bitmap of the given name, looking in both the current dir
   and the library dir. */

static Pixmap
tk_load_bitmap(Tk_Window rootwin, char *name, char *ext, int *wp, int *hp)
{
#if 0
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
#endif
    return None;
}

/* if (force): prefer data over rawdata; always re-create pixmaps */

ImageFamily *
tk_interp_imf(ImageFamily *imf, Image *img, int force)
{
    TkImageFamily *tkimf;

    if (tmp_valid) {
	if (imf->hook == NULL) {
	    imf->hook = (char *)xmalloc(sizeof(TkImageFamily));
	    tkimf = (TkImageFamily *) imf->hook;
	    tkimf->rootwin = tmp_root_window;
	    /* Record the cloned family for later lookup. */
	    tkimages[numtkimages++] = imf;
	}
    }
    if (img == NULL) {
	for_all_images(imf, img) {
	    tk_interp_image(imf, img, force);
	}
    } else {
	tk_interp_image(imf, img, force);
    }
    return imf;
}

/* Interpret an image and any possible subimages. */

static void
tk_interp_image(ImageFamily *imf, Image *img, int force)
{
    int subi;

    /* Skip all basic terrain images except power 4 and 5
    if we are low on memory. */
    if (poor_memory
        && img->isterrain
        && img->w != 24
        && img->w != 44) {
    	return;
    }
    /* Terrain patterns are not supported yet on Windows, 
    so no need to load them there. */
    if (!use_clip_mask
    	&& img->istile
    	&& img->w > 1) {
    	return;
    }
    /* Iterate through any subimages. */
    if (img->isborder && img->subimages != NULL) {
	for (subi = 0; subi < 16; ++subi) {
	    tk_interp_image_1(imf, img, img->subimages[subi], subi, force);
	}
    } else if (img->isconnection && img->subimages != NULL) {
	for (subi = 0; subi < 64; ++subi) {
	    tk_interp_image_1(imf, img, img->subimages[subi], subi, force);
	}
    } else if (img->istransition && img->subimages != NULL) {
	for (subi = 0; subi < 4 * 4; ++subi) {
	    tk_interp_image_1(imf, img, img->subimages[subi], subi, force);
	}
    } else if (img->numsubimages > 0 && img->subimages != NULL) {
	for (subi = 0; subi < img->numsubimages; ++subi) {
	    tk_interp_image_1(imf, img, img->subimages[subi], subi, force);
	}
    } else {
	tk_interp_image_1(imf, img, img, 0, force);
    }
}

/* Interpret a single image/subimage. */

static void
tk_interp_image_1(ImageFamily *imf, Image *img, Image *subimg, int subi,
		  int force)
{
    char namebuf[BUFSIZE];
    int w, h, numbytes;
    int dummy, red, grn, blu;
    TkImageFamily *tkimf;
    TkImage *tkimg;
    Tk_Window rootwin;
    Tk_Uid bitmapid;
    XColor col;

    w = img->w;  h = img->h;
    tkimg = get_tk_image(subimg);
    /* Collect the root window, failing if none exists. */
    tkimf = (TkImageFamily *) imf->hook;
    if (tkimf == NULL)
      return;
    rootwin = tkimf->rootwin;

    /* A 1x1 image is just a color - make it into a solid color. */
    if (w == 1 && h == 1 && img->palette != lispnil) {
	/* (should do generic work in generic code) */
	/* force this? */
	img->numcolors = 1;
	parse_lisp_palette_entry(car(img->palette), &dummy, &red, &grn, &blu);
	img->r = red;  img->g = grn;  img->b = blu;
	col.red = red;  col.green = grn;  col.blue = blu;
	tkimg->solid = Tk_GetColorByValue(rootwin, &col);
	if (tkimg->solid == NULL)
	  init_warning("Cannot get color #%2.2x%2.2x%2.2x for %s",
		       col.red, col.green, col.blue, imf->name);
	return;
    }
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
       turn into Tk-specific objects. */
    if (use_clip_mask) {
	/* If clip masks work (real X11), then we can create bitmaps for
	   the mono and mask data. */
	if (subimg->rawmonodata != NULL && (tkimg->mono == None || force)) {
	    numbytes = h * computed_rowbytes(w, 1);
	    reverse_bit_endianness(subimg->rawmonodata, numbytes);
	    sprintf(namebuf, "%s.%d.%d.%d.mono", imf->name, w, h, subi);
	    bitmapid = Tk_GetUid(namebuf);
	    Tk_DefineBitmap(interp, bitmapid, subimg->rawmonodata, w, h);
	    tkimg->mono = Tk_GetBitmap(interp, rootwin, bitmapid);
	    reverse_bit_endianness(subimg->rawmonodata, numbytes);
	}
	if (subimg->rawmaskdata != NULL	&& (tkimg->mask == None || force)) {
	    numbytes = h * computed_rowbytes(w, 1);
	    reverse_bit_endianness(subimg->rawmaskdata, numbytes);
	    sprintf(namebuf, "%s.%d.%d.%d.mask", imf->name, w, h, subi);
	    bitmapid = Tk_GetUid(namebuf);
	    Tk_DefineBitmap(interp, bitmapid, subimg->rawmaskdata, w, h);
	    tkimg->mask = Tk_GetBitmap(interp, rootwin, bitmapid);
	    reverse_bit_endianness(subimg->rawmaskdata, numbytes);
	}
    } else {
	/* Logical operations depend on the to-be-masked areas of images
	   being all black. */
	if (subimg->rawmaskdata != NULL) {
	    if (subimg->rawcolrdata != NULL)
	      blacken_masked_area(imf, subimg, 0, 0, 0);
	    if (subimg->rawmonodata != NULL)
	      blacken_mono_masked_area(imf, subimg, 0, 0, 0);
	}
	/* Make mono and mask data into b/w pixmaps of full depth, to
	   use with AND/OR operations. */
	if (subimg->rawmonodata	&& (tkimg->mono == None || force)) {
	    tk_make_mono_pixmap(rootwin, imf, subimg);
	}
	if (subimg->rawmaskdata	&& (tkimg->mask == None || force)) {
	    tk_make_mask_pixmap(rootwin, imf, subimg);
	}
    }
    /* Whether using clip masks or logical ops, color pixmap creation
       works the same. */
    if (subimg->rawcolrdata && (tkimg->colr == None || force)) {
	tk_make_color_pixmap(rootwin, imf, subimg);
    }
}

/* TK bitmaps are always in little-endian bit order, while IMF images
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

static int try_images = 1;

static int use_images;

#if ( defined(UNIX) || defined(MAC) )
static XImage *test_image;
#endif
static int imdepth;

static char *ximbuf;
static XImage *xim;

static void
tk_make_color_pixmap(Tk_Window rootwin, ImageFamily *imf, Image *img)
{
    int r, ri, rc, depth, c, rmask, pix, use_images_here;
    XColor col, *color;
    char *rp;
    GC gc;
    TkImage *tkimg = (TkImage *) img->hook;
    Display *dpy = Tk_Display(rootwin);

    if (tkimg == NULL || img->rawcolrdata == NULL)
      return;
    /* Can't make color pixmaps if we don't have any colors. */
    if (img->palette == lispnil && img->rawpalette == NULL)
      return;
    DGprintf("Starting %dx%d colr pixmap for %s\n", img->w, img->h, imf->name);
    if (img->rawpalette == NULL)
      make_raw_palette(img);
    Tk_MakeWindowExist(rootwin);
    depth = DefaultDepthOfScreen(Tk_Screen(rootwin));
    if (img->numcolors <= 0) {
	run_warning("No colors?");
	return;
    }
    /* Allocate colors. */
    tkimg->cmap = 
	(XColor **) xmalloc(256 /*img->numcolors*/ * sizeof(XColor *));
    for (c = 0; c < img->numcolors; c++) {
	col.red   = img->rawpalette[4 * c + 1];
	col.green = img->rawpalette[4 * c + 2];
	col.blue  = img->rawpalette[4 * c + 3];
	color = Tk_GetColorByValue(rootwin, &col);
	if (color == NULL)
	  init_warning("Cannot get color #%2.2x%2.2x%2.2x for %s",
		       col.red, col.green, col.blue, imf->name);
	tkimg->cmap[img->rawpalette[4 * c + 0]] = color;
    }
    tkimg->colr = Tk_GetPixmap(dpy, Tk_WindowId(rootwin), img->w, img->h,
			       depth);
    if (tkimg->colr == None) {
	init_warning("color pixmap creation failed");
	return;
    }
    /* Draw the image by plotting each point separately. */
    rmask = (1 << img->pixelsize) - 1;
    rp = img->rawcolrdata;
    gc = Tk_GetGC(rootwin, None, NULL);
    XSetClipMask(dpy, gc, None);
    XSetFillStyle(dpy, gc, FillSolid);
    if (try_images) {
	/* (no XGetImage for color images on Windows) */
#if ( defined(UNIX) || defined(MAC) )
	if (test_image == NULL) {
	    test_image = XGetImage(dpy, tkimg->colr, 0, 0, img->w, img->h,
				   AllPlanes, ZPixmap);
	    /* Only use images with 16-bit color for now. */
	    if (test_image->depth == 16) {
		use_images = TRUE;
		imdepth = 16;
	    }
	}
#else
	/* On Windows, we always seem to have 32-bit images. */
	use_images = TRUE;
	imdepth = 32;
#endif
    }
    use_images_here = use_images;
    if ((img->w & 1) == 1 || (img->h & 1) == 1)
      use_images_here = FALSE;
    if (use_images_here) {
#if ( defined(UNIX) || defined(MAC) )
	ximbuf = (char *) xmalloc(img->w * img->h * (imdepth / 8));
#else
	/* (should do this for both Unix and Windows...) */
	Visual *vis;
	vis = Tk_Visual(rootwin);
	ximbuf = (char *) xmalloc(img->w * img->h * (imdepth / 8));
	xim = XCreateImage(dpy, vis, imdepth, ZPixmap, 0, ximbuf,
			   img->w, img->h, 32, 0);
#endif
    }
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
	    pix = 0;
	    if (tkimg->cmap[rc] && rc < 256)
	      pix = tkimg->cmap[rc]->pixel;
	    if (use_images_here) {
		if (imdepth == 16) {
#if 1
		    *(((short *) ximbuf) + (img->w * r + c)) = pix;
#endif
#if 0
		    ximbuf[(img->w * r + c) * 2 + 1] = pix & 0xff;
		    ximbuf[(img->w * r + c) * 2 + 0] = pix >> 8;
#endif
		} else {
		    XPutPixel(xim, c, r, pix);
		}
	    } else {
		XSetForeground(dpy, gc, pix);
		XFillRectangle(dpy, tkimg->colr, gc, c, r, 1, 1);
	    }
	}
	if ((img->pixelsize * img->w) % 8) {
	    rp++;
	}
    }
    if (use_images_here) {
#if 0 /* this is probably the right approach to use someday */
	int numvisuals;
	XVisualInfo visinfo, *visinfoptr;

	visinfo.screen = Tk_ScreenNumber(rootwin);
	visinfo.visualid = XVisualIDFromVisual(Tk_Visual(rootwin));
	visinfoptr = XGetVisualInfo(dpy, VisualScreenMask | VisualIDMask,
				    &visinfo, &numvisuals);
#endif
#if ( defined(UNIX) || defined(MAC) )
	Visual *vis;
	vis = Tk_Visual(rootwin);
	xim = XCreateImage(dpy, vis, imdepth, ZPixmap, 0, ximbuf,
			   img->w, img->h, 32, 0);
#endif
	/* Move the image's bits into the pixmap. */
	TkPutImage(NULL, 0, dpy, tkimg->colr, gc, xim, 0, 0, 0, 0,
		   img->w, img->h);
	/* Destroy XImage once it is no longer needed. */
	XDestroyImage(xim);
    }
    Tk_FreeGC(dpy, gc);
    DGprintf("   Finished\n");
}

static void
tk_make_mono_pixmap(Tk_Window rootwin, ImageFamily *imf, Image *img)
{
    int r, ri, rc, depth, c, rmask, pix, use_images_here;
    XColor col;
    char *rp;
    GC gc;
    TkImage *tkimg = (TkImage *) img->hook;
    Display *dpy = Tk_Display(rootwin);
    XColor *whitecolor, *blackcolor;

    if (tkimg == NULL || img->rawmonodata == NULL)
      return;
    DGprintf("Starting %dx%d mono pixmap for %s\n", img->w, img->h, imf->name);
    Tk_MakeWindowExist(rootwin);
    depth = DefaultDepthOfScreen(Tk_Screen(rootwin));
    /* Note that depth is expected to be > 1 - we're going to paint black
       and white into a color image. */
    col.red = col.green = col.blue = 65535;
    whitecolor = Tk_GetColorByValue(rootwin, &col);
    col.red = col.green = col.blue = 0;
    blackcolor = Tk_GetColorByValue(rootwin, &col);
    tkimg->mono = Tk_GetPixmap(dpy, Tk_WindowId(rootwin), img->w, img->h,
			       depth);
    if (tkimg->mono == None) {
	init_warning("mono pixmap creation failed");
	return;
    }
    rmask = 1;
    rp = img->rawmonodata;
    gc = Tk_GetGC(rootwin, None, NULL);
    XSetClipMask(dpy, gc, None);
    XSetFillStyle(dpy, gc, FillSolid);
    if (try_images) {
#if ( defined(UNIX) || defined(MAC) )
	if (test_image == NULL) {
	    test_image = XGetImage(dpy, tkimg->mono, 0, 0, img->w, img->h,
				   AllPlanes, ZPixmap);
	    /* Only use images with 16-bit color for now. */
	    if (test_image->depth == 16) {
		use_images = TRUE;
	    }
	}
#else
	/* (should create b/w image) */
	use_images = TRUE;
#endif
    }
    use_images_here = use_images;
    if ((img->w & 1) == 1 || (img->h & 1) == 1)
      use_images_here = FALSE;
    if (use_images_here) {
	ximbuf = (char *) xmalloc(img->w * img->h * 2 * sizeof(char));
    }
    for (r = 0; r < img->h; r++) {
	ri = 7;
	for (c = 0; c < img->w; c++) {
	    /* imf decoding stuff */
	    rc = ((int) (*rp >> ri)) & rmask;
	    if (ri) {
		ri -= 1;
	    } else {
		ri = 7;
		rp++;
	    }
	    pix = (rc ? blackcolor : whitecolor)->pixel;
	    if (use_images_here) {
		ximbuf[(img->w * r + c) * 2 + 0] = pix & 0xff;
		ximbuf[(img->w * r + c) * 2 + 1] = pix >> 8;
	    } else {
		XSetForeground(dpy, gc, pix);
		XFillRectangle(dpy, tkimg->mono, gc, c, r, 1, 1);
	    }
	}
	if (img->w % 8) {
	    rp++;
	}
    }
    if (use_images_here) {
	Visual *vis;

	vis = Tk_Visual(rootwin);
	xim = XCreateImage(dpy, vis, 16, ZPixmap, 0, ximbuf, img->w, img->h,
			   32, 0);

	/* Move the image's bits into the pixmap. */
	TkPutImage(NULL, 0, dpy, tkimg->mono, gc, xim, 0, 0, 0, 0,
		   img->w, img->h);
	XDestroyImage(xim);
    }
    Tk_FreeGC(dpy, gc);
    DGprintf("    Finished\n");
}

static void
tk_make_mask_pixmap(Tk_Window rootwin, ImageFamily *imf, Image *img)
{
    int r, ri, rc, depth, c, rmask, pix, use_images_here;
    XColor col;
    char *rp;
    GC gc;
    TkImage *tkimg = (TkImage *) img->hook;
    Display *dpy = Tk_Display(rootwin);
    XColor *whitecolor, *blackcolor;

    if (tkimg == NULL || img->rawmaskdata == NULL)
      return;
    DGprintf("Starting %dx%d mask pixmap for %s\n", img->w, img->h, imf->name);
    Tk_MakeWindowExist(rootwin);
    depth = DefaultDepthOfScreen(Tk_Screen(rootwin));
    col.red = col.green = col.blue = 65535;
    whitecolor = Tk_GetColorByValue(rootwin, &col);
    col.red = col.green = col.blue = 0;
    blackcolor = Tk_GetColorByValue(rootwin, &col);
    tkimg->mask = Tk_GetPixmap(dpy, Tk_WindowId(rootwin), img->w, img->h,
			       depth);
    if (tkimg->mask == None) {
	init_warning("mask pixmap creation failed");
	return;
    }
    rmask = 1;
    rp = img->rawmaskdata;
    gc = Tk_GetGC(rootwin, None, NULL);
    XSetClipMask(dpy, gc, None);
    XSetFillStyle(dpy, gc, FillSolid);
    if (try_images) {
#if ( defined(UNIX) || defined(MAC) )
	if (test_image == NULL) {
	    test_image = XGetImage(dpy, tkimg->mask, 0, 0, img->w, img->h,
				   AllPlanes, ZPixmap);
	    /* Only use images with 16-bit color for now. */
	    if (test_image->depth == 16) {
		use_images = TRUE;
	    }
	}
#else
	use_images = TRUE;
#endif
    }
    use_images_here = use_images;
    if ((img->w & 1) == 1 || (img->h & 1) == 1)
      use_images_here = FALSE;
    if (use_images_here) {
	ximbuf = (char *) xmalloc(img->w * img->h * 2 * sizeof(char));
    }
    for (r = 0; r < img->h; r++) {
	ri = 7;
	for (c = 0; c < img->w; c++) {
	    /* imf decoding stuff */
	    rc = ((int) (*rp >> ri)) & rmask;
	    if (ri) {
		ri -= 1;
	    } else {
		ri = 7;
		rp++;
	    }
	    pix = (rc ? blackcolor : whitecolor)->pixel;
	    if (use_images_here) {
		ximbuf[(img->w * r + c) * 2 + 0] = pix & 0xff;
		ximbuf[(img->w * r + c) * 2 + 1] = pix >> 8;
	    } else {
		XSetForeground(dpy, gc, pix);
		XFillRectangle(dpy, tkimg->mask, gc, c, r, 1, 1);
	    }
	}
	if (img->w % 8) {
	    rp++;
	}
    }
    if (use_images_here) {
	Visual *vis;

	/* Create XImage to put IMF pixels into. */
	vis = Tk_Visual(rootwin);
	xim = XCreateImage(dpy, vis, 16, ZPixmap, 0, ximbuf, img->w, img->h,
			   32, 0);
	/* Move XImage's bits into the pixmap. */
	TkPutImage(NULL, 0, dpy, tkimg->mask, gc, xim, 0, 0, 0, 0,
		   img->w, img->h);
	/* Free up XImage once it has been transferred to the pixmap. */
	XDestroyImage(xim);
    }
    Tk_FreeGC(dpy, gc);
    DGprintf("    Finished\n");
}

void
make_generic_image_data(ImageFamily *imf)
{
    /* (should write impl?) */
}

ImageFamily *
tk_find_imf(const char *name)
{
    int i;
    ImageFamily *imf;
    Tk_Window tkwin;

    for (i = 0; i < numtkimages; ++i)
      if (strcmp(tkimages[i]->name, name) == 0)
	return tkimages[i];
    tkwin = Tk_MainWindow(interp);
    tmp_root_window = tkwin;
    tmp_valid = TRUE;
    imf = get_generic_images(name);
    tmp_valid = FALSE;
    return imf;
}
