// xConq
// Lifecycle management and serialization/deserialization for GDL images.

// $Id: img.cc,v 1.3 2006/06/02 16:57:43 eric_mcdonald Exp $

/*//////////////////////////// COPYRIGHT NOTICE ////////////////////////////////

  Copyright (C) 1987-1989   Stanley T. Shebs
  Copyright (C) 1991-2001   Stanley T. Shebs
  Copyright (C) 2005-2006   Eric A. McDonald

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
    \brief Lifecycle management and serialization/deserialization for GDL images.
    \ingroup grp_gdl
*/

#include "gdl/media/imf.h"

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Local Enumerations: Image Layer Types

enum {
    K_MONO_,
    K_MASK_,
    K_COLR_,
    K_FILE_,
    K_OTHER_
};

// Global Variables: Behavior Modifiers

short poor_memory = FALSE;

// Local Function Delcarations: Queries

//! Get an image of given size from given family.
/*! Create new image, if necessary. */
static Image *get_subimg(ImageFamily *imf, int w, int h);

// Queries

Image *
get_subimg(ImageFamily *imf, int w, int h)
{
    Image *nimg;

    /* Not found; create a new image and add it to the family. */
    nimg = (Image *) xmalloc(sizeof(Image));
    nimg->w = w;  nimg->h = h;
    nimg->embedx = nimg->embedy = -1;
    nimg->embedw = nimg->embedh = -1;
    nimg->monodata = nimg->colrdata = nimg->maskdata = lispnil;
    nimg->filedata = lispnil;
    nimg->palette = lispnil;
    nimg->actualw = w;  nimg->actualh = h;
    nimg->notes = lispnil;
    nimg->bboxw = w;  nimg->bboxh = h;
    return nimg;
}

Image *
get_img(ImageFamily *imf, int w, int h)
{
    Image *img, *nimg, *previmg;

    for_all_images(imf, img) {
	if (w == img->w && h == img->h)
	  return img;
    }
    /* Not found; create a new image and add it to the family. */
    nimg = (Image *) xmalloc(sizeof(Image));
    nimg->w = w;  nimg->h = h;
    nimg->embedx = nimg->embedy = -1;
    nimg->embedw = nimg->embedh = -1;
    nimg->monodata = nimg->colrdata = nimg->maskdata = lispnil;
    nimg->filedata = lispnil;
    nimg->palette = lispnil;
    nimg->actualw = w;  nimg->actualh = h;
    nimg->notes = lispnil;
    nimg->bboxw = w;  nimg->bboxh = h;
    /* Rely on zeroing of xmalloc blocks to avoid clearing other fields. */
    /* Link in order by size, smallest first. */
    previmg = NULL;
    for_all_images(imf, img) {
	if ((nimg->w < img->w)
	    || (nimg->w == img->w && nimg->h < img->h))
	  break;
	previmg = img;
    }
    if (previmg != NULL) {
	nimg->next = previmg->next;
	previmg->next = nimg;
    } else {
	nimg->next = imf->images;
	imf->images = nimg;
    }
    ++(imf->numsizes);
    return nimg;
}

// Image Preparation

void
compute_image_bbox(Image *img)
{
    int numbytes, i, j = 0, byte, x, y, x1, x2, k;
    int xmin, ymin, xmax, ymax;
    char *data = NULL;
    Obj *datalist, *next;

    datalist = img->maskdata;
    numbytes = img->h * computed_rowbytes(img->w, 1);
    x = y = 0;
    xmin = img->w;  ymin = img->h;
    xmax = 0;  ymax = 0;
    for (i = 0; i < numbytes; ++i) {
	if (img->maskdata != lispnil) {
	    if (data == NULL || data[j] == '\0') {
		next = car(datalist);
		if (!stringp(next)) {
		    syntax_error(datalist, "garbage in image data list");
		    return;
		}
		data = c_string(next);
		j = 0;
		datalist = cdr(datalist);
	    }
	    /* Just skip over slashes, which are for readability only. */
	    if (data[j] == '/')
	      ++j;
	    byte = hextoi(data[j]) * 16 + hextoi(data[j+1]);
	    j += 2;
	} else if (img->rawmaskdata != NULL) {
	    byte = img->rawmaskdata[i] & 0xff;
	} else {
	    byte = 0xff;
	}
	if (byte != 0) {
	    /* Find the most-significant and least-significant bits in
	       the mask byte. */
	    x1 = x2 = -1;
	    k = 0;
	    while (byte != 0) {
		if ((byte & 0x1) != 0 && x2 < 0)
		  x2 = x + 7 - k;
		byte >>= 1;
		if (byte == 0 && x1 < 0)
		  x1 = x + 7 - k;
		++k;
	    }
	    xmin = min(x1, xmin);  ymin = min(y, ymin);
	    xmax = max(x2, xmax);  ymax = max(y, ymax);
	}
	x += 8;
	if (x >= img->w) {
	    x = 0;
	    ++y;
	}
    }
    /* Compute position and size of bounding box. */
    if (xmin <= xmax && ymin <= ymax) {
	img->bboxx = xmin;  img->bboxy = ymin;
	img->bboxw = xmax - xmin;  img->bboxh = ymax - ymin;
    }
}

// GDL I/O

void
interp_image(ImageFamily *imf, Obj *size, Obj *parts)
{
    int w, h, imtype, emx, emy, emw, emh, numsubs, subi;
    char *name;
    Image *img, *subimg;
    Obj *head, *rest, *typ, *prop, *proptype, *datalist;

    w = c_number(car(size));  h = c_number(cadr(size));
    img = get_img(imf, w, h);
    if (img == NULL)
      run_error("no image?");
    if (img->w == 1 && img->h == 1) {
	/* A color is more like a tile than an icon. */
	img->istile = TRUE;
	img->palette = cons(cons(new_number(0), parts), lispnil);
	return;
    }
    if (match_keyword(car(cddr(size)), K_TILE))
      img->istile = TRUE;
    if (match_keyword(car(cddr(size)), K_TERRAIN))
      img->isterrain = TRUE;
    if (match_keyword(car(cddr(size)), K_CONNECTION))
      img->isconnection = TRUE;
    if (match_keyword(car(cddr(size)), K_BORDER))
      img->isborder = TRUE;
    if (match_keyword(car(cddr(size)), K_TRANSITION))
      img->istransition = TRUE;
    numsubs = 0;
    for_all_list(parts, rest) {
	head = car(rest);
	typ = car(head);
	imtype = K_OTHER_;
	if (match_keyword(typ, K_MONO)) {
	    imtype = K_MONO_;
	} else if (match_keyword(typ, K_MASK)) {
	    imtype = K_MASK_;
	} else if (match_keyword(typ, K_COLOR)) {
	    imtype = K_COLR_;
	} else if (match_keyword(typ, K_FILE)) {
	    imtype = K_FILE_;
	} else if (match_keyword(typ, K_EMBED)) {
	    name = c_string(cadr(head));
	    if (img->embedname != NULL
		&& strcmp(img->embedname, name) != 0)
	      run_warning("Changing embed name from \"%s\" to \"%s\" in %dx%d image of \"%s\"",
			  img->embedname, name, w, h, imf->name);
	    img->embedname = name;
	} else if (match_keyword(typ, K_EMBED_AT)) {
	    emx = c_number(cadr(head));  emy = c_number(caddr(head));
	    if ((img->embedx >= 0 && emx != img->embedx)
		|| (img->embedy >= 0 && emy != img->embedy))
	      run_warning("Changing embed x,y from %d,%d to %d,%d in %dx%d image of \"%s\"",
			  img->embedx, img->embedy, emx, emy, w, h, imf->name);
	    img->embedx = emx;  img->embedy = emy;
	} else if (match_keyword(typ, K_EMBED_SIZE)) {
	    emw = c_number(cadr(head));  emh = c_number(caddr(head));
	    if ((img->embedw >= 0 && emw != img->embedw)
		|| (img->embedh >= 0 && emh != img->embedh))
	      run_warning("Changing embed w,h from %d,%d to %d,%d in %dx%d image of \"%s\"",
			  img->embedw, img->embedh, emw, emh, w, h, imf->name);
	    img->embedw = emw;  img->embedh = emh;
	} else if (match_keyword(typ, K_HEXGRID)) {
	    img->hexgridx = c_number(cadr(head));
	    img->hexgridy = c_number(caddr(head));
	    numsubs = img->hexgridx*img->hexgridy;
	} else if (match_keyword(typ, K_NOTES)) {
	    img->notes = cadr(head);
	    syntax_error(head, "extra junk after image notes property");
	} else if (match_keyword(typ, K_X)) {
	    numsubs = c_number(cadr(head));
	    if (cddr(head) != lispnil) {
		img->subx = c_number(caddr(head));
		img->suby = c_number(car(cdddr(head)));
	    }
	} else {
	    run_warning("unknown image property in \"%s\"", imf->name);
	}
	/* If there is no actual image data to process, skip to the next
	   clause in the form. */
	if (imtype == K_OTHER_)
	  continue;
	datalist = cdr(head);
	/* Interpret random image subproperties. */
	while (consp(car(datalist))) {
	    prop = car(datalist);
	    proptype = car(prop);
	    if (match_keyword(proptype, K_ACTUAL)) {
		img->actualw = c_number(cadr(prop));
		img->actualh = c_number(caddr(prop));
	    } else if (match_keyword(proptype, K_PIXEL_SIZE)) {
		img->pixelsize = c_number(cadr(prop));
	    } else if (match_keyword(proptype, K_PALETTE)) {
		img->palette = cdr(prop);
	    } else {
		char imferrbuf[200];

		sprintlisp(imferrbuf, prop, 100);
		run_warning("unknown image subproperty in \"%s\": %s",
			    imf->name, imferrbuf);
	    }
	    datalist = cdr(datalist);
	}
	switch (imtype) {
	  case K_MONO_:
	    if (img->monodata != lispnil && !equal(datalist, img->monodata))
	      run_warning("Changing mono data in %dx%d image of \"%s\"",
			  w, h, imf->name);
	    img->monodata = datalist;
	    break;
	  case K_COLR_:
	    if (img->colrdata != lispnil && !equal(datalist, img->colrdata))
	      run_warning("Changing color data in %dx%d image of \"%s\"",
			  w, h, imf->name);
	    img->colrdata = datalist;
	    break;
	  case K_MASK_:
	    if (img->maskdata != lispnil && !equal(datalist, img->maskdata))
	      run_warning("Changing mask data in %dx%d image of \"%s\"",
			  w, h, imf->name);
	    img->maskdata = datalist;
	    break;
	  case K_FILE_:
	    if (img->filedata != lispnil && !equal(datalist, img->filedata))
	      run_warning("Changing file data in %dx%d image of \"%s\"",
			  w, h, imf->name);
	    img->filedata = datalist;
	    break;
	  default:
	    break;
	}
    }
    /* Allocate space for any subimages that might be needed. */
    /* First set some standard numbers of subimages. */
    if (img->isborder) {
	numsubs = 16;
    } else if (img->isconnection) {
	numsubs = 64;
    } else if (img->istransition) {
	numsubs = 4 * 4;
    /* Limit the number of terrain subimages if we lack memory. */
    } else if (poor_memory) { /* FIXME - what about hexgrid? */
    	numsubs = min(numsubs, 3);
    }
    /* Deal with possible weird situations. */
    if (img->numsubimages > 0 && numsubs != img->numsubimages) {
	run_warning("Going from %d to %d subimages in %dx%d image of \"%s\"",
		    img->numsubimages, numsubs, w, h, imf->name);
	img->subimages = NULL;
    }
    img->numsubimages = numsubs;
    if (img->subimages == NULL) {
	img->subimages = (Image **) xmalloc(numsubs * sizeof(Image *));
	for (subi = 0; subi < numsubs; ++subi) {
	    subimg = get_subimg(imf, img->w, img->h);
	    img->subimages[subi] = subimg;
	}
    }
}

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END
