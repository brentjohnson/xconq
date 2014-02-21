// xConq
// Lifecycle management and ser/deser of game area layers.

// $Id: layers.cc,v 1.2 2006/06/02 16:57:43 eric_mcdonald Exp $

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
    \brief Lifecycle management and ser/deser of game area layers.
    \ingroup grp_gdl
*/

#include "gdl/types.h"
#include "gdl/dir.h"
#include "gdl/unit/unit.h"
#include "gdl/side/side.h"
#include "gdl/gamearea/area.h"
#include "gdl/world.h"
#include "gdl/module.h"

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Local Variables

//! True, if bad terrain warnings should be issued.
static int warnbadterrain = TRUE;
//! The count of cells that did not have valid terrain data.
static int numbadterrain = 0;

static short layer_use_default;
static int layer_default;
static int layer_multiplier;
static int layer_adder;
static short layer_area_x, layer_area_y;
static short layer_area_w, layer_area_h;

static int ignore_specials;

// Local Function Declarations: GDL I/O

//! Read and decode textual run-length encodings.
/*! This basically parses the run lengths
    and calls the function that records what was read. */
static void read_rle(
    Obj *contents, void (*setter)(int, int, int), short *chartable);

//! Translate coordinate of reshaped area to one in original area.
static int original_point(int x1, int y1, int *x2p, int *y2p);

//! Elevation at position minus minimum elevation.
static int fn_elevation_at_offset(int x, int y);

//! Write run of run-length encoding.
/*!
    Write a single run, using the most compact encoding possible.
    0 - 29 is 'a' - '~', 30 - 63 is ':' - '['
*/
static void write_run(int run, int val);

// Game Area Layer Accessors

int
fn_terrain_at(int x, int y)
{
    return terrain_at(x, y);
}

int
fn_aux_terrain_at(int x, int y)
{
    return aux_terrain_at(x, y, tmpttype);
}

int
fn_feature_at(int x, int y)
{
    return raw_feature_at(x, y);
}

int
fn_elevation_at(int x, int y)
{
    return elev_at(x, y);
}

int
fn_people_side_at(int x, int y)
{
    return people_side_at(x, y);
}

int
fn_control_side_at(int x, int y)
{
    return control_side_at(x, y);
}

int
fn_material_at(int x, int y)
{
    return material_at(x, y, tmpmtype);
}

int
fn_temperature_at(int x, int y)
{
    return temperature_at(x, y);
}

int
fn_raw_cloud_at(int x, int y)
{
    return raw_cloud_at(x, y);
}

int
fn_raw_cloud_bottom_at(int x, int y)
{
    return raw_cloud_bottom_at(x, y);
}

int
fn_raw_cloud_height_at(int x, int y)
{
    return raw_cloud_height_at(x, y);
}

int
fn_raw_wind_at(int x, int y)
{
    return raw_wind_at(x, y);
}

int
fn_user_at(int x, int y)
{
    return user_at(x, y);
}

void
fn_set_terrain_at(int x, int y, int val)
{
    /* It's important not to put bad values into the terrain layer. */
    if (!is_terrain_type(val)) {
    	/* Only warn the first few times, just count thereafter. */
    	if (warnbadterrain && numbadterrain < 10) {
	    read_warning("Unknown terrain type (%d) at %d,%d; substituting %s",
			 val, x, y, t_type_name(0));
	}
	val = 0;
	++numbadterrain;
    }
    set_terrain_at(x, y, val);
}

void
fn_set_aux_terrain_at(int x, int y, int val)
{
    /* Filter anything but the basic six bits. */
    val &= 0x3f;
    set_aux_terrain_at(x, y, tmpttype, val);
}

void
set_border_at(int x, int y, int dir, int t, int onoff)
{
    int ox, oy, bord, obord;
    int odir = opposite_dir(dir);

    if (!t_is_border(t))
      return;
    if (!point_in_dir(x, y, dir, &ox, &oy)) {
	run_warning("border on outside of world at %d,%d, can't set", x, y);
	return;
    }
    allocate_area_aux_terrain(t);
    onoff = (onoff ? 1 : 0);  /* make sure it's one bit */
    bord = aux_terrain_at(x, y, t);
    bord = ((onoff << dir) | (bord & ~(1 << dir)));
    set_aux_terrain_at(x, y, t, bord);
    /* Go to the other cell and tweak its border bits. */
    obord = aux_terrain_at(ox, oy, t);
    obord = ((onoff << odir) | (obord & ~(1 << odir)));
    set_aux_terrain_at(ox, oy, t, obord);
}

void
set_connection_at(int x, int y, int dir, int t, int onoff)
{
    int ox, oy, conn, oconn;
    int odir = opposite_dir(dir);

    if (!t_is_connection(t))
      return;
    if (!point_in_dir(x, y, dir, &ox, &oy)) {
	run_warning("connection to outside of world at %d,%d, can't set",
		    x, y);
	return;
    }
    allocate_area_aux_terrain(t);
    onoff = (onoff ? 1 : 0);  /* make sure it's one bit */
    conn = aux_terrain_at(x, y, t);
    conn = ((onoff << dir) | (conn & ~(1 << dir)));
    set_aux_terrain_at(x, y, t, conn);
    /* Go to the other cell and tweak its connection bits. */
    oconn = aux_terrain_at(ox, oy, t);
    oconn = ((onoff << odir) | (oconn & ~(1 << odir)));
    set_aux_terrain_at(ox, oy, t, oconn);
}

void
fn_set_people_side_at(int x, int y, int val)
{
    set_people_side_at(x, y, val);
}

void
fn_set_control_side_at(int x, int y, int val)
{
    set_control_side_at(x, y, val);
}

void
fn_set_user_at(int x, int y, int val)
{
    set_user_at(x, y, val);
}

void
fn_set_raw_feature_at(int x, int y, int val)
{
    set_raw_feature_at(x, y, val);
}

void
fn_set_elevation_at(int x, int y, int val)
{
    set_elev_at(x, y, val);
}

void
fn_set_material_at(int x, int y, int val)
{
    set_material_at(x, y, tmpmtype, val);
}

void
fn_set_temperature_at(int x, int y, int val)
{
    set_temperature_at(x, y, val);
}

void
fn_set_raw_wind_at(int x, int y, int val)
{
    set_raw_wind_at(x, y, val);
}

void
fn_set_raw_cloud_at(int x, int y, int val)
{
    set_raw_cloud_at(x, y, val);
}

void
fn_set_raw_cloud_bottom_at(int x, int y, int val)
{
    set_raw_cloud_bottom_at(x, y, val);
}

void
fn_set_raw_cloud_height_at(int x, int y, int val)
{
    set_raw_cloud_height_at(x, y, val);
}

// Lifecycle Management

void
allocate_area_terrain(void)
{
    check_area_shape();
    /* Get rid of old stuff maybe. (is this desirable?) */
    if (area.terrain != NULL) {
	free((char *) area.terrain);
	free((char *) area.units);
    }
    /* Allocate the basic terrain layer. */
    /* It doesn't matter what ttype 0 is, we're guaranteed that it
       will be defined eventually. */
    area.terrain = malloc_area_layer(char);
    /* Allocate and null out the unit cache. */
    area.units = malloc_area_layer(Unit *);
}

void
allocate_area_aux_terrain(int t)
{
    if (!any_aux_terrain_defined()) {
	area.auxterrain = (char **) xmalloc(numttypes * sizeof(char *));
    }
    if (!aux_terrain_defined(t)) {
	area.auxterrain[t] = malloc_area_layer(char);
    }
}

void
allocate_area_elevations(void)
{
    int x, y;

    if (!elevations_defined()) {
	check_area_shape();
	area.elevations = malloc_area_layer(short);
	/* If we have terrain, make the initial elevations fall into
	   the correct ranges for the types of terrain present. */
	if (terrain_defined()) {
	    for_all_cells(x, y) {
		set_elev_at(x, y, t_elev_min(terrain_at(x, y)));
	    }
	}
    }
}

void
allocate_area_temperatures(void)
{
    if (!temperatures_defined()) {
	check_area_shape();
	area.temperature = malloc_area_layer(short);
    }
    /* We'll need one scratch layer too. */
    allocate_area_scratch(1);
}

void
allocate_area_people_sides(void)
{
    if (!people_sides_defined()) {
	check_area_shape();
	area.peopleside = malloc_area_layer(char);
	/* NOBODY != 0, so need to blast it over the layer. */
	memset(area.peopleside, NOBODY, area.width * area.height);
	memset(area.peopleside, NOBODY, area.width * area.height);
    }
}

void
allocate_area_control_sides(void)
{
    if (!control_sides_defined()) {
	check_area_shape();
	area.controlside = malloc_area_layer(char);
	/* NOCONTROL != 0, so need to blast it over the layer. */
	memset(area.controlside, NOCONTROL, area.width * area.height);
    }
}

void
allocate_area_material(int m)
{
    check_area_shape();
    if (!any_cell_materials_defined()) {
	area.materials = (long **) xmalloc(nummtypes * sizeof(long *));
    }
    if (!cell_material_defined(m)) {
	area.materials[m] = malloc_area_layer(long);
    }
}

void
allocate_area_clouds(void)
{
    if (!clouds_defined()) {
	check_area_shape();
	area.clouds = malloc_area_layer(short);
    }
}

void
allocate_area_cloud_altitudes(void)
{
    allocate_area_cloud_bottoms();
    allocate_area_cloud_heights();
}

void
allocate_area_cloud_bottoms(void)
{
    if (!cloud_bottoms_defined()) {
	check_area_shape();
	area.cloudbottoms = malloc_area_layer(short);
    }
}

void
allocate_area_cloud_heights(void)
{
    if (!cloud_heights_defined()) {
	check_area_shape();
	area.cloudheights = malloc_area_layer(short);
    }
}

void
allocate_area_winds(void)
{
    if (!winds_defined()) {
	check_area_shape();
	area.winds = malloc_area_layer(short);
    }
}

void
allocate_area_users(void)
{
    if (!user_defined()) {
	check_area_shape();
	area.user = malloc_area_layer(short);
    }
}

void
allocate_area_scratch(int n)
{
    check_area_shape();
    if (n >= 1 && !area.tmp1) {
	area.tmp1 = malloc_area_layer(short);
    }
    if (n >= 2 && !area.tmp2) {
	area.tmp2 = malloc_area_layer(short);
    }
    if (n >= 3 && !area.tmp3) {
	area.tmp3 = malloc_area_layer(short);
    }
    if (n >= 4) {
	run_error("can't allocate more than 3 scratch layers");
    }
}

// GDL I/O

static
void
read_rle(Obj *contents, void (*setter)(int, int, int), short *chartable)
{
    char ch, *rowstr;
    int i, x, y, run, val, sawval, sawneg, sgn, x1, y1, numbadchars = 0;
    Obj *rest;

    rest = contents;
    y = layer_area_h - 1;
    while (rest != lispnil && y >= 0) {
	/* should error check ... */
	rowstr = c_string(car(rest));
	i = 0;
	x = 0;  /* depends on shape of saved data... */
	while ((ch = rowstr[i++]) != '\0' && x < layer_area_w) {
	    sawval = FALSE;
	    sawneg = FALSE;
	    if (isdigit(ch) || ch == '-') {
		if (ch == '-') {
		    sawneg = TRUE;
		    ch = rowstr[i++];
		    /* A minus sign by itself is a problem. */
		    if (!isdigit(ch))
		      goto recovery;
		}
		/* Interpret a substring of digits as a run length. */
		run = ch - '0';
		while ((ch = rowstr[i++]) != 0 && isdigit(ch)) {
		    run = run * 10 + ch - '0';
		}
		/* A '*' separates a run and a numeric value. */
		if (ch == '*' && !ignore_specials) {
		    /* A negative run length is a problem. */
		    if (sawneg)
		      goto recovery;
		    ch = rowstr[i++];
		    /* If we're seeing garbled data, skip to the next line. */
		    if (ch == '\0')
		      goto recovery;
		    /* Recognize a negative number. */
		    sgn = 1;
		    if (ch == '-') {
			sgn = -1;
			ch = rowstr[i++];
		    }
		    /* Interpret these digits as a value. */
		    if (isdigit(ch)) {
			val = ch - '0';
			while ((ch = rowstr[i++]) != 0 && isdigit(ch)) {
			    val = val * 10 + ch - '0';
			}
			sawval = TRUE;
			val = sgn * val;
		    } else {
			/* Some other char seen - just ignore the '*' then. */
		    }
		    /* If we're seeing garbled data, skip to the next line. */
		    if (ch == '\0')
		      goto recovery;
		}
		/* If we're seeing garbled data, skip to the next line. */
		if (ch == '\0')
		  goto recovery;
	    } else {
		run = 1;
	    }
	    if (ch == ',' && !ignore_specials) {
		if (!sawval) {
		    /* This was a value instead of a run length. */
		    val = run;
		    /* If it was prefixed with a minus sign originally,
		       negate the value. */
		    if (sawneg)
		      val = - val;
		    run = 1;
		} else {
		    /* Comma is just being a separator. */
		}
	    } else if (chartable != NULL) {
		val = chartable[(int) ch];
	    } else if (between('a', ch, '~')) {
		val = ch - 'a';
	    } else if (between(':', ch, '[')) {
		val = ch - ':' + 30;
	    } else {
		/* Warn about strange characters. */
		++numbadchars;
		if (numbadchars <= 5) {
		    read_warning(
		     "Bad char '%c' (0x%x) in layer, using NUL instead",
				 ch, ch);
		    /* Clarify that we're not going to report all bad chars. */
		    if (numbadchars == 5)
		      read_warning(
		     "Additional bad chars will not be reported individually");
		}
		val = 0;
	    }
	    val = val * layer_multiplier + layer_adder;
	    /* Given a run of values, stuff them into the layer. */
	    while (run-- > 0) {
		x1 = x - layer_area_x;
		y1 = y - layer_area_y;
		if (in_area(x1, y1))
		  (*setter)(x1, y1, val);
		++x;
	    }
	}
      recovery:
	/* Fill-in string may be too short for this row; just leave
	   the rest of it alone, assume that somebody has assured that
	   the contents are reasonable. */
	rest = cdr(rest);
	y--;
    }
    /* Report the count of garbage chars, in case there were a great many. */
    if (numbadchars > 0)
      init_warning("A total of %d bad chars were present", numbadchars);
}

void
read_layer(Obj *contents, void (*setter)(int, int, int))
{
    int i, slen, n, ix, len, usechartable = FALSE;
    char *str;
    short chartable[256];
    Obj *rest, *desc, *rest2, *subdesc, *sym, *num;

    layer_use_default = FALSE;
    layer_default = 0;
    layer_multiplier = 1;
    layer_adder = 0;
    layer_area_x = area.fullx;  layer_area_y = area.fully;
    layer_area_w = area.width;  layer_area_h = area.height;
    if (area.fullwidth > 0)
      layer_area_w = area.fullwidth;
    if (area.fullheight > 0)
      layer_area_h = area.fullheight;
    ignore_specials = FALSE;
    for_all_list(contents, rest) {
	desc = car(rest);
	if (stringp(desc)) {
	    /* Read from here to the end of the list, interpreting as
	       contents. */
	    read_rle(rest, setter, (usechartable ? chartable : NULL));
	    return;
	} else if (consp(desc) && symbolp(car(desc))) {
	    switch (keyword_code(c_string(car(desc)))) {
	      case K_CONSTANT:
		/* should set to a constant value taken from cadr */
		read_warning("Constant layers not supported yet");
		return;
	      case K_SUBAREA:
	        /* should apply data to a subarea */
		read_warning("Layer subareas not supported yet");
		break;
	      case K_XFORM:
		layer_multiplier = c_number(cadr(desc));
		layer_adder = c_number(caddr(desc));
		break;
	      case K_BY_BITS:
		break;
	      case K_BY_CHAR:
		/* Assign each char to its corresponding index. */
		/* First seed the table with a 1-1 map. */
		for (i = 0; i < 255; ++i)
		  chartable[i] = 0;
		for (i = 'a'; i <= '~'; ++i)
		  chartable[i] = i - 'a';
		for (i = ':'; i <= '['; ++i)
		  chartable[i] = i - ':' + 30;
		str = c_string(cadr(desc));
		len = strlen(str);
		for (i = 0; i < len; ++i) {
		    chartable[(int) str[i]] = i;
		    /* If special chars in by-char string, flag it. */
		    if (str[i] == '*' || str[i] == ',')
		      ignore_specials = TRUE;
		}
		usechartable = TRUE;
		break;
	      case K_BY_NAME:
		/* Work through list and match names to numbers. */
		/* First seed the table with a 1-1 map. */
		for (i = 0; i < 255; ++i)
		  chartable[i] = 0;
		for (i = 'a'; i <= '~'; ++i)
		  chartable[i] = i - 'a';
		for (i = ':'; i <= '['; ++i)
		  chartable[i] = i - ':' + 30;
		desc = cdr(desc);
		/* Support optional explicit string a la by-char. */
		if (stringp(car(desc))) {
		    str = c_string(car(desc));
		    slen = strlen(str);
		    for (i = 0; i < slen; ++i)
		      chartable[(int) str[i]] = i;
		    desc = cdr(desc);
		} else {
		    str = NULL;
		}
		i = 0;
		for (rest2 = desc; rest2 != lispnil; rest2 = cdr(rest2)) {
		    subdesc = car(rest2);
		    if (symbolp(subdesc)) {
		    	sym = subdesc;
		    	ix = i++;
		    } else if (consp(subdesc)) {
		    	sym = car(subdesc);
		    	num = cadr(subdesc);
		    	TYPECHECK(numberp, num,
				  "by-name explicit value is not a number");
		    	ix = c_number(num);
		    } else {
		    	read_warning("garbage by-name subdesc, ignoring");
		    	continue;
		    }
		    /* Eval the symbol into something resembling a value. */
		    sym = eval(sym);
		    TYPECHECK(numberishp, sym,
			      "by-name index is not a number or type");
		    n = c_number(sym);
		    chartable[(str ? str[ix] : (ix <= 29 ? ('a' + ix) : (':' + ix - 30)))] = n;
		}
		usechartable = TRUE;
		break;
	      default:
		sprintlisp(readerrbuf, desc, BUFSIZE);
		read_warning("Ignoring garbage terrain description %s",
			     readerrbuf);
	    }
	}
    }
}

void
fill_in_terrain(Obj *contents)
{
    /* We must have some terrain types or we're going to lose bigtime. */
    if (numttypes == 0)
      load_default_game();
    numbadterrain = 0;
    /* Make sure the terrain layer exists. */
    if (!terrain_defined())
      allocate_area_terrain();
    read_layer(contents, fn_set_terrain_at);
    if (numbadterrain > 0) {
	read_warning("%d occurrences of unknown terrain in all",
		     numbadterrain);
    }
}

void
fill_in_aux_terrain(Obj *contents)
{
    int t;
    Obj *typesym = car(contents), *typeval;

    if (symbolp(typesym) && ttypep(typeval = eval(typesym))) {
	t = c_number(typeval);
	contents = cdr(contents);
	/* Make sure aux terrain space exists, but don't overwrite. */
	allocate_area_aux_terrain(t);
	tmpttype = t;
	read_layer(contents, fn_set_aux_terrain_at);
	/* Ensure that borders and connections have all their bits
	   correctly set. */
	patch_linear_terrain(t);
    } else {
	/* not a valid aux terrain type */
    }
}

void
fill_in_people_sides(Obj *contents)
{
    /* Make sure the people sides layer exists. */
    allocate_area_people_sides();
    read_layer(contents, fn_set_people_side_at);
}

void
fill_in_control_sides(Obj *contents)
{
    /* Make sure the people sides layer exists. */
    allocate_area_control_sides();
    read_layer(contents, fn_set_control_side_at);
}

void
fill_in_features(Obj *contents)
{
    int fid;
    Obj *featspec, *flist, *feattype, *featname;
    Feature *feat;

    init_features();
    for_all_list(car(contents), flist) {
	featspec = car(flist);
	fid = 0;
	feat = NULL;
	if (numberp(car(featspec))) {
	    fid = c_number(car(featspec));
	    feat = find_feature(fid);
	    featspec = cdr(featspec);
	}
	if (feat == NULL) {
	    feattype = car(featspec);
	    featname = cadr(featspec);
	    if ((stringp(feattype) || symbolp(feattype))
		&& (stringp(featname) || symbolp(featname))) {
		feat = create_feature(c_string(feattype), c_string(featname));
		if (fid > 0)
		  feat->id = fid;
	    } else {
		syntax_error(featspec, "not a valid feature spec");
	    }
	} else {
	    /* complain if data doesn't match? */
	}
    }
    read_layer(cdr(contents), fn_set_raw_feature_at);
}

void
fill_in_elevations(Obj *contents)
{
    /* Make sure the elevation layer exists. */
    allocate_area_elevations();
    read_layer(contents, fn_set_elevation_at);
    compute_elevation_bounds();
}

void
fill_in_cell_material(Obj *contents)
{
    int m;
    Obj *typesym = car(contents), *typeval;

    if (symbolp(typesym) && mtypep(typeval = eval(typesym))) {
	m = c_number(typeval);
	contents = cdr(contents);
	/* Make sure this material layer exists. */
	allocate_area_material(m);
	tmpmtype = m;
	read_layer(contents, fn_set_material_at);
    } else {
	/* not a valid material type spec, should warn */
    }
}

void
fill_in_temperatures(Obj *contents)
{
    /* Make sure the temperature layer exists. */
    allocate_area_temperatures();
    read_layer(contents, fn_set_temperature_at);
}

void
fill_in_winds(Obj *contents)
{
    /* Make sure the winds layer exists. */
    allocate_area_winds();
    read_layer(contents, fn_set_raw_wind_at);
}

void
fill_in_clouds(Obj *contents)
{
    /* Make sure the clouds layer exists. */
    allocate_area_clouds();
    read_layer(contents, fn_set_raw_cloud_at);
}

void
fill_in_cloud_bottoms(Obj *contents)
{
    /* Make sure the cloud bottoms layer exists. */
    allocate_area_cloud_bottoms();
    read_layer(contents, fn_set_raw_cloud_bottom_at);
}

void
fill_in_cloud_heights(Obj *contents)
{
    /* Make sure the cloud heights layer exists. */
    allocate_area_cloud_heights();
    read_layer(contents, fn_set_raw_cloud_height_at);
}

void
fill_in_users(Obj *contents)
{
    /* Make sure the user layer exists. */
    if (!user_defined())
    	allocate_area_users();
    read_layer(contents, fn_set_user_at);
}

static
int
original_point(int x1, int y1, int *x2p, int *y2p)
{
    *x2p = (((x1 - reshaper->final_subarea_x) * reshaper->subarea_width )
	    / reshaper->final_subarea_width ) + reshaper->subarea_x;
    *y2p = (((y1 - reshaper->final_subarea_y) * reshaper->subarea_height)
	    / reshaper->final_subarea_height) + reshaper->subarea_y;
    return inside_area(*x2p, *y2p);
}

static
int
fn_elevation_at_offset(int x, int y)
{
    return (elev_at(x, y) - area.minelev);
}

static
void
write_run(int run, int val)
{
    if (run > 1) {
	add_num_to_form_no_space(run);
	if (!between(0, val, 63))
	  add_char_to_form('*');
    }
    if (between(0, val, 29)) {
	add_char_to_form(val + 'a');
    } else if (between(30, val, 63)) {
	add_char_to_form(val - 30 + ':');
    } else {
	add_num_to_form_no_space(val);
	add_char_to_form(',');
    }
}

void
write_rle(int (*datafn)(int, int), int lo, int hi, int (*translator)(int),
	  int compress)
{
    int width, height, x, y, x0, y0, run, runval, val, trval;
    int numbad = 0;

    width = area.width;  height = area.height;
    if (doreshape) {
	width = reshaper->final_width;  height = reshaper->final_height;
    }
    /* Iterate through the rows from the top down. */
    for (y = height - 1; y >= 0; --y) {
	space_form();
	space_form();
	add_char_to_form('"');
	run = 0;
	x0 = 0;  y0 = y;
	if (doreshape)
	  original_point(0, y, &x0, &y0);
	val = (*datafn)(x0, y0);
	/* Zero out anything not in the world, unless reshaping. */
	if (!doreshape && !in_area(x0, y0))
	  val = 0;
	/* Check that the data falls within bounds, clip if not. */
	if (lo <= hi && !between(lo, val, hi) && in_area(x0, y0)) {
	    ++numbad;
	    if (val < lo)
	      val = lo;
	    if (val > hi)
	      val = hi;
	}
	runval = val;
	for (x = 0; x < width; ++x) {
	    x0 = x;  y0 = y;
	    if (doreshape)
	      original_point(x, y, &x0, &y0);
	    val = (*datafn)(x0, y0);
	    /* Zero out anything not in the world, unless reshaping. */
	    if (!doreshape && !in_area(x0, y0))
	      val = 0;
	    /* Check that the data falls within bounds, clip if not. */
	    if (lo <= hi && !between(lo, val, hi) && in_area(x0, y0)) {
		++numbad;
		if (val < lo)
		  val = lo;
		if (val > hi)
		  val = hi;
	    }
	    if (val == runval && compress) {
		run++;
	    } else {
		trval = (translator != NULL ? (*translator)(runval) : runval);
		write_run(run, trval);
		/* Start a new run. */
		runval = val;
		run = 1;
	    }
	}
	/* Finish off the row. */
	trval = (translator != NULL ? (*translator)(val) : val);
	write_run(run, trval);
	add_char_to_form('"');
	newline_form();
    }
    if (numbad > 0) {
	run_warning("%d values not between %d and %d", numbad, lo, hi);
    }
}

void
write_area_terrain(int compress)
{
    int t;

    /* Terrain might not be set up when downloading at beginning of game,
       don't try to write if so. */
    if (area.terrain == NULL)
      return;
    start_form(key(K_AREA));
    space_form();
    start_form(key(K_TERRAIN));
    newline_form();
    space_form();
    start_form(key(K_BY_NAME));
    for_all_terrain_types(t) {
	/* Break the list into groups of 5 per line. */
    	if (t % 5 == 0) {
	    newline_form();
	    space_form();
	    space_form();
	    space_form();
	}
	space_form();
	start_form(escaped_symbol(t_type_name(t)));
	add_num_to_form(t);
	end_form();
    }
    end_form();
    newline_form();
    write_rle(fn_terrain_at, 0, numttypes - 1, NULL, compress);
    space_form();
    space_form();
    end_form();
    end_form();
    newline_form();
}

void
write_area_aux_terrain(int compress)
{
    int t;

    for_all_terrain_types(t) {
	if (aux_terrain_defined(t)) {
	    start_form(key(K_AREA));
	    space_form();
	    start_form(key(K_AUX_TERRAIN));
	    add_to_form(escaped_symbol(t_type_name(t)));
	    newline_form();
	    tmpttype = t;
	    write_rle(fn_aux_terrain_at, 0, 127, NULL, compress);
	    space_form();
	    space_form();
	    end_form();
	    end_form();
	    newline_form();
	}
    }
}

void
write_area_features(int compress)
{
    Feature *feature;

    if (featurelist == NULL || !features_defined())
      return;
    start_form(key(K_AREA));
    space_form();
    start_form(key(K_FEATURES));
    space_form();
    start_form("");
    newline_form();
    /* Dump out the list of features first. */
    for_all_features(feature) {
	space_form();
	space_form();
	space_form();
	start_form("");
	add_num_to_form(feature->id);
	add_to_form(escaped_string(feature->feattype));
	add_to_form(escaped_string(feature->name));
	end_form();
	newline_form();
    }
    space_form();
    space_form();
    end_form();
    newline_form();
    /* Now record which features go with which cells. */
    write_rle(fn_feature_at, 0, -1, NULL, compress);
    space_form();
    space_form();
    end_form();
    end_form();
    newline_form();
}

void
write_area_elevations(int compress)
{
    if (!elevations_defined())
      return;
    start_form(key(K_AREA));
    space_form();
    start_form(key(K_ELEVATIONS));
    if (area.minelev != 0) {
	space_form();
	start_form(key(K_XFORM));
	add_num_to_form(1);
	add_num_to_form(area.minelev);
	end_form();
    }
    newline_form();
    write_rle(fn_elevation_at_offset, 0, area.maxelev - area.minelev,
	      NULL, compress);
    space_form();
    space_form();
    end_form();
    end_form();
    newline_form();
}

void
write_area_people_sides(int compress)
{
    Side *side;

    if (!people_sides_defined())
      return;
    start_form(key(K_AREA));
    space_form();
    start_form(key(K_PEOPLE_SIDES));
    newline_form();
    space_form();
    space_form();
    start_form(key(K_BY_NAME));
    for_all_sides(side) {
	/* Break the list into groups of 5 per line. */
	if (side_number(side) % 5 == 0) {
	    newline_form();
	    space_form();
	    space_form();
	    space_form();
	}
	space_form();
	start_form("");
	add_num_to_form(side_number(side)); /* should be symbol */
	add_num_to_form(side_number(side));
	end_form();
    }
    end_form();
    newline_form();
    /* Value of NOBODY is guaranteed to be the largest valid. */
    write_rle(fn_people_side_at, 0, NOBODY, NULL, compress);
    space_form();
    space_form();
    end_form();
    end_form();
    newline_form();
}

void
write_area_control_sides(int compress)
{
    Side *side;

    if (!control_sides_defined())
      return;
    start_form(key(K_AREA));
    space_form();
    start_form(key(K_CONTROL_SIDES));
    newline_form();
    space_form();
    space_form();
    start_form(key(K_BY_NAME));
    for_all_sides(side) {
	/* Break the list into groups of 5 per line. */
	if (side_number(side) % 5 == 0) {
	    newline_form();
	    space_form();
	    space_form();
	    space_form();
	}
	space_form();
	start_form("");
	add_num_to_form(side_number(side)); /* should be symbol */
	add_num_to_form(side_number(side));
	end_form();
    }
    end_form();
    newline_form();
    /* Value of NOCONTROL is guaranteed to be the largest valid. */
    write_rle(fn_control_side_at, 0, NOCONTROL, NULL, compress);
    space_form();
    space_form();
    end_form();
    end_form();
    newline_form();
}

void
write_area_materials(int compress)
{
    int m;

    if (any_cell_materials_defined()) {
	for_all_material_types(m) {
	    if (cell_material_defined(m)) {
		start_form(key(K_AREA));
		space_form();
		start_form(key(K_MATERIAL));
		add_to_form(escaped_symbol(m_type_name(m)));
		newline_form();
		tmpmtype = m;
		write_rle(fn_material_at, 0, 127, NULL, compress);
		end_form();
		end_form();
		newline_form();
	    }
	}
    }
}

void
write_area_temperatures(int compress)
{
    if (!temperatures_defined())
      return;
    start_form(key(K_AREA));
    space_form();
    start_form(key(K_TEMPERATURES));
    newline_form();
    write_rle(fn_temperature_at, -9999, 9999, NULL, compress);
    space_form();
    space_form();
    end_form();
    end_form();
    newline_form();
}

void
write_area_clouds(int compress)
{
    if (clouds_defined()) {
	start_form(key(K_AREA));
	space_form();
	start_form(key(K_CLOUDS));
	newline_form();
	write_rle(fn_raw_cloud_at, 0, 127, NULL, compress);
	space_form();
	space_form();
	end_form();
	end_form();
	newline_form();
    }
    if (cloud_bottoms_defined()) {
	start_form(key(K_AREA));
	space_form();
	start_form(key(K_CLOUD_BOTTOMS));
	newline_form();
	write_rle(fn_raw_cloud_bottom_at, 0, 9999, NULL, compress);
	space_form();
	space_form();
	end_form();
	end_form();
	newline_form();
    }
    if (cloud_heights_defined()) {
	start_form(key(K_AREA));
	space_form();
	start_form(key(K_CLOUD_HEIGHTS));
	newline_form();
	write_rle(fn_raw_cloud_height_at, 0, 9999, NULL, compress);
	space_form();
	space_form();
	end_form();
	end_form();
	newline_form();
    }
}

void
write_area_winds(int compress)
{
    if (winds_defined()) {
	start_form(key(K_AREA));
	space_form();
	start_form(key(K_WINDS));
	newline_form();
	write_rle(fn_raw_wind_at, -32767, 32767, NULL, compress);
	space_form();
	space_form();
	end_form();
	end_form();
	newline_form();
    }
}

void
write_area_users(int compress)
{
    extern int nextid;

    if (!user_defined())
      return;
    start_form(key(K_AREA));
    space_form();
    start_form(key(K_USER));
    newline_form();
    write_rle(fn_user_at, NOUSER, nextid, NULL, compress);
    space_form();
    space_form();
    end_form();
    end_form();
    newline_form();
}

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END
