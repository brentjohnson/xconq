/* Interpretation of generic GDL images for Xconq.
   Copyright (C) 1994-2001 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/* Note!  This file does not use the standard "conq.h" header, so
   can't assume all the usual definitions. */
 
#include "config.h"
#include "misc.h"
#include "lisp.h"
#include "imf.h"
#include "module.h"
#include "system.h"

/* RGB above this value should be considered white. */

#define WHITE_THRESHOLD (65535 - 256)

/* RGB below this value should be considered black. */

#define BLACK_THRESHOLD (0 + 255)

enum {
    K_MONO_,
    K_MASK_,
    K_COLR_,
    K_FILE_,
    K_OTHER_
};

typedef struct _MFEntry {
    int value, count;
} MFEntry;
typedef MFEntry *ModeFilter;

static Image *get_subimg(ImageFamily *imf, int w, int h);
static Image *largest_image(ImageFamily *imf);
static int image_pixel_at(Image *img, int imtype, int x, int y);
static void set_image_pixel_at(Image *img, int imtype, int x, int y, int val);
static ImageFamily *new_imf(char *name);
static int bitmaps_match(int w, int h, Obj *lispdata, char *rawdata);
static int color_matches_mono(Image *img);
static void write_pixmap(FILE *fp, int w, int h, int aw, int ah,
			 int pixelsize, int orig_pixelsize,
			 Obj *palette, int *rawpalette, int numcolors,
			 Obj *lispdata, char *rawdata);
static void write_bitmap(FILE *fp, const char *subtyp, int w, int h,
			 Obj *data, char *rawdata);
static void write_palette_contents(FILE *fp, Obj *palette,
				   int *rawpalette, int numcolors);
static void write_color(FILE *fp, int n, int r, int g, int b);
static ModeFilter new_mf(int maxvals);
static void add_to_mf(ModeFilter mf, int val);
static int mode_of_mf(ModeFilter mf);
static void zero_mf(ModeFilter mf);
static void delete_mf(ModeFilter mf);
static void add_hex_mask(Image *img);
static void remove_hex_mask(Image *img);
static void scale_image_layer(Image *imgin, Image *imgout, int layer,
    int use_mask, ModeFilter mf);
static void scale_image(ImageFamily *imf, Image *img, Image *img2, int dhm);
static Image *add_scaled_image(ImageFamily *imf, Image *img, int w, int h);
static int size_match_score(int wa, int ha, int wb, int hb);
static int calculate_hch(int h);

/* This is the array and count of known image families. */

ImageFamily **images;

int numimages = 0;

/* Head of the linked list of image files. */

ImageFile *image_files;

/* Head of the linked list of "file" images used by several image families. */

FileImage *file_images;

ImageFamily *(*imf_load_hook)(ImageFamily *imf);

ImageFamily *(*imf_interp_hook)(ImageFamily *imf, Image *img, int force);

short write_synthetic_also;

/* There are two ways to use image families.  The "old" way is to use
   the image families' masks and other bitmaps as clip masks.  tcl/tk
   for Windows doesn't support clip masks however, so the "new" way is
   to do a GXand with the mask, carving out a mask-shaped black hole,
   then do a GXor with the image proper.  Note that the part of the
   image that is not included in the mask must be converted to black
   if the ORing is to have the right effect.  X doesn't guarantee that
   ANDing and ORing will work in general though; we just know that the
   X emulation in tcl/tk for Windows can be used in this way. */

/* This should be TRUE everywhere except on Windows. */

int use_clip_mask = TRUE;

/* Flag that indicates limited GDI memory in Windows ME and below. */

short poor_memory = FALSE;

/* Create and return an image family. */

static ImageFamily *
new_imf(char *name)
{
    ImageFamily *imf;

    imf = (ImageFamily *) xmalloc(sizeof(ImageFamily));
    imf->name = name;
    imf->notes = lispnil;
    return imf;
}

ImageFamily *
clone_imf(ImageFamily *imf)
{
    Image *img, *img2, *truenext;
    ImageFamily *imf2;

    imf2 = new_imf(imf->name);
    memcpy(imf2, imf, sizeof(ImageFamily));
    /* Clear the hook, we expect that the caller of this routine will
       supply any new hook that might be necessary. */
    imf2->hook = NULL;
    imf2->images = NULL;
    imf2->numsizes = 0;
    /* Clone the images. */
    for_all_images(imf, img) {
	img2 = get_img(imf2, img->w, img->h);
	truenext = img2->next;
	memcpy(img2, img, sizeof(Image));
	/* Clear the hook, we expect that the caller of this routine
	   will supply any new hook that might be necessary. */
	img2->hook = NULL;
	/* Restore the link. */
	img2->next = truenext;
	/* Note that pointers to raw image data and suchlike can be
	   left as-is, since they should be shared by image clones. */
	/* (should copy anyway, for safety?) */
    }
    return imf2;
}

/* Test that the given name is a valid image family name (all alphanumeric,
   hyphens anywhere but as first char). */

int
valid_imf_name(const char *name)
{
    const char *tmp;

    for (tmp = name; *tmp; ++tmp) {
	if (!(isalnum(*tmp)
	      || (tmp != name && *tmp == '-')))
	  return FALSE;
    }
    return TRUE;
}

/* Bash invalid chars in a prospective imf name. */

void
validify_imf_name(char *buf)
{
    char *tmp;

    for (tmp = buf; *tmp; ++tmp) {
	if (tmp == buf && *tmp == '-')
	  *tmp = 'Z';
	if (!isalnum(*tmp) && *tmp != '-')
	  *tmp = '-';
    }
}

/* Given a name, find or create an image family with that name. */

ImageFamily *
get_imf(const char *name)
{
    ImageFamily *imf = NULL;
    
    if (name == NULL) {
	init_warning("can't get an unnamed image family");
	return NULL;
    }
    if (!valid_imf_name(name)) {
	init_warning("\"%s\" is not a valid image family name", name);
	return NULL;
    }
    if (images == NULL) {
	images =
	  (ImageFamily **) xmalloc(MAXIMAGEFAMILIES * sizeof(ImageFamily *));
    }
    imf = find_imf(name);
    if (imf == NULL) {
	if (numimages >= MAXIMAGEFAMILIES) {
	    init_warning("MAXIMAGEFAMILIES exceeded, skipping image family %s", name);
	    return NULL;
	}
	imf = new_imf(copy_string(name));
	if (imf != NULL) {
	    images[numimages++] = imf;
	}
    }
    return imf;
}

ImageFile *
get_image_file(char *name)
{
    ImageFile *imfile;
    
    if (name == NULL)
      run_error("can't get an unnamed image file");
    for (imfile = image_files; imfile != NULL; imfile = imfile->next) {
	if (strcmp(name, imfile->name) == 0)
	  return imfile;
    }
    imfile = (ImageFile *) xmalloc(sizeof(ImageFile));
    imfile->name = copy_string(name);
    imfile->next = image_files;
    image_files = imfile;
    return imfile;
}

void
load_image_families(FILE *fp, int loadnow,
		    void (*callback)(ImageFamily *imf, int loadnow))
{
    int done = FALSE, first = TRUE;
    char buf[BUFSIZE], *buf1, *buf2, *tmp;
    ImageFamily *imf = NULL;
    ImageFile *imfile;
    
    while (!done) {
	/* Get a line from the file and parse it. */
	if (fgets(buf, BUFSIZE-1, fp)) {
	  buf1 = buf;
	  buf2 = strchr(buf, ' ');
	  if (buf2 == NULL)
	    break;
	  *buf2 = '\0';
	  ++buf2;
	  tmp = strchr(buf2, '\n');
	  if (tmp)
	    *tmp = '\0';
	} else
	  break;
	if (strcmp(buf1, ".") == 0
	    && strcmp(buf2, ".") == 0)
	  done = TRUE;
	else if (first) {
	    if (strcmp(buf1, "ImageFamilyName") == 0
		&& strcmp(buf2, "FileName") == 0)
	      first = FALSE;
	    else {
		init_warning("File not a valid imf dir, will close and ignore");
		/* We've already given a warning message, so pretend we're done
		   so the format error message doesn't get displayed below. */
		done = TRUE;
		break;
	    }
	} else {
	    imf = get_imf(buf1);
	    if (imf != NULL) {
		imfile = get_image_file(buf2);
		imf->location = imfile;
		if (loadnow && !imfile->loaded) {
		    load_imf_file(imfile->name, callback);
		    imfile->loaded = TRUE;
		} else {
		    if (callback != NULL)
		      (*callback)(imf, loadnow);
		}
	    }
	}
    }
    if (!done) {
	init_warning("Format error in imf dir near %s, will only use part",
		     (imf ? imf->name : "???"));
    }
}

/* Given a filename, open it and read/interpret all the image-related
   forms therein. */

int
load_imf_file(char *filename, void (*callback)(ImageFamily *imf, int loadnow))
{
    int startlineno = 1, endlineno = 1;
    Obj *form;
    FILE *fp;

    fp = open_file(filename, "r");
    if (fp != NULL) {
	/* Read everything in the file. */
	while ((form = read_form(fp, &startlineno, &endlineno)) != lispeof) {
	    interp_imf_form(form, filename, callback);
	}
	fclose(fp);
	return TRUE;
    }
    return FALSE;
}

/* Interpret a form, looking specifically for image-related forms. */

void
interp_imf_form(Obj *form, char *filename, void (*imf_callback)(ImageFamily *imf, int loadnow))
{
    Obj *head;
    ImageFamily *imf;
    ImageFile *imfile;

    /* Ignore any non-lists, we might be reading from a normal game design. */
    if (!consp(form))
      return;
    head = car(form);
    if (match_keyword(head, K_IMF)) {
	imf = interp_imf(form);
	if (imf != NULL) {
		imfile = get_image_file(filename);
		imf->location = imfile;
		imf->location->loaded = TRUE;
		if (imf_callback != NULL) {
			(*imf_callback)(imf, TRUE);
		}
	}
    } else {
	/* Ignore any non-image forms, we might be reading from a 
	   normal game design. */
    }
}

/* Find the image family of the given name, if it exists. */

ImageFamily *
find_imf(const char *name)
{
    int i;

    for (i = 0; i < numimages; ++i) {
	if (strcmp(name, images[i]->name) == 0)
	  return images[i];
    }
    return NULL;
}

/* Get an image of the given size from the family, creating a new one
   if necessary. */

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

/* Get an image of the given size from the family, creating a new one
   if necessary. */

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
find_img(ImageFamily *imf, int w, int h)
{
    Image *img;
	
    for_all_images(imf, img) {
	if (w == img->w && h == img->h)
	  return img;
    }
    return NULL;
}

ImageFamily *
interp_imf(Obj *form)
{
    ImageFamily *imf;

    if (stringp(cadr(form))) {
	imf = get_imf(c_string(cadr(form)));
	if (imf != NULL) {
	    interp_imf_contents(imf, cddr(form));
	}
	return imf;
    } else {
	run_warning("image family name must be a string");
    }
    return NULL;
}

/* Interpret the image family definition as a list of images and/or a
   notes property. */

void
interp_imf_contents(ImageFamily *imf, Obj *clauses)
{
    Obj *rest, *clause;

    for_all_list(clauses, rest) {
	clause = car(rest);
	if (consp(clause)) {
	    if (symbolp(car(clause))) {
		if (match_keyword(car(clause), K_NOTES)) {
		    imf->notes = cadr(clause);
		    syntax_error(clause, "extra junk after property value");
		} else {
		    syntax_error(clause, "unknown image family property");
		}
	    } else if (consp(car(clause))) {
		interp_image(imf, car(clause), cdr(clause));
	    } else {
		syntax_error(clause, "not image or image family property");
	    }
	} else {
	    syntax_error(clause, "bogus clause");
	}
    }
    compute_image_bboxes(imf);
}

/* Given an image family, a size, and a list describing the elements
   of a single image, parse the size and elements, put those into the
   right slots of an image object.  Also detect and warn about changes
   to an existing image, since this usually indicates some kind of
   problem. */

void
interp_image(ImageFamily *imf, Obj *size, Obj *parts)
{
    int w, h, imtype, emx, emy, emw, emh, numsubs, subi;
    const char *name;
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

void
compute_image_bboxes(ImageFamily *imf)
{
    Image *img;

    if (imf == NULL)
      return;
    for_all_images(imf, img) {
	compute_image_bbox(img);
    }
}

void
compute_image_bbox(Image *img)
{
    int numbytes, i, j = 0, byte, x, y, x1, x2, k;
    int xmin, ymin, xmax, ymax;
    const char *data = NULL;
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

/* Get a single pixel from the given type of data of an image. */

int
image_pixel_at(Image *img, int imtype, int x, int y)
{
    int rowbytes, psize, i, byte, rslt;
    char *rawdata = NULL;

    if (imtype == K_MONO_) {
	rawdata = img->rawmonodata;
	psize = 1;
    } else if (imtype == K_MASK_) {
	rawdata = img->rawmaskdata;
	psize = 1;
    } else if (imtype == K_COLR_) {
	rawdata = img->rawcolrdata;
	psize = img->pixelsize;
    }
    if (rawdata == NULL)
      return 0;
    rowbytes = computed_rowbytes(img->w, psize);
    i = y * rowbytes + ((x * psize) >> 3);
    byte = rawdata[i];
    rslt = (byte >> ((8 - psize) - ((x * psize) & 0x7))) & ((1 << psize) - 1);
    return rslt;
}

/* Set a single pixel in the given type of data of an image. */

void
set_image_pixel_at(Image *img, int imtype, int x, int y, int val)
{
    int rowbytes, psize, i, byte;
    char *rawdata = NULL;

    if (imtype == K_MONO_) {
	rawdata = img->rawmonodata;
	psize = 1;
    } else if (imtype == K_MASK_) {
	rawdata = img->rawmaskdata;
	psize = 1;
    } else if (imtype == K_COLR_) {
	rawdata = img->rawcolrdata;
	psize = img->pixelsize;
    }
    if (rawdata != NULL) {
	rowbytes = computed_rowbytes(img->w, psize);
	i = y * rowbytes + ((x * psize) >> 3);
	byte = rawdata[i];
        byte &= ~ (((1 << psize) - 1) << ((8 - psize) - ((x * psize) & 0x7)));
	byte |= val << ((8 - psize) - ((x * psize) & 0x7));
	rawdata[i] = byte;
    }
}

/* Create a new data structure for mode filtering; maxvals is how many
   distinct values we might call add_to_mf on.  The way this works is that
   you create the structure with new_mf, then you call add_to_mf a bunch of
   times with different values.  Then you can call mode_of_mf and it'll
   tell which value you used the most times in calls to add_to_mf.  Calling
   zero_mf resets the filter in a way that is cheaper than deleting and
   re-creating it, and delete_mf is called when you're finally finished. */

static ModeFilter
new_mf(int maxval)
{
    ModeFilter rval = (ModeFilter)xmalloc(sizeof(MFEntry)*(maxval+1));
   
    return rval;
}

/* Add a new value to the mode filter. */

static void
add_to_mf(ModeFilter mf, int val)
{
    int i;
    MFEntry mfe;
   
    if (mf == NULL) return;
    for (i = 0; (mf[i].value != val) && (mf[i].count != 0); i++);
    mf[i].value = val;
    mf[i].count++;
    if (mf[i].count > mf[0].count) {
        mfe = mf[i];
        mf[i] = mf[0];
        mf[0] = mfe;
    }
}

/* Find the most common value in the mode filter. */

static int
mode_of_mf(ModeFilter mf)
{
    if (mf != NULL)
        return mf[0].value;
    else
        return 0;
}

/* Empty the mode filter of all counts. */

static void
zero_mf(ModeFilter mf)
{
    int i;
   
    if (mf == NULL) return;
    for (i = 0; mf[i].count != 0; i++)
        mf[i].count = 0;
    mf[0].value = 0;
}

/* Get rid of the mode filter. */

static void
delete_mf(ModeFilter mf)
{
    if (mf != NULL) free(mf);
}

/* Calculate hch by a formula that matches Stan's magic tables, but also
   gives reasonable results at intermediate sizes.  Needed because the
   standard hex shapes have been tweaked away from geometric perfection,
   for graphics expediency. */

static int
calculate_hch(int h)
{
    return (h*3)/4 + h/72 + h/144 + 1;
}

/* Mask the image down to a hexagon that fills the image rectangle - points
   at the centres of the top and bottom, flat sides along left and right. */

static void
add_hex_mask(Image *img)
{
    int tw, th, x, y, numbytes;
   
    if (img == NULL)
        return;
    if (img->w > img->h)
        return;
  
    /* Ensure that binary version of mask exists.  Create one even if none
       was specified. */
    if (img->rawmaskdata == NULL) {
	img->rawmaskdata = (char *)xmalloc(numbytes);
        if (img->maskdata != lispnil)
	    interp_bytes(img->maskdata, numbytes, img->rawmaskdata, 0);
        else
	    for (y = 0; y < img->h; y++)
	        for (x = 0; x < img->w; x++)
	            set_image_pixel_at(img, K_MASK_, x, y, 1);
    }

    /* Calculate size of triangles to mask out */
    tw = img->w / 2 - 1;
    th = img->h - calculate_hch(img->h) - 1;
   
    /* Mask out the bits */
    for (x = 1; x <= tw; x++)
        for (y = 0; y <= (x*th) / tw; y++) {
	    set_image_pixel_at(img, K_MASK_, tw-x, y, 0);
	    set_image_pixel_at(img, K_MASK_, tw+1+x, y, 0);
	    set_image_pixel_at(img, K_MASK_, tw-x, img->h-1-y, 0);
	    set_image_pixel_at(img, K_MASK_, tw+1+x, img->h-1-y, 0);
	}
}

/* Remove the mask from the four triangular areas outside the inscribed
   hexagon (see add_hex_mask above).  Fill in those triangles with data
   copied from the diagonal edges just inside the hex area.  This is
   useful because it means we can do remove_hex_mask, scale up, and then
   add_hex_mask, and the result will have smooth edges instead of
   magnified jaggies from the lower resolution.  Warning: caller must
   load raw mono and color data before calling this, this function won't
   do that. */
 
static void
remove_hex_mask(Image *img)
{
    int tw, th, x, y, ylim, c1, c2, c3, c4, numbytes;
   
    if (img == NULL)
        return;
    if (img->w > img->h)
        return;
  
    /* Ensure that binary version of mask exists.  Abort if no mask.  */
    if (img->rawmaskdata == NULL) {
	img->rawmaskdata = (char *)xmalloc(numbytes);
        if (img->maskdata != lispnil)
	    interp_bytes(img->maskdata, numbytes, img->rawmaskdata, 0);
        else
	    return;
    }

    /* Calculate size of triangles to unmask */
    tw = img->w / 2 - 1;
    th = img->h - calculate_hch(img->h) - 1;
   
    /* Unmask the pixels */
    for (x = 1; x <= tw; x++) {
        ylim = (x*th) / tw;
        c1 = image_pixel_at(img, K_MASK_, tw-x, ylim+1);
	c2 = image_pixel_at(img, K_MASK_, tw+1+x, ylim+1);
	c3 = image_pixel_at(img, K_MASK_, tw-x, img->h-2-ylim);
	c4 = image_pixel_at(img, K_MASK_, tw+1+x, img->h-2-ylim);
        for (y = 0; y <= ylim; y++) {
	    set_image_pixel_at(img, K_MASK_, tw-x, y, c1);
	    set_image_pixel_at(img, K_MASK_, tw+1+x, y, c2);
	    set_image_pixel_at(img, K_MASK_, tw-x, img->h-1-y, c3);
	    set_image_pixel_at(img, K_MASK_, tw+1+x, img->h-1-y, c4);
	}
        if (img->rawmonodata != NULL) {
	    c1 = image_pixel_at(img, K_MONO_, tw-x, ylim+1);
	    c2 = image_pixel_at(img, K_MONO_, tw+1+x, ylim+1);
	    c3 = image_pixel_at(img, K_MONO_, tw-x, img->h-2-ylim);
	    c4 = image_pixel_at(img, K_MONO_, tw+1+x, img->h-2-ylim);
            for (y = 0; y <= ylim; y++) {
	        set_image_pixel_at(img, K_MONO_, tw-x, y, c1);
	        set_image_pixel_at(img, K_MONO_, tw+1+x, y, c2);
	        set_image_pixel_at(img, K_MONO_, tw-x, img->h-1-y, c3);
	        set_image_pixel_at(img, K_MONO_, tw+1+x, img->h-1-y, c4);
	    }
	}
        if (img->rawcolrdata != NULL) {
	    c1 = image_pixel_at(img, K_COLR_, tw-x, ylim+1);
	    c2 = image_pixel_at(img, K_COLR_, tw+1+x, ylim+1);
	    c3 = image_pixel_at(img, K_COLR_, tw-x, img->h-2-ylim);
	    c4 = image_pixel_at(img, K_COLR_, tw+1+x, img->h-2-ylim);
            for (y = 0; y <= ylim; y++) {
	        set_image_pixel_at(img, K_COLR_, tw-x, y, c1);
	        set_image_pixel_at(img, K_COLR_, tw+1+x, y, c2);
	        set_image_pixel_at(img, K_COLR_, tw-x, img->h-1-y, c3);
	        set_image_pixel_at(img, K_COLR_, tw+1+x, img->h-1-y, c4);
	    }
	}
    }
}

/* Scale one layer of an image. */

static void
scale_image_layer(Image *imgin, Image *imgout, int layer, int use_mask,
		  ModeFilter mf)
{
    int u, v, x, y, xa, xb, ya, yb, tmp;

    /* Perform a mode-filtered scaling operation. */
   
    /* The way this works is that u and v are coordinates in the output image
       rval.  For each pixel (u,v) we compute the rectangle (xa..xb,ya..yb)
       in the input image img, which rectangle covers all the pixels that are
       touched by the pixel (u,v) when it's projected onto img, noting that
       a pixel is defined to be closed on the sides with lesser coordinate
       values and open on the sides with greater coordinate values.  We
       perform a mode filter over (xa..xb,ya..yb), that is, we find the
       pixel value that occurs most commonly in that rectangle, splitting
       ties by preferring the value that occurs first in reading order; the
       result is the pixel value for the pixel (u,v).  We do not count input
       pixels that are masked out.  If the scaling is by less than a factor
       of 2 up or down, the result will be basically the same as
       nearest-neighbour resampling. */
   
    /* Loop for each pixel (u,v) */
    for (v = 0; v < imgout->h; v++) {
      
        /* Might as well calculate ya and yb here because v determines them */
        ya = (v*imgin->h)/imgout->h;
        yb = ((v+1)*imgin->h-1)/imgout->h;
      
        for (u = 0; u < imgout->w; u++) {

	    /* Calculate xa and xb */
	    xa = (u*imgin->w)/imgout->w;
	    xb = ((u+1)*imgin->w-1)/imgout->w;
	  
	    /* If the rectangle is small, and we don't have a mask, then
	       skip the mode filter business because the first pixel wins. */
	    if ((xb-xa+1)*(yb-ya+1) <= 2 && !use_mask) {
	        set_image_pixel_at(imgout, layer, u, v,
		    image_pixel_at(imgin, layer, xa, ya));

	    /* Otherwise we have to do the filter thing. */
	    } else {
	      
	        /* Look through the input rectangle and compute modes */
	        for (y = yb; y >= ya; y--)
		    for (x = xb; x >= xa; x--)
		        if (!use_mask
			    || image_pixel_at(imgin, K_MASK_, x, y) != 0)
		        add_to_mf(mf, image_pixel_at(imgin, layer, x, y));
	      
	        /* Set output pixel */
		set_image_pixel_at(imgout, layer, u, v, mode_of_mf(mf));
		zero_mf(mf);
	    }
        }
    }
}

/* Scale an image (or subimage). */

static void
scale_image(ImageFamily *imf, Image *img, Image *img2, int dhm)
{
    int numbytes, numbytes2;
    ModeFilter mf;

    /* Scale the embedding coordinates. */
    img2->embedx = (img->embedx*img2->w)/img->w;
    img2->embedy = (img->embedy*img2->h)/img->h;
    img2->embedw = (img->embedw*img2->w)/img->w;
    img2->embedh = (img->embedh*img2->h)/img->h;

    /* Actually get the input image if we haven't yet. */
    if (img->rawcolrdata == NULL) {
	/* Try different ways to get some image data. */
	if (img->colrdata != lispnil) {
	    numbytes = img->h * computed_rowbytes(img->w, img->pixelsize);
	    img->rawcolrdata = (char *)xmalloc(numbytes);
	    interp_bytes(img->colrdata, numbytes, img->rawcolrdata, 0);
	} else if (img->filedata != lispnil) {
	    make_image_from_file_image(imf, img, img, 0);
	}
    }

    /* Copy over a bunch of other fields. */
    img2->pixelsize = img->pixelsize;
    img2->palette = img->palette;
    img2->rawpalette = img->rawpalette;
    img2->numcolors = img->numcolors;
    img2->istile = img->istile;
    img2->isterrain = img->isterrain;
    img2->isconnection = img->isconnection;
    img2->isborder = img->isborder;
    img2->istransition = img->istransition;
    img2->hexgridx = img->hexgridx;
    img2->hexgridy = img->hexgridy;

    /* Mark the image as having been computed rather than read in. */
    img2->synthetic = TRUE;

    if (img->rawcolrdata != NULL) {
	numbytes2 = img2->h * computed_rowbytes(img2->w, img2->pixelsize);
	img2->rawcolrdata = (char *)xmalloc(numbytes2);
    }
    numbytes = img->h * computed_rowbytes(img->w, 1);
    numbytes2 = img2->h * computed_rowbytes(img2->w, 1);

    /* Ensure that binary version of mono image exists. */
    make_raw_mono_data(img, FALSE);
    if (img->rawmonodata != NULL)
      img2->rawmonodata = (char *)xmalloc(numbytes2);

    /* Ensure that binary version of mask exists. */
    if (img->rawmaskdata == NULL && img->maskdata != lispnil) {
	img->rawmaskdata = (char *)xmalloc(numbytes);
	interp_bytes(img->maskdata, numbytes, img->rawmaskdata, 0);
    }
    if (img->rawmaskdata != NULL)
      img2->rawmaskdata = (char *)xmalloc(numbytes2);
   
    /* Tweak the input image's mask if appropriate */
    if (dhm)
        remove_hex_mask(img);
   
    /* Do the scaling operation */
    mf = new_mf(256);
    if (img2->rawmaskdata != NULL) {
        if (img2->rawcolrdata != NULL)
	    scale_image_layer(img, img2, K_COLR_, TRUE, mf);
        if (img2->rawmonodata != NULL)
	    scale_image_layer(img, img2, K_MONO_, TRUE, mf);
        scale_image_layer(img, img2, K_MASK_, FALSE, mf);
    } else {
        if (img2->rawcolrdata != NULL)
	    scale_image_layer(img, img2, K_COLR_, FALSE, mf);
        if (img2->rawmonodata != NULL)
	    scale_image_layer(img, img2, K_MONO_, FALSE, mf);
    }
    delete_mf(mf);
   
    /* Put the hex masks back */
    if (dhm) {
        add_hex_mask(img);
        add_hex_mask(img2);
    }

    /* Set the image bounding box */
    compute_image_bbox(img2);
   
    /* Recognize flat colors */
    if (img2->w == 1 && img2->h == 1 && img2->rawcolrdata != NULL) {
	if (img2->rawpalette == NULL)
	    make_raw_palette(img2);
        img2->istile = TRUE;
        img2->r = img2->rawpalette[4 * img2->rawcolrdata[0] + 1];
        img2->g = img2->rawpalette[4 * img2->rawcolrdata[0] + 2];
        img2->b = img2->rawpalette[4 * img2->rawcolrdata[0] + 3];
    }
   
    /* Call the interface hook */
    if (imf_interp_hook)
        (*imf_interp_hook)(imf, img2, FALSE);
}

/* Scale image img in family imf to size w by h and add it to the family. */

static Image *
add_scaled_image(ImageFamily *imf, Image *img, int w, int h)
{
    Image *rval;
    int i, dhm;
   
    /* We must have a family and an input image, and the size must be sane. */
    if (imf == NULL || img == NULL || w <= 0 || h <= 0)
        return NULL;
   
    /* Create a data structure for the output. */
    rval = get_img(imf, w, h);
   
    /* Figure out whether to Do Hex Masking */
    dhm = (img->isterrain || img->isconnection
	|| img->istransition) && (img->h >= img->w) && (h >= w);
   
    /* Scale the main image */
    scale_image(imf, img, rval, dhm);
   
    /* Handle subimages */
    if (img->numsubimages > 0 && img->subimages != NULL) {
        rval->numsubimages = img->numsubimages;
        rval->subimages
	    = (Image **) xmalloc(img->numsubimages * sizeof(Image *));
        for (i = 0; i < img->numsubimages; i++) {
	    rval->subimages[i] = get_subimg(imf, rval->w, rval->h);
	    scale_image(imf, img->subimages[i], rval->subimages[i], dhm);
	}
    }

    /* Return result */
    return rval;
}

void
make_raw_mono_data(Image *img, int force)
{
    int numbytes = img->h * computed_rowbytes(img->w, 1);

    if ((img->rawmonodata == NULL || force) && img->monodata != lispnil) {
	img->rawmonodata = (char *)xmalloc(numbytes);
	interp_bytes(img->monodata, numbytes, img->rawmonodata, 0);
    }
}

/* Given a list of strings, interpret the hex digits and put the
   results at the given address. */

void
interp_bytes(Obj *datalist, int numbytes, char *destaddr, int jump)
{
    int i, j = 0;
    const char *data = NULL;

    for (i = 0; i < numbytes; ++i) {
	if (data == NULL || data[j] == '\0') {
	    if (datalist == lispnil) {
		return;
	    } else if (stringp(car(datalist))) {
		data = c_string(car(datalist));
		j = 0;
	    } else {
		syntax_error(datalist, "Non-string in image data list");
		/* Have to give up now. */
		return;
	    }
	    datalist = cdr(datalist);
	}
	/* Just skip over slashes, which are for readability only. */
	if (data[j] == '/')
	  ++j;
	destaddr[i] = hextoi(data[j]) * 16 + hextoi(data[j+1]);
	if (jump == 1 || (jump > 0 && i % jump == 0)) {
	    i += jump;
	    /* Be neat, put a zero in the location we're jumping over. */
	    /* (doesn't work for jump > 1, but that never happens anymore?) */
	    destaddr[i] = 0;
	}
	j += 2;
    }
}

/* Compute a score describing how much scaling or tiling a given image will
   need to match a given size. */

static int
size_match_score(int wa, int ha, int wb, int hb)
{
    if (!wb) wb = 1;  if (!wa) wa = 1;  if (!hb) hb = 1;  if (!ha) ha = 1;
    return (wa*100)/wb + (wb*100)/wa + (ha*100)/hb + (hb*100)/ha - 400;
}

/* Try to find an image within the specified range and as close to the
   specified size as possible in the family.  If no designer-specified
   image is available in the range, generate one of size w by h by
   scaling. */

Image *
best_image_in_range(ImageFamily *imf, int w, int h, int wmin, int hmin,
		    int wmax, int hmax)
{
    Image *img, *best_nonsynth = NULL, *best_in_range = NULL,
        *best_tile = NULL, *exact_match = NULL;
    int best_nonsynth_score = INT_MAX, best_tile_score = INT_MAX, 
	best_in_range_score = INT_MAX, s;

    if (imf == NULL || imf->images == NULL)
        return NULL;

    for_all_images(imf, img) {

        /* Skip all basic terrain images except power 4 and 5
	   if we are low on memory. */
    	if (poor_memory
    	    && img->isterrain
    	    && img->w != 24
    	    && img->w != 44) {
    		continue;
    	}
       
        /* Don't cross the isometric/non-isometric boundary */
        if (img->isterrain || img->isborder || img->isconnection ||
	     img->istransition) {
	    if (img->w > img->h && w <= h)
	        continue;
	    if (img->w <= img->h && w > h)
	        continue;
	}
       
        /* Check if there's an exact match - this is no longer an immediate
	   success because it might be synthetic */
        if (img->w == w && img->h == h && !img->istile)
	    exact_match = img;

        /* Find best image that isn't synthetic or tile (scaling candidate) */
        if (!img->istile && !img->synthetic) {
	    s = size_match_score(w, h, img->w, img->h);
	    if (best_nonsynth == NULL || s < best_nonsynth_score) {
	        best_nonsynth = img;
	        best_nonsynth_score = s;
	    }
	   
	    /* Find best in range */
	    if ((best_in_range == NULL || s < best_in_range_score)
		&& img->w >= wmin && img->h >= hmin
		&& img->w <= wmax && img->h <= hmax) {
	        best_in_range = img;
	        best_in_range_score = s;
	    }
	}

        /* Find best tile */
        if (img->istile) {
	    s = size_match_score(w, h, img->w, img->h);
	    if (best_tile == NULL || s < best_tile_score) {
	        best_tile = img;
	        best_tile_score = s;
	    }
	}
    } /* for_all_images */
   
    /* If we have a "best in range" then use it */
    if (best_in_range != NULL)
        return best_in_range;
   
    /* If we have an exact size match, use that */
    if (exact_match != NULL)
        return exact_match;

    /* If we have a tile, and we're low on memory, we can't scale, or
       the best scaling candidate isn't as good as the tile, use tile. */
    if (best_tile != NULL && (poor_memory || best_nonsynth == NULL ||
			      best_tile_score < best_nonsynth_score))
        return best_tile;
   
    /* Now we know we want to scale, so there had better exist a candidate.
       If not, we still might try looking for a flat colour. */
    if (best_nonsynth == NULL) {
        if ((w != 1) || (h != 1)) {
	    return best_image_in_range(imf, 1, 1, 1, 1, 1, 1);
        } else {
	    return NULL;
	}
    }

    /* Scale the candidate. */
    return add_scaled_image(imf, best_nonsynth, w, h);
}

Image *
smallest_image(ImageFamily *imf)
{
    Image *img, *smallest = NULL;

    if (imf == NULL)
      return NULL;
    for_all_images(imf, img) {
	if (smallest == NULL || (img->w < smallest->w && img->h < smallest->h))
	  smallest = img;
    }
    return smallest;
}

Image *
largest_image(ImageFamily *imf)
{
    Image *img, *largest = NULL;

    if (imf == NULL)
      return NULL;
    for_all_images(imf, img) {
	if (largest == NULL || (img->w > largest->w && img->h > largest->h))
	  largest = img;
    }
    return largest;
}

/* Compute the right location for the given emblem and unit images. */

static int tmpbw;  /* work around Think C bug */

int
emblem_position(Image *uimg, const char *ename, ImageFamily *eimf, int sw, int sh,
		int vpuh, int vphh, int *exxp, int *eyyp, int *ewp, int *ehp)
{
    int ew1, eh1, ex, ey, ew, eh, bx, by, bw, bh, overlap;
    Image *eimg;

    /* Check if correct emblem is part of the unit's image, and don't draw 
       if it is. */
    if (uimg
	&& uimg->embedname
	&& ename != NULL
	&& strcmp(uimg->embedname, ename) == 0) {
	return FALSE;
    }
    /* (should use emblem bbox to help calc) */
    /* Get the size of the emblem, either from the image or by computing
       a reasonable default. */
    if (uimg && uimg->embedw > 0 && uimg->embedh > 0) {
	ew = uimg->embedw;  eh = uimg->embedh;
    } else {
	ew1 = min(sw, max(8, sw / 4));  eh1 = min(sh, max(8, sh / 4));
	eimg = NULL;
	/* Look up the best emblem for the current zoom. */
	if (eimf != NULL) {
	    eimg = best_image(eimf, ew1, eh1);
	}
	if (eimg) {
	    ew = eimg->w;  eh = eimg->h;
	    /* Make a default 8x6 size for a solid color emblem. */
	    if (ew == 1 && eh == 1) {
		ew = 8;  eh = 6;
	    }
	} else {
	    ew = ew1;  eh = eh1;
	}
    }
    /* Position the emblem, either explicitly, or default to UR corner
       (note that we need the emblem's width to do this) */
    if (uimg && uimg->embedx >= 0 && uimg->embedy >= 0) {
	ex = uimg->embedx;  ey = uimg->embedy;
	/* Don't let the emblem stick out of the unit's area. */
	if (ex + ew > sw)
	  ex = sw - ew;
	if (ey + eh > sh)
	  ey = sh - eh;
    } else if (uimg && (uimg->bboxw != uimg->w || uimg->bboxh != uimg->h)) {
	overlap = FALSE;
	/* Scale bounding box by space given to image. */
	bx = (uimg->bboxx * sw) / uimg->w;  by = (uimg->bboxy * sh) / uimg->h;
	tmpbw = (uimg->bboxw * sw) / uimg->w;
	bh = (uimg->bboxh * sh) / uimg->h;
	bw = tmpbw;
	/* Position the emblem outside the image's bbox if possible,
	   moving in if necessary to stay inside the image's allowed
	   area (sw x sh). */
	ex = bx + bw;
	if (ex + ew > sw) {
	    /* Emblem too wide to fit between unit bbox and edge of
	       area; butt it against edge of area, note the
	       overlap. */
	    ex = sw - ew;
	    overlap = TRUE;
	}
	if (ex < 0)
	  ex = 0;
	ey = by;
	if (overlap)
	  ey -= eh;
	if (ey + eh > sh)
	  ey = sh - eh;
	if (ey < 0)
	  ey = 0;
    } else {
	ex = sw - ew;  ey = 0;
    }
    /* Adjust for an oversized unit image. */
    if (sh > vpuh) {
	ey += (vphh - vpuh) / 2;
#if (0)
	/* Tweaked by hand. */
	if (vpuh > 16) {
	    ex -= 1;
	}
#endif
    }
    /* Adjust for a shrunken unit image. */
    if (sh < vpuh) {
	ex = sw - ew;
	/* Tweaked by hand. */
	if (vpuh > 8) {
	    ey = vpuh / 8;
	}
    }
    /* Return the results. */
    *exxp = ex;  *eyyp = ey;
    *ewp = ew;  *ehp = eh;
    return TRUE;
}

/* Transform the masked-out part of an image into a single chosen
   color. */

void
blacken_masked_area(ImageFamily *imf, Image *img, int rd, int g, int b)
{
    int r, ri, rc, c, rmask;
    int rmi, rmc, rmmask;
    char *rp, *rmp;
    int black = -1;
    int rowsz = -1;

    if (img->rawpalette == NULL)
      make_raw_palette(img);
    for (c = 0; c < img->numcolors; c++) {
	if (img->rawpalette[4 * c + 1] == rd
	    && img->rawpalette[4 * c + 2] == g
	    && img->rawpalette[4 * c + 3] == b) {
	    black = c;
	    break;
	}
    }
    if (black < 0
	&& (1 << img->pixelsize) == img->numcolors
	&& img->pixelsize < 8) {
	char *newdata, *nrp;
	int newpsize, nrmask, nri;

	newpsize = img->pixelsize * 2;
	/* Don't need to mess with palette because it already has an
	   additional spot allocated, and we only need the one. */
	rmask = (1 << img->pixelsize) - 1;
	ri = 8 - img->pixelsize;
	rp = img->rawcolrdata;
	newdata = (char *)xmalloc(img->h * computed_rowbytes(img->w, newpsize));
	nrmask = (1 << newpsize) - 1;
	nri = 8 - newpsize;
	nrp = newdata;
	for (r = 0; r < img->h; r++) {
	  for (c = 0; c < img->w; c++) {
	    rc = ((int) (*rp >> ri)) & rmask;
	    /* OR the color data into its new location. */
	    *nrp |= (char) (rc << nri);
	    if (ri) {
		ri -= img->pixelsize;
	    } else {
		ri = 8 - img->pixelsize;
		++rp;
	    }
	    if (nri) {
		nri -= newpsize;
	    } else {
		nri = 8 - newpsize;
		++nrp;
	    }
	  }
	  if ((img->pixelsize * img->w) % 8) {
	    ri = 8 - img->pixelsize;
	    ++rp;
	  }
	  if ((newpsize * img->w) % 8) {
	    nri = 8 - newpsize;
	    ++nrp;
	  }
	}
	img->orig_pixelsize = img->pixelsize;
	img->pixelsize = newpsize;
	img->rawcolrdata = newdata;
    }
    /* If a spare color is available, put black there. */
    if (black < 0 && (1 << img->pixelsize) > img->numcolors) {
	black = img->numcolors;
	img->rawpalette[4 * black + 0] = black;
	img->rawpalette[4 * black + 1] = rd;
	img->rawpalette[4 * black + 2] = g;
	img->rawpalette[4 * black + 3] = b;
	++(img->numcolors);
    }
    rmask = (1 << img->pixelsize) - 1;
    ri = 8 - img->pixelsize;
    rp = img->rawcolrdata;
    /* OK, now we're getting desperate; use the color in the upper left
       corner and pretend it's a transparent color. But, only if it 
       appears to be the vertex of two edges of the same color. */
    /*! \note This is a stupid hack. In cases where the UL corner cannot 
	      be relied upon, the designer should have the option of 
	      specifying a 'mask-color' keyword with a RGB triplet. */
    if (black < 0) {
	black = ((int) *rp >> ri) & rmask;
	rowsz = computed_rowbytes(img->w, img->pixelsize);
	/* Check pixels in UR and LL corners. */
	/* If the corners don't match, then grab something from the LR 
	   corner and hope for the best. (This is very hackish!) */
	if ((black != (((int) rp[rowsz - 1] >> ri) & rmask))
	    || (black != (((int) rp[rowsz * (img->h - 2)] >> ri) & rmask))) {
	    black = ((int) rp[(rowsz * img->h) - 1] >> ri) & rmask;
	}
	img->rawpalette[4 * black + 1] = rd;
	img->rawpalette[4 * black + 2] = g;
	img->rawpalette[4 * black + 3] = b;
    }
    rmmask = 1;
    rmi = 7;
    rmp = img->rawmaskdata;
    for (r = 0; r < img->h; r++) {
	for (c = 0; c < img->w; c++) {
	    rc = ((int) (*rp >> ri)) & rmask;
	    rmc = ((int) (*rmp >> rmi)) & rmmask;
	    if (rmc == 0) {
		/* Mask off the old value. */
		*rp &= (char) (~ (rmask << ri));
		/* Insert the color for black. */
		*rp |= (char) (black << ri);
	    }
	    if (ri) {
		ri -= img->pixelsize;
	    } else {
		ri = 8 - img->pixelsize;
		++rp;
	    }
	    if (rmi) {
		rmi -= 1;
	    } else {
		rmi = 7;
		++rmp;
	    }
	}
	if ((img->pixelsize * img->w) % 8) {
	    ri = 8 - img->pixelsize;
	    ++rp;
	}
	if (img->w % 8) {
	    rmi = 7;
	    ++rmp;
	}
    }
}

void
blacken_mono_masked_area(ImageFamily *imf, Image *img, int rd, int g, int b)
{
    int r, ri, rc, c, rmask;
    int rmi, rmc, rmmask;
    char *rp, *rmp;

    rmask = 1;
    ri = 7;
    rp = img->rawmonodata;
    rmmask = 1;
    rmi = 7;
    rmp = img->rawmaskdata;
    for (r = 0; r < img->h; r++) {
	for (c = 0; c < img->w; c++) {
	    rc = ((int) (*rp >> ri)) & rmask;
	    rmc = ((int) (*rmp >> rmi)) & rmmask;
	    if (rmc == 0) {
		/* Mask off the old value. */
		*rp &= (char) (~ (rmask << ri));
		/* Insert the color for black. */
		*rp |= (char) (1 << ri);
	    }
	    if (ri) {
		ri -= 1;
	    } else {
		ri = 7;
		++rp;
	    }
	    if (rmi) {
		rmi -= 1;
	    } else {
		rmi = 7;
		++rmp;
	    }
	}
	if (img->w % 8) {
	    ri = 7;
	    ++rp;
	}
	if (img->w % 8) {
	    rmi = 7;
	    ++rmp;
	}
    }
}

void
make_raw_palette(Image *img)
{
    int ipal[4][256];
    int c, ln;
    Obj *pal;

    /* Parse the Lispified palette. */
    /* (should allocate and store directly instead of using ipal) */
    c = 0;
    for_all_list(img->palette, pal) {
	parse_lisp_palette_entry(car(pal), &ipal[0][c],
				 &ipal[1][c], &ipal[2][c], &ipal[3][c]);
	c++;
    }
    img->numcolors = c;
    if (c == 0)
      return;
    /* store palette */
    img->rawpalette = (int *) xmalloc(257/*(img->numcolors + 1)*/ * 4 * sizeof(int));
    for (c = 0; c < img->numcolors; c++) {
	for (ln = 0; ln < 4; ln++) {
	    img->rawpalette[4 * c + ln] = ipal[ln][c];
	}
    }
}

/* The comparison function for the image list just does "strcmp" order
   and *requires* that all image families be named and named uniquely. */

static int
image_name_compare(CONST void *imf1, CONST void *imf2)
{
    return strcmp((*((ImageFamily **) imf1))->name,
		  (*((ImageFamily **) imf2))->name);
}

void
sort_all_images(void)
{
    qsort(&(images[0]), numimages, sizeof(ImageFamily *), image_name_compare);
}

/* Check Lisp-format and binary-format data for consistency. */

void
check_imf(ImageFamily *imf)
{
    Image *img;
    
    if (imf == NULL)
      return;
    if (imf->name == NULL) {
	return;
    }
    for_all_images(imf, img) {
	/* Check consistency of Lisp and binary data. */
	if (img->colrdata != lispnil && img->rawcolrdata) {
	    /* (should add color image comparison) */
	}
	if (img->monodata != lispnil && img->rawmonodata) {
	    if (!bitmaps_match(img->w, img->h, img->monodata, img->rawmonodata))
	      run_warning("mono bitmap data not consistent in  %dx%d image of \"%s\"",
			  img->w, img->h, imf->name);
	}
	if (img->maskdata != lispnil && img->rawmaskdata) {
	    if (!bitmaps_match(img->w, img->h, img->maskdata, img->rawmaskdata))
	      run_warning("mask bitmap data not consistent in  %dx%d image of \"%s\"",
			  img->w, img->h, imf->name);
	}
    }
}

static int
bitmaps_match(int w, int h, Obj *lispdata, char *rawdata)
{
    int i, j = 0, rowbytes, numbytes, byte;
    const char *datastr = NULL;

    rowbytes = computed_rowbytes(w, 1);
    numbytes =  h * rowbytes;
    for (i = 0; i < numbytes; ++i) {
	if (datastr == NULL || datastr[j] == '\0') {
		if (!stringp(car(lispdata)))
		  break;
		datastr = c_string(car(lispdata));
		j = 0;
		lispdata = cdr(lispdata);
	}
	if (datastr[j] == '/')
	  ++j;
	byte = hextoi(datastr[j]) * 16 + hextoi(datastr[j+1]);
	j += 2;
	if (byte != rawdata[i])
	  return FALSE;
    }
    return TRUE;
}

/* Write the imf directory for the given set of images. */

void
write_imf_dir(char *filename, ImageFamily **imfimages, int num)
{
    int i;
    char *loc;
    const char *delims;
    char *token;
    ImageFamily *imf;
    FILE *fp;

    fp = open_file(filename, "w");
    if (fp != NULL) {
	fprintf(fp, "ImageFamilyName FileName\n");
	for (i = 0; i < num; ++i) {
	    imf = images[i];
	    loc = "???";
	    if (imf->location && !empty_string(imf->location->name)) {
		/* First remove any Unix, Mac or Windows pathnames. */
		loc = copy_string(imf->location->name);
		delims = "/:\\";
		token = strtok(loc, delims);
		while (token != NULL) {
			loc = token;
			token = strtok(NULL, delims);
		}
		/* Remove any leading dots left from Unix pathnames. */
		loc += strspn(loc, ".");
	    }
	    fprintf(fp, "%s %s\n", imf->name, loc);
	    /* (to write imf files, should scan through images once for
	       each file, writing all images found that are in that file) */
	}
	fprintf(fp, ". .\n");
	fclose(fp);
    } else {
	run_warning("could not open file \"%s\" for writing", filename);
    }
}

/* Write out the entire image family. */

void
write_imf(FILE *fp, ImageFamily *imf)
{
    Obj *palent, *posdata;
    Image *img;
    
    if (fp == NULL || imf == NULL)
      return;
    if (imf->name == NULL) {
	fprintf(fp, "; garbage image family?\n");
	return;
    }
    if (imf->notes != lispnil) {
	fprintf(fp, "(%s \"%s\"", keyword_name(K_IMF), imf->name);
	fprintf(fp, "\n  (%s ", keyword_name(K_NOTES));
	fprintlisp(fp, imf->notes);
	fprintf(fp, "))\n");
    }
    for_all_images(imf, img) {
	if (img->monodata != lispnil
	    || img->maskdata != lispnil
	    || img->colrdata != lispnil
	    || img->filedata != lispnil
	    || img->rawmonodata != NULL
	    || img->rawmaskdata != NULL
	    || img->rawcolrdata != NULL
	    || (img->w == 1 && img->h == 1)) {
	    /* Skip over synthesized images. */
	    if (img->synthetic && !write_synthetic_also) {
		continue;
	    }
	    /* Skip over empty (undefined) 1 x 1 images. */
	    if (img->w == 1 
	        && img->h == 1
	        && img->rawpalette == NULL
	        && img->palette == lispnil) {
	    	continue;
	    }
	    fprintf(fp, "(%s \"%s\"", keyword_name(K_IMF), imf->name);
	    fprintf(fp, " (");
	    fprintf(fp, "(%d %d", img->w, img->h);
	    if (img->istile && !(img->w == 1 && img->h == 1))
	      fprintf(fp, " %s", keyword_name(K_TILE));
	    if (img->isterrain)
	      fprintf(fp, " %s", keyword_name(K_TERRAIN));
	    if (img->isconnection)
	      fprintf(fp, " %s", keyword_name(K_CONNECTION));
	    if (img->isborder)
	      fprintf(fp, " %s", keyword_name(K_BORDER));
	    if (img->istransition)
	      fprintf(fp, " %s", keyword_name(K_TRANSITION));
	    fprintf(fp, ")");
	    if (img->numsubimages > 0) {
		fprintf(fp, " (%s %d", keyword_name(K_X), img->numsubimages);
		if (img->subx > 0 || img->suby > 0)
		  fprintf(fp, " %d %d", img->subx, img->suby);
		fprintf(fp, ")");
	    }
	    if (img->hexgridx > 0 && img->hexgridy > 0) {
	        fprintf(fp, " (%s %d %d)", keyword_name(K_HEXGRID),
		    img->hexgridx, img->hexgridy);
	    }
	    if (img->embedname) {
		fprintf(fp, " (%s \"%s\")",
			keyword_name(K_EMBED), img->embedname);
	    }
	    if (img->embedx >= 0 && img->embedy >= 0) {
		fprintf(fp, " (%s %d %d)",
			keyword_name(K_EMBED_AT), img->embedx, img->embedy);
	    }
	    if (img->embedw >= 0 && img->embedh >= 0) {
		fprintf(fp, " (%s %d %d)",
			keyword_name(K_EMBED_SIZE), img->embedw, img->embedh);
	    }
	    if (img->notes != lispnil) {
		fprintf(fp, "\n  (%s ", keyword_name(K_NOTES));
		fprintlisp(fp, img->notes);
		fprintf(fp, ")\n ");
	    }
	    /* Write a single color if that's what this image is. */
	    if (img->w == 1 && img->h == 1) {
		if (img->rawpalette != NULL) {
		    write_color(fp, -1,
				img->rawpalette[1],
				img->rawpalette[2],
				img->rawpalette[3]);
		} else if (img->palette != lispnil) {
		    palent = cdr(car(img->palette));
		    if (stringp(car(palent)) || symbolp(car(palent))) {
			fprintf(fp, " %s", c_string(car(palent)));
		    } else {
			write_color(fp, -1,
				    c_number(car(palent)),
				    c_number(cadr(palent)),
				    c_number(caddr(palent)));
		    }
		}
	    } else if (img->filedata != lispnil) {
		fprintf(fp, " (%s ", keyword_name(K_FILE));
		fprintf(fp, " \"%s\"", c_string(car(img->filedata)));
		posdata = cdr(img->filedata);
		if (posdata != lispnil) {
		    if (symbolp(car(posdata))) {
			fprintf(fp, " %s", c_string(car(posdata)));
			posdata = cdr(posdata);
		    }
		    fprintf(fp, " %d %d",
			    c_number(car(posdata)), c_number(cadr(posdata)));
		}
		fprintf(fp, ")");
	    } else if ((img->colrdata != lispnil || img->rawcolrdata)
			&& !color_matches_mono(img)) {
		fprintf(fp, "\n  ");
		write_pixmap(fp, img->w, img->h, img->actualw, img->actualh,
			     img->pixelsize, img->orig_pixelsize,
			     img->palette, img->rawpalette, img->numcolors,
			     img->colrdata, img->rawcolrdata);
	    }
	    if (img->monodata != lispnil || img->rawmonodata) {
		fprintf(fp, "\n  ");
		write_bitmap(fp, keyword_name(K_MONO), img->w, img->h,
			     img->monodata, img->rawmonodata);
	    }
	    if (img->maskdata != lispnil || img->rawmaskdata) {
		fprintf(fp, "\n  ");
		write_bitmap(fp, keyword_name(K_MASK), img->w, img->h,
			     img->maskdata, img->rawmaskdata);
	    }
	    fprintf(fp, "))\n");
	}
    }
}

/* Study an ostensibly color image to see if its color table includes
   black and white only (white first, then black), and if its data is
   the same as the mono version of the image. */

static int
color_matches_mono(Image *img)
{
    int i, cj, mj, rowbytes, numbytes, cbyte, mbyte;
    int col[2], red[2], grn[2], blu[2];
    const char *cdatastr = NULL, *mdatastr = NULL;
    Obj *clispdata = img->colrdata, *mlispdata = img->monodata, *palette;

    if (img->pixelsize != 1)
      return FALSE;

    /* No match possible if not a black-white-only palette. */
    if (img->numcolors > 2)
      return FALSE;

    if (img->rawpalette != NULL) {
	for (i = 0; i < 2; i++) {
	    col[i] = img->rawpalette[4*i+0];
	    red[i] = img->rawpalette[4*i+1];
	    grn[i] = img->rawpalette[4*i+2];
	    blu[i] = img->rawpalette[4*i+3];
	}
    } else if (img->palette != lispnil) {
	palette = img->palette;
	parse_lisp_palette_entry(car(palette), &col[0],
				 &red[0], &grn[0], &blu[0]);
	if (cdr(palette) == lispnil) {
	    /* If only one color in the palette, say the other one is
	       black. */
	    col[1] = 1;
	    red[1] = grn[1] = blu[1] = 0;
	    /* If the one color is black, say it's white. */
	    if (col[0] == 0
		&& red[0] < BLACK_THRESHOLD
		&& grn[0] < BLACK_THRESHOLD
		&& blu[0] < BLACK_THRESHOLD) {
		col[0] = 0;
		red[0] = grn[0] = blu[0] = 65535;
	    }
	} else {
	    /* Parse the second entry in the palette. */
	    parse_lisp_palette_entry(cadr(palette), &col[1],
				     &red[1], &grn[1], &blu[1]);
	}
    } else {
	return FALSE;
    }

    if (!(col[0] == 0
	  && red[0] > WHITE_THRESHOLD
	  && grn[0] > WHITE_THRESHOLD
	  && blu[0] > WHITE_THRESHOLD
	  && col[1] == 1
	  && red[1] < BLACK_THRESHOLD
	  && grn[1] < BLACK_THRESHOLD
	  && blu[1] < BLACK_THRESHOLD))
	return FALSE;

    /* Now compare the contents. */
    rowbytes = computed_rowbytes(img->w, 1);
    numbytes =  img->h * rowbytes;
    cj = mj = 0;
    for (i = 0; i < numbytes; ++i) {
	/* Extract one byte of the color image. */
	if (clispdata != lispnil) {
	    if (cdatastr == NULL || cdatastr[cj] == '\0') {
		if (!stringp(car(clispdata)))
		  break;
		cdatastr = c_string(car(clispdata));
		cj = 0;
		clispdata = cdr(clispdata);
	    }
	    if (cdatastr[cj] == '/')
	      ++cj;
	    cbyte = hextoi(cdatastr[cj]) * 16 + hextoi(cdatastr[cj+1]);
	    cj += 2;
	} else if (img->rawcolrdata != NULL) {
	    cbyte = (img->rawcolrdata)[i];
	} else {
	    return FALSE;
	}
	/* Extract one byte of the mono image. */
	if (mlispdata != lispnil) {
	    if (mdatastr == NULL || mdatastr[mj] == '\0') {
		if (!stringp(car(mlispdata)))
		  break;
		mdatastr = c_string(car(mlispdata));
		mj = 0;
		mlispdata = cdr(mlispdata);
	    }
	    if (mdatastr[mj] == '/')
	      ++mj;
	    mbyte = hextoi(mdatastr[mj]) * 16 + hextoi(mdatastr[mj+1]);
	    mj += 2;
	} else if (img->rawmonodata != NULL) {
	    mbyte = (img->rawmonodata)[i];
	} else {
	    return FALSE;
	}
	/* Compare the bytes. */
	if (cbyte != mbyte)
	  return FALSE;
    }
    return TRUE;
}

static void
write_pixmap(FILE *fp, int w, int h, int actualw, int actualh,
	     int pixelsize, int orig_pixelsize,
	     Obj *palette, int *rawpalette, int numcolors,
	     Obj *lispdata, char *rawdata)
{
    int dolisp, i, j = 0, rowbytes, numbytes, byte;
    const char *datastr = NULL;

    actualw = (actualw != 0 ? actualw : w);
    actualh = (actualh != 0 ? actualh : h);
    dolisp = (lispdata != lispnil);
    /* If the pixel size was mangled (Windows) we need to use the original pixel size. */
    rowbytes = computed_rowbytes(actualw, (orig_pixelsize ? orig_pixelsize : pixelsize));
    numbytes = actualh * rowbytes;
    fprintf(fp, "(%s", keyword_name(K_COLOR));
    if (actualw != w || actualh != h)
      fprintf(fp, " (%s %d %d)", keyword_name(K_ACTUAL), actualw, actualh);
    /* (should not use orig_pixelsize if !dolisp?) Yes! See above. */
    fprintf(fp, " (%s %d)", keyword_name(K_PIXEL_SIZE),
	    (orig_pixelsize ? orig_pixelsize : pixelsize));
    if (palette != lispnil || (rawpalette && numcolors))
      write_palette_contents(fp, palette, rawpalette, numcolors);
    fprintf(fp, "\n   \"");
    for (i = 0; i < numbytes; ++i) {
	if (i > 0 && i % 32 == 0)
	  fprintf(fp, "\"\n   \"");
	if (i > 0 && i % 32 != 0 && i % rowbytes == 0)
	  fprintf(fp, "/");
	if (dolisp) {
	    if (datastr == NULL || datastr[j] == '\0') {
		if (!stringp(car(lispdata)))
		  break;
		datastr = c_string(car(lispdata));
		j = 0;
		lispdata = cdr(lispdata);
	    }
	    if (datastr[j] == '/')
	      ++j;
	    byte = hextoi(datastr[j]) * 16 + hextoi(datastr[j+1]);
	    j += 2;
	} else {
	    byte = rawdata[i];
	}
	fprintf(fp, "%02x", (unsigned char) byte);
    }
    fprintf(fp, "\")");
}

static void
write_bitmap(FILE *fp, const char *subtyp, int w, int h, Obj *lispdata,
	     char *rawdata)
{
    int dolisp, i, j = 0, rowbytes, numbytes, byte;
    const char *datastr = NULL;

    /* Lisp data overrides raw byte data. */	
    dolisp = (lispdata != lispnil);	
    rowbytes = computed_rowbytes(w, 1);
    numbytes =  h * rowbytes;
    fprintf(fp, "(%s", subtyp);
    if (w > 16 || h > 16)
      fprintf(fp, "\n  ");
    fprintf(fp, " \"");
    for (i = 0; i < numbytes; ++i) {
	if (i > 0 && i % 32 == 0)
	  fprintf(fp, "\"\n   \"");
	if (i > 0 && i % 32 != 0 && i % rowbytes == 0)
	  fprintf(fp, "/");
	if (dolisp) {
	    if (datastr == NULL || datastr[j] == '\0') {
		if (!stringp(car(lispdata)))
		  break;
		datastr = c_string(car(lispdata));
		j = 0;
		lispdata = cdr(lispdata);
	    }
	    /* Ignore any slashes, they're for human readability. */
	    if (datastr[j] == '/')
	      ++j;
	    byte = hextoi(datastr[j]) * 16 + hextoi(datastr[j+1]);
	    j += 2;
	} else {
	    byte = rawdata[i];
	}
	fprintf(fp, "%02x", (unsigned char) byte);
    }
    fprintf(fp, "\")");
}

static void
write_palette_contents(FILE *fp, Obj *palette, int *rawpalette, int numcolors)
{
    int len, i, col, red, grn, blu;
    Obj *restpal;

    len = (palette != lispnil ? length(palette) : numcolors);
    if (len > 2)
      fprintf(fp, "\n  ");
    fprintf(fp, " (%s", keyword_name(K_PALETTE));
    if (palette != lispnil) {
	for_all_list(palette, restpal) {
	    parse_lisp_palette_entry(car(restpal), &col, &red, &grn, &blu);
	    if (len > 2)
	      fprintf(fp, "\n   ");
 	    write_color(fp, col, red, grn, blu);
	}
    } else if (rawpalette != NULL) {
	for (i = 0; i < numcolors; i++) {
	    if (len > 2)
	      fprintf(fp, "\n   ");
	    write_color(fp, rawpalette[4*i],
			rawpalette[4*i+1], rawpalette[4*i+2], rawpalette[4*i+3]);
	}
    } else {
	fprintf(fp, " #| no palette? |# ");
    }
    fprintf(fp, ")");
}

static void
write_color(FILE *fp, int n, int r, int g, int b)
{
    const char *colorname;

    if (n >= 0)
      fprintf(fp, " (%d", n);
    colorname = find_color_name(r, g, b);
    if (!empty_string(colorname)) {
	/* Write color name.  Note that we write as a symbol, so that
	   each instance of "white" doesn't become a separate string. */
	fprintf(fp, " %s", colorname);
    } else {
	/* Write individual color components. */
	fprintf(fp, " %d %d %d", r, g, b);
    }
    if (n >= 0)
      fprintf(fp, ")");
}

/* Given rgb components, return names of standard colors if the match
   is close. */

const char *
find_color_name(int r, int g, int b)
{
    if (r > WHITE_THRESHOLD
	&& g > WHITE_THRESHOLD
	&& b > WHITE_THRESHOLD)
      return "white";
    else if (r < BLACK_THRESHOLD
	&& g < BLACK_THRESHOLD
	&& b < BLACK_THRESHOLD)
      return "black";
    else if (r > WHITE_THRESHOLD
	&& g < BLACK_THRESHOLD
	&& b < BLACK_THRESHOLD)
      return "red";
    else if (r < BLACK_THRESHOLD
	&& g > WHITE_THRESHOLD
	&& b < BLACK_THRESHOLD)
      return "green";
    else if (r < BLACK_THRESHOLD
	&& g < BLACK_THRESHOLD
	&& b > WHITE_THRESHOLD)
      return "blue";
    else
      return NULL;
}

void
parse_lisp_palette_entry(Obj *palentry, int *col, int *red, int *grn, int *blu)
{
    Obj *colorcomp;
    const char *colorname;

    *col = c_number(car(palentry));
    colorcomp = cdr(palentry);
    if (colorcomp == lispnil)
        return;
    if (symbolp(car(colorcomp)) || stringp(car(colorcomp))) {
	colorname = c_string(car(colorcomp));
	*red = *grn = *blu = 0;
	if (strcmp(colorname, "white") == 0) {
	    *red = *grn = *blu = 65535;
	} else if (strcmp(colorname, "black") == 0) {
	    /* done */
	} else if (strcmp(colorname, "red") == 0) {
	    *red = 65535;
	} else if (strcmp(colorname, "green") == 0) {
	    *grn = 65535;
	} else if (strcmp(colorname, "blue") == 0) {
	    *blu = 65535;
	} else {
	    init_warning("No color named \"%s\" found, substituting gray",
			 colorname);
	    *red = *grn = *blu = 128 * 256;
	}
    } else if (numberp(car(colorcomp))) {
	*red = c_number(car(colorcomp));
	*grn = c_number(cadr(colorcomp));
	*blu = c_number(caddr(colorcomp));
	/* Assume small values are 8-bit rather than 16-bit colors.
	   (Works because as 16-bit colors, they would all be nearly
	   identical shades of black.) */
	if (*red < 256)
	  *red *= 256;
	if (*grn < 256)
	  *grn *= 256;
	if (*blu < 256)
	  *blu *= 256;
    } else {
	init_warning("palette color info is not a name or set of numbers, ignoring");
    }
}

/* Given a filename, find or create a file image structure for it. */

FileImage *get_file_image(const char *fname)
{
    FileImage *fimg, *newfimg;

    for (fimg = file_images; fimg != NULL; fimg = fimg->next) {
	if (strcmp(fimg->name, fname) == 0)
	  return fimg;
    }
    newfimg = (FileImage *) xmalloc(sizeof(FileImage));
    newfimg->name = fname;
    newfimg->next = file_images;
    file_images = newfimg;
    return newfimg;
}

/* Collect the file image for the given image and use it to generate
   the image's (or subimage's) raw data. */

void
make_image_from_file_image(ImageFamily *imf, Image *img, Image *subimg,
			   int subi)
{
    int hch, pad = 2, stdlayout = FALSE;
    int xoffset, yoffset, xoff, yoff;
    Obj *posdata;
    FileImage *fimg;

    if (img->filedata != lispnil) {
	if (img->file_image == NULL) {
	    img->file_image = get_file_image(c_string(car(img->filedata)));
	}
    }
    fimg = img->file_image;
    load_file_image(fimg);
    if (!fimg->loaded) {
	init_warning("Could not load file \"%s\" for image family \"%s\", ignoring",
		     fimg->name, imf->name);
	return;
    }
    xoffset = yoffset = 0;
    stdlayout = FALSE;
    posdata = cdr(img->filedata);
    if (posdata != lispnil) {
	if (match_keyword(car(posdata), K_STD)) {
	    stdlayout = TRUE;
	    posdata = cdr(posdata);
	}
	xoffset = c_number(car(posdata));
	yoffset = c_number(cadr(posdata));
	if (stdlayout) {
	    xoffset = (xoffset * (img->w + 2)) + 2;
	    yoffset = (yoffset * (img->h + 2)) + 2;
	}
    }
    /* All subimages share color data. */
    if (subimg == img ||
	(img->subimages != NULL && subimg == img->subimages[0])) {
	subimg->pixelsize = 8;
	/* Copy the palette over verbatim from the file image. */
	subimg->numcolors = fimg->numcolors;
	subimg->rawpalette =
	  (int *) xmalloc(257 /*subimg->numcolors*/ * 4 * sizeof(int));
	memcpy(subimg->rawpalette, fimg->palette,
	       subimg->numcolors * 4 * sizeof(int));
    } else {
	/* Inherit color data from the first subimage. */
	subimg->pixelsize = img->subimages[0]->pixelsize;
	subimg->numcolors = img->subimages[0]->numcolors;
	subimg->rawpalette = img->subimages[0]->rawpalette;
    }
    if (img->isborder) {
	hch = calculate_hch(img->h);
	xoff = xoffset + (subi % 4) * img->w;
	yoff = yoffset + (subi / 4) * hch;
	copy_from_file_image(subimg, fimg, xoff, yoff, 0, hch);
    } else if (img->isconnection) {
	xoff = xoffset + (subi % 8) * (img->w + pad);
	yoff = yoffset + (subi / 8) * (img->h + pad);
	copy_from_file_image(subimg, fimg, xoff, yoff, 0, 0);
    } else if (img->istransition) {
	xoff = xoffset + (subi % 4) * (img->w + pad);
	yoff = yoffset + (subi / 4) * (img->h + pad);
	copy_from_file_image(subimg, fimg, xoff, yoff, 0, 0);
    } else if (img->numsubimages > 0) {
	/* Figure where the next subimage is at. */
        if (img->hexgridx > 0 && img->hexgridy > 0) {
	    xoff = xoffset + (subi%img->hexgridx) * img->w;
	    if ((subi/img->hexgridx)%2 != 0)
	        xoff += (img->w/2);
	    yoff = yoffset + (subi/img->hexgridx) * ((img->h*3)/4 + 1);
	} else if (img->subx != 0 || img->suby != 0) {
	    xoff = xoffset + subi * img->subx;
	    yoff = yoffset + subi * img->suby;
	} else {
	    xoff = xoffset + subi * img->w;
	    yoff = yoffset;
	}
	copy_from_file_image(subimg, fimg, xoff, yoff, 0, 0);
    } else {
	copy_from_file_image(img, fimg, xoffset, yoffset, 0, 0);
    }
   
    /* If this is any sort of terrain, apply a hex mask, and blow away
       the LISP version of the mask if it no longer matches */
    if (img->isterrain || img->isconnection ||
	img->istransition) {
        add_hex_mask(subimg);
        if (subimg->maskdata != lispnil && !bitmaps_match(subimg->w, subimg->h,
	    subimg->maskdata, subimg->rawmaskdata))
	    subimg->maskdata = lispnil;
    }
}

/* Extract a single color image from a file image.  Also create mask
   data for the image, if the file image has any transparent colors.  */

void
copy_from_file_image(Image *img, FileImage *fimg, int xoffset, int yoffset,
		     int actualw, int actualh)
{
    char pix;
    int i, j, k, ii, val, kk, kkb;

    /* Make space for the color data (assuming 1 byte/pixel). */
    img->rawcolrdata = (char *) xmalloc(img->w * img->h);
    if (fimg->numtransparent > 0)
      img->rawmaskdata =
	(char *) xmalloc(img->h * computed_rowbytes(img->w, 1));
    /* Scan through all the pixels of the subimage we're building. */
    for (i = 0; i < img->h; ++i) {
	for (j = 0; j < img->w; ++j) {
	    k = (yoffset + i) * fimg->width + xoffset + j;
	    pix = fimg->data[k];
	    kk = i * img->w + j;
	    img->rawcolrdata[kk] = pix;
	    if ((actualw > 0 && j >= actualw)
		|| (actualh > 0 && i >= actualh))
	      img->rawcolrdata[kk] = img->rawcolrdata[0];
	    /* If there are transparent colors in the image, modify
	       the mask bitmap. */
	    if (fimg->numtransparent > 0) {
		val = 1;
		for (ii = 0; ii < fimg->numtransparent; ++ii) {
		    if (pix == fimg->transparent[ii]) {
			val = 0;
			break;
		    }
		}
		if (val) {
		    kkb = i * computed_rowbytes(img->w, 1) + j / 8;
		    img->rawmaskdata[kkb] |= 1 << (7 - j % 8);
		    if ((actualw > 0 && j >= actualw)
			|| (actualh > 0 && i >= actualh))
		      img->rawmaskdata[kkb] = 0;
		}
	    }
	}
    }
}

/* Give a file image, attempt to load it into memory. */

void
load_file_image(FileImage *fimg)
{
    int rslt;

    if (fimg->loaded)
      return;
    /* Only doing GIFs right now. */
    rslt = get_gif(fimg);
    if (rslt)
      fimg->loaded = TRUE;
}

/* Generic image setup. */

static short imf_dir_loaded;

ImageFamily *
get_generic_images(const char *name)
{
    FILE *fp;
    ImageFamily *imf;
    LibraryPath *path;

    imf = get_imf(name);
    if (imf == NULL)
      return NULL;
    if (imf->numsizes > 0 && imf_interp_hook != NULL)
      imf = (*imf_interp_hook)(imf, NULL, FALSE);
    if (imf_load_hook != NULL)
      imf = (*imf_load_hook)(imf);
    /* Always check that the imf location is loaded 
	(fixes emblem loading from saved games on the mac). */
    if (imf->numsizes == 0 
	|| (imf->location && imf->location->loaded != TRUE)) {
	/* Maybe collect the names/locations of all image families. */
	if (!imf_dir_loaded) {
	    /* (should let name be decided by platform?) */
	    fp = open_library_file("imf.dir");
	    if (fp != NULL) {
		load_image_families(fp, FALSE, NULL);
		fclose(fp);
	    } else {
		init_warning("Cannot open \"%s\", will not use it", "imf.dir");
	    }
	    imf_dir_loaded = TRUE;
	}
	/* Get a (possibly empty) family. */
	imf = get_imf(name);
	if (imf == NULL)
	  return NULL;
	if (imf->location != NULL) {
	    /* Load data filling in the family. */
	    for_all_library_paths(path) {
		make_pathname(path->path, imf->location->name, "", spbuf);
		if (load_imf_file(spbuf, NULL)) {
			imf->location->loaded = TRUE;
			break;
		}
	    }
	    /* Maybe try the plain filename, just in case. */
	    if (!imf->location->loaded) {
	    	imf->location->loaded = 
		    load_imf_file(imf->location->name, NULL);
	    }
	    /* We don't complain here about not finding the file, because
	       we'll get a more useful warning alert later on. */
	    if (imf_interp_hook != NULL)
	      imf = (*imf_interp_hook)(imf, NULL, FALSE);
	}
    }
    return imf;
}
