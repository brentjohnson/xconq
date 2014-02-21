/* Print dialog for the X11 interface to Xconq.
   Copyright (C) 1994, 1995 Massimo Campostrini & Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/* This is a print setup dialog, originally written by
   Massimo Campostrini <campo@sunthpi3.difi.unipi.it> in May 1994. */

#include "conq.h"
#include "xtconq.h"

typedef struct a_widgetId {
  String      name;
  int         newline;
} WidgetId;

typedef struct a_choice {
  int number;
  String legends[5];
} Choice;

Choice choices[N_CHOICE] = {
  {3, 
     {"no names",
      "interesting names",
      "all names",
      NULL, NULL}
 }
};

static void print_ok(Side *side);
static void print_skip(Side *side);
static void open_print_help(Side *side);
static int w_comm_ind(Side *side, Widget w);
static void update_values(Side *side);
static void read_all(Side *side);

static void handle_do_print_dialog(Widget w, XEvent *event,
				   String *params, Cardinal *num_params);

static void print_setup_button_callback(Widget w, XtPointer client_data,
					XtPointer call_data);
static void metric_convert_callback(Widget w, XtPointer client_data,
				    XtPointer call_data);
static void done_print_help_callback(Widget w, XtPointer client_data,
				     XtPointer call_data);
static void handle_done_print_help(Widget w, XEvent *event,
				   String *params, Cardinal *num_params);

static XtActionsRec xprint_actions[] = {
  { "DoPrintDialog", handle_do_print_dialog },
  { "DonePrintHelp", handle_done_print_help },
};

WidgetId w_table[N_WIDGET] = {
  {"help",         1 },
  {"print",        0 },
  {"quit",         0 },
  {"names",        1 },
  {"coord",        0 },
  {"dither",       0 },
  {"double",       1 },
  {"features",     0 },
  {"summary",      0 },
  {"metric",       0 },
  {"cellSize",     1 },
  {"gridSize",     0 },
  {"borderWidth",  1 },
  {"connWidth",    0 },
  {"pageWidth",    1 },
  {"pageHeight",   0 },
  {"topMargin",    1 },
  {"bottomMargin", 0 },
  {"leftMargin",   1 },
  {"rightMargin",  0 },
  {"terrainGray",  1 },
  {"enemyGray",    0 },
};

static String helpText =
"Modify values, then press \"print\" (or \"quit\" to skip printing).\n\
 \n\
Multiple choices (click to cycle)\n\
o    print unit names:  no names, interesting names, all names.\n\
 \n\
Binary options (dark background = set)\n\
o    coord:    print coordinates of map corners\n\
o    dither:   use dithering to print terrain gray\n\
o    double:   map a pixel into 2x2 pixels wile dithering\n\
o    features: print geographical features\n\
o    summary:  print summary of units in cell\n\
o    metric:   print distances in centimetres (otherwise in inches)\n\
 \n\
Numerical options (accept floating-point values)\n\
o    cell size:  printed size of hexagonal cells\n\
o    grid thick.:  thickness of cell grid (0 to suppress)\n\
o    border/conn thick.:  thickness of borders and connections\n\
o    page width/height:  dimension of sheets to print on\n\
o    top/bottom/left/right margin:  margins of the region where\n\
cells are printed; labels and titles are printed in the margins\n\
o    border/connection thickness\n\
o    terrain gray:  from 0 (black) to 1 (white)\n\
o    enemy gray:    from 0 (black) to 1 (white), [not implemented]";

int
popup_print_setup_dialog(Side *side)
{
    int i;
    Widget w_form, w_left, w_up, w_oldup;
    WidgetClass Class;
    
    XtAppAddActions(thisapp, xprint_actions, XtNumber(xprint_actions));
    
    if (!side->ui->print_shell) {
	side->ui->print_shell =
	  XtVaCreatePopupShell("PrintSetup", topLevelShellWidgetClass,
			       side->ui->shell, NULL);
  
	w_form = XtVaCreateManagedWidget ("form", formWidgetClass, side->ui->print_shell, NULL);
	w_left = w_up = w_oldup = NULL;
  
	for (i = 0; i < N_WIDGET; i++) {
	    if (i < N_BUTTON)
	      Class = commandWidgetClass;
	    else if (i < N_BUTTON + N_TOGGLE)
	      Class = toggleWidgetClass;
	    else
	      Class = dialogWidgetClass;
    
	    side->ui->print_cmds[i] =
	      XtVaCreateManagedWidget (w_table[i].name, Class, w_form,
				       NULL);
	    if (w_table[i].newline) {
		w_oldup = w_up;
		w_up = side->ui->print_cmds[i];
		w_left = NULL;
	    }
	    XtVaSetValues(side->ui->print_cmds[i],
			  XtNfromVert, w_oldup,
			  XtNfromHoriz, w_left,
			  NULL);
	    w_left = side->ui->print_cmds[i];
    
	    if (strcmp(w_table[i].name, "metric") == 0) {
		XtAddCallback (side->ui->print_cmds[i], XtNcallback,
			       metric_convert_callback, NULL);
		side->ui->i_metric = i;
	    }
	    /* yes, we really need to cast commandWidgetClass into WidgetClass */
	    if (Class == (WidgetClass) commandWidgetClass) {
		XtAddCallback (side->ui->print_cmds[i], XtNcallback,
			       print_setup_button_callback, NULL);
	    }
	}
  
	XtRealizeWidget (side->ui->print_shell);
    }    
    
    /* unelegant... */
    side->ui->choi[ 0] = side->ui->ps_pp->names;
    side->ui->flag[ 0] = side->ui->ps_pp->corner_coord;
    side->ui->flag[ 1] = side->ui->ps_pp->terrain_dither;
    side->ui->flag[ 2] = side->ui->ps_pp->terrain_double;
    side->ui->flag[ 3] = side->ui->ps_pp->features;
    side->ui->flag[ 4] = side->ui->ps_pp->cell_summary;
    side->ui->flag[ 5] = side->ui->ps_pp->cm;
    side->ui->parm[ 0] = side->ui->ps_pp->cell_size;
    side->ui->parm[ 1] = side->ui->ps_pp->cell_grid_width;
    side->ui->parm[ 2] = side->ui->ps_pp->border_width;
    side->ui->parm[ 3] = side->ui->ps_pp->connection_width;
    side->ui->parm[ 4] = side->ui->ps_pp->page_width;
    side->ui->parm[ 5] = side->ui->ps_pp->page_height;
    side->ui->parm[ 6] = side->ui->ps_pp->top_margin;
    side->ui->parm[ 7] = side->ui->ps_pp->bottom_margin;
    side->ui->parm[ 8] = side->ui->ps_pp->left_margin;
    side->ui->parm[ 9] = side->ui->ps_pp->right_margin;
    side->ui->parm[10] = side->ui->ps_pp->terrain_gray;
    side->ui->parm[11] = side->ui->ps_pp->enemy_gray;
    update_values(side);
    
    XtPopup(side->ui->print_shell, XtGrabNone);
    
    return 0;
}

static void
print_ok(Side *side)
{
    double conv;

    /* unelegant... */
    side->ui->ps_pp->names =		side->ui->choi[ 0];
    side->ui->ps_pp->corner_coord =		side->ui->flag[ 0];
    side->ui->ps_pp->terrain_dither =	side->ui->flag[ 1];
    side->ui->ps_pp->terrain_double =	side->ui->flag[ 2];
    side->ui->ps_pp->features =		side->ui->flag[ 3];
    side->ui->ps_pp->cell_summary =		side->ui->flag[ 4];
    side->ui->ps_pp->cm =			side->ui->flag[ 5];
    side->ui->ps_pp->cell_size =		side->ui->parm[ 0];
    side->ui->ps_pp->cell_grid_width =	side->ui->parm[ 1];
    side->ui->ps_pp->border_width =		side->ui->parm[ 2];
    side->ui->ps_pp->connection_width =	side->ui->parm[ 3];
    side->ui->ps_pp->page_width =		side->ui->parm[ 4];
    side->ui->ps_pp->page_height =		side->ui->parm[ 5];
    side->ui->ps_pp->top_margin =		side->ui->parm[ 6];
    side->ui->ps_pp->bottom_margin =	side->ui->parm[ 7];
    side->ui->ps_pp->left_margin =		side->ui->parm[ 8];
    side->ui->ps_pp->right_margin =		side->ui->parm[ 9];
    side->ui->ps_pp->terrain_gray =		side->ui->parm[10];
    side->ui->ps_pp->enemy_gray =		side->ui->parm[11];
    if (side->ui->print_shell) {
	XtPopdown (side->ui->print_shell);
    }
    if (side->ui->print_help_shell) {
	XtPopdown (side->ui->print_help_shell);
    }
    /* convert from cm or in */
    if (side->ui->ps_pp->cm) {
	conv = 72/2.54;
    } else {
	conv = 72;
    }
    side->ui->ps_pp->cell_size *= conv;
    side->ui->ps_pp->cell_grid_width *= conv;
    side->ui->ps_pp->border_width *= conv;
    side->ui->ps_pp->connection_width *= conv;
    side->ui->ps_pp->page_width *= conv;
    side->ui->ps_pp->page_height *= conv;
    side->ui->ps_pp->top_margin *= conv;
    side->ui->ps_pp->bottom_margin *= conv;
    side->ui->ps_pp->left_margin *= conv;
    side->ui->ps_pp->right_margin *= conv;
    
    /* dump view */    
    notify(side, "dumping view to file \"view.xconq\" ...");
    flush_output(side);
    dump_ps_view(side, side->ui->ps_pp, "view.xconq");
    notify(side, "... done.");
    
}

static void 
print_skip(Side *side)
{
    double conv;

    if (side->ui->print_shell)
      XtPopdown (side->ui->print_shell);
    if (side->ui->print_help_shell)
      XtPopdown (side->ui->print_help_shell);
 
    /* Convert from cm or in. */
    conv = 72;
    if (side->ui->ps_pp->cm)
      conv /= 2.54;

    side->ui->ps_pp->cell_size *= conv;
    side->ui->ps_pp->cell_grid_width *= conv;
    side->ui->ps_pp->border_width *= conv;
    side->ui->ps_pp->connection_width *= conv;
    side->ui->ps_pp->page_width *= conv;
    side->ui->ps_pp->page_height *= conv;
    side->ui->ps_pp->top_margin *= conv;
    side->ui->ps_pp->bottom_margin *= conv;
    side->ui->ps_pp->left_margin *= conv;
    side->ui->ps_pp->right_margin *= conv;
}

static void 
update_values(Side *side)
{
    char buffer[80];
    int i;
    char flg;
    Arg args[1];

    for (i = 0; i < N_CHOICE; i++) {
	XtSetArg(args[0], XtNlabel,
		 (XtArgVal) choices[i].legends[side->ui->choi[i]]);
	XtSetValues(side->ui->print_cmds[i + N_COMMAND], args, 1);
    }
    for (i = 0; i < N_TOGGLE; i++) {
	flg = side->ui->flag[i];
	XtSetArg(args[0], XtNstate, (XtArgVal) flg);
	XtSetValues(side->ui->print_cmds[i + N_BUTTON], args, 1);
    }
    for (i = 0; i < N_DIALOG; i++) {
	sprintf(buffer, "%.11g", side->ui->parm[i]);
	XtSetArg(args[0], XtNvalue, (XtArgVal) buffer);
	XtSetValues(side->ui->print_cmds[i + N_BUTTON + N_TOGGLE], args, 1);
    }
}

static int
w_comm_ind(Side *side, Widget w)
{
    int i;

    for (i = 0; i <= N_WIDGET; i++) {
	if (w == side->ui->print_cmds[i])
	  return i;
    }
    fprintf(stderr, "w_comm_ind: widget not found\n");
    return -1;
}

static void
handle_do_print_dialog(Widget w, XEvent *event, String *params, Cardinal *numparms)
{
    Side *side;

    if (!find_side_via_widget(w, &side))
      return;

    read_all(side);
    update_values(side);
}

static void 
read_all(Side *side)
{
    String string;
    int i;
    double result;
    char flg;
    Arg args[1];

    for (i = 0; i < N_TOGGLE; i++) {
	XtSetArg(args[0], XtNstate, (XtArgVal) &flg);
	XtGetValues(side->ui->print_cmds[i + N_BUTTON], args, 1);
	side->ui->flag[i] = flg;
    }
    for (i = 0; i < N_DIALOG; i++) {
	string = XawDialogGetValueString(side->ui->print_cmds[i + N_BUTTON + N_TOGGLE]);
	sscanf(string, "%lf", &result);
	side->ui->parm[i] = result;
    }
}

static void 
print_setup_button_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
    int i;
    Side *side;

    if (!find_side_via_widget(w, &side))
      return;

    i = w_comm_ind(side, w);

    read_all(side);

    if (i == 0) {
	open_print_help(side); 
    } else if (i == 1) {
	print_ok(side);
    } else if (i == 2) {
        print_skip(side);
    } else if (i >= N_COMMAND) {
	i -= N_COMMAND;
	side->ui->choi[i] = (side->ui->choi[i] + 1) % choices[i].number;
    }
    update_values(side);
}

/* Convert in to cm, or vice versa. */

static void 
metric_convert_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
    char flg;
    int i;
    double conv;
    Arg args[1];
    Side *side;

    if (!find_side_via_widget(w, &side))
      return;

    XtSetArg(args[0], XtNstate, (XtArgVal) &flg);
    XtGetValues(side->ui->print_cmds[side->ui->i_metric], args, 1);
    conv = flg ? 2.54 : (1 / 2.54);
    read_all(side);
    for (i = 0; i < N_DIMEN_D; i++) {
	side->ui->parm[i] *= conv;
    }
    update_values(side);
}

static void
open_print_help(Side *side)
{
    Widget w_form, w_done, w_text;

    if (!side->ui->print_help_shell) {
	/* If first time, create it. */
	side->ui->print_help_shell =
	  XtVaCreatePopupShell("PrintSetupHelp", topLevelShellWidgetClass,
			       side->ui->shell,
			       NULL);
	w_form =
	  XtVaCreateManagedWidget("helpForm", formWidgetClass, side->ui->print_help_shell,
				  NULL);
	w_text =
	  XtVaCreateManagedWidget("helpText", labelWidgetClass, w_form,
				  XtNlabel, helpText,
				  NULL);
	w_done =
	  XtVaCreateManagedWidget("helpDone", commandWidgetClass, w_form,
				  XtNfromVert, w_text,
				  NULL);
	XtAddCallback(w_done, XtNcallback, done_print_help_callback, NULL);
    }

    XtPopup(side->ui->print_help_shell, XtGrabNone);
}

static void 
done_print_help_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
    Side *side;

    if (!find_side_via_widget(w, &side))
      return;

    if (side->ui->print_help_shell)
      XtPopdown(side->ui->print_help_shell);
}


static void
handle_done_print_help(Widget w, XEvent *event, String *params, Cardinal *numparms)
{
    Side *side;

    if (!find_side_via_widget(w, &side))
      return;

    if (side->ui->print_help_shell)
      XtPopdown(side->ui->print_help_shell);
}
