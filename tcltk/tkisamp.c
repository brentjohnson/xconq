/* Imf sample list code for the tcl/tk interface to Xconq.
   Copyright (C) 1998-2000 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/* This file implements the "imfsample" widget, which is used for
   displaying a list of given image families in a gridded pattern.
   The case of one image family is useful for displaying images
   embedded in other windows. */

/* Note that this widget depends only on image machinery, and has
   no knowledge of Xconq constructs such as units and sides. */

#ifdef __cplusplus
extern "C" {
#endif

/* For tcltk 8.4.0 source compatibility. */
#define USE_NON_CONST

#ifdef MAC
#include <tclMacCommonPch.h>
#include <tclMac.h>			/* Includes tcl.h. */
#include <tkMac.h>			/* Includes tk.h. */
#else
#include <tcl.h>
#include <tk.h>
#endif /* MAC */

#ifdef __cplusplus
}
#endif

#include <math.h>

#include "config.h"
#include "misc.h"
#include "lisp.h"
#include "imf.h"

#ifndef MAC
#include "xlibstuff.h"
#endif

#include "tkimf.h"

extern Display *tmp_display;
extern Tcl_Interp *interp;
extern int tmp_valid;

static XColor *black_color = NULL;
static XColor *white_color = NULL;

/* Image family list display widget. */

typedef struct {
    Tk_Window tkwin;			/* Window that embodies the imfsample.  NULL
				 		* means window has been deleted but
				 		* widget record hasn't been cleaned up yet. */
    Display *display;			/* X's token for the window's display. */
    Tcl_Interp *interp;			/* Interpreter associated with widget. */
    Tcl_Command widgetCmd;		/* Token for imfsample's widget command. */
    int border_width;			/* Width of 3-D border around whole widget. */
    Tk_3DBorder bg_border;		/* Used for drawing background. */
    Tk_3DBorder fg_border;		/* Used for drawing foreground. */
    Tk_3DBorder cu_border;		/* Used for drawing closeups. */
    int relief;				/* Indicates whether window as a whole is
				 		* raised, sunken, or flat. */
    GC copygc;				/* Graphics context for copying from
						 * off-screen pixmap onto screen. */
    GC gc;
    int double_buffer;			/* Non-zero means double-buffer redisplay
						 * with pixmap;  zero means draw straight
						 * onto the display. */
    int update_pending;			/* Non-zero means a call to imfsample_display
				 		* has already been scheduled. */
 
    int pad;					/* Extra space between images. */
    int iwidth, iheight;
    int show_color;
    int show_names;
    int show_masks;
    int show_bbox;
    int show_solid;
    int show_grid;

    int width, height;

    XColor *fill_color;

    char *main_imf_name;
    int numimages;
    ImageFamily **imf_list;

    int with_terrain;
    int with_emblem;

    int eltw, elth;
    int rows, cols;

    int numvisrows;
    int firstvisrow;
    
    /* IMFAapp-specific stuff. */
    int imfapp;				/* True if we are running IMFAapp. */
    int selected;				/* The selected image in IMFAapp. */
    int previous;				/* The previously selected image. */
    int oldfirst;				/* Cache of first visible row. */
    int redraw;				/* True if the widget should be redrawn. */

} Imfsample;

static Tk_ConfigSpec config_specs[] = {
    {TK_CONFIG_BORDER, "-background", "background", "Background",
	"white", Tk_Offset(Imfsample, bg_border), TK_CONFIG_COLOR_ONLY},
    {TK_CONFIG_SYNONYM, "-bd", "borderWidth", (char *) NULL,
	(char *) NULL, 0, 0},
    {TK_CONFIG_SYNONYM, "-bg", "background", (char *) NULL,
	(char *) NULL, 0, 0},
    {TK_CONFIG_PIXELS, "-borderwidth", "borderWidth", "BorderWidth",
	"0", Tk_Offset(Imfsample, border_width), 0},
    {TK_CONFIG_INT, "-dbl", "doubleBuffer", "DoubleBuffer",
	"1", Tk_Offset(Imfsample, double_buffer), 0},
    {TK_CONFIG_SYNONYM, "-fg", "foreground", (char *) NULL,
	(char *) NULL, 0, 0},
    {TK_CONFIG_BORDER, "-foreground", "foreground", "Foreground",
	"black", Tk_Offset(Imfsample, fg_border), TK_CONFIG_COLOR_ONLY},
    {TK_CONFIG_BORDER, "-closeup", "closeup", "Closeup",
	"white", Tk_Offset(Imfsample, cu_border), TK_CONFIG_COLOR_ONLY},
    {TK_CONFIG_PIXELS, "-height", "height", "Height",
	"0", Tk_Offset(Imfsample, height), 0},
#if 0 /* disabled until we debug */
    {TK_CONFIG_STRING, "-imf", "imf", "Imf",
	"0", Tk_Offset(Imfsample, main_imf_name), 0},
#endif
    {TK_CONFIG_INT, "-iheight", "iheight", "IHeight",
	"0", Tk_Offset(Imfsample, iheight), 0},
    {TK_CONFIG_INT, "-iwidth", "iwidth", "IWidth",
	"0", Tk_Offset(Imfsample, iwidth), 0},
    {TK_CONFIG_INT, "-pad", "pad", "Pad",
	"0", Tk_Offset(Imfsample, pad), 0},
    {TK_CONFIG_RELIEF, "-relief", "relief", "Relief",
	"raised", Tk_Offset(Imfsample, relief), 0},
    {TK_CONFIG_INT, "-showbbox", "showBBox", "ShowBBox",
	"0", Tk_Offset(Imfsample, show_bbox), 0},
    {TK_CONFIG_INT, "-showcolor", "showColor", "ShowColor",
	"1", Tk_Offset(Imfsample, show_color), 0},
    {TK_CONFIG_INT, "-showgrid", "showGrid", "ShowGrid",
	"0", Tk_Offset(Imfsample, show_grid), 0},
    {TK_CONFIG_INT, "-shownames", "showNames", "ShowNames",
	"0", Tk_Offset(Imfsample, show_names), 0},
    {TK_CONFIG_INT, "-showmasks", "showMasks", "ShowMasks",
	"1", Tk_Offset(Imfsample, show_masks), 0},
    {TK_CONFIG_INT, "-showsolid", "showSolid", "ShowSolid",
	"0", Tk_Offset(Imfsample, show_solid), 0},
    {TK_CONFIG_INT, "-imfapp", "imfApp", "ImfApp",
	"0", Tk_Offset(Imfsample, imfapp), 0},
    {TK_CONFIG_COLOR, "-fillcolor", "fillColor", "FillColor",
	"gray", Tk_Offset(Imfsample, fill_color), 0},
    {TK_CONFIG_PIXELS, "-width", "width", "Width",
	"0", Tk_Offset(Imfsample, width), 0},
    {TK_CONFIG_PIXELS, "-emblem", "emblem", "Emblem",
	"-1", Tk_Offset(Imfsample, with_emblem), 0},
    {TK_CONFIG_PIXELS, "-terrain", "terrain", "Terrain",
	"-1", Tk_Offset(Imfsample, with_terrain), 0},
    {TK_CONFIG_END, (char *) NULL, (char *) NULL, (char *) NULL,
	(char *) NULL, 0, 0}
};

static void imfsample_cmd_deleted_proc(ClientData cldata);
static int imfsample_configure(Tcl_Interp *interp, Imfsample *imfsample,
			       int argc, char **argv, int flags);
static void imfsample_destroy(char *memPtr);
static void imfsample_display(ClientData cldata);
static void imfsample_event_proc(ClientData cldata, XEvent *eventPtr);
static int imfsample_widget_cmd(ClientData cldata, Tcl_Interp *interp,
				int argc, char **argv);
static void imfsample_add_imf(Imfsample *imfsample, char *imfname);
static void imfsample_replace_imf(Imfsample *imfsample, char *imfname);
static void imfsample_replace_emblem(Imfsample *imfsample, char *imfname);
static void imfsample_remove_imf(Imfsample *imfsample, char *imfname);
static void draw_one_main_image(Imfsample *imfsample, Drawable d, GC gc,
				ImageFamily *imf,
				int sx, int sy, int sw, int sh);
static void draw_one_image(Imfsample *imfsample, Drawable d, GC gc,
			   Image *img, int sx, int sy, int sw, int sh);

/* The command to create an image sample window. */

int
imfsample_cmd(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    Tk_Window mainw = (Tk_Window) cldata;
    Imfsample *imfsample;
    Tk_Window tkwin;

    if (argc < 2) {
	Tcl_AppendResult(interp, "wrong # args: should be \"",
		argv[0], " pathName ?options?\"", (char *) NULL);
	return TCL_ERROR;
    }

    tkwin = Tk_CreateWindowFromPath(interp, mainw, argv[1], (char *) NULL);
    if (tkwin == NULL)
      return TCL_ERROR;

    Tk_SetClass(tkwin, "Imfsample");

    /* Allocate and initialize the widget record.  */

    imfsample = (Imfsample *) ckalloc(sizeof(Imfsample));
    imfsample->tkwin = tkwin;
    imfsample->display = Tk_Display(tkwin);
    imfsample->interp = interp;
    imfsample->widgetCmd =
      Tcl_CreateCommand(interp,
			Tk_PathName(imfsample->tkwin), imfsample_widget_cmd,
			(ClientData) imfsample, imfsample_cmd_deleted_proc);
    imfsample->border_width = 0;
    imfsample->bg_border = NULL;
    imfsample->fg_border = NULL;
    imfsample->cu_border = NULL;
    imfsample->relief = TK_RELIEF_FLAT;
    imfsample->copygc = None;
    imfsample->gc = None;
    imfsample->double_buffer = 1;
    imfsample->update_pending = 0;
    imfsample->show_color = 1;
    imfsample->show_names = 0;
    imfsample->show_masks = 0;
    imfsample->show_grid = 0;
    imfsample->fill_color = NULL;
    
    imfsample->with_terrain = -1;
    imfsample->with_emblem = -1;

    imfsample->main_imf_name = "";
    imfsample->numimages = 0;
    imfsample->imf_list =
      (ImageFamily **) xmalloc(MAXIMAGEFAMILIES * sizeof(ImageFamily *));
    imfsample->numvisrows = 0;
    imfsample->firstvisrow = 0;

    /* IMFApp-specific stuff. */
    imfsample->imfapp = 0;
    imfsample->selected = -1;
    imfsample->previous = -1;
    imfsample->oldfirst = 0;    
    imfsample->redraw = 0;    
    
    Tk_CreateEventHandler(imfsample->tkwin, ExposureMask|StructureNotifyMask,
			  imfsample_event_proc, (ClientData) imfsample);
    if (imfsample_configure(interp, imfsample, argc-2, argv+2, 0) != TCL_OK) {
	Tk_DestroyWindow(imfsample->tkwin);
	return TCL_ERROR;
    }

    interp->result = Tk_PathName(imfsample->tkwin);
    return TCL_OK;
}

static int
imfsample_widget_cmd(ClientData cldata, Tcl_Interp *interp, int argc, char **argv)
{
    Imfsample *imfsample = (Imfsample *) cldata;
    int result = TCL_OK;
    size_t cmdlength;
    char c;
    int x, y, col, row, n;
    char tclbuf[100];
    int rslt;

    if (argc < 2) {
	Tcl_AppendResult(interp, "wrong # args: should be \"",
		argv[0], " option ?arg arg ...?\"", (char *) NULL);
	return TCL_ERROR;
    }
    Tcl_Preserve((ClientData) imfsample);
    c = argv[1][0];
    cmdlength = strlen(argv[1]);
    if ((c == 'c') && (strncmp(argv[1], "cget", cmdlength) == 0)
	&& (cmdlength >= 2)) {
	if (argc != 3) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"",
		    argv[0], " cget option\"",
		    (char *) NULL);
	    goto error;
	}
	result = Tk_ConfigureValue(interp, imfsample->tkwin, config_specs,
		(char *) imfsample, argv[2], 0);
    } else if ((c == 'c') && (strncmp(argv[1], "configure", cmdlength) == 0)
	       && (cmdlength >= 2)) {
	if (argc == 2) {
	    result = Tk_ConfigureInfo(interp, imfsample->tkwin, config_specs,
		    (char *) imfsample, (char *) NULL, 0);
	} else if (argc == 3) {
	    result = Tk_ConfigureInfo(interp, imfsample->tkwin, config_specs,
		    (char *) imfsample, argv[2], 0);
	} else {
	    result = imfsample_configure(interp, imfsample, argc-2, argv+2,
		    TK_CONFIG_ARGV_ONLY);
	}
    } else if ((c == 'c') && (strncmp(argv[1], "curselection", cmdlength) == 0)
	       && (cmdlength >= 2)) {
	sprintf(interp->result, "%d", imfsample->selected);
    } else if ((c == 'a') && (strncmp(argv[1], "add", cmdlength) == 0)
	       && (cmdlength >= 2)) {
	if (strcmp(argv[2], "imf") == 0) {
	    imfsample_add_imf(imfsample, argv[3]);
	} else if (strcmp(argv[2], "all") == 0) {
	    imfsample_add_imf(imfsample, "-all");
	}
    } else if ((c == 'e') && (strncmp(argv[1], "emblem", cmdlength) == 0)
	       && (cmdlength >= 2)) {
	n = strtol(argv[2], NULL, 10);
	imfsample->with_emblem = n;
    } else if ((c == 'r') && (strncmp(argv[1], "redraw", cmdlength) == 0)
	       && (cmdlength >= 2)) {
	imfsample->redraw = TRUE;
	if (!imfsample->update_pending) {
		Tcl_DoWhenIdle(imfsample_display, cldata);
		imfsample->update_pending = 1;
	}
    } else if ((c == 'r') && (strncmp(argv[1], "replace", cmdlength) == 0)
	       && (cmdlength >= 2)) {
	if (strcmp(argv[2], "imf") == 0) {
	    imfsample_replace_imf(imfsample, argv[3]);
	} else if (strcmp(argv[2], "emblem") == 0) {
	    imfsample_replace_emblem(imfsample, argv[3]);
	}
    } else if ((c == 'r') && (strncmp(argv[1], "remove", cmdlength) == 0)
	       && (cmdlength >= 2)) {
	if (strcmp(argv[2], "imf") == 0) {
	    imfsample_remove_imf(imfsample, argv[3]);
	} else if (strcmp(argv[2], "emblem") == 0) {
	    imfsample->numimages = 1;
	} else if (strcmp(argv[2], "all") == 0) {
	    imfsample_remove_imf(imfsample, "-all");
	    if (imfsample->imfapp) {
	    	    imfsample->selected = -1;
	    	    imfsample->previous = -1;
		    /* Turn off the scrollbar. */
		    sprintf(tclbuf, ".images.scroll set 0 1");
		    rslt = Tcl_Eval(interp, tclbuf);
		    if (rslt == TCL_ERROR) {
			fprintf(stderr, "Error: %s\n", interp->result);
		    }
	    }
	}
    } else if ((c == 's') && (strncmp(argv[1], "select", cmdlength) == 0)
	       && (cmdlength >= 2)) {
	if (imfsample->numimages) {
		x = strtol(argv[2], NULL, 10);
		y = strtol(argv[3], NULL, 10);
		col = x / imfsample->eltw;
		row = y / imfsample->elth + imfsample->firstvisrow;
		n = row * imfsample->cols + col;
		if (n < 0 
		    || n >= imfsample->numimages
		    || col >= imfsample->cols) {
			n = -1;
		}
		/* This rather complicated scheme is to ensure that we can both select
		images and then deselect them by clicking a second time on the image or
		by clicking in an empty region. */
		if (imfsample->selected != n) {
			imfsample->previous = imfsample->selected;
			imfsample->selected = n;
		} else if (n == -1) {
			imfsample->selected = -1;
		} else {
			imfsample->previous = n;
			imfsample->selected = -1;
		}
	}
    } else if ((c == 't') && (strncmp(argv[1], "terrain", cmdlength) == 0)
	       && (cmdlength >= 2)) {
	n = strtol(argv[2], NULL, 10);
	imfsample->with_terrain = n;
    } else if ((c == 'x') && (strncmp(argv[1], "xview", cmdlength) == 0)) {
    } else if ((c == 'y') && (strncmp(argv[1], "yview", cmdlength) == 0)) {
	int count, type, nrow = imfsample->firstvisrow;
	double fraction, fraction2;

	type = Tk_GetScrollInfo(interp, argc, argv, &fraction, &count);
	switch (type) {
		  case TK_SCROLL_ERROR:
		goto error;
		  case TK_SCROLL_MOVETO:
		nrow = fraction * imfsample->rows;
		break;
		  case TK_SCROLL_PAGES:
		nrow += (count * imfsample->numvisrows * 4) / 5;
		break;
		  case TK_SCROLL_UNITS:
		nrow += count;
		break;
	}
	/* Don't allow negative row numbers. */
	imfsample->firstvisrow = max(0, nrow);
	/* Compute the bounds of the visible window. */
	fraction = (double) imfsample->firstvisrow / imfsample->rows;
	fraction2 = (double) (imfsample->firstvisrow + imfsample->numvisrows) / imfsample->rows;
	if (imfsample->imfapp) {
		/* We must set the scrollbar explicitly since the imfsample widget
		lacks a built-in yscrollcommand. */
		sprintf(tclbuf, ".images.scroll set %f %f", fraction, fraction2);
		rslt = Tcl_Eval(interp, tclbuf);
		if (rslt == TCL_ERROR) {
			fprintf(stderr, "Error: %s\n", interp->result);
		}
	}
    } else {
	Tcl_AppendResult(interp, "bad option \"", argv[1],
		"\": must be cget, configure, position, replace, remove, select",
		(char *) NULL);
	goto error;
    }
    if (!imfsample->update_pending) {
	Tcl_DoWhenIdle(imfsample_display, (ClientData) imfsample);
	imfsample->update_pending = 1;
    }
    Tcl_Release((ClientData) imfsample);
    return result;

error:
    Tcl_Release((ClientData) imfsample);
    return TCL_ERROR;
}

static void
imfsample_add_imf(Imfsample *imfsample, char *imfname)
{
    int i;
    ImageFamily *imf = NULL;
    char tclbuf[100];
    int rslt;

    if (strcmp(imfname, "-all") == 0) {
	imfsample->numimages = 0;
	for (i = 0; i < numimages; ++i) {
	    if (imfsample->imfapp) {
		sprintf(tclbuf, 
".closeup.content itemconfigure status -text \"Loading %s\"", 
			images[i]->name);
		rslt = Tcl_Eval(interp, tclbuf);
		if (rslt == TCL_ERROR) {
		    fprintf(stderr, "Error: %s\n", interp->result);
		}
		sprintf(tclbuf, "update idletasks");
		rslt = Tcl_Eval(interp, tclbuf);
		if (rslt == TCL_ERROR) {
		    fprintf(stderr, "Error: %s\n", interp->result);
		}
	    }
	    imf = tk_find_imf(images[i]->name);
	    if (imf == NULL) {
		fprintf(stderr, "Missing imf %s\n", imfname);
		return;
	    }
	    imfsample->imf_list[imfsample->numimages++] = images[i];
	}
	return;
    }
    imf = tk_find_imf(imfname);
    if (imf == NULL) {
	fprintf(stderr, "Missing imf %s\n", imfname);
	return;
    }
    imfsample->imf_list[imfsample->numimages++] = imf;
}

static void
imfsample_replace_imf(Imfsample *imfsample, char *imfname)
{
    ImageFamily *imf = NULL;

    if (empty_string(imfname)) {
	imfsample->imf_list[0] = NULL;
	return;
    }
    if (imfsample->numimages == 0) {
	imfsample_add_imf(imfsample, imfname);
	return;
    }
    imf = tk_find_imf(imfname);
    if (imf == NULL) {
	fprintf(stderr, "Missing imf %s\n", imfname);
	return;
    }
    imfsample->imf_list[0] = imf;
}

static void
imfsample_replace_emblem(Imfsample *imfsample, char *imfname)
{
    ImageFamily *imf = NULL;

    if (imfsample->numimages == 1) {
	imfsample_add_imf(imfsample, imfname);
	return;
    }
    imf = tk_find_imf(imfname);
    if (imf == NULL) {
	fprintf(stderr, "Missing imf %s\n", imfname);
	return;
    }
    imfsample->imf_list[1] = imf;
}

extern int numtkimages;

static void
imfsample_remove_imf(Imfsample *imfsample, char *imfname)
{
    if (imfsample->imfapp) {
	    if (strcmp(imfname, "-all") == 0) {
		numtkimages = 0;
		numimages = 0;
	    }
    }
    imfsample->numimages = 0;
}

static int
imfsample_configure(Tcl_Interp *interp, Imfsample *imfsample,
		    int argc, char **argv, int flags)
{
    /* Interpret the configuration arguments according to the specs. */
    if (Tk_ConfigureWidget(interp, imfsample->tkwin, config_specs,
			   argc, argv, (char *) imfsample, flags) != TCL_OK)
      return TCL_ERROR;

    /* Set the background for the window and create a graphics context
       for use during redisplay.  */
    Tk_SetWindowBackground(imfsample->tkwin,
			   Tk_3DBorderColor(imfsample->fg_border)->pixel);
    Tk_SetWindowBackground(imfsample->tkwin,
			   Tk_3DBorderColor(imfsample->bg_border)->pixel);
    Tk_SetWindowBackground(imfsample->tkwin,
			   Tk_3DBorderColor(imfsample->cu_border)->pixel);
    if ((imfsample->copygc == None) && 1/*imfsample->double_buffer*/) {
	XGCValues gcValues;

	gcValues.function = GXcopy;
	gcValues.graphics_exposures = False;
	imfsample->copygc = XCreateGC(imfsample->display,
				      DefaultRootWindow(imfsample->display),
				      GCFunction|GCGraphicsExposures,
				      &gcValues);
    }
    if (imfsample->gc == None) {
	imfsample->gc = XCreateGC(imfsample->display,
				  DefaultRootWindow(imfsample->display),
				  None, NULL);
    }

    /* Register the desired geometry for the window, then arrange for
       the window to be redisplayed.  */
    Tk_GeometryRequest(imfsample->tkwin, imfsample->width, imfsample->height);
    Tk_SetInternalBorder(imfsample->tkwin, imfsample->border_width);
    /* Make sure the resized widget is redrawn. */
    imfsample->redraw = TRUE;
    if (!imfsample->update_pending) {
	Tcl_DoWhenIdle(imfsample_display, (ClientData) imfsample);
	imfsample->update_pending = 1;
    }
    return TCL_OK;
}

static void
imfsample_event_proc(ClientData cldata, XEvent *eventPtr)
{
    Imfsample *imfsample = (Imfsample *) cldata;

    if (eventPtr->type == Expose) {
	if (!imfsample->update_pending) {
		Tcl_DoWhenIdle(imfsample_display, cldata);
		imfsample->update_pending = 1;
	}
    } else if (eventPtr->type == ConfigureNotify) {
	if (!imfsample->update_pending) {
		Tcl_DoWhenIdle(imfsample_display, cldata);
		imfsample->update_pending = 1;
	}
    } else if (eventPtr->type == DestroyNotify) {
	if (imfsample->tkwin != NULL) {
	    imfsample->tkwin = NULL;
	    Tcl_DeleteCommand(imfsample->interp,
			      Tcl_GetCommandName(imfsample->interp,
						 imfsample->widgetCmd));
	}
	if (imfsample->update_pending) {
	    Tcl_CancelIdleCall(imfsample_display, cldata);
	}
	Tcl_EventuallyFree(cldata, imfsample_destroy);
    }
}

static void
imfsample_cmd_deleted_proc(ClientData cldata)
{
    Imfsample *imfsample = (Imfsample *) cldata;
    Tk_Window tkwin = imfsample->tkwin;

    if (tkwin != NULL) {
	imfsample->tkwin = NULL;
	Tk_DestroyWindow(tkwin);
    }
}

/* Needed for drawing directly to the screen on the Mac. */
static Tk_3DBorder black_border = NULL;
static Tk_3DBorder white_border = NULL;

static void
imfsample_display(ClientData cldata)
{
    char *str;
    int row, col, namex, namey, n, sx, sy, update = FALSE, done;
    Imfsample *imfsample = (Imfsample *) cldata;
    Display *dpy = imfsample->display;
    Tk_Window tkwin = imfsample->tkwin;
    GC gc;
    Pixmap pm = None;
    Drawable d;
    Tk_Font tkfont;
    int winwidth = Tk_Width(Tk_Parent(tkwin)); 
    int winheight = Tk_Height(Tk_Parent(tkwin));
    char tclbuf[100];
    int rslt;
	
    imfsample->update_pending = 0;
    if (!Tk_IsMapped(tkwin)) {
	return;
    }
    /* Check if we need to redraw the entire imfsample. */
    if (imfsample->imfapp) {
	if (imfsample->oldfirst != imfsample->firstvisrow
	    || imfsample->redraw) {
		imfsample->oldfirst = imfsample->firstvisrow;
		update = TRUE;
	}
    }
    /* Create a pixmap for double-buffering if necessary. */
    if (imfsample->double_buffer) {
	update = TRUE;
	pm = Tk_GetPixmap(imfsample->display, Tk_WindowId(tkwin),
			  Tk_Width(tkwin), Tk_Height(tkwin),
			  DefaultDepthOfScreen(Tk_Screen(tkwin)));
	d = pm;
    } else {
	d = Tk_WindowId(tkwin);
    }
    if (black_color == NULL) {
	black_color = Tk_GetColor(interp, tkwin, Tk_GetUid("black"));
    }
    if (white_color == NULL) {
	white_color = Tk_GetColor(interp, tkwin, Tk_GetUid("white"));
    }
    if (black_border == NULL) {
	black_border = Tk_Get3DBorder(interp, tkwin, "black");
    }
    if (white_border == NULL) {
	white_border = Tk_Get3DBorder(interp, tkwin, "white");
    }
    /* Collect GC and font for subsequent work. */
    gc = imfsample->gc;
    XSetClipMask(dpy, gc, None);
    if (imfsample->show_names) {
#ifdef WIN32
	tkfont = Tk_GetFont(interp, tkwin, "-family arial -size 8");
#elif defined (MAC)
	tkfont = Tk_GetFont(interp, tkwin, "-family helvetica -size 11");
#else
	tkfont = Tk_GetFont(interp, tkwin, "-family helvetica -size 12");
#endif
	XSetFont(imfsample->display, gc, Tk_FontId(tkfont));
    }

    /* Redraw the entire widget background/border, but not if we
       are just updating the selected image. */
    if (imfsample->selected == imfsample->previous
     	/* Always redraw if we have only one image (e.g. closeups). */
    	|| (imfsample->numimages == 1 && imfsample->selected == -1)
    	|| update) {
	    done = FALSE;
	    if (imfsample->with_terrain >= 0
	    	/* Terrain tiles are not supported on Windows yet. */
	    	&& use_clip_mask) {
		ImageFamily *timf = 
		    imfsample->imf_list[imfsample->with_terrain];
		Image *timg;
		TkImage *tkimg;

		timg = best_image(timf, 64, 64);
		if (timg) {
		    tkimg = (TkImage *) timg->hook;
		    if (tkimg && tkimg->colr) {
			XSetFillStyle(dpy, gc, FillTiled);
			XSetTile(dpy, gc, tkimg->colr);
			XFillRectangle(dpy, d, gc, 0, 0,
				       Tk_Width(tkwin), Tk_Height(tkwin));
			done = TRUE;
		    }
		}
	    }
	    if (!done) {
		Tk_Fill3DRectangle(tkwin, d, imfsample->bg_border, 0, 0,
				   Tk_Width(tkwin), Tk_Height(tkwin),
				   imfsample->border_width, imfsample->relief);
	    }
    }
#if 0
    for (i = 0; i < imfsample->numimages; i++) {
	if (imf_interp_hook)
	  imfsample->imf_list[i] =
	    (*imf_interp_hook)(imfsample->imf_list[i], NULL, TRUE);
    }
#endif
    /* Tweak the default item width/height to something better. */
    if (imfsample->iheight == 0) {
	imfsample->iheight = 32;
	if (imfsample->numimages == 1)
	  imfsample->iheight = imfsample->height;
    }
    if (imfsample->iwidth == 0) {
	imfsample->iwidth = 32;
	if (imfsample->numimages == 1)
	  imfsample->iwidth = imfsample->width;
    }
    imfsample->eltw = imfsample->iwidth;
    if (imfsample->show_grid)
      imfsample->eltw += 2;
    else
      imfsample->eltw += 2 * imfsample->pad;
    if (imfsample->show_names && !imfsample->show_grid)
      imfsample->eltw += 80;
    imfsample->elth = imfsample->iheight;
    if (imfsample->show_grid)
      imfsample->elth += 2;
    else
      imfsample->elth += 2 * imfsample->pad;
    /* Fix a lower bound on the vertical spacing. */
    /* (should be determined by choice of app font) */
    if (imfsample->elth < 10 && !imfsample->show_grid)
      imfsample->elth = 10;
    /* Compute and save the number of columns to use. */
    imfsample->cols = winwidth / imfsample->eltw;
    if (imfsample->cols <= 0)
      imfsample->cols = 1;
    /* We can get a little wider spacing by recalculating the element
       width. */
    if (imfsample->show_names && !imfsample->show_grid)
      imfsample->eltw = (winwidth - 10) / imfsample->cols;
    imfsample->rows = imfsample->numimages / imfsample->cols;
    /* Account for a last partial row. */
    if (imfsample->rows * imfsample->cols < imfsample->numimages)
      ++(imfsample->rows);
    /* Compute the number of visible rows.  It would be time-consuming
       to render all the images, so try to do only the visible ones. */
    imfsample->numvisrows = winheight / imfsample->elth;
    if (imfsample->numvisrows > imfsample->rows)
      imfsample->numvisrows = imfsample->rows;
    if (imfsample->numvisrows < 1)
      imfsample->numvisrows = min(1, imfsample->rows);
    if (imfsample->firstvisrow + imfsample->numvisrows > imfsample->rows)
      imfsample->firstvisrow = imfsample->rows - imfsample->numvisrows;
    /* Imfapp-specific code that adjusts the canvas content to fit a resized
    window and also sets the canvas scrollregion correctly. */
    if (imfsample->imfapp
    	&& imfsample->redraw) {  
	    imfsample->width = Tk_Width(Tk_Parent(tkwin));
	    imfsample->height = imfsample->rows * imfsample->elth + 7;
	    Tk_GeometryRequest(tkwin, imfsample->width, imfsample->height);
	    /* There must be a better way to do this ... */
	    sprintf(tclbuf, 
		    ".images.canvas configure -scrollregion [ list 0 0 0 %d ]", 
		    imfsample->height);
	    rslt = Tcl_Eval(interp, tclbuf);
	    if (rslt == TCL_ERROR) {
		fprintf(stderr, "Error: %s\n", interp->result);
	    }
	    /* Force a redraw of the scrollbar if the window was resized. */
	    if (imfsample->numimages) {
		sprintf(tclbuf, ".images.canvas.content yview scroll 0 units");
	    } else {
		sprintf(tclbuf, ".images.scroll set 0 1");
	   }
	    rslt = Tcl_Eval(interp, tclbuf);
	    if (rslt == TCL_ERROR) {
		fprintf(stderr, "Error: %s\n", interp->result);
	    }
    }
    /* Now iterate through all the images we want to draw. */
    for (row = imfsample->firstvisrow;
	 row <= (imfsample->firstvisrow + imfsample->numvisrows);
	 ++row) {
	if (row < 0)
	  continue;
	for (col = 0; col < imfsample->cols; ++col) {
	    n = row * imfsample->cols + col;
	    if (n >= imfsample->numimages)
	      break;
	    sx = col * imfsample->eltw;
	    sy = (row - imfsample->firstvisrow) * imfsample->elth;
	    /* Erase the old selected imf if we picked a new one. */
	    if (n == imfsample->previous && n != imfsample->selected) {
		done = FALSE; 
		if (imfsample->with_terrain >= 0
		      /* Terrain tiles are not supported on Windows yet. */
		    && use_clip_mask) {
		    ImageFamily *timf = 
			imfsample->imf_list[imfsample->with_terrain];
		    Image *timg;
		    TkImage *tkimg;

		    timg = best_image(timf, 64, 64);
		    if (timg) {
			tkimg = (TkImage *) timg->hook;
			if (tkimg && tkimg->colr) {
			    XSetFillStyle(dpy, gc, FillTiled);
			    XSetTile(dpy, gc, tkimg->colr);
			    if (imfsample->show_grid) {
				XFillRectangle(dpy, d, gc, sx, sy + 2,
					       imfsample->eltw, 
					       imfsample->elth);
			    } else {
				XFillRectangle(dpy, d, gc, sx, sy + 7,
					       imfsample->eltw, 
					       imfsample->elth);
			    }
			    done = TRUE;
			}
		    }
		}
		if (!done) {
		    if (imfsample->show_grid) {
			Tk_Fill3DRectangle(tkwin, d, imfsample->bg_border, 
					   sx, sy + 2,
					   imfsample->eltw, imfsample->elth,
					   imfsample->border_width, 
					   imfsample->relief);
		    } else {
			Tk_Fill3DRectangle(tkwin, d, imfsample->bg_border, 
					   sx, sy + 7,
					   imfsample->eltw, imfsample->elth,
					   imfsample->border_width, 
					   imfsample->relief);
		    }
		}
	    }
	    /* Just draw the old erased image if we selected a new one, else
	       draw every image. */
	    if (imfsample->selected == imfsample->previous
            	|| n == imfsample->previous
           	|| update) {
		    if (imfsample->show_grid) {
			Tk_Fill3DRectangle(tkwin, d, imfsample->cu_border,
				sx + 2, sy + 2,
				imfsample->iwidth, imfsample->iheight,
				imfsample->border_width, imfsample->relief);
			draw_one_main_image(imfsample, d, gc, 
					    imfsample->imf_list[n],
					    sx + 2, sy + 2,
					    imfsample->iwidth, 
					    imfsample->iheight);
		    } else {
			draw_one_main_image(imfsample, d, gc, 
					    imfsample->imf_list[n],
					    sx + imfsample->pad, 
					    sy + imfsample->pad,
					    imfsample->iwidth, 
					    imfsample->iheight);
		    }
		    if (imfsample->show_names && !imfsample->show_grid) {
			namex = sx + 5;
			namey = sy + imfsample->elth + 3;
			XSetClipMask(dpy, gc, None);
			XSetFillStyle(dpy, gc, FillSolid);
			XSetForeground(dpy, gc, 
				       Tk_3DBorderColor(
					imfsample->fg_border)->pixel);
			str = imfsample->imf_list[n]->name;
			Tk_DrawChars(dpy, d, gc, tkfont, str, strlen(str),
				     namex, namey);
		    }
	    }
	    /* Box the selected imf. */
	    if (n == imfsample->selected) {
		XSetClipMask(dpy, gc, None);
		XSetFillStyle(dpy, gc, FillSolid);
		XSetForeground(dpy, gc, Tk_3DBorderColor(imfsample->fg_border)->pixel);
	  	if (imfsample->show_grid) {
		/* A rectangle on the Mac is 1 pixel smaller in both directions. */
#ifdef MAC
			XDrawRectangle(dpy, d, gc, sx + 2, sy + 2,
				       imfsample->eltw - 2, imfsample->elth - 2);
#else
			XDrawRectangle(dpy, d, gc, sx + 2, sy + 2,
				       imfsample->eltw - 3, imfsample->elth - 3);
#endif
	  	} else {
#ifdef MAC
			XDrawRectangle(dpy, d, gc, sx, sy + 7,
				       imfsample->eltw, imfsample->elth);
#else
			XDrawRectangle(dpy, d, gc, sx, sy + 7,
				       imfsample->eltw - 1, imfsample->elth - 1);
#endif
		}
	    }
	}
    }
    /* Reset the old selected image to the new one if it exists. */
    if (imfsample->selected != -1) {
	imfsample->previous = imfsample->selected;
    }
    /* Reset the redraw flag. */
    imfsample->redraw = FALSE;
    /* If double-buffered, copy to the screen and release the pixmap.  */
    if (imfsample->double_buffer) {
	XCopyArea(imfsample->display, pm, Tk_WindowId(tkwin),
		  imfsample->copygc,
		  0, 0, (unsigned) winwidth, (unsigned) winheight, 0, 0);
	Tk_FreePixmap(imfsample->display, pm);
    }
    if (imfsample->show_names) {
	Tk_FreeFont(tkfont);
    }
    /* In theory this shouldn't be necessary, but in practice the
       interface widgets (esp. the progress bar fill color) are
       affected by the last value of the foreground left over from
       drawing, so set to a consistent value. */
    /* (Note that as of 2000-09-16, some color errors persist, so
       this might not really be necessary -sts) */
    XSetForeground(imfsample->display, imfsample->gc, black_color->pixel);
    XSetBackground(imfsample->display, imfsample->gc, white_color->pixel);
    XSetForeground(imfsample->display, imfsample->copygc, black_color->pixel);
    XSetBackground(imfsample->display, imfsample->copygc, white_color->pixel);
}

static void
imfsample_destroy(char *ptr)
{
    Imfsample *imfsample = (Imfsample *) ptr;

    Tk_FreeOptions(config_specs, (char *) imfsample, imfsample->display, 0);
    if (imfsample->copygc != None)
      XFreeGC(imfsample->display, imfsample->copygc);
    if (imfsample->gc != None)
      XFreeGC(imfsample->display, imfsample->gc);
    ckfree((char *) imfsample);
}

/* Draw a single image plus optional emblem at a given location. */

static void
draw_one_main_image(Imfsample *imfsample, Drawable d, GC gc, ImageFamily *imf,
		    int sx, int sy, int sw, int sh)
{
    ImageFamily *eimf;
    Image *img, *eimg;
    int ex, ey, ew, eh;

    img = best_image(imf, sw, sh);
    if (img == NULL)
      return;
    draw_one_image(imfsample, d, gc, img, sx, sy, sw, sh);
    if (imfsample->with_emblem >= 0) {
	eimf = imfsample->imf_list[imfsample->with_emblem];
	if (eimf != NULL
	    && emblem_position(img, eimf->name, eimf, sw, sh,
			       sh, sh, &ex, &ey, &ew, &eh)) {
	  eimg = best_image(eimf, ew, eh);
	  if (eimg == NULL)
	    return;
	  draw_one_image(imfsample, d, gc, eimg, sx + ex, sy + ey, ew, eh);
	}
    }
}

/* Draw a single image at a given location. */

static void
draw_one_image(Imfsample *imfsample, Drawable d, GC gc, Image *img,
	       int sx, int sy, int sw, int sh)
{
    int sx2, sy2;
    TkImage *tkimg;
    Tk_Window tkwin = imfsample->tkwin;
    Display *dpy = Tk_Display(tkwin);
    XColor *imagecolor = black_color;
    XColor *maskcolor = white_color;

    if (!img->istile) {
	/* Offset the image to draw in the middle of its area,
	   whether larger or smaller than the given area. */
	sx2 = sx + (sw - img->w) / 2;  
	sy2 = sy + (sh - img->h) / 2;
	/* Only change the size of the rectangle being drawn if it's
	   smaller than what was passed in. */
	if (img->w < sw) {
	    sx = sx2;
	    sw = img->w;
	}
	if (img->h < sh) {
	    sy = sy2;
	    sh = img->h;
	}
    }
    /* Pick out a specific subimage if necessary. */
    if (img->numsubimages > 0 && img->subimages)
      img = img->subimages[img->numsubimages - 1];
    tkimg = (TkImage *) img->hook;
    if (tkimg == NULL)
      return;
    XSetClipMask(dpy, gc, None);
    if (tkimg->colr != None && imfsample->show_color) {
	if (img->istile) {
	    XSetFillStyle(dpy, gc, FillTiled);
	    XSetTile(dpy, gc, tkimg->colr);
	    XFillRectangle(dpy, d, gc, sx, sy, sw, sh);
	} else {
	    if (use_clip_mask) {
		if (tkimg->mask != None && imfsample->show_masks) {
		    XSetClipOrigin(dpy, gc, sx2, sy2);
		    /* Use the image's mask to do clipped drawing. */
		    XSetClipMask(dpy, gc, tkimg->mask);
		}
	    } else {
		if (tkimg->mask != None && imfsample->show_masks) {
		    XSetFunction(dpy, gc, GXand);
		    XCopyArea(dpy, tkimg->mask, d, gc, 0, 0, sw, sh, sx, sy);
		}
		XSetFunction(dpy, gc, GXor);
	    }
	    /* Draw the color image. */
	    XCopyArea(dpy, tkimg->colr, d, gc, 0, 0, sw, sh, sx, sy);
	    if (!use_clip_mask)
	      XSetFunction(dpy, gc, GXcopy);
	}
    } else if (tkimg->solid != NULL && imfsample->show_color) {
	XSetFillStyle(dpy, gc, FillSolid);
	XSetForeground(dpy, gc, tkimg->solid->pixel);
	XFillRectangle(dpy, d, gc, sx, sy, sw, sh);
    } else if (tkimg->mono != None || tkimg->mask != None) {
	if (use_clip_mask) {
	    /* The XFillRectangle code below does not work when we draw 
		directly to the screen on the Mac, so we use XCopyArea with a 
		small colored offscreen pixmap instead (this method does not 
		work on Unix or during double buffered drawing on the Mac). */ 
#ifdef MAC
	    if (!imfsample->double_buffer) {
		Pixmap pm = 
		    Tk_GetPixmap(dpy, Tk_WindowId(tkwin), sw, sh, 
				 DefaultDepthOfScreen(Tk_Screen(tkwin)));

		/* Draw the mask. */
		if (tkimg->mask && imfsample->show_masks) {
		    XSetClipMask(dpy, gc, tkimg->mask);
		} else {
		    XSetClipMask(dpy, gc, tkimg->mono);
		}
		if (tkimg->mono != None) {
		    Tk_Fill3DRectangle(tkwin, pm, white_border, 0, 0, 
				       sw, sh, 0, 0);
		} else {
		    Tk_Fill3DRectangle(tkwin, pm, black_border, 0, 0, 
				       sw, sh, 0, 0);
		}
		XCopyArea(dpy, pm, Tk_WindowId(tkwin), gc, 0, 0, 
			  sw, sh, sx, sy);
		/* Draw the image proper. */
		if (tkimg->mono != None) {
		    XSetClipMask(dpy, gc, tkimg->mono);
		    Tk_Fill3DRectangle(tkwin, pm, black_border, 0, 0, 
				       sw, sh, 0, 0);
		    XCopyArea(dpy, pm, Tk_WindowId(tkwin), gc, 0, 0, 
			      sw, sh, sx, sy);
		}
		Tk_FreePixmap(dpy, pm);
		return;
	    }
#endif

		XSetFillStyle(dpy, gc, FillSolid);
		/* Set the color we're going to use for the mask; use the
		imagecolor if we'll be using the mask as the only image. */
		XSetForeground(dpy, gc,
			(tkimg->mono == None ? imagecolor : maskcolor)->pixel);
		XSetClipOrigin(dpy, gc, sx2, sy2);
		/* Set the clip mask to be explicit mask or unit's image. */
		if (tkimg->mask && imfsample->show_masks) {
			XSetClipMask(dpy, gc, tkimg->mask);
		} else {
			XSetClipMask(dpy, gc, tkimg->mono);
		}
		/* Draw the mask. */
		XFillRectangle(dpy, d, gc, sx, sy, sw, sh);
		/* Draw the image proper. */
		if (tkimg->mono != None) {
		    XSetForeground(dpy, gc, imagecolor->pixel);
		    XSetClipMask(dpy, gc, tkimg->mono);
		    XFillRectangle(dpy, d, gc, sx, sy, sw, sh);
		}
	/* The Win32 case. */
	} else {
	    if (tkimg->mask != None && imfsample->show_masks) {
		XSetFunction(dpy, gc, GXand);
		XCopyArea(dpy, tkimg->mask, d, gc, 0, 0, sw, sh, sx, sy);
	    }
	    if (tkimg->mono != None) {
		XSetFunction(dpy, gc, GXor);
		XCopyArea(dpy, tkimg->mono, d, gc, 0, 0, sw, sh, sx, sy);
	    }
	    XSetFunction(dpy, gc, GXcopy);
	}
    }
}
