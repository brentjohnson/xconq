// xConq
// Lifecycle management and ser/deser of GDL images.

// $Id: img.h,v 1.3 2006/06/02 16:58:34 eric_mcdonald Exp $

/*//////////////////////////// COPYRIGHT NOTICE ////////////////////////////////

  Copyright (C) 1987-1989   Stanley T. Shebs
  Copyright (C) 1991-2000   Stanley T. Shebs
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
    \brief Lifecycle management and ser/deser of GDL images.
    \ingroup grp_gdl
*/

#ifndef XCONQ_GDL_MEDIA_IMG_H
#define XCONQ_GDL_MEDIA_IMG_H

#include "gdl/lisp.h"

// Function Macros

//! How many bytes per row in image data?
#define computed_rowbytes(w, pixelsize) (((w * pixelsize) + 7) / 8)

//! Convert hexadecimal digit to decimal number.
#define hextoi(c) (((c) >= '0' && (c) <= '9') ? ((c) - '0') : ((c) - 'a' + 10))

// Iterator Macros

//! Iterate over all images in GDL image family.
#define for_all_images(imf,img) \
  for ((img) = (imf)->images; (img) != NULL; (img) = (img)->next)

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Data Structures

//! GDL File Image
/*!
    Structure holding data about a file image, which is the raw result
    of loading from an image file.  A file image often holds an array
    of smaller images that will be extracted to make up image
    families.
*/
typedef struct a_file_image {
    //! Name of image from file.
    char *name;
    //! Type of image.
    short type;
    //! True, if image is loaded.
    short loaded;
    //! Width of image.
    short width;
    //! Height of image.
    short height;
    //! Raw image data.
    char *data;
    //! Number of colors in palette.
    short numcolors;
    //! Array of palette colors.
    int *palette;
    //! Number of transparent colors in palette.
    short numtransparent;
    //! Array of transparent color indices into palette.
    char *transparent;
    //! Next file image.
    struct a_file_image *next;
} FileImage;

//! GDL Image
typedef struct a_image {
    //! Nominal width of image.
    short w;
    //! Nominal height of image.
    short h;
    //! True, if image may be used as tile.
    short istile;
    //! True, if image is for cell terrain.
    short isterrain;
    //! True, if image is for connector terrain.
    short isconnection;
    //! True, if image is for border terrain.
    short isborder;
    //! True, if image is for terrain transition fuzz.
    short istransition;
    //! Number of subimages.
    short numsubimages;
    //! X-offset of subimage.
    short subx;
    //! Y-offset of subimage.
    short suby;
    //! Name of image embedded within image.
    char *embedname;
    //! X-position to draw embedded image.
    short embedx;
    //! Y-position to draw embedded image.
    short embedy;
    //! Width of embedded image.
    short embedw;
    //! Height of embedded image.
    short embedh;
    //! GDL monochrome data.
    Obj *monodata;
    //! GDL color data.
    Obj *colrdata;
    //! GDL mask data.
    Obj *maskdata;
    //! GDL file data.
    Obj *filedata;
    //! Image from file.
    struct a_file_image *file_image;
    //! Actual width of image.
    short actualw;
    //! Actual height of image.
    short actualh;
    //! Bits per pixel.
    short pixelsize;
    //! Original bits per pixel.
    short orig_pixelsize;
    //! GDL palette.
    Obj *palette;
    //! GDL notes.
    Obj *notes;
    //! True, if image was synthesized from another.
    short synthetic;
    //! Raw monochrome data.
    char *rawmonodata;
    //! Raw color data.
    char *rawcolrdata;
    //! Raw mask data.
    char *rawmaskdata;
    //! Raw palette.
    int *rawpalette;
    //! Number of colors in raw palette.
    short numcolors;
    //! Red value of solid color.
    short r;
    //! Green value of solid color.
    short g;
    //! Blue value of solid color.
    short b;
    //! X-position of image data bounding box.
    short bboxx;
    //! Y-position of image data bounding box.
    short bboxy;
    //! Width of image data bounding box.
    short bboxw;
    //! Height of image data bounding box.
    short bboxh;
    //! Width (in cells) of image "hexallation" grid.
    /*! \bug Should be named 'hexgridw'. */
    short hexgridx;
    //! Height (in cells) of image "hexallation" grid.
    /*! \bug Should be named 'hexgridh'. */
    short hexgridy;
    //! UI-specific data.
    char *hook;
    //! Array of subimages, if any.
    struct a_image **subimages;
    //! Next image in family.
    struct a_image *next;
} Image;

// Global Variables: Behavior Modifiers

//! Flag that indicates limited GDI memory in certain versions of Windows.
extern short poor_memory;

// Queries

//! Get or create image of given specs from given image family.
extern Image *get_img(struct a_image_family *imf, int w, int h);

// Image Preparation

//! Compute region of interest in GDL image data.
extern void compute_image_bbox(Image *img);

// GDL I/O

//! Read GDL image from GDL forms.
/*!
    Given an image family, a size, and a list describing the elements
    of a single image, parse the size and elements, put those into the
    right slots of an image object.  Also detect and warn about changes
    to an existing image, since this usually indicates some kind of
    problem.
*/
extern void interp_image(struct a_image_family *imf, Obj *size, Obj *parts);

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

#endif // ifndef XCONQ_GDL_MEDIA_IMG_H
