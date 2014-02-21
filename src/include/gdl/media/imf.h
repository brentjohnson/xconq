// xConq
// Lifecycle management and ser/deser for GDL image families.

// $Id: imf.h,v 1.3 2006/06/02 16:58:34 eric_mcdonald Exp $

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
    \brief Lifecycle management and ser/deser for GDL image families.
    \ingroup grp_gdl
*/

#ifndef XCONQ_GDL_MEDIA_IMF_H
#define XCONQ_GDL_MEDIA_IMF_H

#include "gdl/media/img.h"

//! Maximum number of image families to allocate.
/*! \todo Should remove these fixed limits. */
#define MAXIMAGEFAMILIES 3000

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Data Structures

//! GDL Image File
/*!
    Image files are files that contain raw image data in a standard
    format, such as GIF or PNG.  We get actual usable images by
    extracting from the images in these files.
*/
typedef struct a_image_file {
    //! Name of file.
    char *name;
    //! True, if file has already been loaded.
    short loaded;
    //! Next image file.
    struct a_image_file *next;
} ImageFile;

//! GDL Image Family
/*!
    An image family is like a Mac Finder icon family, but allows
    multiple kinds of images of an arbitrary set of sizes.  Individual
    images in a family may have both "lisp" form and "raw" form, the
    former being used for reading and writing, while the raw form is an
    intermediary for conversion to and from platform-specific
    representations.
*/
typedef struct a_image_family {
    //! Name of family.
    char *name;
    //! True, if family is a substitute.
    short ersatz;
    //! Image file.
    struct a_image_file *location;
    //! GDL designer notes on family.
    Obj *notes;
    //! UI-specific data.
    char *hook;
    //! Number of images in family.
    short numsizes;
    //! Head of family's image list.
    Image *images;
    //! Next image family.
    struct a_image_family *next;
} ImageFamily;

// Global Variables

//! Array of all image families loaded.
extern ImageFamily **images;
//! Number of image families loaded.
extern int numimages;

//! All GDL image families on record.
extern ImageFamily **recorded_imfs;
//! Number of GDL image families on record.
extern int num_recorded_imfs;

// Queries

//! Find image family, if it exists, from given name.
extern ImageFamily *find_imf(char *name);
//! Given name, find or create image family with that name.
extern ImageFamily *get_imf(char *name);

// Validation

//! Check if given name is valid for image family.
extern int valid_imf_name(char *name);

// Image Preparation

//! Compute regions of interest in image data.
extern void compute_image_bboxes(ImageFamily *imf);

// GDL I/O

//! Lookup color name from its RGB triplet.
/*!
    Given rgb components, return names of standard colors if the match
    is close.
*/
extern char *find_color_name(int r, int g, int b);

//! Read palette color from GDL palette entry.
extern void parse_lisp_palette_entry(
    Obj *palentry, int *c, int *r, int *g, int *b);

//! Read GDL form into GDL image family.
/*!
    Interpret the image family definition as a list of images and/or a
    notes property.
*/
extern void interp_imf_contents(ImageFamily *imf, Obj *form);

//! Read GDL image family from GDL form.
extern ImageFamily *interp_imf(Obj *form);

//! Serialize GDL image family to GDL.
extern void write_imf(FILE *fp, ImageFamily *imf);
//! Serialize images to GDL.
extern void write_images(void);

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

#endif // ifndef XCONQ_GDL_MEDIA_IMF_H
