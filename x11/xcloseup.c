/* Closeups for the X11 interface to Xconq.
   Copyright (C) 1995-1999 Massimo Campostrini and Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/*  1) unit closeup                        */
/*  2) side closeup                        */
/*  3) unit list                           */
/*  4) list (summary) of unit closeups     */

#include "conq.h"
#include "xtconq.h"
extern void create_orders_window(Side *side, Map *map);

static void buffer_append(int nl);
static void add_to_closeup_summary(UnitCloseup *closeup, Side *side);
static void remove_from_closeup_summary(UnitCloseup *closeup,
						 Side *side);
static void summary_list_callback(Widget w, XtPointer dummy,
					   XawListReturnStruct *list);
static void unit_closeup_update(Widget w, XtPointer cldata,
					 XtPointer cadata);
static void unit_closeup_focus(Widget w, XtPointer cldata,
					XtPointer cadata);
static void unit_closeup_close(Widget w, XtPointer cldata,
					XtPointer cadata);
static void side_closeup_update(Widget w, XtPointer cldata,
					 XtPointer cadata);
static void side_closeup_close(Widget w, XtPointer cldata,
					XtPointer cadata);
static void side_closeup_close(Widget w, XtPointer cldata,
					XtPointer cadata);
static void side_closeup_type_callback(Widget w, XtPointer cldata,
						XtPointer cadata);
static void side_closeup_all_types_callback(Widget w,
						     XtPointer cldata,
						     XtPointer cadata);
static void build_unit_list(Widget w, XtPointer cldata,
				     XtPointer cadata);
static void unit_list_callback(Widget w, XtPointer dummy,
					XawListReturnStruct *list);
static void unit_list_close(Widget w, XtPointer cldata,
				     XtPointer cadata);
static UnitList *find_unit_list_via_button(Widget w, Side *side);
void destroy_unit_list(Side *side, UnitList *unit_list);
char *pad_string(char *str, int n, int min);
void one_line_unit_summary(char *buf, Unit *unit, Side *side,
				    int pos);
void hp_and_acp_desc(char *buf, Unit *unit, char *sep);
void list_unit_types(char *buff, int nums[]); 

Widget new_order_widgets(Side *side, OrderInterface *ordi, int num,
				  Widget up);
void sorder_cond_name(char *buffer, StandingOrder *sorder,
			       Side *side);
void update_order_widgets(Side *side, OrderInterface *ordi, 
				   StandingOrder *sorder, char *types,
				   Task *task);
static void orders_help_call(Widget w, XtPointer cldata,
				      XtPointer cadata);
static void orders_done_help_call(Widget w, XtPointer cldata,
					   XtPointer cadata);
static void handle_done_orders_help(Widget w, XEvent *event,
					     String *params,
					     Cardinal *num_params);
static void handle_done_string_select(Widget w, XEvent *event,
					       String *params,
					       Cardinal *num_params);
static void orders_add_call(Widget w, XtPointer cldata,
				     XtPointer cadata);
static void orders_clone_call(Widget w, XtPointer cldata,
				       XtPointer cadata);
static void orders_add_named(Widget w, char *string);
static void orders_delete_call(Widget w, XtPointer cldata,
					XtPointer cadata);
static void orders_undo_call(Widget w, XtPointer cldata,
				      XtPointer cadata);
static void orders_save_call(Widget w, XtPointer cldata,
				      XtPointer cadata);
static void orders_restore_call(Widget w, XtPointer cldata,
					 XtPointer cadata);
static void orders_close_call(Widget w, XtPointer cldata,
				       XtPointer cadata);
static void order_toggle_call(Widget w, XtPointer cldata,
				       XtPointer cadata);
static void order_types_call(Widget w, XtPointer cldata,
				      XtPointer cadata);
static void order_etype_call(Widget w, XtPointer cldata,
				      XtPointer cadata);
static void order_eparms_call(Widget w, XtPointer cldata,
				       XtPointer cadata);
static void order_task_call(Widget w, XtPointer cldata,
				     XtPointer cadata);
static void order_tparms_call(Widget w, XtPointer cldata,
				       XtPointer cadata);
void set_cell_from_map(Side *side, Map *map, char *prompt,
				void *px, void *py, int size);
static void aux_set_cell(Side *side, Map *map, int cancel);
void set_unit_from_map(Side *side, Map *map, char *prompt,
				int *puid);
static void aux_set_unit(Side *side, Map *map, int cancel);
int find_side_and_ordi_via_widget(Widget w, Side **sidep,
					   OrderInterface **ordip);
void deactivate_orders(Side *side);
OrderInterface *active_ordi(Side *side);
int utype_select_popup(Side *side, Position x, Position y);
static void utype_select_call(Widget w, XtPointer cldata,
				       XtPointer cadata);
int dir_select_popup(Side *side, Position x, Position y);
static void dir_select_call(Widget w, XtPointer cldata,
				     XtPointer cadata);
int integer_select_popup(Side *side, Position x, Position y,
				  int init, char *prompt);
void string_select_popup(Side *side, Position x, Position y,
				  char *result, char *init, char *prompt);
int cond_select_popup(Side *side, Position x, Position y);
static void cond_select_call(Widget w, XtPointer cldata,
				      XtPointer cadata);
int task_select_popup(Side *side, Position x, Position y);
static void task_select_call(Widget w, XtPointer cldata,
				      XtPointer cadata);
int count_sorder(Side *side);

static void orders_check(Side *side);

static char buffer[200], *longbuffer = NULL;
/* initial lengths of arrays, they will grow when needed */
static int longbuffer_length = 1000, unitlist_size = 100, summary_size = 100;

#define OTHERFILTERS 4  /* filters not based on unit typs */
#define LISTLINE 80

/* Unit closeups. */

UnitCloseup *
find_unit_closeup(Side *side, Unit *unit)
{
    UnitCloseup *unitcloseup;
    
    for_all_unit_closeups(unitcloseup, side) {
	if (unitcloseup->unit == unit && unitcloseup->shell)
	  return unitcloseup;
    }
    return NULL;
}

UnitCloseup *
find_unit_closeup_via_button(Widget w, Side *side)
{
    UnitCloseup *unitcloseup;
    Widget shell = XtParent(XtParent(w));

    for_all_unit_closeups(unitcloseup, side) {
	if (unitcloseup->shell == shell)
	  return unitcloseup;
    }
    return NULL;
}

UnitCloseup *
create_unit_closeup(Side *side, Map *map, Unit *unit)
{
    Widget cmd, up, left, form;
    UnitCloseup *unitcloseup;
    Side *side2;

    if (!active_display(side) || unit == NULL)
      return NULL;
    unitcloseup = (UnitCloseup *) xmalloc(sizeof(UnitCloseup));
    side2 = unit->side;
    if (side2 == NULL)
      side2 = indepside;

    unitcloseup->unit = unit;
    unitcloseup->map = map;

    /* create the popup */
    strcpy(buffer, "Xconq closeup: ");
    strcat(buffer, unit_handle(side, unit));
    strcpy(spbuf, "Xconq: ");
    strcat(spbuf, short_unit_handle(unit));
    unitcloseup->shell =
      XtVaCreatePopupShell("unitCloseup", topLevelShellWidgetClass,
			   side->ui->shell,
			   XtNtitle, buffer,
			   XtNiconName, spbuf,
			   NULL);
    form =
      XtVaCreateManagedWidget("form", formWidgetClass, unitcloseup->shell,
			      NULL);
    left = up = 
      XtVaCreateManagedWidget("icon", labelWidgetClass, form,
			      XtNbitmap, get_unit_picture(unit->type, side),
			      XtNtop,    XawChainTop, 
			      XtNbottom, XawChainTop, 
			      XtNleft,   XawChainLeft, 
			      XtNright,  XawChainLeft,
			      NULL);
    left =
      XtVaCreateManagedWidget("label", labelWidgetClass, form,
			      XtNfromHoriz, left,
			      XtNlabel,  unit_handle(side, unit),
			      XtNtop,    XawChainTop, 
			      XtNbottom, XawChainTop, 
			      XtNleft,   XawChainLeft, 
			      XtNright,  XawChainLeft,
			      NULL);
    left =
      XtVaCreateManagedWidget("flag", labelWidgetClass, form,
			      XtNbitmap, get_side_picture(side, side2),
			      XtNlabel, "",
			      XtNfromHoriz, left,
			      XtNtop,    XawChainTop, 
			      XtNbottom, XawChainTop, 
			      XtNleft,   XawChainLeft, 
			      XtNright,  XawChainLeft,
			      NULL);
    left =
      XtVaCreateManagedWidget("side", labelWidgetClass, form,
			      XtNfromHoriz, left,
			      XtNlabel,  side_adjective(side2),
			      XtNtop,    XawChainTop, 
			      XtNbottom, XawChainTop, 
			      XtNleft,   XawChainLeft, 
			      XtNright,  XawChainLeft,
			      NULL);
    up = unitcloseup->info =
      XtVaCreateManagedWidget("info", asciiTextWidgetClass, form,
			      XtNfromVert, up,
			      XtNdisplayCaret, False,
			      XtNeditType, XawtextRead,
			      XtNscrollHorizontal, XawtextScrollWhenNeeded, 
			      XtNscrollVertical, XawtextScrollWhenNeeded,
			      XtNtop,    XawChainTop, 
			      XtNbottom, XawChainBottom, 
			      XtNleft,   XawChainLeft, 
			      XtNright,  XawChainRight, 
			      NULL);
    left = cmd =
      XtVaCreateManagedWidget("update", commandWidgetClass, form,
			      XtNfromVert,  up,
			      XtNtop,    XawChainBottom, 
			      XtNbottom, XawChainBottom, 
			      XtNleft,   XawChainLeft, 
			      XtNright,  XawChainLeft, 
			      NULL);
    XtAddCallback(cmd, XtNcallback, unit_closeup_update, NULL);
    left = cmd =
      XtVaCreateManagedWidget("focus", commandWidgetClass, form,
			      XtNfromHoriz, left,
			      XtNfromVert,  up,
			      XtNtop,    XawChainBottom, 
			      XtNbottom, XawChainBottom, 
			      XtNleft,   XawChainLeft, 
			      XtNright,  XawChainLeft, 
			      NULL);
    XtAddCallback(cmd, XtNcallback, unit_closeup_focus, NULL);
    cmd =
      XtVaCreateManagedWidget("close", commandWidgetClass, form,
			      XtNfromHoriz, left,
			      XtNfromVert,  up,
			      XtNtop,    XawChainBottom, 
			      XtNbottom, XawChainBottom, 
			      XtNleft,   XawChainLeft, 
			      XtNright,  XawChainLeft, 
			      NULL);
    XtAddCallback(cmd, XtNcallback, unit_closeup_close, NULL);

    /* insert in the side's unit closeup list */
    unitcloseup->next = side->ui->unitcloseuplist;
    side->ui->unitcloseuplist = unitcloseup;

    XtPopup(unitcloseup->shell, XtGrabNone);

    add_to_closeup_summary(unitcloseup, side);

    return unitcloseup;
}

void
destroy_unit_closeup(Side *side, UnitCloseup *unitcloseup)
{
    UnitCloseup *uc;

    XtPopdown(unitcloseup->shell);
    XtDestroyWidget(unitcloseup->shell);

    remove_from_closeup_summary(unitcloseup, side);

    /* remove from the side's unit closeup list */
    if (side->ui->unitcloseuplist == unitcloseup) {
	side->ui->unitcloseuplist = unitcloseup->next;
    } else {
	for_all_unit_closeups(uc,side) {
	    if (uc->next == unitcloseup) {
		uc->next = unitcloseup->next;
	    }
	}
    }
    free(unitcloseup);
}

/* conveniency routine to append the contents of "buffer" */
/* into "longbuffer", growing it if needed */
/* append a newline if nl is not zero */
static void 
buffer_append(int nl) 
{
    int len, longl;

    longl = strlen(longbuffer);
    if (nl) 
      strcat(buffer, "\n");
    len = strlen(buffer);

    if (len+longl > longbuffer_length-2) {
	longbuffer_length *= 2;
	longbuffer = (char *) realloc(longbuffer,
				      longbuffer_length*sizeof(char));
    }
    strcat(longbuffer, buffer);
    buffer[0] = '\0';  /* clear buffer */
}

void
draw_unit_closeup(Side *side, UnitCloseup *unitcloseup)
{
    int m, u, i;
    Unit *unit = unitcloseup->unit, *occupant;
    char *featurename;

    if (!active_display(side))
      return;
    if (!in_play(unit)
	|| (!side_controls_unit(side, unit) && !endofgame)) {
	/* If the unit is no longer alive and ours, shut down the window. */
	destroy_unit_closeup(side, unitcloseup);
	return;
    }

    if (longbuffer == NULL) {
	longbuffer = (char *) xmalloc(longbuffer_length*sizeof(char));
    }
    longbuffer[0] = '\0';

    /* Draw the unit's side and type. */
    strcpy(buffer, unit_handle(side, unit));
    pad_string(buffer, 40, 3);
    buffer_append(0);

    /* Draw the unit's HP, CP and ACP */
    hp_and_acp_desc(buffer, unit, "   ");
    buffer_append(1);

    /* Draw the unit's location. */
    if (unit->transport != NULL) {
	sprintf(buffer, "In %s (", short_unit_handle(unit->transport));
    }
    if (terrain_visible(side, unit->x, unit->y)) {
	sprintf(spbuf, "In %s", t_type_name(terrain_at(unit->x, unit->y)));
	linear_desc(spbuf, unit->x, unit->y);
	if (unit->transport != NULL) {
	    spbuf[0] = tolower(spbuf[0]);
	}
	strcat(buffer, spbuf);
	if (unit->transport == NULL) {
	    featurename = feature_name_at(unit->x, unit->y);
	    if (!empty_string(featurename))
	      tprintf(buffer, " (%s)", featurename);
	    if (temperatures_defined())
	      tprintf(buffer, " (T %d)", temperature_at(unit->x, unit->y));
	    if (elevations_defined())
	      tprintf(buffer, " (El %d)", elev_at(unit->x, unit->y));
	    /* (should list local weather also) */
	}
	strcat(buffer," ");
    }
    tprintf(buffer, "at %d,%d", unit->x, unit->y);
    if (unit->transport != NULL) {
	strcat(buffer,")");
    }
    pad_string(buffer, 40, 3);
    buffer_append(0);

    /* Draw the unit's supplies. */
    i = 0;
    for_all_material_types(m) {
	if (um_storage_x(unit->type, m) > 0) {
	    sprintf(buffer, "%s%s %d/%d", i ? ", " : "",
		    m_type_name(m), unit->supply[m],
		    um_storage_x(unit->type, m));
	    i = 1;
	    buffer_append(0);
	}
    }
    buffer_append(1);

    /* Draw the unit's toolup level (if any) */
    if (unit->tooling != NULL) {
	i = 0;
	for_all_unit_types(u) {
	    i = i || unit->tooling[u];
	}
	if (i) {
	    buffer_append(1);
	    strcpy(buffer, "Toolup level: ");
	    for_all_unit_types(u) {
		if (unit->tooling[u]) {
		    if (unit->tooling[u]<uu_tp_max(unit->type, u)) {
			tprintf(buffer, "%d/%d%1s ",
				unit->tooling[u], uu_tp_max(unit->type, u),
				utype_name_n(u, 1));
		    } else {
			tprintf(buffer, "%d%1s ", unit->tooling[u],
				utype_name_n(u, 1));
		    }
		}
	    }
	    buffer_append(1);
	}
    }
	
    /* Draw the unit's plan, if it has one. */
    if (unit->plan) {
	Task *task;
	Plan *plan = unit->plan;

	strcpy(buffer, "\nplan:  ");
	buffer_append(0);

	plan_desc(buffer, unit);

    	if (plan->maingoal) {
	    strcat(buffer, "   ");
	    /* (should use a "goal_desc" routine) */
	    strcat(buffer, goal_desig(plan->maingoal));
    	}
    	if (plan->formation) {
	    strcat(buffer, "   ");
	    /* (should use a "goal_desc" routine) */
	    strcat(buffer, goal_desig(plan->formation));
    	}
	buffer_append(1);
	if (plan->tasks) {
	    for (task = plan->tasks; task != NULL; task = task->next) {
		task_desc(buffer, unit->side, unit, task);
		buffer_append(1);
	    }
	}
    }

    /* Draw the unit's occupants */
    if (unit->occupant != NULL) {
	strcpy(buffer,"\n        Occupants:");
	buffer_append(1);
	for_all_occupants(unit,occupant) {
	    one_line_unit_summary(buffer, occupant, side, 0);
	    buffer_append(1);
	}
    }

    XtVaSetValues(unitcloseup->info, XtNstring, longbuffer, NULL);

    raise_widget(unitcloseup->shell);
}

/* Closeups for sides. */

SideCloseup *
find_side_closeup(Side *side, Side *side2)
{
    SideCloseup *sidecloseup;

    if (side2 == NULL)
      side2 = indepside;
    for_all_side_closeups(sidecloseup, side) {
	if (sidecloseup->side == side2
	    && sidecloseup->shell)
	  return sidecloseup;
    }
    return NULL;
}

SideCloseup *
find_side_closeup_via_button(Widget w, Side *side)
{
    SideCloseup *sidecloseup;
    Widget shell = XtParent(XtParent(w));

    for_all_side_closeups(sidecloseup, side) {
	if (sidecloseup->shell == shell)
	  return sidecloseup;
    }
    return NULL;
}

SideCloseup *
create_side_closeup(Side *side, Map *map, Side *side2)
{
    int u, fn;
    Widget cmd, up, left, box, form, radio;
    SideCloseup *sidecloseup;

    if (!active_display(side))
      return NULL;
    sidecloseup = (SideCloseup *) xmalloc(sizeof(SideCloseup));
    sidecloseup->filter =
      (Widget *) xmalloc((numutypes + OTHERFILTERS + 1) * sizeof(Widget));
    /* It is possible to have a closeup of the "independent side". */
    if (side2 == NULL)
      side2 = indepside;
    sidecloseup->side = side2;
    sidecloseup->map = map;
    /* Create the popup. */
    strcpy(buffer, "Xconq side closeup: ");
    strcat(buffer, short_side_title(side2));
    strcpy(spbuf, "Xconq: ");
    strcat(spbuf, short_side_title(side2));
    sidecloseup->shell =
      XtVaCreatePopupShell("sideCloseup", topLevelShellWidgetClass,
			   side->ui->shell,
			   XtNtitle, buffer,
			   XtNiconName, spbuf,
			   NULL);
    form =
      XtVaCreateManagedWidget("form", formWidgetClass, sidecloseup->shell,
			      NULL);
    left = up =
      XtVaCreateManagedWidget("flag", labelWidgetClass, form,
			      XtNbitmap, get_side_picture(side, side2),
			      XtNlabel, "",
			      XtNtop,    XawChainTop, 
			      XtNbottom, XawChainTop, 
			      XtNleft,   XawChainLeft, 
			      XtNright,  XawChainLeft,
			      NULL);

    left =
      XtVaCreateManagedWidget("side", labelWidgetClass, form,
			      XtNfromHoriz, left,
			      XtNlabel,  short_side_title(side2),
			      XtNtop,    XawChainTop, 
			      XtNbottom, XawChainTop, 
			      XtNleft,   XawChainLeft, 
			      XtNright,  XawChainLeft,
			      NULL);

    up = sidecloseup->info =
      XtVaCreateManagedWidget("info", asciiTextWidgetClass, form,
			      XtNfromVert, up,
			      XtNdisplayCaret, False,
			      XtNeditType, XawtextRead,
			      XtNscrollHorizontal, XawtextScrollWhenNeeded, 
			      XtNscrollVertical, XawtextScrollWhenNeeded,
			      XtNtop,    XawChainTop, 
			      XtNbottom, XawChainBottom, 
			      XtNleft,   XawChainLeft, 
			      XtNright,  XawChainRight, 
			      NULL);

    up = box =
      XtVaCreateManagedWidget("filter", boxWidgetClass, form,
			      XtNfromVert,  up,
			      XtNtop,    XawChainBottom, 
			      XtNbottom, XawChainBottom, 
			      XtNleft,   XawChainLeft, 
			      XtNright,  XawChainLeft, 
			      NULL);
    XtVaCreateManagedWidget("label", labelWidgetClass, box,
			    NULL);

    fn = 0;
    /* Unit type filters; managed by their callbacks. */
    sidecloseup->filter[fn] =
      XtVaCreateManagedWidget("allTypes", toggleWidgetClass, box,
			      XtNstate, True,
			      NULL);
    XtAddCallback(sidecloseup->filter[fn++], XtNcallback,
		  side_closeup_all_types_callback, NULL);

    for_all_unit_types(u) {
	sidecloseup->filter[fn] =
	  XtVaCreateManagedWidget(utype_name_n(u, 8), toggleWidgetClass, box,
				  XtNbitmap, get_unit_picture(u, side),
				  XtNstate, False,
				  NULL);
	XtAddCallback(sidecloseup->filter[fn++], XtNcallback,
		      side_closeup_type_callback, NULL);
    }
    /* Other filters; managed as a radio group (at most one is set). */
    radio = sidecloseup->filter[fn] =
      XtVaCreateManagedWidget("completed", toggleWidgetClass, box,
			      XtNstate, True,
			    NULL);
    XawToggleChangeRadioGroup(sidecloseup->filter[fn++], radio);
    sidecloseup->filter[fn] =
      XtVaCreateManagedWidget("waiting", toggleWidgetClass, box,
			      XtNstate, False,
			      NULL);
    XawToggleChangeRadioGroup(sidecloseup->filter[fn++], radio);
    sidecloseup->filter[fn] =
      XtVaCreateManagedWidget("acpLeft", toggleWidgetClass, box,
			      XtNstate, False,
			      NULL);
    XawToggleChangeRadioGroup(sidecloseup->filter[fn++], radio);

    left = cmd =
      XtVaCreateManagedWidget("update", commandWidgetClass, form,
			      XtNfromVert,  up,
			      XtNtop,    XawChainBottom, 
			      XtNbottom, XawChainBottom, 
			      XtNleft,   XawChainLeft, 
			      XtNright,  XawChainLeft, 
			      NULL);
    XtAddCallback(cmd, XtNcallback, side_closeup_update, NULL);

    left = cmd =
      XtVaCreateManagedWidget("listUnits", commandWidgetClass, form,
			      XtNfromHoriz, left,
			      XtNfromVert,  up,
			      XtNtop,    XawChainBottom, 
			      XtNbottom, XawChainBottom, 
			      XtNleft,   XawChainLeft, 
			      XtNright,  XawChainLeft, 
			      NULL);
    XtAddCallback(cmd, XtNcallback, build_unit_list, NULL);

    cmd =
      XtVaCreateManagedWidget("close", commandWidgetClass, form,
			      XtNfromHoriz, left,
			      XtNfromVert,  up,
			      XtNtop,    XawChainBottom, 
			      XtNbottom, XawChainBottom, 
			      XtNleft,   XawChainLeft, 
			      XtNright,  XawChainLeft, 
			      NULL);
    XtAddCallback(cmd, XtNcallback, side_closeup_close, NULL);

    /* Insert in the side's side closeup list, */
    sidecloseup->next = side->ui->sidecloseuplist;
    side->ui->sidecloseuplist = sidecloseup;
    /* Make the window appear. */
    XtPopup(sidecloseup->shell, XtGrabNone);
    return sidecloseup;
}

void
destroy_side_closeup(Side *side, SideCloseup *sidecloseup)
{
    SideCloseup *tmpcloseup;

    /* Make the window go away. */
    XtPopdown(sidecloseup->shell);
    XtDestroyWidget(sidecloseup->shell);
    /* Unsplice from the side's side closeup list. */
    if (side->ui->sidecloseuplist == sidecloseup) {
	side->ui->sidecloseuplist = sidecloseup->next;
    } else {
	for_all_side_closeups(tmpcloseup, side) {
	    if (tmpcloseup->next == sidecloseup) {
		tmpcloseup->next = sidecloseup->next;
	    }
	}
    }
    /* Free up any allocated memory. */
    free(sidecloseup);
}

void 
draw_side_closeup(Side *side, SideCloseup *sidecloseup)
{
    int u, i;
    int *numc;
    int *numi;

    Side *side1 = sidecloseup->side, *side2;
    Unit *unit;

    numc = (int *)xmalloc(sizeof (numc[0]) * numutypes);
    numi = (int *)xmalloc(sizeof (numi[0]) * numutypes);

    if (!active_display(side))
      return;

    if (longbuffer == NULL)
      longbuffer = (char *) xmalloc(longbuffer_length * sizeof(char));
    longbuffer[0] = '\0';

    /* draw side name and class */
    strcpy(buffer, short_side_title(side1));
    if (side1->sideclass) {
	tprintf(buffer, " (%s)", side1->sideclass);
    }
    buffer_append(1);
    buffer_append(1);

    /* count and draw side's units (completed and incomplete) */
    /* (should be generic code?) */
    for_all_unit_types(u) {
	numc[u] = 0;
	numi[u] = 0;
    }
    for_all_side_units(side1,unit) {
	if (completed(unit)) {
	    numc[unit->type]++;
	} else {
	    numi[unit->type]++;
	}
    }
    strcpy(buffer, "Completed units: ");
    list_unit_types(buffer, numc);
    buffer_append(1);
    strcpy(buffer, "Units under construction: ");
    list_unit_types(buffer, numi);
    buffer_append(1);

    /* list side's tech level (if any) */
    /* (should be generic code) */
    if (side1->tech != NULL) {
	i = 0;
	for_all_unit_types(u) {
	    i = i || side1->tech[u];
	}
	if (i) {
	    buffer_append(1);
	    strcpy(buffer, "Tech levels: ");
	    for_all_unit_types(u) {
		if (side1->tech[u]) {
		    if (side1->tech[u]<u_tech_max(u)) {
			tprintf(buffer, "%d/%d%1s ",
				side1->tech[u], u_tech_max(u),
				utype_name_n(u, 1));
		    } else {
			tprintf(buffer, "%d%1s ", side1->tech[u],
				utype_name_n(u, 1));
		    }
		}
	    }
	    buffer_append(1);
	}
    }

    /* list trusted sides (if any) */
    /* (should be generic code) */
    i = 0;
    for_all_sides(side2) 
      i = i || (trusted_side(side1, side2) && side1 != side2);

    if (i) {
	buffer_append(1);
	strcpy(buffer, "Trusted sides: ");
	for_all_sides(side2) {
	    if (trusted_side(side1, side2) && side1 != side2) 
	      tprintf(buffer, "%s, ", side_name(side2));
	}
	/* chop last two characters */
	buffer[strlen(buffer)-2] = '\0';
	buffer_append(1);
    }

    XtVaSetValues(sidecloseup->info, XtNstring, longbuffer, NULL);

    raise_widget(sidecloseup->shell);

    free (numc);
    free (numi);
}

/* manage a list (summary) of all unit closeups of a side */

static void
add_to_closeup_summary(UnitCloseup *unitcloseup, Side *side)
{
    CloseupSummary *summary = side->ui->closeupsummary;
    Widget form, port;
    char *label;

    if (summary == NULL) {
	/* initialize and popup the summary */
	summary = side->ui->closeupsummary =
	  (CloseupSummary *) xmalloc(sizeof(CloseupSummary));
	summary->unitcloseups =
	  (UnitCloseup **) xmalloc(summary_size*sizeof(UnitCloseup *));
	summary->number = 0;
	summary->labels = (char **) xmalloc(summary_size*sizeof(char *));
	
	summary->shell =
	  XtVaCreatePopupShell("closeupList", topLevelShellWidgetClass,
			       side->ui->shell,
			       NULL);
	form =
	  XtVaCreateManagedWidget("form", formWidgetClass, summary->shell,
				  NULL);
	summary->label =
	  XtVaCreateManagedWidget("label", labelWidgetClass, form,
				  /* reserve space for label */
				  XtNlabel,  "9999 unit closeups",
				  XtNtop,    XawChainTop, 
				  XtNbottom, XawChainTop, 
				  XtNleft,   XawChainLeft, 
				  XtNright,  XawChainLeft,
				  NULL);
	port = 
	  XtVaCreateManagedWidget("port", viewportWidgetClass, form,
				  XtNfromVert, summary->label,
				  XtNallowVert, True,
				  XtNtop,    XawChainTop, 
				  XtNbottom, XawChainBottom, 
				  XtNleft,   XawChainLeft, 
				  XtNright,  XawChainRight, 
				  NULL);
	summary->list =
	  XtVaCreateManagedWidget("list", listWidgetClass, port,
				  XtNlongest, 0,
				  NULL);
	XtAddCallback(summary->list, XtNcallback,
		      (XtCallbackProc) summary_list_callback,
		      (XtPointer) NULL);

	XtPopup(summary->shell, XtGrabNone);
    }

    summary->number++;
    if (summary->number + 1 > summary_size) {
	/* grow arrays */
	summary_size *= 2;
	summary->unitcloseups =
	  (UnitCloseup **) realloc(summary->unitcloseups,
				   summary_size*sizeof(UnitCloseup *));
	summary->labels =
	  (char **) realloc(summary->labels,
			    summary_size*sizeof(char **));
    }

    /* append new closeup */
    summary->unitcloseups[summary->number - 1] = unitcloseup;
    label = summary->labels[summary->number - 1] =
      (char *)xmalloc((LISTLINE + 1) * sizeof(char));
    one_line_unit_summary(buffer, unitcloseup->unit, side, 1);
    strncpy(label, buffer, LISTLINE);
    label[LISTLINE] = '\0';

    XawListChange(summary->list, summary->labels, summary->number, 0, True);

    sprintf(buffer, "%d unit closeup%s", summary->number,
	    (summary->number == 1) ? "" : "s");
    XtVaSetValues(summary->label, XtNlabel, buffer, NULL);

    /* don't raise this window */
}

static void
remove_from_closeup_summary(UnitCloseup *unitcloseup, Side *side)
{
    CloseupSummary *summary = side->ui->closeupsummary;
    int i, found;

    /* find and remove closeup */
    found = 0;
    for (i = 0; i < summary->number; i++) {
	if (summary->unitcloseups[i] == unitcloseup) {
	    found = 1;
	    free(summary->labels[i]);
	}
	if (found) {
	    summary->unitcloseups[i] = summary->unitcloseups[i+1];
	    summary->labels[i] = summary->labels[i+1];
	}
    }
    summary->number--;

    XawListChange(summary->list, summary->labels, summary->number, 0, True);

    sprintf(buffer, "%d unit closeup%s", summary->number,
	    (summary->number == 1) ? "" : "s");
    XtVaSetValues(summary->label, XtNlabel, buffer, NULL);

    if (summary->number == 0) {
	/* destroy window and cleanup summary */
	XtPopdown(summary->shell);
	XtDestroyWidget(summary->shell);
	free(summary->unitcloseups);
	free(summary->labels);
	free(side->ui->closeupsummary);
	side->ui->closeupsummary = NULL;
    }
}

/* callback for closeup list */

static void 
summary_list_callback(Widget w, XtPointer dummy, XawListReturnStruct *list)
{
    Side *side;

    if (!find_side_via_widget(w, &side))
      return;

    draw_unit_closeup(side,
		      side->ui->closeupsummary->unitcloseups[list->list_index]);
}

/* Redo the information in the unit's closeup. */

static void 
unit_closeup_update(Widget w, XtPointer client_data, XtPointer call_data)
{
    Side *side;
    UnitCloseup *unitcloseup;

    if (!find_side_via_widget(w, &side))
      return;

    unitcloseup = find_unit_closeup_via_button(w, side);
    draw_unit_closeup(side, unitcloseup);
}

/* Raise the appropriate map and focus on the unit in the closeup. */

static void 
unit_closeup_focus(Widget w, XtPointer client_data, XtPointer call_data)
{
    Side *side;
    Map *map;
    UnitCloseup *unitcloseup;

    if (!find_side_via_widget(w, &side))
      return;

    unitcloseup = find_unit_closeup_via_button(w, side);
    draw_unit_closeup(side, unitcloseup);
    for_all_maps(side, map) {
	if (map == unitcloseup->map) {
	    /* unitcloseup->map is up */
	    set_current_unit(side, map, unitcloseup->unit);
	    raise_widget(XtParent(map->mainwidget));
	    return;
	}
    }
    /* choose another map */
    if (side->ui->maps == NULL)
      return;

    set_current_unit(side, side->ui->maps, unitcloseup->unit);
    raise_widget(XtParent(side->ui->maps->mainwidget));
}

static void 
unit_closeup_close(Widget w, XtPointer client_data, XtPointer call_data)
{
    Side *side;
    UnitCloseup *unitcloseup;

    if (!find_side_via_widget(w, &side))
      return;

    unitcloseup = find_unit_closeup_via_button(w, side);
    destroy_unit_closeup(side, unitcloseup);
}

/* callbacks for side closeups */

static void 
side_closeup_close(Widget w, XtPointer client_data, XtPointer call_data)
{
    Side *side;
    SideCloseup *sidecloseup;

    if (!find_side_via_widget(w, &side))
      return;

    sidecloseup = find_side_closeup_via_button(w, side);
    destroy_side_closeup(side, sidecloseup);
}

static void 
side_closeup_update(Widget w, XtPointer client_data, XtPointer call_data)
{
    Side *side;
    SideCloseup *sidecloseup;

    if (!find_side_via_widget(w, &side))
      return;

    sidecloseup = find_side_closeup_via_button(w, side);
    draw_side_closeup(side, sidecloseup);
}

/* if a unit type filter is toggled, clear the "all types" filter */

static void 
side_closeup_type_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
    Side *side;
    SideCloseup *sidecloseup;

    if (!find_side_via_widget(w, &side))
      return;

    sidecloseup = find_side_closeup_via_button(XtParent(w), side);
    if (!sidecloseup)
      return;
    XtVaSetValues(sidecloseup->filter[0], XtNstate, False, NULL);
}

/* if the "all types" filter is set, clear all unit type filters */
/* if the "all types" filter is cleared, */
/* clear all unit type filters but set the first one */

static void 
side_closeup_all_types_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
    Side *side;
    SideCloseup *sidecloseup;
    int u;
    Boolean state;

    if (!find_side_via_widget(w, &side))
      return;

    sidecloseup = find_side_closeup_via_button(XtParent(w), side);
    if (sidecloseup == NULL)
      return;
    XtVaGetValues(sidecloseup->filter[0], XtNstate, &state, NULL);

    for_all_unit_types(u) {
	XtVaSetValues(sidecloseup->filter[u+1], XtNstate,
		      u==0 && !state, NULL);
    }
}

/* manage unit lists */

static void 
build_unit_list(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget form, left, up;
    Side *side, *side1;
    SideCloseup *sidecloseup;
    Unit *unit;
    UnitList *unit_list;
    int i, u, size;
    Boolean *filters;

    if (!find_side_via_widget(w, &side))
      return;

    filters = (Boolean*)xmalloc (sizeof (filters[0]) * (numutypes + OTHERFILTERS + 1));
    sidecloseup = find_side_closeup_via_button(w, side);
    for (i = 0; i < numutypes + OTHERFILTERS; ++i) {
	XtVaGetValues(sidecloseup->filter[i], XtNstate, &filters[i], NULL);
    }
    side1 = sidecloseup->side;
    unit_list = (UnitList *) xmalloc(sizeof(UnitList));
    unit_list->map = sidecloseup->map;
    unit_list->units  = (Unit **) xmalloc(unitlist_size * sizeof(Unit **));
    unit_list->labels = (char **) xmalloc(unitlist_size * sizeof(char **));

    /* build the filtered unit list */
    size = 0;
    for_all_side_units(side1, unit) {
	if (!alive(unit))
	  continue;
	/* right type? */
	if (!filters[0] && !filters[unit->type+1])
	  continue;
	/* completed? */
	if (filters[numutypes + 1] && !completed(unit))
	  continue;
	/* waiting for orders? */
	if (filters[numutypes + 2] && 
	    !(unit->plan &&
	      !unit->plan->asleep &&
	      !unit->plan->reserve &&
	      !unit->plan->delayed &&
	      unit->plan->waitingfortasks))
	  continue;
	/* acp left? */
	if ((filters[numutypes+2] || filters[numutypes+3]) &&
	    !has_acp_left(unit))
	  continue;

	/* unit is OK */

	if (size + 1 > unitlist_size) {
	    /* grow arrays */
	    unitlist_size *= 2;
	    unit_list->units =
	      (Unit **) realloc(unit_list->units,
				unitlist_size*sizeof(Unit **));
	    unit_list->labels =
	      (char **) realloc(unit_list->labels,
				unitlist_size*sizeof(char **));
	}

	/* append unit to list */
	unit_list->units[size] = unit;
	unit_list->labels[size] = (char *)xmalloc((LISTLINE+1)*sizeof(char));
	one_line_unit_summary(buffer, unit, side, 1);
	strncpy(unit_list->labels[size], buffer, LISTLINE);
	unit_list->labels[size][LISTLINE] = '\0';
	size++;
    }

    unit_list->number = size;

    /* create the popup */
    strcpy(buffer, "Xconq unit list: ");
    strcat(buffer, side_adjective(side1));
    unit_list->shell =
      XtVaCreatePopupShell("unitList", topLevelShellWidgetClass,
			   side->ui->shell,
			   XtNtitle, buffer,
			   XtNiconName, buffer,
			   NULL);

    form =
      XtVaCreateManagedWidget("form", formWidgetClass, unit_list->shell,
			      NULL);
    left = up =
      XtVaCreateManagedWidget("flag", labelWidgetClass, form,
			      XtNbitmap, get_side_picture(side, side1),
			      XtNlabel, "",
			      XtNtop,    XawChainTop, 
			      XtNbottom, XawChainTop, 
			      XtNleft,   XawChainLeft, 
			      XtNright,  XawChainLeft,
			      NULL);
    left =
      XtVaCreateManagedWidget("side", labelWidgetClass, form,
			      XtNfromHoriz, left,
			      XtNlabel,  side_adjective(side1),
			      XtNtop,    XawChainTop, 
			      XtNbottom, XawChainTop, 
			      XtNleft,   XawChainLeft, 
			      XtNright,  XawChainLeft,
			      NULL);
    sprintf(buffer,"%d unit%s", size, ((size == 1) ? "" : "s"));
    unit_list->label =
      XtVaCreateManagedWidget("label", labelWidgetClass, form,
			      XtNlabel,  buffer,
			      XtNfromHoriz, left,
			      XtNtop,    XawChainTop, 
			      XtNbottom, XawChainTop, 
			      XtNleft,   XawChainLeft, 
			      XtNright,  XawChainLeft,
			      NULL);

    /* show the filters used to produce the list */
    left = NULL;
    if (filters[0]) {
	left =
	  XtVaCreateManagedWidget("allTypes", labelWidgetClass, form,
				  XtNfromVert, up,
				  XtNtop,    XawChainTop, 
				  XtNbottom, XawChainTop, 
				  XtNleft,   XawChainLeft, 
				  XtNright,  XawChainLeft,
				  NULL);
    } else {
	for_all_unit_types(u) {
	    if (filters[u+1]) {
		left =
		  XtVaCreateManagedWidget(utype_name_n(u,8),
					  labelWidgetClass, form,
					  XtNfromHoriz, left,
					  XtNfromVert, up,
					  XtNbitmap, get_unit_picture(u, side),
					  XtNtop,    XawChainTop, 
					  XtNbottom, XawChainTop, 
					  XtNleft,   XawChainLeft, 
					  XtNright,  XawChainLeft,
					  NULL);
	    }
	}
    }
    if (filters[numutypes+1]) {
	left =
	  XtVaCreateManagedWidget("completed", labelWidgetClass, form,
				  XtNfromHoriz, left,
				  XtNfromVert, up,
				  XtNtop,    XawChainTop, 
				  XtNbottom, XawChainTop, 
				  XtNleft,   XawChainLeft, 
				  XtNright,  XawChainLeft,
				  NULL);
    }
    if (filters[numutypes+2]) {
	left =
	  XtVaCreateManagedWidget("waiting", labelWidgetClass, form,
				  XtNfromHoriz, left,
				  XtNfromVert, up,
				  XtNtop,    XawChainTop, 
				  XtNbottom, XawChainTop, 
				  XtNleft,   XawChainLeft, 
				  XtNright,  XawChainLeft,
				  NULL);
    }
    if (filters[numutypes+3]) {
	    left =
	      XtVaCreateManagedWidget("acpLeft", labelWidgetClass, form,
				      XtNfromHoriz, left,
				      XtNfromVert, up,
				      XtNtop,    XawChainTop, 
				      XtNbottom, XawChainTop, 
				      XtNleft,   XawChainLeft, 
				      XtNright,  XawChainLeft,
				      NULL);
	}
    if (left != NULL)
      up = left;

    if (size) {
	/* we found some units, show them */
	up =
	  XtVaCreateManagedWidget("port", viewportWidgetClass, form,
				  XtNfromVert, up,
				  XtNallowVert, True,
				  XtNtop,    XawChainTop, 
				  XtNbottom, XawChainBottom, 
				  XtNleft,   XawChainLeft, 
				  XtNright,  XawChainRight, 
				  NULL);
	unit_list->list =
	  XtVaCreateManagedWidget("list", listWidgetClass, up,
				  XtNlongest, 0,
				  NULL);
	XawListChange(unit_list->list, unit_list->labels,
		      size, 0, True);
	XtAddCallback(unit_list->list, XtNcallback,
		      (XtCallbackProc) unit_list_callback, (XtPointer) NULL);
    } else {
	/* we found no units, show a warning */
	unit_list->list = NULL;
	up =
	  XtVaCreateManagedWidget("failed", labelWidgetClass, form,
				  XtNfromVert, up,
				  XtNtop,    XawChainTop, 
				  XtNbottom, XawChainBottom, 
				  XtNleft,   XawChainLeft, 
				  XtNright,  XawChainLeft, 
				  NULL);
    }
    left = unit_list->close =
      XtVaCreateManagedWidget("close", commandWidgetClass, form,
			      XtNfromVert,  up,
			      XtNtop,    XawChainBottom, 
			      XtNbottom, XawChainBottom, 
			      XtNleft,   XawChainLeft, 
			      XtNright,  XawChainLeft, 
			      NULL);
    XtAddCallback(unit_list->close, XtNcallback, unit_list_close, NULL);

    XtPopup(unit_list->shell, XtGrabNone);

    /* append to list */
    unit_list->next = side->ui->unitlistlist;
    side->ui->unitlistlist = unit_list;

    free(filters);
}

static void 
unit_list_callback(Widget w, XtPointer dummy, XawListReturnStruct *list)
{
    Side *side;
    UnitList *unitlist;
    UnitCloseup *unitcloseup;
    Unit *unit;
    Map *map;
    int i;

    if (!find_side_via_widget(w, &side))
      return;
    unitlist = find_unit_list_via_button(XtParent(w), side);
    if (unitlist == NULL)
      return;

    unit = unitlist->units[list->list_index];
    i = 0;
    for_all_maps(side, map) {
	if (map == unitlist->map)
	  i = 1;
    }
    if (i) {
	map = unitlist->map;
    } else {
	/* unitlist->map not found */
	map = side->ui->maps;
    }
    if (map == NULL)
      return;

    unitcloseup = find_unit_closeup(side, unit);
    if (!unitcloseup)
      /* should never happen */
      unitcloseup = create_unit_closeup(side, map, unit);
    draw_unit_closeup(side, unitcloseup);
}

static void
unit_list_close(Widget w, XtPointer client_data, XtPointer call_data)
{
    Side *side;
    UnitList *unitlist;

    if (!find_side_via_widget(w, &side))
      return;

    unitlist = find_unit_list_via_button(w, side);
    destroy_unit_list(side, unitlist);
}

void
destroy_unit_list(Side *side, UnitList *unit_list)
{
    UnitList *unitlist;
    int i;

    XtPopdown(unit_list->shell);
    XtDestroyWidget(unit_list->shell);

    /* Unsplice from the side's list. */
    if (side->ui->unitlistlist == unit_list) {
	side->ui->unitlistlist = unit_list->next;
    } else {
	for_all_unit_lists(unitlist,side) {
	    if (unitlist->next == unit_list) {
		unitlist->next = unit_list->next;
	    }
	}
    }
    /* Release the memory used. */
    for (i = 0; i < unit_list->number; i++) 
      free(unit_list->labels[i]);
    free(unit_list->units);
    free(unit_list->labels);
    free(unit_list);
}

static UnitList *
find_unit_list_via_button(Widget w, Side *side)
{
    UnitList *unitlist;
    Widget shell = XtParent(XtParent(w));

    for_all_unit_lists(unitlist, side) {
	if (unitlist->shell == shell)
	  return unitlist;
    }
    return NULL;
}

/* miscellanea of utilities; to be moved in the appropriate files */

/* pad str with blanks to reach a length n */
/* if min is positive, add a minimum of min blanks */
/* if min is negative, chop to n character, */
/* including a minimum of |min| blanks */

char *
pad_string(char *str, int n, int minim)
{
    int i;
    int len = strlen(str);
    int lim = ((minim >= 0) ? max(n, len + minim) : n);

    for (i = len; i < lim; i++) {
	str[i] = ' ';
    }
    str[lim] = '\0';
    if (minim < 0) {
	for (i = n + minim; i < n; i++) {
	   str[i] = ' ';
       }
    } 
    return str;
}

void
hp_and_acp_desc(char *buf, Unit *unit, char *sep)
{
    /* Draw the unit's hit points and CP. */
    hp_desc(spbuf, unit, TRUE);
    strcat(buf, spbuf);

    /* Draw the unit's current ACP, if applicable. */
    if (u_acp(unit->type) > 0) {
	strcat(buf, sep);
	acp_desc(spbuf, unit, TRUE);
	strcat(buf, spbuf);
    }
}

/* a one-line description of unit and occupants */
/* add position info if pos!=0 */

void
one_line_unit_summary(char *buf, Unit *unit, Side *side, int pos)
{
    Unit *occupant;
    int u;

    strcpy(buf, unit_handle(side, unit));
    if (pos) {
	if (unit->transport != NULL) {
	    tprintf(buf, " in %s", short_unit_handle(unit->transport));
	} else {
	    tprintf(buf, " in %s at %d,%d",
		    t_type_name(terrain_at(unit->x, unit->y)),
		    unit->x, unit->y);
	}
    }
    pad_string(buf, pos ? 45 : 40, -2);
    hp_and_acp_desc(buf, unit, "  ");
    if (unit->occupant != NULL) {
	/* Very briefly list the numbers and types of the occupants. */
	strcat(buf, "  Occ ");
	for_all_unit_types(u)
	    tmp_u_array[u] = 0;
	for_all_occupants(unit, occupant)
	    tmp_u_array[occupant->type]++;
	list_unit_types(buf, tmp_u_array);
    }
}

/* produce a list of the number of units for each type */
/* from the given number array */

void
list_unit_types(char *buff, int nums[])
{
    int u, total = 0;

    for_all_unit_types(u) {
	if (nums[u] > 0) {
	    total += nums[u];
	    tprintf(buffer, "%d%1s ", nums[u], utype_name_n(u, 1));
	}
    }
    if (!total) {
	strcat(buff, "none  ");
    }
}

/* cache and return a unit picture */

Pixmap 
get_unit_picture(int u, Side *side)
{
    Pixmap pic;
    Display *dpy = side->ui->dpy;

    if (side->ui->unitpics[4][u] == None) {
	pic = XCreatePixmap(dpy, side->ui->rootwin,
			    min_w_for_unit_image, min_h_for_unit_image,
			    DefaultDepth(dpy, side->ui->screen));
	XFillRectangle(dpy, pic, side->ui->gc,
		       0, 0, min_w_for_unit_image, min_h_for_unit_image);
	XSetForeground(dpy, side->ui->gc, side->ui->fgcolor);
	draw_unit_image(uimages[u], side, pic, 0, 0,
			min_w_for_unit_image, min_h_for_unit_image,
			-1, -1, -1, 0);
	side->ui->unitpics[4][u] = pic;
    }
    return side->ui->unitpics[4][u];
}

/* cache and return a side picture (flag) */
/* should use mask somehow */

Pixmap 
get_side_picture(Side *side, Side *side1)
{
    int s = side_number(side1);
    Pixmap pic;
    Image *img;
    Display *dpy = side->ui->dpy;

    if (side->ui->emblempics[s] == None) {
	img = best_image(side->ui->eimages[s],
			 min_w_for_unit_image, min_h_for_unit_image);
	if (img == NULL)
	  return None;
	pic = XCreatePixmap(dpy, side->ui->rootwin, img->w, img->h,
			    DefaultDepth(dpy, side->ui->screen));
	XSetForeground(dpy, side->ui->gc, side->ui->bgcolor);
	XFillRectangle(dpy, pic, side->ui->gc,
		       0, 0, img->w, img->h);
	XSetForeground(dpy, side->ui->gc, side->ui->fgcolor);
	draw_side_emblem(side, pic, 0, 0, img->w, img->h, s, 0);
	side->ui->emblempics[s] = pic;
    }
    return side->ui->emblempics[s];
}

/* standing order support */

#define ORDER_COND_NUM 4
static char* order_cond[ORDER_COND_NUM] =
    { " none ", "  at  ", "  in  ", "within" };

static XtActionsRec xorders_actions[] = {
  { "DoneOrdersHelp", handle_done_orders_help },
  { "DoneStringSelect", handle_done_string_select },
};

static char sorderfile[] = "standing.orders";

static String helpText =
"Only one order at a time can be active (i.e. it can be edited);\n\
it is marked by an highlighted border.\n\
Clicking on the leftmost button of an order will toggle its activity.\n\
Deactivating an order will apply all changes.\n\
\n\
Clicking on an element of an active order will edit it.\n\
Clicking on an element of an inactive order will visualize it:\n\
if it is a cell address, the cell will be made current.\n\
\n\
The \"add\" button will add a new order.\n\
The \"clone\" button will clone the active order.\n\
The \"delete\" button will delete the active order.\n\
The \"undo\" button will cancel all the changes to the active order.\n\
The \"save\" button will save all the orders into a file.\n\
The \"restore\" button will delete all orders, then restore\n\
    saved orders from a file.\n\
The \"close\" button will popdown the orders window.";

void
create_orders_window(Side *side, Map *map)
{
    Widget form, up, left, port;
    StandingOrder *sorder;
    OrderInterface *ordi, **ordilist;
    int num, i;

    if (side->ui->sorder_edit == NULL)
      side->ui->sorder_edit = (StandingOrder *) xmalloc(sizeof(StandingOrder));
    if (side->ui->sorder_types_edit == NULL)
      side->ui->sorder_types_edit = (char *) xmalloc(numutypes*sizeof(char));
    if (side->ui->sorder_task_edit == NULL)
      side->ui->sorder_task_edit = (Task *) xmalloc(sizeof(Task));

    if (side->ui->orderlist == NULL) {
	/* build the order list */
	ordi = side->ui->orderlist;
	for (sorder = side->orders; sorder != NULL; sorder = sorder->next) {
	    if (side->ui->orderlist) {
		ordi->next =
		    (OrderInterface *) xmalloc(sizeof(OrderInterface));
		ordi = ordi->next;
	    } else {
		side->ui->orderlist =
		    (OrderInterface *) xmalloc(sizeof(OrderInterface));
		ordi = side->ui->orderlist;
	    }
	    ordi->sorder = sorder;
	}
    } 
    orders_check(side);

    num = count_sorder(side);

    if (side->ui->orders_shell == NULL) {
        XtAppAddActions(thisapp, xorders_actions, XtNumber(xorders_actions));

	/* initialize the window */
	side->ui->orders_shell =
	  XtVaCreatePopupShell("orders", topLevelShellWidgetClass,
			       side->ui->shell,
			       NULL);
	form =
	  XtVaCreateManagedWidget("form", formWidgetClass,
				  side->ui->orders_shell,
				  NULL);
	side->ui->orders_label =
	  XtVaCreateManagedWidget("label", labelWidgetClass, form,
				  /* reserve space for label */
				  XtNlabel,  "9999 standing orders",
				  XtNtop,    XawChainTop, 
				  XtNbottom, XawChainTop, 
				  XtNleft,   XawChainLeft, 
				  XtNright,  XawChainLeft,
				  NULL);
	port = 
	  XtVaCreateManagedWidget("port", viewportWidgetClass, form,
				  XtNfromVert, side->ui->orders_label,
				  XtNallowVert, True,
				  XtNtop,    XawChainTop, 
				  XtNbottom, XawChainBottom, 
				  XtNleft,   XawChainLeft, 
				  XtNright,  XawChainRight, 
				  NULL);
	side->ui->orders_form =
	  XtVaCreateManagedWidget("orders", formWidgetClass,
				  port,
				  NULL);

	/* reverse list order */
	if (num > 0) {
	    ordilist = (OrderInterface **) xmalloc(num*sizeof(OrderInterface));
	    for (ordi = side->ui->orderlist,  i = 0;
		 ordi != NULL;
		 ordi = ordi->next,  i++) {
		ordilist[num-1-i] = ordi;
	    }
	    up = NULL;
	    for (i = 0; i<num; i++) {
		up = new_order_widgets(side, ordilist[i], i, up);
	    }
	    free(ordilist);
	}

	left =
	  XtVaCreateManagedWidget("help", commandWidgetClass, form,
				  XtNfromVert,  port,
				  XtNtop,    XawChainBottom, 
				  XtNbottom, XawChainBottom, 
				  XtNleft,   XawChainLeft, 
				  XtNright,  XawChainLeft, 
				  NULL);
	XtAddCallback(left, XtNcallback, orders_help_call, NULL);
	left =
	  XtVaCreateManagedWidget("add", commandWidgetClass, form,
				  XtNfromVert,  port,
				  XtNfromHoriz, left,
				  XtNtop,    XawChainBottom, 
				  XtNbottom, XawChainBottom, 
				  XtNleft,   XawChainLeft, 
				  XtNright,  XawChainLeft, 
				  NULL);
	XtAddCallback(left, XtNcallback, orders_add_call, NULL);
	left =
	  XtVaCreateManagedWidget("clone", commandWidgetClass, form,
				  XtNfromVert,  port,
				  XtNfromHoriz, left,
				  XtNtop,    XawChainBottom, 
				  XtNbottom, XawChainBottom, 
				  XtNleft,   XawChainLeft, 
				  XtNright,  XawChainLeft, 
				  NULL);
	XtAddCallback(left, XtNcallback, orders_clone_call, NULL);
	left =
	  XtVaCreateManagedWidget("delete", commandWidgetClass, form,
				  XtNfromVert,  port,
				  XtNfromHoriz, left,
				  XtNtop,    XawChainBottom, 
				  XtNbottom, XawChainBottom, 
				  XtNleft,   XawChainLeft, 
				  XtNright,  XawChainLeft, 
				  NULL);
	XtAddCallback(left, XtNcallback, orders_delete_call, NULL);
	left =
	  XtVaCreateManagedWidget("undo", commandWidgetClass, form,
				  XtNfromVert,  port,
				  XtNfromHoriz, left,
				  XtNtop,    XawChainBottom, 
				  XtNbottom, XawChainBottom, 
				  XtNleft,   XawChainLeft, 
				  XtNright,  XawChainLeft, 
				  NULL);
	XtAddCallback(left, XtNcallback, orders_undo_call, NULL);
	left =
	  XtVaCreateManagedWidget("save", commandWidgetClass, form,
				  XtNfromVert,  port,
				  XtNfromHoriz, left,
				  XtNtop,    XawChainBottom, 
				  XtNbottom, XawChainBottom, 
				  XtNleft,   XawChainLeft, 
				  XtNright,  XawChainLeft, 
				  NULL);
	XtAddCallback(left, XtNcallback, orders_save_call, NULL);
	left =
	  XtVaCreateManagedWidget("restore", commandWidgetClass, form,
				  XtNfromVert,  port,
				  XtNfromHoriz, left,
				  XtNtop,    XawChainBottom, 
				  XtNbottom, XawChainBottom, 
				  XtNleft,   XawChainLeft, 
				  XtNright,  XawChainLeft, 
				  NULL);
	XtAddCallback(left, XtNcallback, orders_restore_call, NULL);
	left =
	  XtVaCreateManagedWidget("close", commandWidgetClass, form,
				  XtNfromVert,  port,
				  XtNfromHoriz, left,
				  XtNtop,    XawChainBottom, 
				  XtNbottom, XawChainBottom, 
				  XtNleft,   XawChainLeft, 
				  XtNright,  XawChainLeft, 
				  NULL);
	XtAddCallback(left, XtNcallback, orders_close_call, NULL);
    }

    sprintf(buffer, "%d standing order%s", num, (num == 1) ? "" : "s");
    XtVaSetValues(side->ui->orders_label, XtNlabel, buffer, NULL);
    
    XtPopup(side->ui->orders_shell, XtGrabNone);
    raise_widget(side->ui->orders_shell);

    /* this is best done after the widgets has been realized 
       and their widths have been fixed */
    for (ordi = side->ui->orderlist;  ordi != NULL;  ordi = ordi->next) 
      update_order_widgets(side, ordi, ordi->sorder, ordi->sorder->types,
			   ordi->sorder->task);

}

Widget
new_order_widgets(Side *side, OrderInterface *ordi, int num, Widget up)
{
    Widget left;
    XrmDatabase xrdb;
    char *stype;
    XrmValue val;
    XColor color, junk;
    Colormap cmap;
    int res_found = 0;
    
    sprintf(spbuf, "order_%d", num);
    ordi->form = 
      XtVaCreateManagedWidget(spbuf, formWidgetClass,
			      side->ui->orders_form,
			      XtNfromVert, up,
			      XtNtop,    XawChainTop, 
			      XtNbottom, XawChainTop, 
			      XtNleft,   XawChainLeft, 
			      XtNright,  XawChainLeft, 
			      NULL);
    XtVaGetValues(ordi->form, XtNbackground, &(ordi->form_bg), NULL);

    /* get foreground pixel */
    xrdb = XtDatabase(side->ui->dpy);
    if (XrmGetResource(xrdb, "xconq*orders*orders.Form.foreground",
		       "Xconq*orders*orders.Form.Foreground",
		       &stype, &val)) {
	if (!strcmp(stype, "String")) {
	    cmap = XDefaultColormap(side->ui->dpy, side->ui->screen);
	    if (XAllocNamedColor(side->ui->dpy, cmap, val.addr,
				 &color, &junk)) {
		ordi->form_fg = color.pixel;
		res_found = 1;
	    }
	}
    }
    if (!res_found) {
	/* fallback = border pixel */
	XtVaGetValues(ordi->form, XtNborderColor, &(ordi->form_fg), NULL);
    }

    left = ordi->toggle =
      XtVaCreateManagedWidget("toggle", toggleWidgetClass, ordi->form,
			      XtNlabel,  "",
			      XtNstate,  False,
			      XtNleft,   XawChainLeft, 
			      XtNright,  XawChainLeft, 
			      NULL);
    XtAddCallback(left, XtNcallback, order_toggle_call, NULL);
    if (num == 0)
      side->ui->orders_radio = left;
    XawToggleChangeRadioGroup(left, side->ui->orders_radio);

    left = ordi->types = 
      XtVaCreateManagedWidget("types", commandWidgetClass, ordi->form,
			      XtNbitmap, get_unit_picture(0, side),
			      XtNfromHoriz, left,
			      XtNleft,   XawChainLeft, 
			      XtNright,  XawChainLeft, 
			      NULL);
    XtAddCallback(left, XtNcallback, order_types_call, NULL);

    left = ordi->etype =
      XtVaCreateManagedWidget("etype", commandWidgetClass, ordi->form,
			      XtNlabel, "unknown",
			      XtNfromHoriz, left,
			      XtNleft,   XawChainLeft, 
			      XtNright,  XawChainLeft, 
			      NULL);
    XtAddCallback(left, XtNcallback, order_etype_call, NULL);

    left = ordi->eparms = 
      XtVaCreateManagedWidget("eparms", commandWidgetClass, ordi->form,
			      XtNlabel, "               ",
			      XtNfromHoriz, left,
			      XtNleft,   XawChainLeft, 
			      XtNright,  XawChainLeft, 
			      NULL);
    XtAddCallback(left, XtNcallback, order_eparms_call, NULL);

    left = ordi->task = 
      XtVaCreateManagedWidget("task", commandWidgetClass, ordi->form,
			      XtNlabel, "            ",
			      XtNfromHoriz, left,
			      XtNleft,   XawChainLeft, 
			      XtNright,  XawChainLeft, 
			      NULL);
    XtAddCallback(left, XtNcallback, order_task_call, NULL);

    left = ordi->tparms =
      XtVaCreateManagedWidget("tparms", commandWidgetClass, ordi->form,
			      XtNlabel, "            ",
			      XtNfromHoriz, left,
			      XtNleft,   XawChainLeft, 
			      XtNright,  XawChainLeft, 
			      NULL);
    XtAddCallback(left, XtNcallback, order_tparms_call, NULL);
#if 0
    if (ordi->sorder->condtype == 1) {
	left = 
	  XtVaCreateManagedWidget("etype", labelWidgetClass, ordi->form,
				  XtNlabel, "at",
				  XtNfromHoriz, left,
				  XtNleft,   XawChainLeft, 
				  XtNright,  XawChainLeft, 
				  NULL);
	
	sprintf(buffer, "%3d,%3d", ordi->sorder->a1, ordi->sorder->a2);
	left = ordi->eparms = 
	  XtVaCreateManagedWidget("eparms", commandWidgetClass, ordi->form,
				  XtNlabel, buffer,
				  XtNfromHoriz, left,
				  XtNleft,   XawChainLeft, 
				  XtNright,  XawChainLeft, 
				  NULL);
	XtAddCallback(left, XtNcallback, order_eparms_call, NULL);
    } else {
	left = 
	  XtVaCreateManagedWidget("etype", labelWidgetClass, ordi->form,
				  XtNlabel, "unknown",
				  XtNfromHoriz, left,
				  NULL);
    }
    
    if (ordi->sorder->task && ordi->sorder->task->type == TASK_MOVE_TO) {
	left = ordi->task = 
	  XtVaCreateManagedWidget("task", labelWidgetClass, ordi->form,
				  XtNlabel, "move-to",
				  XtNfromHoriz, left,
				  XtNleft,   XawChainLeft, 
				  XtNright,  XawChainLeft, 
				  NULL);
	
	sprintf(buffer, "%3d,%3d",
		ordi->sorder->task->args[0], ordi->sorder->task->args[1]);
	left = ordi->tparms =
	  XtVaCreateManagedWidget("tparms", commandWidgetClass, ordi->form,
				  XtNlabel, buffer,
				  XtNfromHoriz, left,
				  XtNleft,   XawChainLeft, 
				  XtNright,  XawChainLeft, 
				  NULL);
	XtAddCallback(left, XtNcallback, order_tparms_call, NULL);
    } else {
	left = ordi->task = 
	  XtVaCreateManagedWidget("task", labelWidgetClass, ordi->form,
				  XtNlabel, "unknown",
				  XtNfromHoriz, left,
				  XtNleft,   XawChainLeft, 
				  XtNright,  XawChainLeft, 
				  NULL);
	left = ordi->tparms =
	  XtVaCreateManagedWidget("tparms", commandWidgetClass, ordi->form,
				  XtNlabel, "     ",
				  XtNfromHoriz, left,
				  XtNleft,   XawChainLeft, 
				  XtNright,  XawChainLeft, 
				  NULL);
	XtAddCallback(left, XtNcallback, order_tparms_call, NULL);
    }
#endif

    return ordi->form;
}

void
sorder_cond_name(char *buf, StandingOrder *sorder, Side *side)
{
    switch (sorder->condtype) {
    case 0:
	strcpy(buf, "            ");
	return;
    case 1:
	sprintf(buf, "%3d,%3d      ", sorder->a1, sorder->a2);
	return;
    /* let unit_handle and find_unit handle exceptions */
    case 2:
	strcpy(buf, short_unit_handle(find_unit(sorder->a1)));
	if (strlen(buf) < 12)
	  strncat(buf, "            ", 12 - strlen(buf));
	return;
    case 3:
	sprintf(buf, "%2d of %3d,%3d", sorder->a3, sorder->a1, sorder->a2);
    }
}

void
update_order_widgets(Side *side, OrderInterface *ordi, StandingOrder *sorder, char *types, Task *task)
{
    int u;
    char *lab;
    Pixmap icon = None;
    
    for_all_unit_types(u) {
	if (types[u]) {
	    icon = get_unit_picture(u, side);
	}
    }
    XtVaSetValues(ordi->types, XtNbitmap, icon, NULL);

    if (sorder->condtype >= 0 && sorder->condtype < ORDER_COND_NUM) {
	lab = order_cond[(int) sorder->condtype];
    } else {
	lab = "unknown";
    }
    XtVaSetValues(ordi->etype, XtNlabel, lab, NULL);

    sorder_cond_name(buffer, sorder, side);
    XtVaSetValues(ordi->eparms, XtNlabel, buffer, NULL);

    if (task) {
	XtVaSetValues(ordi->task, XtNlabel, taskdefns[task->type].name, NULL);

	/* get task description, chop task name and separator */
	task_desc(buffer, side, NULL, task);
	lab = buffer + strlen(taskdefns[task->type].name);
	while (*lab == ' ' || *lab == ',')
	    lab++;
	XtVaSetValues(ordi->tparms, XtNlabel, lab, NULL);
    } else {
	XtVaSetValues(ordi->task,   XtNlabel, "none        ", NULL);
	XtVaSetValues(ordi->tparms, XtNlabel, "            ", NULL);
    }
}

/* callbacks for orders */

static void 
orders_help_call(Widget w, XtPointer client_data, XtPointer call_data)
{
    Side *side;
    Widget form, done, text;

    if (!find_side_via_widget(w, &side))
      return;

    if (!side->ui->orders_help_shell) {
	side->ui->orders_help_shell =
	  XtVaCreatePopupShell("ordersHelp", topLevelShellWidgetClass,
			       side->ui->shell,
			       NULL);
	form =
	  XtVaCreateManagedWidget("form", formWidgetClass,
				  side->ui->orders_help_shell,
				  NULL);
	text =
	  XtVaCreateManagedWidget("text", labelWidgetClass, form,
				  XtNlabel, helpText,
				  NULL);
	done =
	  XtVaCreateManagedWidget("done", commandWidgetClass, form,
				  XtNfromVert, text,
				  NULL);
	XtAddCallback(done, XtNcallback, orders_done_help_call, NULL);
    }

    XtPopup(side->ui->orders_help_shell, XtGrabNone);
}

static void 
orders_done_help_call(Widget w, XtPointer client_data, XtPointer call_data)
{
    Side *side;

    if (!find_side_via_widget(w, &side))
      return;

    if (side->ui->orders_help_shell)
      XtPopdown(side->ui->orders_help_shell);
}

static void
handle_done_orders_help(Widget w, XEvent *event, String *params, Cardinal *numparms)
{
    Side *side;

    if (!find_side_via_widget(w, &side))
      return;

    if (side->ui->orders_help_shell)
      XtPopdown(side->ui->orders_help_shell);
}

static void 
orders_add_call(Widget w, XtPointer client_data, XtPointer call_data)
{
  /* generate a "blank" order */
  sprintf(spbuf, "%s at 0,0 move-to 0,0", u_type_name(0));
  orders_add_named(w, spbuf);
}

static void 
orders_clone_call(Widget w, XtPointer client_data, XtPointer call_data)
{
  Side *side;
  OrderInterface *ordi;
  char *name;

  if (!find_side_via_widget(w, &side))
    return;

  ordi = active_ordi(side);
  if (ordi == NULL) 
    return;
  
  name = standing_order_desc(side->ui->sorder_edit, spbuf);
  orders_add_named(w, name);
}

static void 
orders_add_named(Widget w, char *string)
{
    OrderInterface *ordi;
    Side *side;
    Widget up;
    int num;

    if (!find_side_via_widget(w, &side))
      return;

    deactivate_orders(side);

    if (parse_standing_order(side, string) != 0) {
	/* parsing failed and no standing order was created */
	return;
    }
    /* the standing order is now in side->orders */

    ordi = (OrderInterface *) xmalloc(sizeof(OrderInterface));
    if (side->ui->orderlist) {
	up = side->ui->orderlist->form;
	ordi->next = side->ui->orderlist;
    } else {
	up = NULL;
	ordi->next = NULL;
    }
    side->ui->orderlist = ordi;
    ordi->sorder = side->orders;

    orders_check(side);

    num = count_sorder(side);
    new_order_widgets(side, ordi, num-1, up);
    sprintf(buffer, "%d standing order%s", num, (num==1) ? "" : "s");
    XtVaSetValues(side->ui->orders_label, XtNlabel, buffer, NULL);
    update_order_widgets(side, ordi, ordi->sorder, ordi->sorder->types,
			 ordi->sorder->task);

    /* activate new order */
    XtVaSetValues(ordi->toggle, XtNstate, True, NULL);
    order_toggle_call(ordi->toggle, NULL, NULL);
}

static void 
orders_delete_call(Widget w, XtPointer client_data, XtPointer call_data)
{
    Side *side;
    OrderInterface *ordi, *activeo;
    StandingOrder *sorder;
    
    if (!find_side_via_widget(w, &side))
      return;

    activeo = active_ordi(side);
    if (activeo == NULL) 
      return;

    /* remove sorder and ordi */
    if (side->ui->orderlist == activeo) {
	side->ui->orderlist = side->ui->orderlist->next;
	side->orders = side->orders->next;
	if (side->orders == NULL) {
	    side->last_order = NULL;
	}
    } else {
	for (sorder = side->orders,  ordi = side->ui->orderlist;
	     sorder != NULL && sorder->next != NULL;
	     sorder = sorder->next,  ordi = ordi->next) {
	    if (ordi->next == activeo) {
		ordi->next = ordi->next->next;
		if (sorder->next == side->last_order) {
		    side->last_order = sorder;
		}
		sorder->next = sorder->next->next;
		break;
	    }
	}
    }

    orders_check(side);

    /* re-create orders window */
    XtPopdown(side->ui->orders_shell);
    XtDestroyWidget(side->ui->orders_shell);
    side->ui->orders_shell = NULL;
    create_orders_window(side, side->ui->maps);
}

static void 
orders_undo_call(Widget w, XtPointer client_data, XtPointer call_data)
{
    Side *side;
    OrderInterface *ordi;

    if (!find_side_via_widget(w, &side))
      return;

    ordi = active_ordi(side);
    if (ordi == NULL)
      return;

    /* copy standing order */
    *(side->ui->sorder_edit) = *(ordi->sorder);
    memcpy(side->ui->sorder_types_edit, ordi->sorder->types,
	   numutypes*sizeof(char));
    *(side->ui->sorder_task_edit) = *(ordi->sorder->task);
    update_order_widgets(side, ordi, ordi->sorder, ordi->sorder->types,
			 ordi->sorder->task);
}

static void 
orders_save_call(Widget w, XtPointer client_data, XtPointer call_data)
{
    Side *side;
    FILE *file;
    StandingOrder *sorder;

    if (!find_side_via_widget(w, &side))
      return;

    deactivate_orders(side);

    if (!(file = fopen(sorderfile, "w"))) {
	run_warning("Cannot open file \"%s\", standing orders not saved!\n");
	return;
    }

    for (sorder = side->orders; sorder != NULL; sorder = sorder->next) {
	fprintf(file, "%s\n", standing_order_desc(sorder, buffer));
    }
    fclose(file);
    notify(side, "Standing orders saved in file \"%s\".", sorderfile);
}

static void 
orders_restore_call(Widget w, XtPointer client_data, XtPointer call_data)
{
    Side *side;
    FILE *file;

    if (!find_side_via_widget(w, &side))
      return;

    if (!(file = fopen(sorderfile, "r"))) {
	run_warning("Cannot open file \"%s\", standing orders not restored!\n",
		    sorderfile);
	return;
    }

    /* clear all stading orders; quick & dirty, without free'ing
       malloc'd stuff (hopefully a negligible amount) */
    side->orders = side->last_order = NULL;

    while (!feof(file)) {
	fgets(buffer, 160, file);
	if (!feof(file) && strlen(buffer)>3) {
	    parse_standing_order(side, buffer);
	}
    }
    fclose(file);
    notify(side, "Standing orders restored from file \"%s\".", sorderfile);

   /* re-create orders window and list */
    XtPopdown(side->ui->orders_shell);
    XtDestroyWidget(side->ui->orders_shell);
    side->ui->orderlist = NULL;
    side->ui->orders_shell = NULL; 
    create_orders_window(side, side->ui->maps);
}

static void 
orders_close_call(Widget w, XtPointer client_data, XtPointer call_data)
{
    Side *side;

    if (!find_side_via_widget(w, &side))
      return;

    XtPopdown(side->ui->orders_shell);

    if (side->ui->orders_help_shell)
      XtPopdown(side->ui->orders_help_shell);
}


static void 
order_toggle_call(Widget w, XtPointer client_data, XtPointer call_data)
{
    Side *side;
    Boolean state;
    OrderInterface *ordi;

    if (!find_side_and_ordi_via_widget(w, &side, &ordi))
      return;

    XtVaGetValues(ordi->toggle, XtNstate, &state, NULL);
    if (state) {
	XtVaSetValues(ordi->form, XtNbackground, ordi->form_fg, NULL);

	/* copy standing order */
	side->ui->ordi_edit = ordi;
	*(side->ui->sorder_edit) = *(ordi->sorder);
	memcpy(side->ui->sorder_types_edit, ordi->sorder->types,
	       numutypes*sizeof(char));
	*(side->ui->sorder_task_edit) = *(ordi->sorder->task);
    } else {
	XtVaSetValues(ordi->form, XtNbackground, ordi->form_bg, NULL);

	/* update standing order */
	*(ordi->sorder) = *(side->ui->sorder_edit);
	memcpy(ordi->sorder->types, side->ui->sorder_types_edit,
	       numutypes*sizeof(char));
	*(ordi->sorder->task) = *(side->ui->sorder_task_edit);
	update_order_widgets(side, ordi, ordi->sorder, ordi->sorder->types,
			     ordi->sorder->task);
    }
}

static void 
order_etype_call(Widget w, XtPointer client_data, XtPointer call_data)
{
    Side *side;
    OrderInterface *ordi;
    Boolean state;
    Map *map;
    Position x, y;
    int i;

    if (!find_side_and_ordi_via_widget(w, &side, &ordi))
      return;
    map = side->ui->maps;

    XtVaGetValues(ordi->toggle, XtNstate, &state, NULL);

    if (!state) 
      return;

    XtVaGetValues(side->ui->orders_shell, XtNx, &x, XtNy, &y, NULL);
    i = cond_select_popup(side, x + 32, y + 32);
    if (i >= 0 && i < ORDER_COND_NUM && i != side->ui->sorder_edit->condtype) {
	XtVaSetValues(ordi->etype, XtNlabel, order_cond[i], NULL);
	side->ui->sorder_edit->condtype = (enum sordercond)i;
	side->ui->sorder_edit->a1 = 0;
	side->ui->sorder_edit->a2 = 0;
	side->ui->sorder_edit->a3 = 0;
	sorder_cond_name(buffer, side->ui->sorder_edit, side);
	XtVaSetValues(ordi->eparms, XtNlabel, buffer, NULL);
    }
}

static void 
order_eparms_call(Widget w, XtPointer client_data, XtPointer call_data)
{
    Side *side;
    OrderInterface *ordi;
    Boolean state;
    Map *map;
    Position x, y;
    Unit *unit;

    if (!find_side_and_ordi_via_widget(w, &side, &ordi))
      return;
    map = side->ui->maps;

    XtVaGetValues(ordi->toggle, XtNstate, &state, NULL);

    if (state) {
	/* edit */
	switch (side->ui->sorder_edit->condtype) {
	case 1:
	    set_cell_from_map(side, map,
			      "select origin cell, then press space",
			      (void *) &side->ui->sorder_edit->a1,
			      (void *) &side->ui->sorder_edit->a2,
			      sizeof(side->ui->sorder_edit->a1));
	    break;
	case 2:
	    set_unit_from_map (side, map, 
			       "select origin unit, then press space",
			       (int *) &side->ui->sorder_edit->a1);
	    break;
	case 3:
	    XtVaGetValues(side->ui->orders_shell, XtNx, &x, XtNy, &y, NULL);
	    side->ui->sorder_edit->a3 =
	      integer_select_popup(side, x+32, y+32, side->ui->sorder_edit->a3,
				   "enter origin radius");
	    if (side->ui->sorder_edit->a3<0)
	      side->ui->sorder_edit->a3 = 0;
	    set_cell_from_map(side, map,
			      "select origin center, then press space",
			      (void *) &side->ui->sorder_edit->a1,
			      (void *) &side->ui->sorder_edit->a2,
			      sizeof(side->ui->sorder_edit->a1));
	    break;
	  default:
	    break;
	}
    } else {
	/* display */
	if (ordi->sorder->condtype == 1 || ordi->sorder->condtype == 3) {
	    if (in_area(ordi->sorder->a1, ordi->sorder->a2)) {
		map->curx = ordi->sorder->a1;
		map->cury = ordi->sorder->a2;
		map->curunit = NULL;
		recenter(side, map, ordi->sorder->a1, ordi->sorder->a2);
		side->ui->curmap = map;
		do_flash(side);
	    } else {
		beep(side);
	    }
	} else if (ordi->sorder->condtype == 2) {
	    if ((unit = find_unit(ordi->sorder->a1)) != NULL) {
	        set_current_unit(side, map, unit);
		map->curx = unit->x;  map->cury = unit->y;
		recenter(side, map, unit->x, unit->y);
		side->ui->curmap = map;
		do_flash(side);
	    } else {
		beep(side);
	    }
	}
    }
}

static void 
order_task_call(Widget w, XtPointer client_data, XtPointer call_data)
{
    Side *side;
    OrderInterface *ordi;
    Boolean state;
    Map *map;
    Position x, y;
    int i, type;

    if (!find_side_and_ordi_via_widget(w, &side, &ordi))
      return;
    map = side->ui->maps;

    XtVaGetValues(ordi->toggle, XtNstate, &state, NULL);

    if (!state) 
      return;

    XtVaGetValues(side->ui->orders_shell, XtNx, &x, XtNy, &y, NULL);
    type = task_select_popup(side, x+32, y+32);
    if (!is_task_type(type) || type==side->ui->sorder_task_edit->type)
      return;

    side->ui->sorder_task_edit->type = (TaskType)type;
    for (i=0; i<MAXTASKARGS; i++)
      side->ui->sorder_task_edit->args[i] = 0;
    side->ui->sorder_task_edit->execnum = 0;
    side->ui->sorder_task_edit->retrynum = 0;

    update_order_widgets(side, side->ui->ordi_edit, side->ui->sorder_edit,
			 side->ui->sorder_types_edit,
			 side->ui->sorder_task_edit);
 
}

static void 
order_types_call(Widget w, XtPointer client_data, XtPointer call_data)
{
    Side *side;
    OrderInterface *ordi;
    Boolean state;
    Pixmap icon;
    int i, oldu = 0, newu;
    Position x, y;

    if (!find_side_and_ordi_via_widget(w, &side, &ordi))
      return;

    XtVaGetValues(ordi->toggle, XtNstate, &state, NULL);
    if (!state)
      return;

    for_all_unit_types(i) {
	if (side->ui->sorder_types_edit[i]) {
	    oldu = i;
	    break;
	}
    }
 
    XtVaGetValues(side->ui->orders_shell, XtNx, &x, XtNy, &y, NULL);
    newu = utype_select_popup(side, x+32, y+32);
    if (newu != NONUTYPE) {
	icon = get_unit_picture(newu, side);
	XtVaSetValues(ordi->types, XtNbitmap, icon, NULL);
	side->ui->sorder_types_edit[oldu] = 0;
	side->ui->sorder_types_edit[newu] = 1;
    }
}

static int select_loop, select_cond, select_task, select_utype, select_dir;

int
cond_select_popup(Side *side, Position x, Position y)
{
    Widget box, shell, command;
    int i;

    select_cond = ORDER_COND_NUM;

    shell =
      XtVaCreatePopupShell("condSelect", transientShellWidgetClass,
			   side->ui->shell,
			   XtNx, x,
			   XtNy, y,
			   NULL);
    box = XtVaCreateManagedWidget("box", boxWidgetClass, shell,
			   NULL);

    for (i=0; i<ORDER_COND_NUM; i++) {
	command =
	  XtVaCreateManagedWidget(order_cond[i], commandWidgetClass, box,
				  NULL);
	XtAddCallback(command, XtNcallback, cond_select_call,
		      (XtPointer) i);
    }
    command =
      XtVaCreateManagedWidget("cancel", commandWidgetClass, box,
			      NULL);
    XtAddCallback(command, XtNcallback, cond_select_call,
		  (XtPointer) ORDER_COND_NUM); 

    XtPopup(shell, XtGrabExclusive);

    XWarpPointer(side->ui->dpy, None, XtWindow(box), 0, 0, 0, 0, 32, 32);

    /* replacement for XtAppMainLoop(thisapp): */
    select_loop = 1;
    while (select_loop) {
	XEvent event;

	XtAppNextEvent(thisapp, &event);
	XtDispatchEvent(&event);
    }
    XtPopdown(shell);
    XtDestroyWidget(shell);

    return select_cond;
}

static void 
cond_select_call(Widget w, XtPointer client_data, XtPointer call_data)
{
    int i;

    for (i=0; i<ORDER_COND_NUM; i++) {
	if (client_data == (XtPointer) i) {
	    select_cond = i;
	    select_loop = 0;
	    return;
	}
    }
    select_cond = ORDER_COND_NUM;
    select_loop = 0;
    return;
}

int
task_select_popup(Side *side, Position x, Position y)
{
    Widget box, shell, command;
    int i;

    select_task = -1;

    shell =
      XtVaCreatePopupShell("taskSelect", transientShellWidgetClass,
			   side->ui->shell,
			   XtNx, x,
			   XtNy, y,
			   NULL);
    box = XtVaCreateManagedWidget("box", boxWidgetClass, shell,
			   NULL);

    for (i = 0; taskdefns[i].name != NULL; i++) {
        command =
	  XtVaCreateManagedWidget(taskdefns[i].name, commandWidgetClass,
				  box, NULL);
	XtAddCallback(command, XtNcallback, task_select_call,
		      (XtPointer) i);
    }
    command =
      XtVaCreateManagedWidget("cancel", commandWidgetClass, box,
			      NULL);
    XtAddCallback(command, XtNcallback, task_select_call,
		  (XtPointer) -1); 

    XtPopup(shell, XtGrabExclusive);

    XWarpPointer(side->ui->dpy, None, XtWindow(box), 0, 0, 0, 0, 32, 32);

    /* replacement for XtAppMainLoop(thisapp): */
    select_loop = 1;
    while (select_loop) {
	XEvent event;

	XtAppNextEvent(thisapp, &event);
	XtDispatchEvent(&event);
    }
    XtPopdown(shell);
    XtDestroyWidget(shell);

    return select_task;
}

static void 
task_select_call(Widget w, XtPointer client_data, XtPointer call_data)
{
    int i;

    for (i = 0; taskdefns[i].name != NULL; i++) {
	if (client_data == (XtPointer) i) {
	    select_task = i;
	    select_loop = 0;
	    return;
	}
    }
    select_task = -1;
    select_loop = 0;
    return;
}

int
utype_select_popup(Side *side, Position x, Position y)
{
    Widget box, shell, command;
    Pixmap icon;
    int u;

    select_utype = NONUTYPE;

    shell =
      XtVaCreatePopupShell("utypeSelect", transientShellWidgetClass,
			   side->ui->shell,
			   XtNx, x,
			   XtNy, y,
			   NULL);
    box = XtVaCreateManagedWidget("box", boxWidgetClass, shell,
			   NULL);

    for_all_unit_types(u) {
	icon = get_unit_picture(u, side);
	build_name(buffer, "", u_type_name(u));
	command =
	   XtVaCreateManagedWidget(buffer, commandWidgetClass, box,
				   XtNlabel, "",
				   XtNbitmap, icon,
				   NULL);
	XtAddCallback(command, XtNcallback, utype_select_call,
		      (XtPointer) u);
    }
    command =
      XtVaCreateManagedWidget("cancel", commandWidgetClass, box,
			      NULL);
    XtAddCallback(command, XtNcallback, utype_select_call,
		  (XtPointer) NONUTYPE); 

    XtPopup(shell, XtGrabExclusive);

    XWarpPointer(side->ui->dpy, None, XtWindow(box), 0, 0, 0, 0, 32, 32);

    /* replacement for XtAppMainLoop(thisapp): */
    select_loop = 1;
    while (select_loop) {
	XEvent event;

	XtAppNextEvent(thisapp, &event);
	XtDispatchEvent(&event);
    }
    XtPopdown(shell);
    XtDestroyWidget(shell);

    return select_utype;
}

static void 
utype_select_call(Widget w, XtPointer client_data, XtPointer call_data)
{
    int u;

    for_all_unit_types(u) {
	if (client_data == (XtPointer) u) {
	    select_utype = u;
	    select_loop = 0;
	    return;
	}
    }
    select_utype = NONUTYPE;
    select_loop = 0;
    return;
}

int
dir_select_popup(Side *side, Position x, Position y)
{
    Widget box, shell, command;
    int dir;

    select_dir = -1;

    shell =
      XtVaCreatePopupShell("dirSelect", transientShellWidgetClass,
			   side->ui->shell,
			   XtNx, x,
			   XtNy, y,
			   NULL);
    box = XtVaCreateManagedWidget("box", boxWidgetClass, shell,
			   NULL);

    for_all_directions(dir) {
	command =
	   XtVaCreateManagedWidget(dirnames[dir], commandWidgetClass, box,
				   NULL);
	XtAddCallback(command, XtNcallback, dir_select_call,
		      (XtPointer) dir);
    }

    XtPopup(shell, XtGrabExclusive);

    XWarpPointer(side->ui->dpy, None, XtWindow(box), 0, 0, 0, 0, 32, 32);

    /* replacement for XtAppMainLoop(thisapp): */
    select_loop = 1;
    while (select_loop) {
	XEvent event;

	XtAppNextEvent(thisapp, &event);
	XtDispatchEvent(&event);
    }
    XtPopdown(shell);
    XtDestroyWidget(shell);

    return select_dir;
}

static void 
dir_select_call(Widget w, XtPointer client_data, XtPointer call_data)
{
    int dir;

    for_all_directions(dir) {
	if (client_data == (XtPointer) dir) {
	    select_dir = dir;
	    select_loop = 0;
	    return;
	}
    }
    select_dir = -1;
    select_loop = 0;
    return;
}

int
integer_select_popup(Side *side, Position x, Position y, int value, char *prompt)
{
    int result;

    sprintf(tmpbuf, "%d", value);
    string_select_popup(side, x, y, buffer, tmpbuf, prompt);
    if (sscanf(buffer, "%d", &result) != 1)
      result = 0;

    return result;
}

void
string_select_popup(Side *side, Position x, Position y, char *result, char *value, char *prompt)
{
    Widget shell, dialog;
    char *string;

    select_task = -1;

    shell =
      XtVaCreatePopupShell("stringSelect", transientShellWidgetClass,
			   side->ui->shell,
			   XtNx, x,
			   XtNy, y,
			   NULL);

    dialog = XtVaCreateManagedWidget("dialog", dialogWidgetClass, shell,
					  XtNlabel, prompt,
					  XtNvalue, value,
					  NULL);

    XtPopup(shell, XtGrabExclusive);
    XWarpPointer(side->ui->dpy, None, XtWindow(dialog), 0, 0, 0, 0, 32, 32);

    /* replacement for XtAppMainLoop(thisapp): */
    select_loop = 1;
    while (select_loop) {
	XEvent event;

	XtAppNextEvent(thisapp, &event);
	XtDispatchEvent(&event);
    }

    string = XawDialogGetValueString(dialog);
    /* safer to save the string, XtDestroyWidget may deallocate it */
    strcpy(result, string);

    XtPopdown(shell);
    XtDestroyWidget(shell);
}

static void 
handle_done_string_select(Widget w, XEvent *event, String *params, Cardinal *numparms)
{
    select_loop = 0;
}


static void 
order_tparms_call(Widget w, XtPointer client_data, XtPointer call_data)
{
    Side *side;
    OrderInterface *ordi;
    Boolean state;
    Map *map;
    char *lab;
    int utype, uid, i;
    Unit *unit;
    Position x, y;
    Task *task;

    if (!find_side_and_ordi_via_widget(w, &side, &ordi))
      return;
    map = side->ui->maps;

    XtVaGetValues(ordi->toggle, XtNstate, &state, NULL);

    XtVaGetValues(side->ui->orders_shell, XtNx, &x, XtNy, &y, NULL);
    x += 32;  y += 32;

    task = side->ui->sorder_task_edit;

    if (state) {
	/* edit */

	for (i = 0; i < MAXTASKARGS; ++i)
	  side->ui->sorder_edit->task->args[i] = 0;

	switch (task->type) {

	case TASK_NONE:
	case TASK_DISBAND:
	    break;

	case TASK_BUILD:
	    utype = utype_select_popup(side, x, y);
	    if (utype != NONUTYPE) {
		task->args[0] = utype;
		task->args[3] = 99;
	    }
	    break;

	case TASK_CAPTURE:
	case TASK_HIT_UNIT:
	    set_unit_from_map (side, map,
			       "select target unit, then press space",
			       &uid);
	    unit = find_unit(uid);
	    task->args[0] = unit->x;
	    task->args[1] = unit->y;
	    if (task->type == TASK_HIT_UNIT) {
		task->args[2] = NONUTYPE;
		task->args[3] = -1;
	    }
	    break;

	case TASK_MOVE_DIR:
	    task->args[0] = dir_select_popup (side, x, y);
	    task->args[1] = integer_select_popup(side, x, y, 99,
						 "enter distance");
	    break;

	case TASK_MOVE_TO:
	    task->args[2] = 0;  /* is this useful or just annoing?  Massimo */
	    /* integer_select_popup(side, x, y, 0, "enter distance"); */

	    set_cell_from_map (side, map,
			       "select destination cell, then press space",
			       (void *) &task->args[0],
			       (void *) &task->args[1],
			       sizeof(task->args[0]));
	    break;

	case TASK_HIT_POSITION:
	    set_cell_from_map (side, map,
			       "select target cell, then press space",
			       (void *) &task->args[0],
			       (void *) &task->args[1],
			       sizeof(task->args[0]));
	    break;

	case TASK_OCCUPY:
	case TASK_PICKUP:
	    set_unit_from_map (side, map,
			       "select target unit, then press space",
			       (int *) &uid);
	    break;

	case TASK_SENTRY:
	    task->args[0] = integer_select_popup(side, x, y, 99,
						 "enter turn #");
	    break;

	default:
	    beep(side);
	}
	/* get task description, chop task name and separator */
	task_desc(buffer, side, NULL, task);
	lab = buffer + strlen(taskdefns[task->type].name);
	while (*lab == ' ' || *lab == ',')
	  lab++;
	XtVaSetValues(side->ui->ordi_edit->tparms, XtNlabel, lab, NULL);

    } else {
	/* display */
	if (ordi->sorder->task->type == TASK_MOVE_TO) {
	    if (in_area(ordi->sorder->a1, ordi->sorder->a2)) {
		map->curx = ordi->sorder->task->args[0];
		map->cury = ordi->sorder->task->args[1];
		map->curunit = NULL;
		recenter(side, map, ordi->sorder->task->args[0],
			            ordi->sorder->task->args[1]);
		side->ui->curmap = map;
		do_flash(side);
	    } else {
		beep(side);
	    }
	}
    }
}

void 
set_cell_from_map (Side *side, Map *map, char *prompt, void *px, void *py, int size)
{
    save_cur(side, map);
    raise_widget(XtParent(map->mainwidget));
    map->curtool = looktool;
    set_tool_cursor(side, map);
    update_controls(side, map);
    side->ui->grok_p1 = (int *)px;
    side->ui->grok_p2 = (int *)py;
    side->ui->grok_size = size;
    ask_position(side, map, prompt, aux_set_cell);
}

static void
aux_set_cell(Side *side, Map *map, int cancel)
{
    int x, y;
    Unit *unit;
    Boolean state;

    if (cancel)
      return;
    if (grok_position(side, map, &x, &y, &unit)) {
/*	erase_current(side, map, x, y, NULL); */

	XtVaGetValues(side->ui->ordi_edit->toggle, XtNstate, &state, NULL);
	if (!state) {
	    beep(side);
	    return;
	}
	raise_widget(side->ui->orders_shell);
	switch (side->ui->grok_size) {
	case 1:
	    *((char *)  side->ui->grok_p1) = x;
	    *((char *)  side->ui->grok_p2) = y;
	    break;
	case 2:
	    *((short *) side->ui->grok_p1) = x;
	    *((short *) side->ui->grok_p2) = y;
	    break;
	case 4:
	    *((int *)   side->ui->grok_p1) = x;
	    *((int *)   side->ui->grok_p2) = y;
	    break;
	default:
	    beep(side);
	    return;
	}
	update_order_widgets(side, side->ui->ordi_edit, side->ui->sorder_edit,
			     side->ui->sorder_types_edit,
			     side->ui->sorder_task_edit);
    } else {
	map->modalhandler = aux_set_cell;
    }
}

void 
set_unit_from_map (Side *side, Map *map, char *prompt, int *puid)
{
    save_cur(side, map);
    raise_widget(XtParent(map->mainwidget));
    map->curtool = looktool;
    set_tool_cursor(side, map);
    update_controls(side, map);
    side->ui->grok_p1 = puid;
    ask_position(side, map, prompt, aux_set_unit);
}

static void
aux_set_unit(Side *side, Map *map, int cancel)
{
    int x, y;
    Unit *unit;
    Boolean state;

    if (cancel)
      return;
    if (grok_position(side, map, &x, &y, &unit)) {
/*	erase_current(side, map, x, y, NULL); */

	XtVaGetValues(side->ui->ordi_edit->toggle, XtNstate, &state, NULL);
	if (!state) {
	    beep(side);
	    return;
	}
	raise_widget(side->ui->orders_shell);

	if (map->curunit == NULL)
	  return;
	*((int *) side->ui->grok_p1) = map->curunit->id;

	update_order_widgets(side, side->ui->ordi_edit, side->ui->sorder_edit,
			     side->ui->sorder_types_edit,
			     side->ui->sorder_task_edit);
    } else {
	map->modalhandler = aux_set_unit;
    }
}

int 
find_side_and_ordi_via_widget(Widget w, Side **sidep, OrderInterface **ordip)
{
    OrderInterface *ordi;
    Widget form = XtParent(w);

    if (!find_side_via_widget(w, sidep))
      return 0;

    for (ordi = (*sidep)->ui->orderlist; ordi != NULL; ordi = ordi->next) {
	if (ordi->form == form) {
	    *ordip = ordi;
	    return 1;
	}
    }

    return 0;
}

void
deactivate_orders(Side *side)
{
    OrderInterface *ordi;

    /* de-activate active order */
    ordi = active_ordi(side);
    if (ordi) {
	XtVaSetValues(ordi->toggle, XtNstate, False, NULL);
	order_toggle_call(ordi->toggle, NULL, NULL);
    }
}

OrderInterface *
active_ordi(Side *side)
{
    OrderInterface *ordi;
    Boolean state;

    for (ordi = side->ui->orderlist; ordi != NULL; ordi = ordi->next) {
	if (ordi->toggle) {
	    XtVaGetValues(ordi->toggle, XtNstate, &state, NULL);
	    if (state)  return ordi;
	}
    }
    return NULL;
}

int
count_sorder(Side *side)
{
    StandingOrder *sorder;
    int num;

    for (sorder = side->orders,  num = 0;
	 sorder != NULL;
	 sorder = sorder->next,  num++);

    return num;
}

int
num_tasks(void)
{
    int i;

    for (i = 0; taskdefns[i].name != NULL; i++) {}
    return i;
}

/* debugging stuff */

static void 
orders_check (Side *side)
{
    StandingOrder *sorder;
    OrderInterface *ordi;
    int i;

    if (side->orders == NULL) {
	if (side->ui->orderlist) {
	    run_error("check #1 failed in orders_check for %s",
		      short_side_title(side));
	} else {
	    return;
	}
    }
    for (sorder = side->orders,  ordi = side->ui->orderlist, i = 0;
	 sorder != NULL && sorder->next != NULL;
	 sorder = sorder->next, ordi = ordi->next, i++) {
	if (ordi->sorder != sorder) {
	    run_error("check #2 failed in orders_check for %s at i = %d",
		      short_side_title(side), i);
	}
    }
    if (sorder->next != NULL || sorder != side->last_order ||
	ordi   == NULL || ordi->next   != NULL) {
	run_error("check #3 failed in orders_check for %s",
		  short_side_title(side));
    }

}
