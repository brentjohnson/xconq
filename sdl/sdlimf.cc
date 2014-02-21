/* SDL-specific functions for image families in Xconq.
   Copyright (C) 2000, 2001 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "sdlpreconq.h"
#include "config.h"
#include "misc.h"
#include "lisp.h"
#include "module.h"
#include "system.h"
#include "imf.h"

#include "SDL.h"

extern Uint32 random_color(SDL_Surface *surf);

extern ImageFamily *sdl_interp_imf(ImageFamily *imf, Image *img, int force);

static void sdl_interp_image(ImageFamily *imf, Image *img, int force);
static void sdl_interp_image_1(ImageFamily *imf, Image *img, Image *subimg,
			       int subi, int force);

static void sdl_copy_color_image(ImageFamily *imf, Image *img,
				 SDL_Surface *surface);
static void sdl_copy_mono_image(Image *img, SDL_Surface *surface);

/* if (force): prefer data over rawdata; always re-create pixmaps */

ImageFamily *
sdl_interp_imf(ImageFamily *imf, Image *img, int force)
{
    if (img == NULL) {
	for_all_images(imf, img) {
	    sdl_interp_image(imf, img, force);
	}
    } else {
	sdl_interp_image(imf, img, force);
    }
    return imf;
}

/* Interpret an image and any possible subimages. */

static void
sdl_interp_image(ImageFamily *imf, Image *img, int force)
{
    int subi;

    if (img->numsubimages > 0 && img->subimages != NULL) {
	/* Iterate through any subimages. */
	for (subi = 0; subi < img->numsubimages; ++subi) {
	    sdl_interp_image_1(imf, img, img->subimages[subi], subi, force);
	}
    } else {
	sdl_interp_image_1(imf, img, img, 0, force);
    }
}

/* Interpret a single image/subimage. */

static void
sdl_interp_image_1(ImageFamily *imf, Image *img, Image *subimg, int subi,
		  int force)
{
    int w, h, numbytes;
    int dummy, red, grn, blu;
    SDL_Surface *surf;

    w = img->w;  h = img->h;
    /* A 1x1 image is just a color - make it into a solid color. */
    if (w == 1 && h == 1 && img->palette != lispnil) {
	/* (should do generic work in generic code) */
	/* force this? */
	img->numcolors = 1;
	parse_lisp_palette_entry(car(img->palette), &dummy, &red, &grn, &blu);
	img->r = red;  img->g = grn;  img->b = blu;
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
       turn into an SDL surface. */
    surf = SDL_AllocSurface(SDL_SWSURFACE, img->w, img->h, 32,
			    0x00ff0000, 0x0000ff00, 0x000000ff, 0);
    subimg->hook = (char *) surf;
    if (img->rawpalette == NULL)
      make_raw_palette(img);
    if (subimg->rawcolrdata != NULL) {
	sdl_copy_color_image(imf, subimg, surf);
    } else if (subimg->rawmonodata != NULL) {
	sdl_copy_mono_image(subimg, surf);
    } else {
	SDL_FillRect(surf, NULL, random_color(surf));
    }
}

static void
sdl_copy_color_image(ImageFamily *imf, Image *img, SDL_Surface *surface)
{
    int rmask, r, ri, c, rc, col;
    char *rp;
    Uint32 *row;

    /* First make all the masked areas into the color that we will use
       for the color key. */
    if (img->rawmaskdata != NULL)
      blacken_masked_area(imf, img, 65535, 0, 65535);
    rmask = (1 << img->pixelsize) - 1;
    rp = img->rawcolrdata;
    for (r = 0; r < img->h; r++) {
	/* Set the initial intra-byte position. */
	ri = 8 - img->pixelsize;
	row = (Uint32 *) ((Uint8 *) surface->pixels + r * surface->pitch);
	for (c = 0; c < img->w; c++) {
	    /* Extract a pixel from the current byte. */
	    rc = ((int) (*rp >> ri)) & rmask;
	    /* Convert into an SDL-type color. */
	    col = (((img->rawpalette[4 * rc + 1] / 256) << 16)
		   | ((img->rawpalette[4 * rc + 2] / 256) << 8)
		   | ((img->rawpalette[4 * rc + 3] / 256)));
	    *row++ = col;
	    /* Go to the next pixel. */
	    if (ri) {
		ri -= img->pixelsize;
	    } else {
		ri = 8 - img->pixelsize;
		++rp;
	    }
	}
	/* Skip to the next byte if the width is not an even number of
	   bytes. */
	if ((img->pixelsize * img->w) % 8)
	  ++rp;
    }
    SDL_SetColorKey(surface, SDL_SRCCOLORKEY|SDL_RLEACCEL,
		    SDL_MapRGB(surface->format, 255, 0, 255));
}

static void
sdl_copy_mono_image(Image *img, SDL_Surface *surface)
{
    int rmask, rmmask, r, ri, rmi, c, rc, rmc;
    int col;
    char *rp, *rmp;
    Uint32 *row;

    rmask = 1;
    rp = img->rawmonodata;
    rmmask = 1;
    rmp = img->rawmaskdata;
    for (r = 0; r < img->h; r++) {
	ri = 7;
	rmi = 7;
	row = (Uint32 *) ((Uint8 *) surface->pixels + r * surface->pitch);
	for (c = 0; c < img->w; c++) {
	    /* imf decoding stuff */
	    rc = ((int) (*rp >> ri)) & rmask;
	    col = (rc ? 0 : 0xffffff);
	    if (rmp) {
		rmc = ((int) (*rmp >> rmi)) & rmmask;
		if (rmc == 0)
		  col = 0xff00ff;
	    }
	    *row++ = col;
	    if (ri) {
		ri -= 1;
	    } else {
		ri = 7;
		rp++;
	    }
	    if (rmi) {
		rmi -= 1;
	    } else {
		rmi = 7;
		if (rmp)
		  ++rmp;
	    }
	}
	if (img->w % 8) {
	    ri = 7;
	    ++rp;
	}
	if (img->w % 8) {
	    rmi = 7;
	    if (rmp)
	      ++rmp;
	}
    }
    SDL_SetColorKey(surface, SDL_SRCCOLORKEY|SDL_RLEACCEL,
		    SDL_MapRGB(surface->format, 255, 0, 255));
}

void
make_generic_image_data(ImageFamily *imf)
{
    /* (should write impl?) */
}
