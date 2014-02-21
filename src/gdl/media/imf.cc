// xConq
// Lifecycle management and serialization/deserialization of GDL image families.

// $Id: imf.cc,v 1.4 2006/06/02 16:57:43 eric_mcdonald Exp $

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
    \brief Lifecycle management and serialization/deserialization of GDL image families.
    \ingroup grp_gdl
*/

#include "gdl/media/imf.h"
#include "gdl/module.h"

// Local Constant Macros

//! RGB above this value should be considered white.
#define WHITE_THRESHOLD (65535 - 256)
//! RGB below this value should be considered black.
#define BLACK_THRESHOLD (0 + 255)

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Global Variables: Image Families

ImageFamily **images;
int numimages = 0;

ImageFamily **recorded_imfs;
int num_recorded_imfs;

// Local Variables: Behavior Options

short write_synthetic_also;

// Local Function Declarations: Sorting

//! Compare two GDL image families.
static int image_name_compare(const void *imf1, const void *imf2);

//! Sort all recorded images.
/*!
    The comparison function for the image list just does "strcmp" order
    and *requires* that all image families be named and named uniquely.
*/
static void sort_all_recorded_images(void);

// Local Function Declarations: Lifecycle Management

//! Create new image family.
static ImageFamily *new_imf(char *name);

// Local Function Declarations: GDL I/O

//! Write contents of GDL palette to file.
static void write_palette_contents(
    FILE *fp, Obj *palette, int *rawpalette, int numcolors);

//! Does color image match mono equivalent of it?
/*!
    Study an ostensibly color image to see if its color table includes
    black and white only (white first, then black), and if its data is
    the same as the mono version of the image.
*/
static int color_matches_mono(Image *img);
//! Write color to file.
static void write_color(FILE *fp, int n, int r, int g, int b);
//! Write bitmap to file.
static void write_bitmap(
    FILE *fp, char *subtyp, int w, int h, Obj *data, char *rawdata);
//! Write pixmap to file.
static void write_pixmap(
    FILE *fp,
    int w, int h, int aw, int ah,
    int pixelsize, int orig_pixelsize,
    Obj *palette, int *rawpalette, int numcolors,
    Obj *lispdata, char *rawdata);

// Queries

ImageFamily *
find_imf(char *name)
{
    int i;

    for (i = 0; i < numimages; ++i) {
	if (strcmp(name, images[i]->name) == 0)
	  return images[i];
    }
    return NULL;
}

ImageFamily *
get_imf(char *name)
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

// Validation

int
valid_imf_name(char *name)
{
    char *tmp;

    for (tmp = name; *tmp; ++tmp) {
	if (!(isalnum(*tmp)
	      || (tmp != name && *tmp == '-')))
	  return FALSE;
    }
    return TRUE;
}

// Image Preparation

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

// Sorting

static
int
image_name_compare(const void *imf1, const void *imf2)
{
    return strcmp((*((ImageFamily **) imf1))->name,
		  (*((ImageFamily **) imf2))->name);
}

static
void
sort_all_recorded_images(void)
{
    qsort(&(recorded_imfs[0]), num_recorded_imfs, sizeof(ImageFamily *),
	  image_name_compare);
}

// Lifecycle Management

static
ImageFamily *
new_imf(char *name)
{
    ImageFamily *imf;

    imf = (ImageFamily *) xmalloc(sizeof(ImageFamily));
    imf->name = name;
    imf->notes = lispnil;
    return imf;
}

// GDL I/O

void
parse_lisp_palette_entry(Obj *palentry, int *col, int *red, int *grn, int *blu)
{
    Obj *colorcomp;
    char *colorname;

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

char *
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

static
void
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

static
int
color_matches_mono(Image *img)
{
    int i, cj, mj, rowbytes, numbytes, cbyte, mbyte;
    int col[2], red[2], grn[2], blu[2];
    char *cdatastr = NULL, *mdatastr = NULL;
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

static
void
write_color(FILE *fp, int n, int r, int g, int b)
{
    char *colorname;

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

static
void
write_bitmap(FILE *fp, char *subtyp, int w, int h, Obj *lispdata,
	     char *rawdata)
{
    int dolisp, i, j = 0, rowbytes, numbytes, byte;
    char *datastr = NULL;

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

static
void
write_pixmap(FILE *fp, int w, int h, int actualw, int actualh,
	     int pixelsize, int orig_pixelsize,
	     Obj *palette, int *rawpalette, int numcolors,
	     Obj *lispdata, char *rawdata)
{
    int dolisp, i, j = 0, rowbytes, numbytes, byte;
    char *datastr = NULL;

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

void
write_images(void)
{
    int i;

    if (recorded_imfs == NULL || num_recorded_imfs == 0)
      return;
    sort_all_recorded_images();
    for (i = 0; i < num_recorded_imfs; ++i) {
	write_imf(wfp, recorded_imfs[i]);
    }
}

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END
