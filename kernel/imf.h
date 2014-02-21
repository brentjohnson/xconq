/* Definitions for Xconq images.
   Copyright (C) 1992-2000 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/*! \file kernel/imf.h
 * \brief Definitions for the image manipulation routines
 */

/*!
 * An image family is like a Mac Finder icon family, but allows
 * multiple kinds of images of an arbitrary set of sizes.  Individual
 * images in a family may have both "lisp" form and "raw" form, the
 * former being used for reading and writing, while the raw form is an
 * intermediary for conversion to and from platform-specific
 * representations.
 */
typedef struct a_image {
    short w;					/*!< Nominal size of the image - width */
    short h;					/*!< Nominal size of the image - height */
    short istile;				/*!< True if image may be used as tile */
    short isterrain;			/*!< True if image may be used for basic terrain. */
    short isconnection;			/*!< True if image may be used for conns. */
    short isborder;			/*!< True if image may be used for bords. */
    short istransition;			/*!< True if image is a transition. */
    short numsubimages;		/*!< Number of sub images. */
    short subx;					/*!< Subimage x. */
    short suby;					/*!< Subimage y. */
    char *embedname;				/*!< Name of an embedded subimage (imf name) */
    short embedx;					/*!< Position to draw an embedded subimage - x. */
    short embedy;					/*!< Position to draw an embedded subimage - y. */
    short embedw;					/*!< Size of space for embedded subimage - width. */
    short embedh;					/*!< Size of space for embedded subimage - height. */
    Obj *monodata;				/*!< Monochrome data, in GDL form */
    Obj *colrdata;					/*!< Color data, in GDL form */
    Obj *maskdata;				/*!< Mask data, in GDL form */
    Obj *filedata;					/*!< How to find data in a file */
    struct a_file_image *file_image;	/*!< Pointer to file image */
    short actualw;					/*!< Actual Size of image - width. */
    short actualh;					/*!< Actual size of image - height. */
    short pixelsize;				/*!< Number of bits per pixel */
    short orig_pixelsize;			/*!< Pixels before modification */
    Obj *palette;					/*!< Color palette, in list form */
    Obj *notes;					/*!< designer notes about the image */
    short synthetic;				/*!< True if image was computed */
    char *rawmonodata;				/*!< Monochrome data, as array of bytes */
    char *rawcolrdata;				/*!< Color data, as array of bytes */
    char *rawmaskdata;				/*!< Mask data, as array of bytes */
    int *rawpalette;				/*!< Color palette, in raw form */
    short numcolors;				/*!< Number of colors in raw palette */
    short r;						/*!< Solid color data as RGB values - red. */
    short g;						/*!< Solid color data as RGB values - green. */
    short b;						/*!< Solid color data as RGB values - blue.  */
    short bboxx;					/*<! Position of actual data within image - x. */
    short bboxy;					/*<! Position of actual data within image - y. */
    short bboxw;					/*<! Dimensions of actual data within image - width. */
    short bboxh;					/*<! Dimensions of actual data within image -height. */
    short hexgridx;                                     /*<! Hex grid width in cells. */
    short hexgridy;                                     /*<! Hex grid height in cells. */
    char *hook;					/*<! Pointer to interface-specific data */
    struct a_image **subimages;		/*<! Pointer to collection of subimages */
    struct a_image *next;			/*<! Pointer to next image in family */
} Image;

typedef struct a_image_family {
    char *name;					/* Name of the family */
    short ersatz;					/* True if this image is a substitute */
    struct a_image_file *location;  		/* File or whatever to look for data */
    Obj *notes;					/* designer notes about the image family */
    char *hook;					/* Pointer to interface-specific data */
    short numsizes;				/* Number of images in the list */
    Image *images;				/* Pointer to chain of images */
    struct a_image_family *next;		/* Pointer to next image family in a list */
} ImageFamily;

/* Image files are files that contain raw image data in a standard
   format, such as GIF or PNG.  We get actual usable images by
   extracting from the images in these files. */

typedef struct a_image_file {
    char *name;			/* Name of the file */
    short loaded;		/* True if it has already been loaded */
    struct a_image_file *next;	/* Link to the next file. */
} ImageFile;

/* Structure holding data about a file image, which is the raw result
   of loading from an image file.  A file image often holds an array
   of smaller images that will be extracted to make up image
   families. */

typedef struct a_file_image {
    char *name;
    short type;
    short loaded;
    short width, height;
    char *data;
    short numcolors;
    int *palette;
    short numtransparent;
    char *transparent;
    struct a_file_image *next;
} FileImage;

extern ImageFamily **images;
extern int numimages;
extern ImageFile *image_files;
extern FileImage *file_images;
extern ImageFamily *(*imf_load_hook)(ImageFamily *imf);
extern ImageFamily *(*imf_interp_hook)(ImageFamily *imf, Image *img,
				       int force);
extern int use_clip_mask;

/* Flag that indicates limited GDI memory in Windows ME and below. */
extern short poor_memory;

/* (should remove these fixed limits someday) */

#define MAXIMAGEFAMILIES 3000

/* Some handy macros. */

#define computed_rowbytes(w, pixelsize) (((w * pixelsize) + 7) / 8)

#define hextoi(c) (((c) >= '0' && (c) <= '9') ? ((c) - '0') : ((c) - 'a' + 10))

#define for_all_images(imf,img) \
  for ((img) = (imf)->images; (img) != NULL; (img) = (img)->next)

typedef void (*readimf_hook)(ImageFamily *, int);

extern ImageFamily *clone_imf(ImageFamily *imf);
extern ImageFamily *get_imf(char *name);
extern ImageFamily *find_imf(char *name);
extern Image *find_img(ImageFamily *imf, int w, int h);
extern Image *get_img(ImageFamily *imf, int w, int h);
extern int valid_imf_name(char *name);

extern ImageFile *get_image_file(char *name);
extern void load_image_families(FILE *fp, int loadnow, readimf_hook callback);
extern int load_imf_file(char *filename, readimf_hook callback);
extern void interp_imf_form(Obj *form, char *filename, readimf_hook callback);

extern ImageFamily *interp_imf(Obj *form);
extern void interp_imf_contents(ImageFamily *imf, Obj *form);
extern void interp_image(ImageFamily *imf, Obj *size, Obj *parts);
extern void interp_bytes(Obj *datalist, int numbytes, char *destaddr,
			 int jump);

extern Image *best_image_in_range(ImageFamily *imf, int w, int h,
				  int wmin, int hmin, int wmax, int hmax);
#define best_image(imf, w, h)  best_image_in_range(imf, w, h, 0, 0, -1, -1)
extern Image *smallest_image(ImageFamily *imf);
extern int emblem_position(Image *uimg, char *ename, ImageFamily *eimf,
			   int sw, int sh, int vpuh, int vphh,
			   int *exxp, int *eyyp, int *ewp, int *ehp);

extern void blacken_masked_area(ImageFamily *imf, Image *img,
				int r, int g, int b);
extern void blacken_mono_masked_area(ImageFamily *imf, Image *img,
				     int r, int g, int b);

extern void make_raw_palette(Image *img);

extern void sort_all_images(void);

extern void check_imf(ImageFamily *imf);

extern void write_imf(FILE *fp, ImageFamily *imf);

extern void make_generic_image_data(ImageFamily *imf);

extern void validify_imf_name(char *buf);
extern void compute_image_bboxes(ImageFamily *imf);
extern void compute_image_bbox(Image *img);
extern void write_imf_dir(char *filename, ImageFamily **imfimages, int num);
extern char *find_color_name(int r, int g, int b);
extern void parse_lisp_palette_entry(Obj *palentry, int *c,
				     int *r, int *g, int *b);

extern void make_image_from_file_image(ImageFamily *imf, 
				  Image *img, Image *subimg, int subi);
extern void load_file_image(FileImage *fimg);
extern void copy_from_file_image(Image *img, FileImage *fimg,
				 int xoffset, int yoffset,
				 int actualw, int actualh);
extern int get_gif(FileImage *fimg);
extern void make_raw_mono_data(Image *img, int force);
extern FileImage *get_file_image(char *fname);

extern ImageFamily *get_generic_images(char *name);
