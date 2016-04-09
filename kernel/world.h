/* Definitions relating to worlds and areas in Xconq.
   Copyright (C) 1987-1989, 1991-2000 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/*! \file kernel/world.h
 * \brief Dfinitions relating to worlds and areas in Xconq.
 */

/*! \brief World.
 *
 * This structure contains variables which affect the entire world map.
 */
typedef struct a_world {
    int circumference;      	/*!< number of cells going around the world */
    int daylength;          		/*!< number of turns in a day */
    int yearlength;         	/*!< number of turns in a year */
    int axial_tilt;         		/*!< controls extrema of seasons */
    int daylight_fraction;  	/*!< percentage of circumference that is lit */
    int twilight_fraction;  	/*!< percentage of circumference that is partly lit */
    int daylight_width;     	/*!< width of world that is lit */
    int twilight_width;     	/*!< width of world that is at least partly lit */
} World;

/*! \brief Minimum Area width.
 *
 * This is the smallest allowable width for a map.
 * \note Theoretically, there is no maximum size to Xconq areas, but the minimum
 * size is set by mystical properties, and is not negotiable. */
#define MINWIDTH 3

/*! \brief Minimum Area height.
 *
 * This is the smallest allowable height for a map.
 * \see MINWIDTH.
 */
#define MINHEIGHT 3

/*! \brief Area.
 *
 * An "area" is always basically a rectangular array of positions.  The
 * hex effect can be achieved by interpreting neighborliness and direction
 * differently, but that's all that's needed.  All of the "layers" are
 * dynamically allocated as needed, to save (considerable!) space. 
 *
 * See the discussion of \ref layers for a more complet discussion of the
 * various types of layers.
 */
typedef struct a_area {
    short width;            				/*!< Width of the area. */
    short height;           				/*!< Height of the area */
    short halfwidth;        			/*!< width / 2 */
    short halfheight;       			/*!< height / 2 */
    short maxdim;           			/*!< max of the two dims */
    short xwrap;            			/*!< true if x coords wrap around */
    int numcells;           				/*!< number of cells in area */
    int latitude;           				/*!< position within whole world */
    int longitude;					/*!< position within whole world */
    short projection;       			/*!< Isometric projection flag. ??? */
    int cellwidth;          				/*!< distance across one cell */
    short fullwidth;        			/*!< width of the area being used for data */
    short fullheight;       			/*!< height of the area being used for data */
    short fullx;            				/*!< x offset within full area to get data from */
    short fully;            				/*!< y offset within full area to get data from */
    short sunx;             				/*!< x location at which sun is overhead */
    short suny;             				/*!< y location at which sun is overhead */
    Obj *temp_year;       	  		/*!< cycle of temperatures within year */
    const char *image_name;			/*!< name of image to display for map */
    /*!< Pointers to the various "layers". */
    struct a_unit **units;  			/*!< pointer to units if any <I>layer</I>*/
    char *terrain;          			/*!< terrain type at this spot <I>layer</I>*/
    char **auxterrain;      			/*!< vector of extra types <I>layer</I>*/
    char *peopleside;       			/*!< the overt side alignment of the locals <I>layer</I>*/
    char *controlside;      			/*!< the side actually in charge <I>layer</I>*/
    short *features;        			/*!< layer of ids of features <I>layer</I>*/
    short *elevations;      			/*!< layer of elevations <I>layer</I>*/
    short avgelev;          				/*!< calculated average of elevations <I>layer</I>*/
    short minelev;          			/*!< calculated min/max of elevations <I>layer</I>*/
    short maxelev;          			/*!< calculated min/max of elevations <I>layer</I>*/
    long **materials;     	 		/*!< layer of materials in each cell <I>layer</I>*/
    short *temperature;   	  		/*!< layer of cell temperatures <I>layer</I>*/
    short *clouds;          			/*!< types of clouds in the layer <I>layer</I>*/
    short *cloudbottoms;    			/*!< altitudes of clouds of cloud layer <I>layer</I>*/
    short *cloudheights;    			/*!< heights of clouds in the cloud layer <I>layer</I>*/
    short *winds;           			/*!< layer of force/dir of winds. <I>layer</I>*/
    /* These layers are for temporary use in calculations. */
    short *tmp1;            			/*!< Temporary layer 1. <I>layer</I>*/
    short *tmp2;            			/*!< Temporary layer 2. <I>layer</I>*/
    short *tmp3;            			/*!< Temporary layer 3. <I>layer</I>*/
    struct a_region **terrain_regions;	/*!< Region list. Area is divided by terrain types. <I>layer</I>*/
    struct a_region **landsea_regions;	/*!< Region list. Area is devided into land and sea. <I>layer</I>*/
    short *user;            				/*!< IDs of units using materials in each cell <I>layer</I>*/
} Area;

/*! \brief Feature.
 *
 * Named geographical features. */
typedef struct a_feature {
    int type;               		/*!< index of the general type */
    short id;         		      	/*!< which one this is */
    const char *name;             	/*!< the name of the region */
    const char *feattype;         	/*!< its category, such as "island" or "bay"
    					*   something for syntax? "foo bay" vs "bay of foo" */
    struct a_feature *next;	/*!< arranged in a linked list */
    /* caches */
    int size;           		/*!< Cache: size */
    int x;              			/*!< Cache: x co-ordinate. */
    int y;              			/*!< Cache: y co-ordinate. */
    short xmin;         		/*!< Cache: Minimum x co-ordinate. */
    short ymin;         		/*!< Cache: Minimum y co-ordinate. */
    short xmax;         		/*!< Cache: Maximum x co-ordinate. */
    short ymax;         		/*!< Cache: Maximum y co-ordinate. */
    int mindiam;       		 /*!< Cache: Minimum diameter. */
    int maxdiam;        		/*!< Cache: Maximum diameter. */
    int relabel;        		/*!< Cace: re label. ??? */
} Feature;

/* Paths. */

/*! \brief Waypoint.
 *
 * Co-ordinate of point in path. ??? */
typedef struct a_waypoint {
    int x;      	/*!< x co-ordinate of point. */
    int y;      	/*!< y co-ordiante of point. */
    int note;		/*!< note. ??? */
} Waypoint;

/*! \brief Path.
 *
 * Array of waypoints defining path. 
 */
typedef struct a_path {
    int numwps;         		/*!< Number of points in path. */
    Waypoint wps[100];  	/*!< Array of 100 points in path. */
} Path;

/* Regions. */

/*! \brief Region. */
typedef struct a_region {
    int ttype;                          			/*!< Terrain type. */
    int size;                           			/*!< Size. */
    short xmin;                         		/*!< Minimum x co-ordinate. */
    short ymin;                         		/*!< Minimum y co-ordinate. */
    short xmax;                         		/*!< Maximum x co-ordinate. */
    short ymax;                        		/*!< Maximum y co-ordinate. */
    struct a_region *merge;             	/*!< Merge with region. ??? */
    struct a_region_link *neighbors;    	/*!< List of neighbor regions. */
    struct a_region *next;              		/*!< Link to next region. */
} TRegion;

/*! Terrain Region Link. */
typedef struct a_region_link {
    struct a_region *neighbor;          	/*! Neighbor. */
    struct a_region_link *next;         	/*! Next */
} TRegionLink;

/*! \brief Allocate layer.
 *
 * Use this macro to get a area-spanning layer of the given type.
 * This allocates enough memory to hold a value of TYPE for every
 * cell in the world.
 * \param TYPE is the type of the layer to allocate.
 * \return pointer to allocated area.
 */
#define malloc_area_layer(TYPE)  \
  ((TYPE *) xmalloc(area.width * area.height * sizeof(TYPE)))

/*! \brief Zero area.
 *
 * This sets an allocated layer to zero using memset by calculating
 * the number of cells in the area times the size of TYPE.
 * It uses the size of the global area to determine the area size.
 * \param ADDR is the address of the area to zero.
 * \param TYPE is the type of the layer to allocate.
 * \return the return value of memset.
 */
#define zero_area_layer(ADDR, TYPE)  \
  (memset(ADDR, 0, area.width * area.height * sizeof(TYPE)))

/* General 2D malloced area array usage.  Names from Lisp. */

/*! \brief Area refernce.
 *
 * Returns a value at a cell in the layer.
 * \param m is a memory pointer to the start of a layer.
 * \param x is the x co-ordinate of the cell.
 * \param y is the y co-ordinate of the cell.
 * \return the value of the cell.
 * \Note: aref now wraps x.
 */
#define aref(m,x,y) \
	(in_area((x),(y)) ? (m)[area.width * (y) + wrapx((x))] : 0)

/*! \brief Area set.
 * 
 * Set the value of a cell in a layer.
 * \param m is a memory pointer to the start of a layer.
 * \param x is the x co-ordinate of the cell.
 * \param y is the y co-ordinate of the cell.
 * \param v is the new value of the cell.
 * \return the new value of the cell.
 * \Note: aset now wraps x.
 */
#define aset(m,x,y,v) \
	(in_area((x),(y)) ? (m)[area.width * (y) + wrapx((x))] = (v) : 0)

/*! \brief Area add.
 * 
 * Increment the value of a cell in a layer.
 * \param m is a memory pointer to the start of a layer.
 * \param x is the x co-ordinate of the cell.
 * \param y is the y co-ordinate of the cell.
 * \param v is the value to add to the cell.
 * \return the new value of the cell.
 * \Note: aadd now wraps x.
 */
#define aadd(m,x,y,v) \
	(in_area((x),(y)) ? (m)[area.width * (y) + wrapx((x))] += (v) : 0)

/* Advanced unit support. */

/*! \brief No user id.
 *
 * This needs to not be the id of any unit. */
#define NOUSER (0)

/*! \brief Is user area defined?
 *
 * \return
 *    - TRUEE if the area.user is not NULL;
 *    - FALSE otherwize.
 */
#define user_defined() (area.user != NULL)

/*! \brief User at?
 *
 * Get the User id at cell.
 * \param x is the x co-ordinate of the cell.
 * \param y is the y co-ordinate of the cell.
 * \return the user id at cell(x,y).
 */
#define user_at(x, y) aref(area.user, x, y)

#define set_user_at(x, y, u) aset(area.user, x, y, u)

/*! \brief Number of cells covered by a radius.
 *
 * Calculate number of cells that could be covered by a radius sweeping 
 * from an origin. Obviously, one should not provide a radius length < 0.
 * \param dist is the length of the radius.
 * \return Number of cells covered by the radius.
 */
#define radius_covers_n_cells(dist) (1 + (((dist) + 1) * (dist) * 3))

/*! \brief Is cell in range?
 *
 * Test if cell at (x1, y1) is within n steps from (x, y)
 * \param x is starting x location.
 * \param y is starting y location.
 * \param n is the range.
 * \param x1 is the ending x location.
 * \param y1 is the ending y location.
 * \return TRUE if (x1,y1) is within n units of (x,y).
 */
#define cell_is_within_range(x, y, n, x1, y1) \
	(( (x) - (x1) > (n) || (x1) - (x) > (n) || (y) - (y1) > (n) \
	|| (y1) - (y) > (n) || (x) + (y) - (x1) - (y1) > (n) \
	|| (x1) + (y1) - (x) - (y) > (n) ) ? FALSE : TRUE)

/*! \brief Is cell in reach?
 *
 * Test if cell at (x1, y1) is within reach of \Unit.
 * \param u is a pointer to \Unit.
 * \param x1 is the ending x location.
 * \param y1 is the ending y location.
 * \return TRUE if the unit is in reach of (x1, y1).
 */
#define cell_is_within_reach(u, x1, y1) \
	cell_is_within_range((u)->x, (u)->y, (u)->reach, (x1), (y1))

/*! \brief Iterate over range.
 *
 * For header to repeat for all cells within range of starting cell.
 * \param x is the starting x location.
 * \param y is the starting y location.
 * \param n is the range.
 * \param x1 is the iteration x location of a cell in range.
 * \param y1 is the iteration y location of a cell in range.
 */
#define for_all_cells_within_range(x, y, n, x1, y1) \
	for((x1) = (x) - (n); (x1) <= (x) + (n); (x1)++) \
	for((y1) = max((y) - (n), (y) - (n) + (x) - (x1)); \
	       (y1) <= min((y) + (n), (y) + (n) + (x) - (x1)); (y1)++)

/*! \brief Iterate over all cells in range.
 *
 * Repeat for all cells withing reach of a unit.
 * \see for_all_cells_within_range.
 * \param u is a pointer to \Unit.
 * \param x1 is the iteration x location of a cell in range.
 * \param y1 is the iteration y location of a cell in range.
 */
#define for_all_cells_within_reach(u, x1, y1) \
	for_all_cells_within_range((u)->x, (u)->y, (u)->reach, (x1), (y1))

/*! \brief Unit at cell.
 *
 * The \Unit is a raw pointer - this macro is used a *lot*.  This could
 * be space-optimized by using a 16-bit unit id.  It uses the area.units global
 * layer.  Multiple units can be stacked by building a list using the 
 * \Unit->nexthere pointer.
 * \see aref.
 * \param x is the x location.
 * \param y is the y location.
 * \return pointer to \Unit at (x,y).
 */
#define unit_at(x,y) aref(area.units, x, y)

/*! \brief Set unit at cell.
 *
 * Set the pointer to the \Unit at cell(x,y).
 * \note This macro DOES NOT link the Unit to any previous Units at
 * the location, so it has to be done manually.
 * \param x is the x location.
 * \param y is the y location.
 * \param u is the \Unit pointer.
 */
#define set_unit_at(x,y,u) aset(area.units, x, y, u)

/*! \brief Iterate Unit stack.
 *
 * Iterate through all \Units in this cell (but not their occs).
 * \param x is the x location.
 * \param y is the y location.
 * \param var is the \Unit pointer.
 */
#define for_all_stack(x,y,var)  \
  for ((var) = unit_at((x), (y)); (var) != NULL;  (var) = (var)->nexthere)

/*! \brief Iterate Unit stack and occupants.
 *
 * Iterate through all units in this cell including occs within occs
 * within occs.   
 * This nifty little macro will climb the occupant : nexthere tree and
 * follow all branches four levels down to find all the occs within
 * occs within occs within units in the stack.
 * \param x is the x location.
 * \param y is the y location.
 * \param var is the \Unit pointer.
 */
#define for_all_stack_with_occs(x,y,var)  \
  for ((var) = unit_at((x), (y)); \
          (var) != NULL; \
  	 (var) = ((var)->occupant != NULL ? \
  	 	        (var)->occupant : \
  		      ((var)->nexthere != NULL ? \
  		        (var)->nexthere : \
  		      ((var)->transport != NULL && \
  		        (var)->transport->nexthere != NULL ? \
  		        (var)->transport->nexthere : \
  		      ((var)->transport != NULL && \
  		        (var)->transport->transport != NULL && \
  		        (var)->transport->transport->nexthere != NULL ? \
  		        (var)->transport->transport->nexthere : \
  		      ((var)->transport != NULL && \
  		        (var)->transport->transport != NULL && \
  		        (var)->transport->transport->transport != NULL && \
  		        (var)->transport->transport->transport->nexthere != NULL ? \
  		        (var)->transport->transport->transport->nexthere : NULL))))))

/*! \brief Is Terrain defined.
 *
 * Test if the terrain has been allocated yet.
 * \return 
 *   - TRUE if area.terrain is not NULL;
 *   - FALSE otherwise.
 */
#define terrain_defined() (area.terrain != NULL)

/*! \brief Terrain at. 
 *
 * The Terrain at each cell is just the number of the Terrain type.
 * \see aset.
 * \param x is the x location.
 * \param y is the y location.
 * \return the Terrain type at the cell.
 */
#define terrain_at(x,y) aref(area.terrain, x, y)

/*! \brief Set Terrain at.
 *
 * Set the Terrain type at cell(x,y);
 * \see aset.
 * \param x is the x location.
 * \param y is the y location.
 * \param t is the Terran type to which to set the cell.
 * \return the Terrain type at the cell.
 */
#define set_terrain_at(x,y,t) aset(area.terrain, x, y, t)

/* Auxiliary terrain array of layers. */

/*! \brief Is any auxillary terrain defined?
 *
 * Check to see if there is any auxillary terrain defined.
 * \return
 *   - TRUE if area.auxterrain is not NULL;
 *   - FALSE otherwise.
 */
#define any_aux_terrain_defined() (area.auxterrain != NULL)

/*! \brief Is kind of auxillary terrain defined?
 *
 * Check to see if a specific type of auxillary terrain is 
 * defined.
 * \see any_aux_terrain_defined.
 * \param at is the auxillary Terrain type.
 * \return 
 *   - TRUE if any_aux_terrain_defined() is TRUE and 
 *     area.auxterrain[t] is not NULL;
 *   - FALSE otherwise.
 */
#define aux_terrain_defined(at)  \
  (any_aux_terrain_defined() && area.auxterrain[at] != NULL)

/*! \brief Auxillary Terrain at.
 *
 * Get the auxillary terrain at a cell.
 * \see aref.
 * \param x is the x location.
 * \param y is the y location.
 * \param at is the auxillary Terrain type.
 * \return the auxillary Terrain type at cell(x,y).
 */
#define aux_terrain_at(x,y,at) aref(area.auxterrain[at], x, y)

/*! \brief Set auxillary Terrain at.
 *
 * Set the auxillary terrain at a cell.
 * \see aset.
 * \param x is the x location.
 * \param y is the y location.
 * \param at is the auxillary Terrain type.
 * \param v is the new value of the auxillary terrain.
 * \return the new value of the auxillary Terrain type at cell(x,y).
 */
#define set_aux_terrain_at(x,y,at,v) aset(area.auxterrain[at], x, y, v)

/*! \brief Are there borders at?
 *
 * Check for a border at a given cell.
 * \todo Not really correct, should finish.
 * \see aux_terrain_at.
 * \param x is the x location.
 * \param y is the y location.
 * \param b is the border Terrain type.
 * \return
 *   - TRUE if the border value at cell(x,y) is non-zero;
 *   - FALSE otherwise.
 */
#define any_borders_at(x,y,b) (aux_terrain_at(x, y, b) != 0)

/*! \brief Is Border in dirction at.
 *
 * See if there is a border in the specified direction at cell(x,y) for
 * the specified border type.
 * \see aux_terrain_at.
 * \param x is the x location.
 * \param y is the y location.
 * \param dir is the direction to check.
 * \param b is the border Terrain type.
 * \return 
 *   - TRUE if the cell(x,y) has a border in a particular direction;
 *   - FALSE otherwise.
 */
#define border_at(x,y,dir,b)  \
  (aux_terrain_defined(b) ? (aux_terrain_at(x, y, b) & (1 << (dir))) : FALSE)

/*! \brief Are there connections at?
 *
 * Check for a connections at a given cell.
 * \see aux_terrain_at.
 * \param x is the x location.
 * \param y is the y location.
 * \param c is the connections Terrain type.
 * \return 
 *   - TRUE if the connection value at cell(x,y) is non-zero;
 *   - FALSE otherwise.
 */
#define any_connections_at(x,y,c) (aux_terrain_at(x, y, c) != 0)

/*! \brief Is connection in dirction at.
 *
 * See if there is a connection in the specified direction at cell(x,y) for
 * the specified connection type.
 * \see aux_terrain_at.
 * \param x is the x location.
 * \param y is the y location.
 * \param dir is the direction to check.
 * \param c is the connection Terrain type.
 * \return 
 *   - TRUE if the cell(x,y) has a connection in a particular direction;
 *   - FALSE otherwise.
 */
#define connection_at(x,y,dir,c)  \
  (aux_terrain_defined(c) ? (aux_terrain_at(x, y, c) & (1 << (dir))) : FALSE)

/* Elevation layer. */

/*! \brief Are elevations defined?
 *
 * Check to see if the elevation layer has been defined.
 * \return
 *   - TRUE if area.elevations has been allocated;
 *   - FALSE if pointer is NULL.
 */
#define elevations_defined() (area.elevations != NULL)

/*! \brief Elevation at.
 *
 * Get the elevation at cell(x,y).  Elevations are stored in
 * area.elevations.
 * \see aref.
 * \param x is the x location.
 * \param y is the y location.
 * \return elevation at cell(x,y).
 */
#define elev_at(x,y) aref(area.elevations, x, y)

/*! \brief Set elevation at.
 *
 * Set the elevation at cell(x,y).  Elevations are stored in
 * area.elevations.
 * \see aref.
 * \param x is the x location.
 * \param y is the y location.
 * \param v is the new value for the elevation.
 * \return new elevation at cell(x,y).
 */
#define set_elev_at(x,y,v) aset(area.elevations, x, y, v)

/* Feature layer. */

/*! \brief Are features defined?
 *
 * Check to see if features are defined in this game.
 * \return
 *   - TRUE if area.features is not NULL;
 *   - FALSE otherwise.
 */
#define features_defined() (area.features != NULL)

/*! \brief Raw feature at.
 * Get the id fo the feature at cell(x,y).  
 * The "raw feature" is its "short" identifier.
 * \see aref.
 * \param x is the x location.
 * \param y is the y location.
 * \return the short feature id.
 */
#define raw_feature_at(x,y) aref(area.features, x, y)

/*! \brief Set raw feature at.
 * Set the id fo the feature at cell(x,y).  
 * The "raw feature" is its "short" identifier.
 * \see aref.
 * \param x is the x location.
 * \param y is the y location.
 * \param f is the feature id.
 * \return the short feature id.
 */
#define set_raw_feature_at(x,y,f) aset(area.features, x, y, f)

/* Population layer. */

/*! \brief Population defined?
 *
 * Check to see if people are assigned to sides.
 * \return
 *   - TRUE if area.peopleside is not NULL;
 *   - FALSE otherwise.
 */
#define people_sides_defined() (area.peopleside != NULL)

/*! \brief Side of people at.
 *
 * Get the Side number of the people at cell(x,y).
 * \param x is the x location.
 * \param y is the y location.
 * \return Side number of people at cell(x,y).
 */
#define people_side_at(x,y) aref(area.peopleside, x, y)

/*! \brief Set Side of people at.
 *
 * Set the Side number of the people at cell(x,y).
 * \param x is the x location.
 * \param y is the y location.
 * \param s is the new Side number.
 * \return new Side number of people at cell(x,y).
 */
#define set_people_side_at(x,y,s) aset(area.peopleside, x, y, s)

/*! \brief Nobody.
 *
 * A cell might be entirely uninhabited, so need an extra value to indicate.
 * This value is chosen to be well above any possible MAXSIDES, and encodes
 * in layers as 'X', which is convenient.
 * \note It might be better to define this in terms of \ref MAXSIDES, to 
 * make sure there is no conflict.
 */
#define NOBODY (60)

/*! \brief Is populated?
 *
 * Check to see if the cell(x,y) is populated.
 * \param x is the x location.
 * \param y is the y location.
 * \return
 *   - TRUE if Side number is not \ref NOBODY;
 *   _ FALSE otherwise.
 */
#define populated(x,y) (people_side_at(x,y) != NOBODY)

/* Control layer. */

/*! \brief Is Area control defined?
 *
 * Check to see if the control layer is defined.
 * \return 
 *   - TRUE if araa.controlside is not NULL;
 *   - FALSE otherwise.
 */
#define control_sides_defined() (area.controlside != NULL)

/*! \brief Controled by at.
 *
 * Retrieve the side that controls cell(x,y).
 * \param x is the x location.
 * \param y is the y location.
 * \return 
 *   - Side id if controlled by a side.
 *   - NOCONTROL if not controlled by a side.
 */
#define control_side_at(x,y) aref(area.controlside, x, y)

/*! \brief Set controled by at.
 *
 * Set the side that controls cell(x,y).
 * \param x is the x location.
 * \param y is the y location.
 * \param s is the new Side id that controls the cell.
 * \return 
 *   - Side id if controlled by a side.
 *   - NOCONTROL if not controlled by a side.
 */
#define set_control_side_at(x,y,s) aset(area.controlside, x, y, s)

/*! A cell might be entirely uncontrolled, so need an extra value to indicate.
 *  This value is chosen to be well above any possible MAXSIDES, and encodes
 *  in layers as 'X', which is convenient.
 *
 *  \note It might be better to define this in terms of \ref MAXSIDES, to 
 *  make sure there is no conflict.
 */
#define NOCONTROL (60)

/*! \brief Is Controlled?
 *
 * Check to see if the cell(x,y) is controlled by a side.
 * \see control_side_at.
 * \param x is the x location.
 * \param y is the y location.
 * \return
 *   - TRUE if Side number is not \ref NOCONTROL;
 *   _ FALSE otherwise.
 */
#define controlled(x,y) (control_side_at(x,y) != NOCONTROL)

/* Array of material layers. */

/*! \brief Are cell Materials defined?
 *
 * Check to see if materials in cells are used.
 * \return
 *   - TRUE if area.materials is not NULL;
 *   - FALSE otherwise.
 */
#define any_cell_materials_defined() (area.materials != NULL)

/*! \brief Is this material defined in cells?
 *
 * Check to see if a specific material is defined in cells.
 * \param m is the material type.
 * \return 
 *   - TRUE if the area.materials[m] is not NULL;
 *   - FALSE otherwise.
 */
#define cell_material_defined(m) (area.materials[m] != NULL)

/*! \brief Material at cell.
 *
 * Get the amount of a specific material in cexll(x,y).
 * \see aref.
 * \param x is the x location.
 * \param y is the y location.
 * \param m is the Material type.
 * \return the amount of the material at cell(x,y).
 */
#define material_at(x,y,m) aref(area.materials[m], x, y)

/*! \brief Set Material at cell.
 *
 * Set the amount of a specific material in cexll(x,y).
 * \see aref.
 * \param x is the x location.
 * \param y is the y location.
 * \param m is the Material type.
 * \param v is the new value of the amount of Material at cell(x.y)
 * \return the amount of the material at cell(x,y).
 */
#define set_material_at(x,y,m,v) aset(area.materials[m], x, y, v)

/* Temperature layer. */

/*! \brief Is temperature defined?
 *
 * Check to see if temperature is defined.
 * \return
 *   - TRUE if area.temperature is not NULL;
 *   - FALSE otherwise.
 */
#define temperatures_defined() (area.temperature != NULL)

/*! \brief Temperature at cell.
 *
 * Get temperature at cell(x,y).
 * \see aref.
 * \param x is the x location.
 * \param y is the y location.
 * \return the temperature at cell(x,y).
 */
#define temperature_at(x,y) aref(area.temperature, x, y)

/*! \brief Set temperature at cell.
 *
 * Set temperature at cell(x,y).
 * \see aset.
 * \param x is the x location.
 * \param y is the y location.
 * \param v is the new temperature.
 * \return the temperature at cell(x,y).
 */
#define set_temperature_at(x,y,v) aset(area.temperature, x, y, v)

/* Clouds layer. */

/*! \brief Are clouds defined?
 *
 * Check to see if clouds are used in game.
 * \return
 *   - TRUE if area.clouds is not NULL;
 *   - FALSE otherwise.
 */
#define clouds_defined() (area.clouds != NULL)

/*! \brief Clouds at cell.
 *
 * Get clouds at cell(x,y).
 * \see aref.
 * \param x is the x location.
 * \param y is the y location.
 * \return the clouds at cell(x,y).
 */
#define raw_cloud_at(x,y) aref(area.clouds, x, y)

/*! \brief Set clouds at cell.
 *
 * set clouds at cell(x,y).
 * \see aset.
 * \param x is the x location.
 * \param y is the y location.
 * \param v is the new value of clouds.
 * \return the clouds at cell(x,y).
 */
#define set_raw_cloud_at(x,y,v) aset(area.clouds, x, y, v)

/*! \brief Are cloud bottoms defined?
 *
 * Check to see if cloud bottoms are used in game.
 * \return
 *   - TRUE if area.cloud bottoms is not NULL;
 *   - FALSE otherwise.
 */
#define cloud_bottoms_defined() (area.cloudbottoms != NULL)

/*! \brief Cloud bottom at cell.
 *
 * Get cloud bottom at cell(x,y).
 * \see aref.
 * \param x is the x location.
 * \param y is the y location.
 * \return the cloud bottom at cell(x,y).
 */
#define raw_cloud_bottom_at(x,y) aref(area.cloudbottoms, x, y)

/*! \brief Set cloud bottoms at cell.
 *
 * set cloud bottom at cell(x,y).
 * \see aset.
 * \param x is the x location.
 * \param y is the y location.
 * \param v is the new value of the cloud bottom.
 * \return the cloud bottom at cell(x,y).
 */
#define set_raw_cloud_bottom_at(x,y,v) aset(area.cloudbottoms, x, y, v)

/*! \brief Are cloud tops defined?
 *
 * Check to see if cloud tops are used in game.
 * \return
 *   - TRUE if area.cloud tops is not NULL;
 *   - FALSE otherwise.
 */
#define cloud_heights_defined() (area.cloudheights != NULL)

/*! \brief Cloud top at cell.
 *
 * Get cloud top at cell(x,y).
 * \see aref.
 * \param x is the x location.
 * \param y is the y location.
 * \return the cloud top at cell(x,y).
 */
#define raw_cloud_height_at(x,y) aref(area.cloudheights, x, y)

/*! \brief Set cloud top at cell.
 *
 * set cloud top at cell(x,y).
 * \see aset.
 * \param x is the x location.
 * \param y is the y location.
 * \param v is the new value of the cloud top.
 * \return the cloud top at cell(x,y).
 */
#define set_raw_cloud_height_at(x,y,v) aset(area.cloudheights, x, y, v)

/* Winds layer. */

/*! \brief Are winds defined?
 *
 * Check to see if winds are used in game.
 * \return
 *   - TRUE if area.winds is not NULL;
 *   - FALSE otherwise.
 */
#define winds_defined() (area.winds != NULL)

/*! \brief Raw wind at cell.
 *
 * Get raw wind value at cell(x,y).  The raw wind value is
 * a combination of direction and force.  The direction is
 * in the bottom 3 bits of the cell value.
 * \see aref.
 * \param x is the x location.
 * \param y is the y location.
 * \return the raw wind at cell(x,y).
 */
#define raw_wind_at(x,y) aref(area.winds, x, y)

/*! \brief Set raw wind at cell.
 *
 * Set raw wind value at cell(x,y).  The raw wind value is
 * a combination of direction and force.  The direction is
 * in the bottom 3 bits of the cell value.
 * \see aset.
 * \param x is the x location.
 * \param y is the y location.
 * \param v is the new raw wind value.
 * \return the raw wind at cell(x,y).
 */
#define set_raw_wind_at(x,y,v) aset(area.winds, x, y, v)

/*! \brief Wind direction.
 *
 * Extract the wind direction from the raw wind value.
 * \param w is the raw wind value.
 * \return the wind direction.
 */
#define wind_dir(w) ((w) & 0x07)

/*! \brief Wind froce.
 *
 * Extract the wind force from the raw wind value.
 * \param w is the raw wind value.
 * \return the wind force.
 */
#define wind_force(w) ((w) >> 3)

/*! \brief Wind direction at.
 *
 * Get the wind direction at cell(x,y).
 * \see wind_dir, raw_wind_at.
 * \param x is the x location.
 * \param y is the y location.
 * \return the wind direction at cell(x,y).
 */
#define wind_dir_at(x,y) (wind_dir(raw_wind_at(x, y)))

/*! \brief Wind force at.
 *
 * Get the wind force at cell(x,y).
 * \see wind_force, raw_wind_at.
 * \param x is the x location.
 * \param y is the y location.
 * \return the wind force at cell(x,y).
 */
#define wind_force_at(x,y) (wind_force(raw_wind_at(x, y)))

/*! \brief Set wind at cell.
 *
 * Set wind value at cell(x,y).
 * \see set_raw_wind_at.
 * \param x is the x location.
 * \param y is the y location.
 * \param d is the new wind direction value.
 * \param f is the new wind force value.
 * \return the raw wind at cell(x,y).
 */
#define set_wind_at(x,y,d,f) (set_raw_wind_at(x, y, ((f) << 3) | (d)))

/*! \brief Calm wind force. */
#define CALM (0)

/* Handlers for scratch layers. */

/*! \brief Get scratch layer 1.
 *
 * Get the value of scratch layer 1 at cell(x,y).
 * \param x is the x location.
 * \param y is the y location.
 * \return the value of scratch layer 1 at cell(x,y).
 */
#define tmp1_at(x,y) aref(area.tmp1, x, y)

/*! \brief Set scratch layer 1.
 *
 * Set the value of scratch layer 1 at cell(x,y).
 * \param x is the x location.
 * \param y is the y location.
 * \param v is the new value.
 * \return the value of scratch layer 1 at cell(x,y).
 */
#define set_tmp1_at(x,y,v) aset(area.tmp1, x, y, v)

/*! \brief Get scratch layer 2.
 *
 * Get the value of scratch layer 2 at cell(x,y).
 * \param x is the x location.
 * \param y is the y location.
 * \return the value of scratch layer 2 at cell(x,y).
 */
#define tmp2_at(x,y) aref(area.tmp2, x, y)

/*! \brief Set scratch layer 2.
 *
 * Set the value of scratch layer 2 at cell(x,y).
 * \param x is the x location.
 * \param y is the y location.
 * \param v is the new value.
 * \return the value of scratch layer 2 at cell(x,y).
 */
#define set_tmp2_at(x,y,v) aset(area.tmp2, x, y, v)

/*! \brief Get scratch layer 3.
 *
 * Get the value of scratch layer 3 at cell(x,y).
 * \param x is the x location.
 * \param y is the y location.
 * \return the value of scratch layer 3 at cell(x,y).
 */
#define tmp3_at(x,y) aref(area.tmp3, x, y)

/*! \brief Set scratch layer 3.
 *
 * Set the value of scratch layer 3 at cell(x,y).
 * \param x is the x location.
 * \param y is the y location.
 * \param v is the new value.
 * \return the value of scratch layer 3 at cell(x,y).
 */
#define set_tmp3_at(x,y,v) aset(area.tmp3, x, y, v)

/*! \brief Wrap x.
 *
 * This little macro implements wraparound in the x direction.
 * \note The stupid add of shifted width is for the benefit of brain-damaged
 * mod operators that don't handle negative numbers properly.
 * \param x is the unwrapped x value.
 * \return the value within the world bounds.
 */
#define wrapx(x) (area.xwrap ? (((x) + (area.width << 8)) % area.width) : (x))

/*! \brief Constrain y co-ordinate.
 *
 * Constrain y to northern and southern edges.
 * \see max, min.
 * \param y is raw y co-ordiante.
 * \return y co-ordinate withing world boundary.
 */
#define limity(y) (max(0, min((y), (area.height-1))))

/*! \brief Constrain y to inerior of world.
 *
 * Constrain y co-ordinate to be within world.
 * \see max, min.
 * \param y is raw y co-ordiante.
 * \return y co-ordinate in interior of world.
 */
#define interior(y) (max(1, min((y), (area.height-2))))

/*! \brief Coordinate in direction.
 *
 * Calculate the co-ordinates of moving a distance of one in a 
 * direction, contraining the x co-ordiante to be wrapped.
 * \note The y co-odinate is not limited or constrained.
 * \see wrapx..
 * \param x is the current x co-ordinate.
 * \param y is the current y co-ordinate.
 * \param d is the direction to move.
 * \param nx is the new x co-ordinate.
 * \param ny is the new y co-ordiante.
 * \return the new y co-ordiante.
 */
#define xy_in_dir(x,y,dir,nx,ny) \
  (nx) = wrapx((x) + dirx[dir]);  (ny) = (y) + diry[dir];

/*! \brief Is in area?
 *
 * Test whether x,y is a valid position anywhere in the current area.
 * \note in_area and inside_area are very heavily used; any 
 * optimization will likely speed up Xconq overall.
 * \see between, x_in_area.
 * \param x is the x co-ordinate.
 * \param y is the y co-ordiante.
 * \return
 *    - TRUE if cell(x,y) is in area;
 *    - FALSE otherwise.
 */
#define in_area(x, y)  \
  (between(0, (y), area.height-1) && x_in_area(x, y))

/*! \brief Is x in area?
 *
 * Test whether x is a inside the current area.
 * \note in_area and inside_area are very heavily used; any 
 * optimization will likely speed up Xconq overall.
 * \see between.
 * \param x is the x co-ordinate.
 * \param y is the y co-ordiante.
 * \return
 *    - TRUE if cell(x,y) is in area;
 *    - FALSE otherwise.
 */
#define x_in_area(x, y)  \
  (area.xwrap ? TRUE : (between(0, (x), area.width-1) &&  \
			between(area.halfheight,  \
				(x)+(y),  \
				area.width+area.halfheight-1)))

/*! \brief Is valid unit position?
 *
 * Test whether x,y is a valid position anywhere in the current area.
 * \note in_area and inside_area are very heavily used; any 
 * optimization will likely speed up Xconq overall.
 * \bug Does x testing work right for even/odd heights?
 * \see between, x_inside_area.
 * \param x is the x co-ordinate.
 * \param y is the y co-ordiante.
 * \return
 *    - TRUE if cell(x,y) is inside area;
 *    - FALSE otherwise.
 */
#define inside_area(x, y)  \
  (between(1, (y), area.height-2) && x_inside_area(x, y))

/*! \brief Is x a valid unit postion?
 *
 * Test whether x is a inside the current area.
 * \note in_area and inside_area are very heavily used; any 
 * optimization will likely speed up Xconq overall.
 * \bug Does x testing work right for even/odd heights?
 * \see between.
 * \param x is the x co-ordinate.
 * \param y is the y co-ordiante.
 * \return
 *    - TRUE if cell(x,y) is in area;
 *    - FALSE otherwise.
 */
#define x_inside_area(x, y)  \
  (area.xwrap ? TRUE : (between(1, (x), area.width-2) &&  \
			between(area.halfheight+1,  \
				(x)+(y),  \
				area.width+area.halfheight-2)))

/*! \brief Iterate over all cells.
 *
 * Iteration over all valid cell positions in a area.  These should be
 * used carefully, since they don't (can't) have any grouping braces
 * embedded. 
 * \see x_in_area.
 * \param x is the x co-ordinate.
 * \param y is the y co-ordiante.
 */
#define for_all_cells(x,y)  \
  for (x = 0; x < area.width; ++x)  \
    for (y = 0; y < area.height; ++y)  \
      if (x_in_area(x, y))

/* This doesn't generate positions along area edges.  Typically more
   useful within game. */
/*! \brief Iterate over all interior cells.
 *
 * This doesn't generate positions along area edges.  Typically more
 * useful within game than for_all_cells().
 * Iteration over all valid interior cell positions in a area.  These should be
 * used carefully, since they don't (can't) have any grouping braces
 * embedded. 
 * \see x_inside_area.
 * \param x is the x co-ordinate.
 * \param y is the y co-ordiante.
 */
#define for_all_interior_cells(x,y)  \
  for (x = 0; x < area.width; ++x)  \
    for (y = 1; y < area.height - 1; ++y)  \
      if (x_inside_area(x, y))

/*! \brief Iterate through features.
 *
 * Walk thrhoug all defined features.
 * \param feat is the feature iteration variable.
 */
#define for_all_features(feat)  \
  for ((feat) = featurelist; (feat) != NULL; (feat) = (feat)->next)

/*! \brief Lighting at cell.
 *
 * Returns the lighting state of a given position.
 * \todo (should opencode distance call here)
 * \param x is the cell's x co-ordinate.
 * \param y is the cell's y co-ordinate.
 * \param snx is the sun's x co-ordinate.
 * \param sny is the sun's y co-ordiante.
 * \return
 *   - two if in sunlight;
 *   - one if in twilight; or
 *   - zero if in darkness.
 */
#define lighting(x,y,snx,sny)  \
  ((world_distance(x, y, snx, sny) < world.daylight_width) ? 2 : \
   ((world_distance(x, y, snx, sny) < world.twilight_width) ? 1 : 0))

/*! \brief Night at cell?
 *
 * Is it night at cell(x,y)
 * \see lighting, sunx, suny.
 * \param x is the cell's x co-ordinate.
 * \param y is the cell's y co-ordinate.
 * \return 
 *   - TRUE if lighting is 0 at cell;
 *   - FALSE otherwise.
 */
#define night_at(x,y)  \
  (daynight && lighting((x), (y), (int) sunx, (int) suny) == 0)

/* World-related variables. */

/*! World global. */
extern World world;

/*! Area global. */
extern Area area;

/*! Mid turn restore flag. ???*/
extern int midturnrestore;

/*! Number of features. */
extern int numfeatures;

/*! Minimum elevation. */
extern int minelev;
/*! Maximum elevation. */
extern int maxelev;
/*! Minimum termperature. */
extern int mintemp;
/*! Maximum termperature. */
extern int maxtemp;
/*! Minimum wind forces. */
extern int minwindforce;
/*! Maximum windforce. */
extern int maxwindforce;

/*! Any Materials in terrain? ??? */
extern int any_materials_in_terrain;
/*! Any elevation variation? ??? */
extern int any_elev_variation;
/*! Any temperture variation? ??? */
extern int any_temp_variation;
/*! Any temperature variation in layer? ??? */
extern int any_temp_variation_in_layer;
/*! Any wind variation? ??? */
extern int any_wind_variation;
/*! Any wind variation in layer? ??? */
extern int any_wind_variation_in_layer;
/*! Any clouds? ??? */
extern int any_clouds;

/* World-related functions. */

extern void init_world(void);
extern int set_world_circumference(int circum, int warn);
extern int set_area_shape(int width, int height, int warn);
extern int valid_area_shape(int width, int height, int warn);
extern void check_area_shape(void);
extern void calculate_world_globals(void);
extern void final_init_world(void);
extern void compute_elevation_bounds(void);

extern void allocate_area_terrain(void);
extern void allocate_area_aux_terrain(int t);
extern void allocate_area_scratch(int n);
extern void allocate_area_elevations(void);
extern void allocate_area_temperatures(void);
extern void allocate_area_people_sides(void);
extern void allocate_area_control_sides(void);
extern void allocate_area_material(int m);
extern void allocate_area_clouds(void);
extern void allocate_area_cloud_altitudes(void);
extern void allocate_area_cloud_bottoms(void);
extern void allocate_area_cloud_heights(void);
extern void allocate_area_winds(void);

extern int fn_terrain_at(int x, int y);
extern int fn_aux_terrain_at(int x, int y);
extern int fn_feature_at(int x, int y);
extern int fn_elevation_at(int x, int y);
extern int fn_people_side_at(int x, int y);
extern int fn_control_side_at(int x, int y);
extern int fn_material_at(int x, int y);
extern int fn_temperature_at(int x, int y);
extern int fn_raw_cloud_at(int x, int y);
extern int fn_raw_cloud_bottom_at(int x, int y);
extern int fn_raw_cloud_height_at(int x, int y);
extern int fn_raw_wind_at(int x, int y);
extern void allocate_area_users(void);
extern int fn_user_at(int x, int y);
extern void fn_set_user_at(int x, int y, int val);
extern void fn_set_terrain_at(int x, int y, int val);
extern void fn_set_aux_terrain_at(int x, int y, int val);
extern void fn_set_people_side_at(int x, int y, int val);
extern void fn_set_control_side_at(int x, int y, int val);
extern void fn_set_raw_feature_at(int x, int y, int val);
extern void fn_set_elevation_at(int x, int y, int val);
extern void fn_set_material_at(int x, int y, int val);
extern void fn_set_temperature_at(int x, int y, int val);
extern void fn_set_raw_wind_at(int x, int y, int val);
extern void fn_set_raw_cloud_at(int x, int y, int val);
extern void fn_set_raw_cloud_bottom_at(int x, int y, int val);
extern void fn_set_raw_cloud_height_at(int x, int y, int val);

extern void change_terrain_type(int x, int y, int t2);

extern int limited_search_around(int x0, int y0, int range, 
				 int (*pred)(int, int, int *, ParamBox *),
				 int incr, int *counter, 
				 int rsltlimit, ParamBox *parambox);
extern int search_around(int x0, int y0, int range, 
			 int (*pred)(int, int, int *, ParamBox *),
			 int incr, ParamBox *parambox);
extern int search_around(int x0, int y0, int maxdist, int (*pred)(int, int),
			 int *rxp, int *ryp, int incr);
extern int limited_search_around(int x0, int y0, int maxdist, 
                                 int (*pred)(int, int, int *), int *rxp, 
                                 int *ryp, int incr, int rsltlimit);
extern int search_and_apply(int x0, int y0, int maxdist, int (*pred)(int, int),
			    int *rxp, int *ryp, int incr,
			    void (*fn)(int, int), int num);
extern void apply_to_area(int x0, int y0, int dist, void (*fn)(int, int));
extern void apply_to_area_plus_edge(int x0, int y0, int dist,
				    void (*fn)(int, int));
extern void apply_to_ring(int x0, int y0, int distmin, int distmax,
			  void (*fn)(int, int));
extern void apply_to_hexagon(int x0, int y0, int w2, int h2,
			     void (*fn)(int, int));
extern int limited_search_under_arc(int x, int y, int dir, int range, 
                                    int dirbias,
                                    int (*pred)(int x, int y, int *counter,
                                                ParamBox *parambox),
                                    int *counter, int rsltlimit, 
                                    ParamBox *parambox);
extern int search_under_arc(int x, int y, int dir, int range, int dirbias, 
                            int (*pred)(int x, int y, int *counter, 
                                        ParamBox *parambox),
                            ParamBox *parambox);
extern void apply_to_path(int fx, int fy, int tx, int ty,
			  int (*dirtest)(int x, int y, int dir),
			  int (*dirsort)(int x, int y, int *dirchoices,
					 int numchoices),
			  int (*fn)(int x, int y, int dir, int j,
				    int numchoices),
			  int shortest);
#if 0
int find_path(int fx, int fy, int tx, int ty,
	      int (*chooser)(int, int, int, int, int *),
	      int maxwps, Waypoint *waypoints, int *numwpsp);
#endif
extern int search_straight_line(int x0, int y0, int x1, int y1,
				int (*pred)(int, int), int *rxp, int *ryp);

extern void set_border_at(int x, int y, int dir, int t, int onoff);
extern void set_connection_at(int x, int y, int dir, int t, int onoff);
extern void patch_linear_terrain(int t);

extern void init_features(void);
extern Feature *find_feature(int fid);
extern Feature *feature_at(int x, int y);
extern void compute_all_feature_centroids(void);
extern void compute_feature_centroid(Feature *feature);

extern int point_in_dir(int x, int y, int dir, int *xp, int *yp);
extern int interior_point_in_dir(int x, int y, int dir, int *xp, int *yp);
extern int point_in_dir_n(int x, int y, int dir, int n, int *xp, int *yp);
extern int interior_point_in_dir_n(int x, int y, int dir, int n,
				   int *xp, int *yp);
extern int random_point(int *xp, int *yp);
extern int random_edge_point(int *xp, int *yp);
extern int random_point_near(int cx, int cy, int radius, int *xp, int *yp);
extern int random_point_in_area(int cx, int cy, int rx, int ry,
				int *xp, int *yp);
extern void terrain_subtype_warning(char *context, int t);
extern int approx_dir(int dx, int dy);
extern int hextant(int dx, int dy);
extern int distance(int x1, int y1, int x2, int y2);
extern int world_distance(int x1, int y1, int x2, int y2);
extern int closest_dir(int x, int y);

extern void xy_to_latlong(int x, int y, int xf, int yf, int *latp, int *lonp);
extern int latlong_to_xy(int lat, int lon, int *xp, int *yp,
			 int *xfp, int *yfp);
extern int num_people_at(int x, int y);
