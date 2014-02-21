/* Designer tools for the X11 interface to Xconq.
   Copyright (C) 1992-1997 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "conq.h"
#include "kpublic.h"
#include "xtconq.h"

#ifdef DESIGNERS

static void design_tool_callback(Widget w, XtPointer client_data,
				 XtPointer call_data);
static void design_terrain_callback(Widget w, XtPointer client_data,
				    XtPointer call_data);
static void design_bg_terrain_callback(Widget w, XtPointer client_data,
				       XtPointer call_data);
static void design_unit_callback(Widget w, XtPointer client_data,
				 XtPointer call_data);
static void design_unit_side_callback(Widget w, XtPointer client_data,
				      XtPointer call_data);
static void design_people_side_callback(Widget w, XtPointer client_data,
					XtPointer call_data);
static void design_feature_callback(Widget w, XtPointer client_data,
				    XtPointer call_data);
static void design_new_feature_callback(Widget w, XtPointer client_data,
					XtPointer call_data);
static void design_brush_callback(Widget w, XtPointer client_data,
				  XtPointer call_data);
static void design_quit_callback(Widget w, XtPointer client_data,
				 XtPointer call_data);

static void set_design_tool(Side *side, int tool);

void
popup_design(Side *side)
{
    if (!side->ui->design_shell)
      create_design(side);
    /* Force this to "looktool" always, not much value in remembering current
       tool across popdowns and re-popups. */
    set_design_tool(side, looktool);
    XtPopup(side->ui->design_shell, XtGrabNone);
    /* Expose event will cause window contents to be drawn now. */
}

void
create_design(Side *side)
{
    int designw, designh, paneh;
    Widget pane, label, button;
    Display *dpy = side->ui->dpy;

    paneh = 50;
    designw = 200;  designh = 6 * paneh;
    side->ui->design_shell =
      XtVaCreatePopupShell("Design", topLevelShellWidgetClass, side->ui->shell,
			   XtNwidth, designw,
			   XtNheight, designh,
			   NULL);
    side->ui->design =
      XtVaCreateManagedWidget("Design", panedWidgetClass, side->ui->design_shell,
			      XtNwidth, designw,
			      XtNheight, designh,
			      NULL);

    pane =
      XtVaCreateManagedWidget("NormalPane", boxWidgetClass, side->ui->design,
			      XtNwidth, designw,
			      XtNheight, paneh,
			      XtNmax, paneh,
			      XtNmin, paneh,
			      XtNshowGrip, False,
			      XtNskipAdjust, True,
			      NULL);
    side->ui->normal_button =
      XtVaCreateManagedWidget("Normal", toggleWidgetClass, pane,
			      XtNlabel, "Norm",
			      NULL);
    XtAddCallback(side->ui->normal_button, XtNcallback,
		  design_tool_callback, (XtPointer) looktool);
    side->ui->normal_label =
      XtVaCreateManagedWidget("NormalLabel", labelWidgetClass, pane,
			      XtNborderWidth, 0,
			      XtNjustify, XtJustifyLeft,
			      XtNlabel, " ",
			      XtNresize, False,
			      XtNwidth, 150,
			      NULL);
    button =
      XtVaCreateManagedWidget("Quit", commandWidgetClass, pane,
			      XtNlabel, "Quit",
			      NULL);
    XtAddCallback(button, XtNcallback,
		  design_quit_callback, NULL);


    pane =
      XtVaCreateManagedWidget("TerrainPane", boxWidgetClass, side->ui->design,
			      XtNwidth, designw,
			      XtNheight, paneh,
			      XtNmax, paneh,
			      XtNmin, paneh,
			      XtNshowGrip, False,
			      XtNskipAdjust, True,
			      NULL);
    side->ui->terrain_button =
      XtVaCreateManagedWidget("Terrain", toggleWidgetClass, pane,
			      XtNlabel, "Terr",
			      NULL);
    XtAddCallback(side->ui->terrain_button, XtNcallback,
		  design_tool_callback, (XtPointer) cellpainttool);
    side->ui->terrain_label =
      XtVaCreateManagedWidget("Terrain", labelWidgetClass, pane,
			      XtNborderWidth, 0,
			      XtNjustify, XtJustifyLeft,
			      XtNresize, False,
			      XtNwidth, 150,
			      NULL);
    button =
      XtVaCreateManagedWidget("fgTerrMinus", commandWidgetClass, pane,
			      XtNlabel, "fg-",
			      NULL);
    XtAddCallback(button, XtNcallback,
		  design_terrain_callback, (XtPointer) (-1));
    button =
      XtVaCreateManagedWidget("fgTerrPlus", commandWidgetClass, pane,
			      XtNlabel, "fg+",
			      NULL);
    XtAddCallback(button, XtNcallback,
		  design_terrain_callback, (XtPointer) (1));
    button =
      XtVaCreateManagedWidget("bgTerrMinus", commandWidgetClass, pane,
			      XtNlabel, "bg-",
			      NULL);
    XtAddCallback(button, XtNcallback,
		  design_bg_terrain_callback, (XtPointer) (-1));
    button =
      XtVaCreateManagedWidget("bgTerrPlus", commandWidgetClass, pane,
			      XtNlabel, "bg+",
			      NULL);
    XtAddCallback(button, XtNcallback,
		  design_bg_terrain_callback, (XtPointer) (1));
    update_curttype(side);

    pane =
      XtVaCreateManagedWidget("UnitPane", boxWidgetClass, side->ui->design,
			      XtNwidth, designw,
			      XtNheight, paneh,
			      XtNmax, paneh,
			      XtNmin, paneh,
			      XtNshowGrip, False,
			      XtNskipAdjust, True,
			      NULL);
    side->ui->unit_button =
      XtVaCreateManagedWidget("Unit", toggleWidgetClass, pane,
			      XtNlabel, "Unit",
			      NULL);
    XtAddCallback(side->ui->unit_button, XtNcallback,
		  design_tool_callback, (XtPointer) unitaddtool);
    side->ui->unit_label =
      XtVaCreateManagedWidget("UnitLabel", labelWidgetClass, pane,
			      XtNborderWidth, 0,
			      XtNjustify, XtJustifyLeft,
			      XtNresize, False,
			      XtNwidth, 150,
			      NULL);
    button =
      XtVaCreateManagedWidget("unitMinus", commandWidgetClass, pane,
			      XtNlabel, "u-",
			      NULL);
    XtAddCallback(button, XtNcallback,
		  design_unit_callback, (XtPointer) (-1));
    button =
      XtVaCreateManagedWidget("unitPlus", commandWidgetClass, pane,
			      XtNlabel, "u+",
			      NULL);
    XtAddCallback(button, XtNcallback,
		  design_unit_callback, (XtPointer) (1));
    button =
      XtVaCreateManagedWidget("unitSideMinus", commandWidgetClass, pane,
			      XtNlabel, "s-",
			      NULL);
    XtAddCallback(button, XtNcallback,
		  design_unit_side_callback, (XtPointer) (-1));
    button =
      XtVaCreateManagedWidget("unitSidePlus", commandWidgetClass, pane,
			      XtNlabel, "s+",
			      NULL);
    XtAddCallback(button, XtNcallback,
		  design_unit_side_callback, (XtPointer) (1));
    update_curutype(side);

    pane =
      XtVaCreateManagedWidget("PeoplePane", boxWidgetClass, side->ui->design,
			      XtNwidth, designw,
			      XtNheight, paneh,
			      XtNmax, paneh,
			      XtNmin, paneh,
			      XtNshowGrip, False,
			      XtNskipAdjust, True,
			      NULL);
    side->ui->people_button =
      XtVaCreateManagedWidget("People", toggleWidgetClass, pane,
			      XtNlabel, "Peop",
			      NULL);
    XtAddCallback(side->ui->people_button, XtNcallback,
		  design_tool_callback, (XtPointer) peoplepainttool);
    side->ui->people_label =
      XtVaCreateManagedWidget("PeopleLabel", labelWidgetClass, pane,
			      XtNborderWidth, 0,
			      XtNjustify, XtJustifyLeft,
			      XtNresize, False,
			      XtNwidth, 150,
			      NULL);
    button =
      XtVaCreateManagedWidget("peopleMinus", commandWidgetClass, pane,
			      XtNlabel, "p-",
			      NULL);
    XtAddCallback(button, XtNcallback,
		  design_people_side_callback, (XtPointer) (-1));
    button =
      XtVaCreateManagedWidget("peoplePlus", commandWidgetClass, pane,
			      XtNlabel, "p+",
			      NULL);
    XtAddCallback(button, XtNcallback,
		  design_people_side_callback, (XtPointer) (1));
    update_cursidenumber(side);

    pane =
      XtVaCreateManagedWidget("FeaturePane", boxWidgetClass, side->ui->design,
			      XtNwidth, designw,
			      XtNheight, paneh,
			      XtNmax, paneh,
			      XtNmin, paneh,
			      XtNshowGrip, False,
			      XtNskipAdjust, True,
			      NULL);
    side->ui->feature_button =
      XtVaCreateManagedWidget("Feature", toggleWidgetClass, pane,
			      XtNlabel, "Feat",
			      NULL);
    XtAddCallback(side->ui->feature_button, XtNcallback,
		  design_tool_callback, (XtPointer) featurepainttool);
    side->ui->feature_label =
      XtVaCreateManagedWidget("FeatureLabel", labelWidgetClass, pane,
			      XtNborderWidth, 0,
			      XtNjustify, XtJustifyLeft,
			      XtNresize, False,
			      XtNwidth, 150,
			      NULL);
    button =
      XtVaCreateManagedWidget("featureMinus", commandWidgetClass, pane,
			      XtNlabel, "f-",
			      NULL);
    XtAddCallback(button, XtNcallback,
		  design_feature_callback, (XtPointer) (-1));
    button =
      XtVaCreateManagedWidget("featurePlus", commandWidgetClass, pane,
			      XtNlabel, "f+",
			      NULL);
    XtAddCallback(button, XtNcallback,
		  design_feature_callback, (XtPointer) (1));
    button =
      XtVaCreateManagedWidget("newFeature", commandWidgetClass, pane,
			      XtNlabel, "new",
			      NULL);
    XtAddCallback(button, XtNcallback,
		  design_new_feature_callback, NULL);
    update_curfeature(side);

    pane =
      XtVaCreateManagedWidget("BrushPane", boxWidgetClass, side->ui->design,
			      XtNwidth, designw,
			      XtNheight, paneh,
			      XtNmax, paneh,
			      XtNmin, paneh,
			      XtNshowGrip, False,
			      XtNskipAdjust, True,
			      NULL);
    label =
      XtVaCreateManagedWidget("brushRadius", labelWidgetClass, pane,
			      XtNborderWidth, 0,
			      XtNlabel, "  ",
			      NULL);
    side->ui->brush_radius_label =
      XtVaCreateManagedWidget("brushRadiusLabel", labelWidgetClass, pane,
			      XtNborderWidth, 0,
			      XtNresize, False,
			      XtNwidth, 150,
			      NULL);
    button =
      XtVaCreateManagedWidget("brushMinus", commandWidgetClass, pane,
			      XtNlabel, "-",
			      NULL);
    XtAddCallback(button, XtNcallback,
		  design_brush_callback, (XtPointer) (-1));
    button =
      XtVaCreateManagedWidget("brushPlus", commandWidgetClass, pane,
			      XtNlabel, "+",
			      NULL);
    XtAddCallback(button, XtNcallback,
		  design_brush_callback, (XtPointer) (1));
    button =
      XtVaCreateManagedWidget("brushReset", commandWidgetClass, pane,
			      XtNlabel, "0",
			      NULL);
    XtAddCallback(button, XtNcallback,
		  design_brush_callback, (XtPointer) (0));
    update_curbrushradius(side);

    /* (should add other tools here) */

    XtRealizeWidget(side->ui->design_shell);

    /* XtWindow works only after a call to XtRealizeWidget. */
    XSetWMProtocols(dpy, XtWindow(side->ui->design_shell),
		    &side->ui->kill_atom, 1);
    XtOverrideTranslations(side->ui->design_shell,
	XtParseTranslationTable("<Message>WM_PROTOCOLS: wm-quit()"));

}

static void
design_tool_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
    int which = (int) client_data;
    Side *side;
    Map *map;

    if (find_side_via_widget(w, &side)) {
    	set_design_tool(side, which);
    	for_all_maps(side, map) {
	    update_controls(side, map);
	    set_tool_cursor(side, map);
	}
    }
}

static void
set_design_tool(Side *side, int tool)
{
    Map *map;

    side->ui->curdesigntool = tool;
    nargs = 0;
    XtSetArg(tmpargs[nargs], XtNstate, (Boolean) (tool == looktool));  nargs++;
    XtSetValues (side->ui->normal_button, tmpargs, nargs);
    nargs = 0;
    XtSetArg(tmpargs[nargs], XtNstate, (Boolean) (tool == cellpainttool));  nargs++;
    XtSetValues (side->ui->terrain_button, tmpargs, nargs);
    nargs = 0;
    XtSetArg(tmpargs[nargs], XtNstate, (Boolean) (tool == unitaddtool));  nargs++;
    XtSetValues (side->ui->unit_button, tmpargs, nargs);
    nargs = 0;
    XtSetArg(tmpargs[nargs], XtNstate, (Boolean) (tool == peoplepainttool));  nargs++;
    XtSetValues (side->ui->people_button, tmpargs, nargs);
    nargs = 0;
    XtSetArg(tmpargs[nargs], XtNstate, (Boolean) (tool == featurepainttool));  nargs++;
    XtSetValues (side->ui->feature_button, tmpargs, nargs);
    /* As a special case, set the actual terrain tool based on terrain
       subtype. */
    if (tool == cellpainttool && t_is_border(side->ui->curttype))
      side->ui->curdesigntool = bordpainttool;
    if (tool == cellpainttool && t_is_connection(side->ui->curttype))
      side->ui->curdesigntool = connpainttool;
    for_all_maps(side, map) {
	map->curtool = side->ui->curdesigntool;
    }
}

/* This macro implements cycling of a variable through a set of
   consecutive values, with given dir.  If the limit is 0, then the
   cycling part is not done. */

#define OPTION_CYCLE(var, lo, hi, which)  \
  if ((hi) - (lo) > 0) {  \
    (var) = (((var) + (which) - (lo) + ((hi) - (lo))) % ((hi) - (lo))) + (lo);  \
  } else {  \
    (var) = ((var) + (which));  \
  }

static void
design_terrain_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
    int which = (int) client_data;
    Side *side;
    Map *map;

    if (!find_side_via_widget(w, &side))
      return;

    OPTION_CYCLE(side->ui->curttype, 0, numttypes, which);
    update_curttype(side);
    /* Do this also, because the terrain type change might necessitate
       a tool cursor change.  If the type should not be
       "cellpainttool", this will change it to the appropriate one. */
    set_design_tool(side, cellpainttool);
    for_all_maps(side, map) {
	update_controls(side, map);
	set_tool_cursor(side, map);
    }
}

static void
design_bg_terrain_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
    int which = (int) client_data;
    Side *side;
    Map *map;

    if (!find_side_via_widget(w, &side))
      return;

    /* (should only cycle through allowed types?) */
    side->ui->curbgttype =
      (side->ui->curbgttype + which + numttypes) % numttypes;
    update_curttype(side);
    set_design_tool(side, cellpainttool);
    for_all_maps(side, map) {
	update_controls(side, map);
	set_tool_cursor(side, map);
    }
}

static void
design_unit_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
    int which = (int) client_data;
    Side *side;

    if (!find_side_via_widget(w, &side))
      return;

    /* (should only cycle through allowed types?) */
    side->ui->curutype = (side->ui->curutype + which + numutypes) % numutypes;
    update_curutype(side);
}

static void
design_unit_side_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
    int which = (int) client_data;
    Side *side;

    if (!find_side_via_widget(w, &side))
      return;

    /* (should only cycle through allowed types?) */
    side->ui->curusidenumber =
      (side->ui->curusidenumber + which + (numsides + 1)) % (numsides + 1);
    update_curutype(side);
}

static void
design_people_side_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
    int which = (int) client_data;
    Side *side;

    if (!find_side_via_widget(w, &side))
      return;

    side->ui->cursidenumber =
      (side->ui->cursidenumber + which + (numsides + 1)) % (numsides + 1);
    update_cursidenumber(side);
}

static void
design_feature_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
    int which = (int) client_data;
    Side *side;
    extern int nextfid;

    if (!find_side_via_widget(w, &side))
      return;

    if (!features_defined())
      return;

    OPTION_CYCLE(side->ui->curfid, 0, nextfid, which);
    update_curfeature(side);
}

static void
design_new_feature_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
    Feature *feature;
    Side *side;
    extern int nextfid;

    if (!find_side_via_widget(w, &side))
      return;

    sprintf(spbuf, "%d", nextfid);
    feature = net_create_feature("feature", copy_string(spbuf));
    if (feature != NULL) {
	side->ui->curfid = feature->id;
    }
    update_curfeature(side);
}

static void
design_brush_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
    int which = (int) client_data;
    Side *side;

    if (!find_side_via_widget(w, &side))
      return;

    switch (which) {
      case -1:
	if (side->ui->curbrushradius > 0)
	  --(side->ui->curbrushradius);
	break;
      case 0:
	side->ui->curbrushradius = 0;
	break;
      case 1:
	++(side->ui->curbrushradius);
	break;
      default:
	run_error("huh??");
	break;
    }
    update_curbrushradius(side);
}

static void
design_quit_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
    Side *side;
    Map *map;

    if (!find_side_via_widget(w, &side))
      return;

    popdown_design(side);
    net_become_nondesigner(side);
    for_all_maps(side, map) {
	map->curtool = looktool;
	set_tool_cursor(side, map);
	update_controls(side, map);
    }
}

void
update_curttype(Side *side)
{
    char buf[BUFSIZE];
#if 0
    Control *control = &(map->controlbs[CURTTYPE]);
    Image *timg = best_image(side->ui->timages[side->ui->curttype], 16, 16);
    X11Image *ximg = timg ? (X11Image *) timg->hook : NULL;
    GC gc = side->ui->textgc;
    Display *dpy = side->ui->dpy;
    int x=0, y=0, w=16, h=16;
#endif

    switch (t_subtype(side->ui->curttype)) {
      case cellsubtype:
	sprintf(buf, "%s/%s",
		t_type_name(side->ui->curttype),
		t_type_name(side->ui->curbgttype));
	break;
      case bordersubtype:
	sprintf(buf, "%s (bord)", t_type_name(side->ui->curttype));
	break;
      case connectionsubtype:
	sprintf(buf, "%s (conn)", t_type_name(side->ui->curttype));
	break;
      case coatingsubtype:
	sprintf(buf, "%s (coat)", t_type_name(side->ui->curttype));
	break;
      default:
	terrain_subtype_warning("ttype update", side->ui->curttype);
	break;
    }
#if 0
    if (t_subtype(side->ui->curttype) == connectionsubtype) {
	/* draw a horizontal line */
	y = 5;
	h = 5;
    } else if (t_subtype(side->ui->curttype) == bordersubtype) {
	/* draw a vertical line */
	x = 5;
	w = 5;
    }

    /* clear the pixmap */
    XSetForeground(dpy, gc, side->ui->bgcolor);
    XFillRectangle(dpy, control->pic, gc, 0, 0, 16, 16);

    control->color = side->ui->cellcolor[side->ui->curttype];
    XSetForeground(dpy, gc, control->color);

    /* XSetTSOrigin coordinates are tweaked to get roads "right" */
    if (side->ui->monochrome && ximg && ximg->mono != None) {
	XSetFillStyle(dpy, gc, FillStippled);
	XSetStipple(dpy, gc, ximg->mono);
	XSetTSOrigin(dpy, gc, 2, 2);
    } else if (!side->ui->monochrome && side->ui->usecolorimages && ximg && ximg->colr != None) {
	XSetFillStyle(dpy, gc, FillTiled);
	XSetTile(dpy, gc, ximg->colr);
	XSetTSOrigin(dpy, gc, 2, 2);
    }
    XFillRectangle(dpy, control->pic, gc, x, y, w, h);
    XtVaSetValues(control->control, XtNbitmap, control->pic, NULL); 

    XSetFillStyle(dpy, gc, FillSolid);
#endif

    nargs = 0;
    XtSetArg(tmpargs[nargs], XtNlabel, buf);  nargs++;
#if 0 
    XtSetArg(tmpargs[nargs], XtNleftBitmap, pic);  nargs++;
#endif
    XtSetValues(side->ui->terrain_label, tmpargs, nargs);
}

void
update_curutype(Side *side)
{
    char buf[BUFSIZE];
    Pixmap pic;

    sprintf(buf, "%s (%s)",
 	    u_type_name(side->ui->curutype),
 	    side_adjective(side_n(side->ui->curusidenumber)));

    /* (should have a unit pixmap generator for this...) */
    pic = XCreatePixmap(side->ui->dpy, side->ui->rootwin,
			min_w_for_unit_image, min_h_for_unit_image,
			DefaultDepth(side->ui->dpy, side->ui->screen));
    /* (should include side emblem also) */
    XSetFillStyle(side->ui->dpy, side->ui->unitgc, FillSolid);
    XSetForeground(side->ui->dpy, side->ui->unitgc, side->ui->bgcolor);
    XFillRectangle(side->ui->dpy, pic,
		   side->ui->unitgc, 0, 0, 32, 32);
    draw_unit_image(uimages[side->ui->curutype], side, pic, 0, 0, 16, 16,
		    -1, 1, 0, 0);

    nargs = 0;
    XtSetArg(tmpargs[nargs], XtNlabel, buf);  nargs++;
    XtSetArg(tmpargs[nargs], XtNleftBitmap, pic);  nargs++;
    XtSetValues(side->ui->unit_label, tmpargs, nargs);
}

void
update_cursidenumber(Side *side)
{
    char buf[BUFSIZE];

    sprintf(buf, "%s (%d)",
	    side_adjective(side_n(side->ui->cursidenumber)),
	    side->ui->cursidenumber);
    XtVaSetValues(side->ui->people_label, XtNlabel, buf, NULL);
}

void
update_curfeature(Side *side)
{
    char buf[BUFSIZE], *rslt;
    Feature *feature;

    feature = find_feature(side->ui->curfid);
    rslt = feature_desc(feature, buf);
    if (rslt == NULL) {
	if (side->ui->curfid > 0) {
	    sprintf(buf, "??? (%d)", side->ui->curfid);
	    rslt = buf;
    	} else
	  rslt = "no feature";
    }
    XtVaSetValues(side->ui->feature_label, XtNlabel, rslt, NULL);
}

void
update_curbrushradius(Side *side)
{
    char buf[BUFSIZE];

    sprintf(buf, "brush radius = %d", side->ui->curbrushradius);
    XtVaSetValues(side->ui->brush_radius_label, XtNlabel, buf, NULL);
}

/* Given a location in a map, pick up whatever is there and make
   it the current terrain/unit/whatever, according to the design tool
   currently in use. */

void
set_designer_cur_from_map(Side *side, Map *map, int sx, int sy)
{
    int x, y;
    Unit *unit;

    if (!side->designer)
      return;
    if (!x_nearest_cell(side, map, sx, sy, &x, &y)) {
	beep(side);
	return;
    }
    switch (map->curtool) {
      case cellpainttool:
	side->ui->curttype = terrain_at(x, y);
	update_curttype(side);
	/* Do this also, because the terrain type change might necessitate
	   a tool cursor change.  If the type should not be
	   "cellpainttool", this will change it to the appropriate one. */
	set_design_tool(side, cellpainttool);
	break;
      case bordpainttool:
	/* (should make this work) */
	beep(side);
	break;
      case connpainttool:
	/* (should make this work) */
	beep(side);
	break;
      case unitaddtool:
	/* (should be unit exactly under sx,sy) */
        unit = unit_at(x, y);
	if (unit) {
	    side->ui->curutype = unit->type;
	    update_curutype(side);
	} else {
	    beep(side);
	}
	break;
      case peoplepainttool:
	if (people_sides_defined()) {
	    side->ui->cursidenumber = people_side_at(x, y);
	    update_cursidenumber(side);
	} else {
	    beep(side);
	}
	break;
      case featurepainttool:
	if (features_defined()) {
	    side->ui->curfid = raw_feature_at(x, y);
	    update_curfeature(side);
	} else {
	    beep(side);
	}
	break;
      default:
	/* error eventually */
	break;
    }
}

void
handle_designer_map_click(Side *side, Map *map, int sx, int sy)
{
    int ax, ay, bx, by, dir, x, y;
    int oldt, painttype;
    int oldpeop, paintpeop;
    int oldfid, paintfid;
    Unit *unit;

    if (!x_nearest_cell(side, map, sx, sy, &ax, &ay)) {
	beep(side);
	return;
    }
    switch (map->curtool) {
      case cellpainttool:
      case bordpainttool:
      case connpainttool:
	x = ax;  y = ay;
	switch (t_subtype(side->ui->curttype)) {
	  case cellsubtype:
	    /* Choose to paint fg or bg type, depending on what's already
	       there. */
	    oldt = terrain_at(x, y);
	    painttype = (side->ui->curttype == oldt ? side->ui->curbgttype :
			 side->ui->curttype);
	    net_paint_cell(side, x, y, side->ui->curbrushradius, painttype);
	    break;
	  case bordersubtype:
	    if (!x_nearest_boundary(side, map, sx, sy, &bx, &by, &dir)) {
		beep(side);
		return;
	    }
	    if (inside_area(bx, by)) {
		painttype = side->ui->curttype;
		net_paint_border(side, bx, by, dir, painttype, -1);
	    }
	    break;
	  case connectionsubtype:
	    if (!x_nearest_boundary(side, map, sx, sy, &bx, &by, &dir)) {
		beep(side);
		return;
	    }
	    if (inside_area(bx, by)) {
		painttype = side->ui->curttype;
		net_paint_connection(side, bx, by, dir, painttype, -1);
	    }
	    break;
	  case coatingsubtype:
	    break;
	  default:
	    case_panic("terrain subtype", t_subtype(side->ui->curttype));
	    break;
        }
	break;
      case unitaddtool:
	unit = net_designer_create_unit(side, side->ui->curutype,
					side->ui->cursidenumber, ax, ay);
	if (unit != NULL) {
		/* (should make it be current) */
	} else {
		/* Something's wrong. */
		beep(side);
	}
	break;
      case peoplepainttool:
	    oldpeop = people_side_at(ax, ay);
	    paintpeop =
	      (side->ui->cursidenumber == oldpeop ? NOBODY : side->ui->cursidenumber);
	    net_paint_people(side, ax, ay, side->ui->curbrushradius, paintpeop);
	break;
      case featurepainttool:
	oldfid = raw_feature_at(ax, ay);
	paintfid = (side->ui->curfid == oldfid ? 0 : side->ui->curfid);
	net_paint_feature(side, ax, ay, side->ui->curbrushradius, paintfid);
	break;
      default:
	/* complain or ignore? */
	break;
    }
}

void
popdown_design(Side *side)
{
    if (side->ui->design_shell)
      XtPopdown(side->ui->design_shell);
}

#endif /* DESIGNERS */

