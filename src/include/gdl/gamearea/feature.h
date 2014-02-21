// xConq
// Lifecycle management and ser/deser of game area features.

// $Id: feature.h,v 1.2 2006/06/02 16:58:34 eric_mcdonald Exp $

/*//////////////////////////// COPYRIGHT NOTICE ////////////////////////////////

  Copyright (C) 1987-1989   Stanley T. Shebs
  Copyright (C) 1991-2000   Stanley T. Shebs
  Copyright (C) 2006        Eric A. McDonald

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
    \brief Lifecycle management and ser/deser of game area features.
    \ingroup grp_gdl
*/

#ifndef XCONQ_GDL_GAMEAREA_FEATURE_H
#define XCONQ_GDL_GAMEAREA_FEATURE_H

#include "gdl/base.h"

// Iterator Macros

//! Iterate through features.
#define for_all_features(feat)  \
  for ((feat) = featurelist; (feat) != NULL; (feat) = (feat)->next)

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Data Structures

//! Feature.
/*! Named geographical features. */
typedef struct a_feature {
    //! Index of general type.
    int type;
    //! ID of this feature.
    short id;
    //! Name of this feature.
    char *name;
    //! Feature type.
    char *feattype;
    //! Next feature.
    struct a_feature *next;
    // Caches
    //! Size
    int size;
    //! X-coordinate.
    int x;
    //! Y-coordinate.
    int y;
    //! Minimum x-coordinate.
    short xmin;
    //! Minimum y-coordinate.
    short ymin;
    //! Maximum x-coordinate.
    short xmax;
    //! Maximum y-coordinate.
    short ymax;
    //! Minimum diameter.
    int mindiam;
    //! Maximum diameter.
    int maxdiam;
    //! ?
    int relabel;
} Feature;

// Global Variables

//! List of features.
extern Feature *featurelist;

// Queries

//! Get geographical feature with given ID.
extern Feature *find_feature(int fid);

// Lifecycle Management

//! Create new geographical feature.
extern Feature *create_feature(char *feattype, char *name);

// Game Setup

//! Initialize storage for geographical features.
extern void init_features(void);

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

#endif // ifndef XCONQ_GDL_GAMEAREA_FEATURE_H
