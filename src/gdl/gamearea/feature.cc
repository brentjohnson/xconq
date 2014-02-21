// xConq
// Lifecycle management and ser/deser of game area features.

// $Id: feature.cc,v 1.2 2006/06/02 16:57:43 eric_mcdonald Exp $

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

#include "gdl/gamearea/area.h"

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Global Variables

Feature *featurelist = NULL;

// Local Variables

//! Tail of list of features.
static Feature *last_feature = NULL;
//! Count of features created.
static int numfeatures;

// Local Variables: Counters

//! ID of next feature to be created.
/*! Feature id 0 means no geographical feature defined. */
static int nextfid = 1;

// Queries

Feature *
find_feature(int fid)
{
    Feature *feature;

    /* Feature id of 0 is always a non-feature. */
    if (fid == 0)
      return NULL;
    for_all_features(feature) {
	if (feature->id == fid)
	  return feature;
    }
    return NULL;
}

// Lifecycle Management

Feature *
create_feature(char *feattype, char *name)
{
    Feature *newfeature = (Feature *) xmalloc(sizeof(Feature));

    /* If we're going to have a feature, we need to have the layer
       too.  init_features clears other data, which is why we do it at
       the beginning of this function. */
    if (!features_defined())
      init_features();
    newfeature->id = nextfid++;
    newfeature->feattype = feattype;
    newfeature->name = name;
    /* Add to the end of the feature list. */
    if (last_feature != NULL) {
    	last_feature->next = newfeature;
    } else {
    	featurelist = newfeature;
    }
    last_feature = newfeature;
    ++numfeatures;
    return newfeature;
}

// Game Setup

void
init_features(void)
{
    if (features_defined())
      return;
    featurelist = last_feature = NULL;
    numfeatures = 0;
    area.features = malloc_area_layer(short);
    /* No need to fill layer with default value, as long as feature 0
       means "no feature". */
}

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END
