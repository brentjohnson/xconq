/* Utilities needed for X11 utility programs.
   Copyright (C) 1993-1999 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "config.h"
#include "misc.h"
#include "lisp.h"
#include "imf.h"
#include "xcutil.h"
#include "ximf.h"

#ifdef __cplusplus
extern "C" {
#endif

#if (!defined(__CYGWIN32__) && !defined(__MINGW32__))
#include <X11/Xos.h>
#endif
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <X11/Xmu/Drawing.h>

#ifdef HAVE_XPM
#include <X11/xpm.h>
#endif

#ifdef __cplusplus
}
#endif

char *outdirname = NULL;

/* extern LibraryPath *xconq_libs; */

char spbuf[BUFSIZE];

char readerrbuf[1000];

/* This is needed because error handling refers to it. */

void
close_displays(void)
{
}

/* This is needed because error handling refers to it. */

int
write_entire_game_state(char *fname)
{
    return 0;
}

/* Need these so we can link in Lisp reader code. */

void
announce_read_progress(void)
{
}

void  
syntax_error(Obj *x, char *msg)
{
    sprintlisp(readerrbuf, x, BUFSIZE);
    init_warning("syntax error in `%s' - %s", readerrbuf, msg);
}

void
low_init_error(char *str)
{
    fprintf(stderr, "Error: %s.\n", str);
    fflush(stderr);
}

/* A warning just gets displayed, no other action is taken. */

void
low_init_warning(char *str)
{
    fprintf(stdout, "Warning: %s.\n", str);
    fflush(stdout);
}

/* A run error is fatal. */

void
low_run_error(char *str)
{
    fprintf(stderr, "Error: %s.\n", str);
    fflush(stderr);
    exit(1);
}

void
low_run_warning(char *str)
{
    fprintf(stdout, "Warning: %s.\n", str);
    fflush(stdout);
}

/* Fake definitions of unneeded routines called by lisp.c. */

int
keyword_code(char *str)
{
    run_warning("fake keyword_code being called");
    return 0;
}

/* Make the table so keyword lookup works. */

struct a_key {
    char *name;
} keywordtable[] = {

#undef  DEF_KWD
#define DEF_KWD(NAME,code)  { NAME },

#include "keyword.def"

    { NULL }
};

char *
keyword_name(enum keywords k)
{
    return keywordtable[k].name;
}

void
init_predefined_symbols()
{
}

int
lazy_bind(Obj *sym)
{
    run_warning("fake lazy_bind being called");
    return FALSE;
}

void
prealloc_debug()
{
}

/* Read/write images in X format. */

Image *
read_xbm_file(char *filename, ImageFamily *imf, readimf_hook hook)
{
    FILE *ifp;
    int rows, cols, rowbytes, numbytes;
    unsigned int urows, ucols;
    unsigned char *urawdata;
    char *rawdata;
    Image *img;

    if (imf == NULL)
      return NULL;
    if ((ifp = fopen(filename, "r")) == NULL)
      return NULL;
    if (XmuReadBitmapData(ifp, &ucols, &urows, &urawdata, NULL, NULL)
	!= BitmapSuccess)
      return NULL;
    cols = ucols;
    rows = urows;
    rawdata = (char *) urawdata;
    fclose(ifp);
    img = get_img(imf, cols, rows);
    if (img == NULL)
      return NULL;
    if (hook) {
	/* Force image recalculation. */
	img->monodata = img->colrdata = img->maskdata = lispnil;
    }
    rowbytes = computed_rowbytes(cols, 1);
    numbytes = rows * rowbytes;
    if (filename[strlen(filename)-1] == 'm') {
	img->rawmaskdata = (char *)xmalloc(numbytes);
	memcpy(img->rawmaskdata, rawdata, numbytes);
	reverse_bit_endianness(img->rawmaskdata, numbytes);
    } else {
	img->rawmonodata = (char *)xmalloc(numbytes);
	memcpy(img->rawmonodata, rawdata, numbytes);
	reverse_bit_endianness(img->rawmonodata, numbytes);
    }
    return img;
}

Image *
read_xpm_file(char *filename, ImageFamily *imf, readimf_hook hook)
{
#ifdef HAVE_XPM
    int c, numbytes, *dp, r, ri, cols, rows, rowbytes;
    char *rp, rmask;
    XpmImage xpmimage;
    XpmInfo info;
    Image *img;

    if (imf == NULL)
      return NULL;
    if (XpmReadFileToXpmImage(filename, &xpmimage, &info) != XpmSuccess)
      return NULL;
    cols = xpmimage.width;
    rows = xpmimage.height;
    img = get_img(imf, cols, rows);
    if (img == NULL)
      return NULL;
    if (hook) {
	/* Force image recalculation. */
	img->monodata = img->colrdata = img->maskdata = lispnil;
    }
    img->actualw = cols;
    img->actualh = rows;
    img->numcolors = xpmimage.ncolors;
    if (xpmimage.ncolors > 256) {
	run_warning("Image with more then 256 colors not supported");
	return NULL;
    } else if (xpmimage.ncolors > 16) {
	img->pixelsize = 8;
    } else if (xpmimage.ncolors > 4) {
	img->pixelsize = 4;
    } else if (xpmimage.ncolors > 2) {
	img->pixelsize = 2;
    } else {
	img->pixelsize = 1;
    }
    rowbytes = computed_rowbytes(img->w, img->pixelsize);
    img->rawpalette = (int *) xmalloc(img->numcolors * 4 * sizeof(int));
    for (c = 0; c < img->numcolors; c++) {
	img->rawpalette[4 * c] = c;
	parse_xpm_colors(xpmimage.colorTable[c].c_color,
			 &img->rawpalette[4 * c + 1],
			 &img->rawpalette[4 * c + 2],
			 &img->rawpalette[4 * c + 3]);
    }
    /* This seems to be Stan's default ;-) */
    /* (Actually, this is a Mac-ism, that black is given all 1-bits,
        even when there are less than 2^n colors in an image -sts) */
    img->rawpalette[4 * (img->numcolors - 1)] = (1 << img->pixelsize) - 1;
    numbytes = img->h * rowbytes;
    img->rawcolrdata = xmalloc(numbytes);
    memset(img->rawcolrdata, '\0', numbytes);
    rp = img->rawcolrdata;
    dp = (int *) xpmimage.data;
    rmask = (1 << img->pixelsize) - 1;
    for (r = 0; r < img->h; r++) {
	ri = 8 - img->pixelsize;
	for (c = 0; c < img->w; c++) {
	    *rp |= (img->rawpalette[4*(*dp)] & rmask) << ri;
	    dp++;
	    if (ri) {
		ri -= img->pixelsize;
	    } else {
		ri = 8 - img->pixelsize;
		rp++;
	    }
	}
	if ((img->pixelsize * img->w) % 8) {
	    rp++;
	}
    }
    return img;
#else
    return NULL;
#endif
}

/* Half-witted replacement for XParseColor, which we can't use since
   we could have no display open. */

void
parse_xpm_colors(char *name, int *r, int *g, int *b)
{
    int n, q;
    char xpmcbuf[32];

    /* sometimes "pixmap" comes up with these: */
    if (strcmp(name, "white") == 0 || strcmp(name, "White") == 0) {
	*r = *g = *b = 0xffff;
	return;
    }
    if (strcmp(name, "black") == 0 || strcmp(name, "Black") == 0) {
	*r = *g = *b = 0;
	return;
    }
    if (name[0] != '#') {
	run_warning("Error parsing color %s", name);
	return;
    }
    name++;
    n = strlen(name);
    if (n % 3) {
	run_warning("Error parsing color %s", name);
	return;
    }
    n /= 3;
    sprintf(xpmcbuf, "%%%dx%%%dx%%%dx", n, n, n);
    q = sscanf(name, xpmcbuf, r, g, b);
    if (q != 3) {
	run_warning("Error parsing color %s", name);
	return;
    }
    if (n < 4) {
	*r <<= 4 * (4 - n);
	*g <<= 4 * (4 - n);
	*b <<= 4 * (4 - n);
    }
}

void
write_x11_bitmaps(ImageFamily *imf, int mkfiles)
{
    int w, h, rowbytes, numbytes;
    char ext[20], fname[255];
    FILE *fp;
    Image *img;

    if (imf == NULL || imf->name == NULL)
      return;
    for_all_images(imf, img) {
	w = img->w;  h = img->h;
	/* unified size marker in extension.  Massimo */
	sprintf(ext, ".%dx%d", w, h);
	if (img->monodata != lispnil && img->rawmonodata == NULL) {
	    rowbytes = computed_rowbytes(w, 1);
	    numbytes = h * rowbytes;
	    img->rawmonodata = (char *)xmalloc(numbytes);
	    interp_bytes(img->monodata, numbytes, img->rawmonodata, 0);
	}
	if (img->rawmonodata) {
	    if (mkfiles) {
		sprintf(fname, "%s/%s%s.b", outdirname, imf->name, ext);
		fp = fopen(fname, "w");
	    } else {
		fp = stdout;
	    }
	    if (fp != NULL) {
		reverse_bit_endianness(img->rawmonodata, numbytes);
		write_xbm_file(fp, imf->name, w, h, img->rawmonodata);
		reverse_bit_endianness(img->rawmonodata, numbytes);
		if (fp != stdout)
		  fclose(fp);
	    }
	}
	if (img->maskdata != lispnil && img->rawmaskdata == NULL) {
	    rowbytes = computed_rowbytes(w, 1);
	    numbytes = h * rowbytes;
	    img->rawmaskdata = (char *)xmalloc(numbytes);
	    interp_bytes(img->maskdata, numbytes, img->rawmaskdata, 0);
	}
	if (img->rawmaskdata) {
	    if (mkfiles) {
		sprintf(fname, "%s/%s%s.m", outdirname, imf->name, ext);
		fp = fopen(fname, "w");
	    } else {
		fp = stdout;
	    }
	    if (fp != NULL) {
		reverse_bit_endianness(img->rawmaskdata, numbytes);
		write_xbm_file(fp, imf->name, w, h, img->rawmaskdata);
		reverse_bit_endianness(img->rawmaskdata, numbytes);
		if (fp != stdout)
		  fclose(fp);
	    }
	}
	if (img->colrdata != lispnil && img->rawcolrdata == NULL) {
	    rowbytes = computed_rowbytes(w, img->pixelsize);
	    numbytes = h * rowbytes;
	    img->rawcolrdata = (char *)xmalloc(numbytes);
	    interp_bytes(img->colrdata, numbytes, img->rawcolrdata, 0);
	}
	if (img->rawcolrdata) {
	    if (mkfiles) {
		sprintf(fname, "%s/%s%s.xpm", outdirname, imf->name, ext);
		fp = fopen(fname, "w");
	    } else {
		fp = stdout;
	    }
	    if (fp != NULL) {
		write_xpm_file(fp, imf->name, img);
		if (fp != stdout)
		  fclose(fp);
	    }
	}
    }
}

/* Write a bitmap in more-or-less standard X11 format. */

void
write_xbm_file(FILE *fp, char *name, int cols, int rows, char *data)
{
    int row, i, col, bytesperline = 8, byte, byte2, j, firstitem = 1;
    int numbytes = computed_rowbytes(cols, 1) * rows;
    char fixedname[BUFSIZE];

    for (i = 0; name[i] != '\0'; ++i) {
      fixedname[i] = (name[i] == '-' ? '_' : name[i]);
    }
    fixedname[i] = '\0';
    reverse_bit_endianness(data, numbytes);

    fprintf(fp, "#define %s_width %d\n", fixedname, cols);
    fprintf(fp, "#define %s_height %d\n", fixedname, rows);
    fprintf(fp, "static char %s_bits[] = {", fixedname);

    i = 0;
    for (row = 0; row < rows; row++) {
	for (col = 0; col < cols; col++) {
	    if (col % 8 == 0) {
		if (firstitem) {
		    firstitem = 0;
		} else {
		    putc(',', fp);
		    putc(' ', fp);
		}
		if (bytesperline >= 8) {
		    fprintf(fp, "\n   ");
		    bytesperline = 0;
		}
		/* Make little-endian bytes. */
		byte = data[i++];
		byte2 = 0;
		for (j = 0; j < 8; ++j) {
		    byte2 = (byte2 << 1) | (byte & 1);
		    byte >>= 1;
		}
		fprintf(fp, "0x%x", byte2);
		++bytesperline;
	    }
	}
    }
    fprintf(fp, "};\n" );

    reverse_bit_endianness(data, numbytes);
}

/* Write a pixmap in XPM format.  Note that this does not need to
   be conditionalized on HAVE_XPM, does not use any XPM functions. */

void
write_xpm_file(FILE *fp, char *name, Image *img)
{
    int i, r, ri, rc, c, numcols, rmask, col, red, grn, blu;
    char pch[256], *rp;
    int idx[256];
    Obj *restpal;
    char fixedname[BUFSIZE];

    if (img->palette == lispnil && !(img->rawpalette && img->numcolors))
      return;

    /* C-ify the name. */
    for (i = 0; name[i] != '\0'; ++i) {
	fixedname[i] = (name[i] == '-' ? '_' : name[i]);
    }
    fixedname[i] = '\0';
    if (img->palette == lispnil) {
	numcols = img->numcolors;
    } else {
	numcols = length(img->palette);
    }
    fprintf(fp, "/* XPM */\n");
    fprintf(fp, "static char * %s [] = {\n", fixedname);
    fprintf(fp, "\"%d %d %d %d\",\n", img->w, img->h, numcols, 1);
    for (c = 0; c < numcols; c++) {
	if (c < 26) {
	    pch[c] = 'a' + c;
	} else if (c < 52) {
	    pch[c] = 'A' + c - 26;
	} else if (c < 62) {
	    pch[c] = '0' + c - 52;
	} else {
	    pch[c] = '#' + c - 62;
	}	
    }  
    if (img->palette == lispnil) {
	for (c = 0; c < numcols; c++) {
	    fprintf(fp, "\"%c\tc #%4.4x%4.4x%4.4x\",\n",
		    pch[c], 
		    img->rawpalette[4 * c + 1],
		    img->rawpalette[4 * c + 2],
		    img->rawpalette[4 * c + 3]);
	    idx[img->rawpalette[4 * c]] = c;
	}
    } else {
	c = 0;
	for_all_list (img->palette, restpal) {
	    parse_lisp_palette_entry(car(restpal), &col, &red, &grn, &blu);
	    fprintf(fp, "\"%c\tc #%4.4x%4.4x%4.4x\",\n",
		    pch[c], red, grn, blu);
	    idx[col] = c;
	    c++;
	}
    }
    if (numcols > 74) {
	run_warning("write_xpm_file not implemented for %d colors",
		    numcols);
	return;
    }

    rmask = (1 << img->pixelsize) - 1;
    rp = img->rawcolrdata;
    for (r = 0; r < img->h; r++) {
	ri = 8 - img->pixelsize;
	fputc('"', fp);
	for (c = 0; c < img->w; c++) {
	    rc = ((int) (*rp >> ri)) & rmask;
	    if (ri) {
		ri -= img->pixelsize;
	    } else {
		ri = 8 - img->pixelsize;
		rp++;
	    }
	    fputc(pch[idx[rc]], fp);
	}
	if (r == img->h - 1) {
	    fprintf(fp, "\"};\n");
	} else {
	    fprintf(fp, "\",\n");
	}
	if ((img->pixelsize * img->w) % 8) {
	    rp++;
	}
    }
}

/* Given a raw filename, come up with a image family name. */

char *
find_imf_name(char *rawname)
{
    int lastpos;
    char *a, *b;

    b = copy_string(rawname);
    lastpos = strlen(b) - 1;
    /* Remove leading path. */
    for (a = b + lastpos - 1; a > b; --a) {
	if (*a == '/') {
	    b = a + 1;
	    break;
	}
    }
    /* Remove trailing extension. */
    if ((a = strchr(b, '.'))) {
	*a = '\0';
    }
    return b;
}

int
read_any_file(char *filename, readimf_hook hook)
{
    ImageFamily *imf;
    Image *img;
    char *ext;

    /* Find the extension. */
    for (ext = filename + strlen(filename) - 1; ext > filename; ext--) {
	if (*ext == '.') {
	    ext++;
	    break;
	}
    }
    if (ext == filename)
      ext = "";
#ifdef HAVE_XPM
    if (strcmp(ext, "xpm") == 0) {
	/* try a XPM file */
	imf = get_imf(find_imf_name(filename));
	if (imf == NULL)
	  return 0;
	img = read_xpm_file(filename, imf, hook);
	if (hook && img)
	  (*hook)(imf, 0);
	return (img != NULL);
    }
#endif /* HAVE_XPM */
    if (strcmp(ext, "b") == 0 || strcmp(ext, "m") == 0) {
	/* try a XBM file */
	imf = get_imf(find_imf_name(filename));
	if (imf == NULL)
	  return 0;
	img = read_xbm_file(filename, imf, hook);
	if (hook && img)
	  (*hook)(imf, 0);
	return (img != NULL);
    } else if (strcmp(ext, "imf") == 0 || strcmp(ext, "g") == 0) {
	return load_imf_file(filename, hook);
    }
    return 0;
}

void 
reverse_rawdata(ImageFamily *imf)
{
    int numbytes;
    Image *img;

    if (imf == NULL)
      return;
    for_all_images(imf, img) {
	numbytes = img->h * computed_rowbytes(img->w, 1);
	if (img->rawmonodata)
	  reverse_bit_endianness(img->rawmonodata, numbytes);
	if (img->rawmaskdata)
	  reverse_bit_endianness(img->rawmaskdata, numbytes);
    }
}
