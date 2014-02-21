/* Tcl/tk based image family application for Xconq.
   Copyright (C) 1998-2000 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#ifdef __cplusplus
extern "C" {
#endif

/* For tcltk 8.4 source compatibility. */
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

#include "system.h"
#include "config.h"
#include "misc.h"
#include "lisp.h"
#include "imf.h"

#include "tkimf.h"

extern int tmp_valid;
extern Tk_Window tmp_root_window;

extern int ui_init(void);
extern void imfapp_main(void);
extern void add_library_path(char *path);

static void init_library_path(char *path);

static int tk_open_imf_dir_file(ClientData cldata, Tcl_Interp *interp,
			    int argc, char *argv[]);
static int tk_load_imf_file(ClientData cldata, Tcl_Interp *interp,
			    int argc, char *argv[]);
static int tk_save_imf_dir_file(ClientData cldata, Tcl_Interp *interp,
			    int argc, char *argv[]);
static int tk_save_imf_file(ClientData cldata, Tcl_Interp *interp,
			    int argc, char *argv[]);
static int get_numimages_cmd(ClientData cldata, Tcl_Interp *interp,
			      int argc, char *argv[]);
static int get_imf_status_cmd(ClientData cldata, Tcl_Interp *interp,
			      int argc, char *argv[]);
static int get_imf_name_cmd(ClientData cldata, Tcl_Interp *interp,
			    int argc, char *argv[]);
static int get_imf_numsizes_cmd(ClientData cldata, Tcl_Interp *interp,
				int argc, char *argv[]);

Tk_Window tkwin;

int depth, screen;

char *read_suggest="", *write_suggest="";

Tcl_Interp *interp;

static int
tk_open_imf_dir_file(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    FILE *fp;

    fp = open_file(argv[1], "r");
    if (fp != NULL) {
    	load_image_families(fp, TRUE, NULL);
	sort_all_images();
	fclose(fp);
    }
    return TCL_OK;
}

static int
tk_load_imf_file(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    load_imf_file(argv[1], NULL);
    sort_all_images();
    return TCL_OK;
}

static int
tk_save_imf_dir_file(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    write_imf_dir(argv[1], images, numimages);
    return TCL_OK;
}

static int
tk_save_imf_file(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    int i, n;
    char *filename, *number;
    FILE *fp;

    filename = argv[1];
    number = argv[2];
    n = strtol(argv[2], NULL, 10);
    fp = fopen(filename, "w");
    if (fp != NULL) {
	if (n == numimages) {
		/* Write out the imf forms of all the image families. */
		for (i = 0; i < numimages; ++i) {
			make_generic_image_data(images[i]);
			write_imf(fp, images[i]);
		}
	} else if (between(0, n, numimages -1)) {
		/* Write only the selected image. */
		make_generic_image_data(images[n]);
		write_imf(fp, images[n]);
	} else {
		run_warning("could not save invalid imf number %d", n);
	}		
	fclose(fp);
    } else {
	run_warning("could not open file for writing");
    }
    return TCL_OK;
}

static int
get_imf_status_cmd(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    sprintf(interp->result, "%d image families loaded", numimages);
    return TCL_OK;
}

static int
get_numimages_cmd(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    sprintf(interp->result, "%d", numimages);
    return TCL_OK;
}

static int
get_imf_name_cmd(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    int n;

    n = strtol(argv[1], NULL, 10);
    if (between(0, n, numimages - 1)) {
	Tcl_SetResult(interp, images[n]->name, TCL_VOLATILE);
    } else {
	Tcl_SetResult(interp, "", TCL_VOLATILE);
    }
    return TCL_OK;
}

static int
get_imf_numsizes_cmd(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    int n, rslt = 0;
    Image *img;

    n = strtol(argv[1], NULL, 10);
    for_all_images(images[n], img)
      if (!img->synthetic)
	++rslt;
    sprintf(interp->result, "%d", rslt);
    return TCL_OK;
}

int
ui_init(void)
{
    int rslt;

    /* NOTE: Should actually provide argv[0] on Unix systems. */
    Tcl_FindExecutable("imfapp");

    interp = Tcl_CreateInterp();

    if (Tcl_Init(interp) == TCL_ERROR)
      return TCL_ERROR;

    if (Tk_Init(interp) == TCL_ERROR)
      return TCL_ERROR;

    Tcl_CreateCommand(interp, "open_imf_dir_file", tk_open_imf_dir_file,
		      (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);
 
   Tcl_CreateCommand(interp, "load_imf_file", tk_load_imf_file,
		      (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);
 
    Tcl_CreateCommand(interp, "save_imf_dir_file", tk_save_imf_dir_file,
		      (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

    Tcl_CreateCommand(interp, "save_imf_file", tk_save_imf_file,
		      (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

    Tcl_CreateCommand(interp, "get_numimages", get_numimages_cmd,
		      (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

    Tcl_CreateCommand(interp, "get_imf_status", get_imf_status_cmd,
		      (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

    Tcl_CreateCommand(interp, "get_imf_name", get_imf_name_cmd,
		      (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

    Tcl_CreateCommand(interp, "get_imf_numsizes", get_imf_numsizes_cmd,
		      (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

    tkwin = Tk_MainWindow(interp);

    Tcl_CreateCommand(interp, "imfsample", imfsample_cmd,
		      (ClientData) tkwin, (Tcl_CmdDeleteProc *) NULL);

    /* Find Imfapp's Tcl script. */
    {
	int loaded = FALSE, i = 0;
	FILE *fp;
	LibraryPath *p;
	char pathbuf[PATH_SIZE];
#ifndef MAC
	char *relpaths [] = {
	    "../share/xconq/tcltk/imfapp", "tcltk/imfapp", "imfapp",
	    "xconq/tcltk/imfapp", NULL
	};
	char *librelpaths [] = {
	    "../tcltk/imfapp", "../imfapp", NULL
	};
#else
	char *relpaths [] = {
	    ":tcltk:imfapp", ":imfapp", "::tcltk:imfapp", NULL
	};
	char *librelpaths [] = {
	    NULL
	};
#endif

	/* We need to init the library path before using it. */
	init_library_path(NULL);
	/* First try paths relative to the working directory. */
	for (i = 0; relpaths[i]; ++i) {
	    make_pathname("", relpaths[i], "tcl", pathbuf);
	    if ((fp = fopen(pathbuf, "r")) != NULL) {
		fclose(fp);
		rslt = Tcl_EvalFile(interp, pathbuf);
		if (rslt == TCL_ERROR)
		  init_error("Error reading tcl from %s: %s",
			     pathbuf, interp->result);
		loaded = TRUE;
		break;
	    }
	}
	/* Then try paths relative to the library directories. */
	if (!loaded) {
	    for_all_library_paths(p) {
		for (i = 0; librelpaths[i]; ++i) {
		    make_pathname(p->path, librelpaths[i], "tcl", pathbuf);
		    if ((fp = fopen(pathbuf, "r")) != NULL) {
			fclose(fp);
			rslt = Tcl_EvalFile(interp, pathbuf);
			if (rslt == TCL_ERROR)
			  init_error("Error reading tcl from %s: %s",
				     pathbuf, interp->result);
			loaded = TRUE;
			break;
		    }
		}
		if (loaded)
		  break;
	    }
	} /* if (!loaded) */
	/* List all the places that were searched if we failed to load the
	   script. */
	if (!loaded) {
	    for (i = 0; relpaths[i]; ++i) {
		make_pathname("", relpaths[i], "tcl", pathbuf);
		init_warning("Failed to load imfapp from: %s\n", pathbuf);
	    }
	    for_all_library_paths(p) {
		for (i = 0; librelpaths[i]; ++i) {
		    make_pathname(p->path, librelpaths[i], "tcl", pathbuf);
		    init_warning("Failed to load imfapp from: %s\n", pathbuf);
		}
	    }
	    init_error("imfapp.tcl file could not be loaded");
	}
    } /* { */

    return rslt;
}

/* Allow environment vars to override compiled-in library and game. */

void
init_library_path(char *path)
{
    char *xconqlib;
    LibraryPath *image_path;

    xconqlib = getenv("XCONQLIB");
    if (empty_string(xconqlib)) {
	if (!empty_string(path)) {
	    xconqlib = path;
	} else {
	    xconqlib = default_library_pathname();
	}
    }
    xconq_libs = (LibraryPath *) xmalloc(sizeof(LibraryPath));
    xconq_libs->path = copy_string(xconqlib);
    /* Add the pathname to the binary images directory, as an
       alternate place to look. */
    image_path = (LibraryPath *) xmalloc(sizeof(LibraryPath));
    image_path->path = copy_string(default_images_pathname(xconqlib));
    xconq_libs->next = image_path;
    last_user_xconq_lib = NULL;
}

void
add_library_path(char *path)
{
    LibraryPath *lib;

    /* A NULL path indicates that all the existing paths should go away. */
    if (path == NULL) {
	xconq_libs = last_user_xconq_lib = NULL;
	return;
    }
    lib = (LibraryPath *) xmalloc(sizeof(LibraryPath));
    lib->path = copy_string(path);
    /* Insert this after the previous user-specified library, if there
       was any, but before the default library places. */
    if (last_user_xconq_lib != NULL) {
	lib->next = last_user_xconq_lib->next;
	last_user_xconq_lib->next = lib;
    } else {
	lib->next = xconq_libs;
	xconq_libs = lib;
    }
    last_user_xconq_lib = lib;
}

/*! \brief Table definition.
 *
 * This is the structure with info about a table.
 */
typedef struct tabledefn {
    char *name;                                 /*!< name of the table */
    int (*getter)(int, int);            /*!< accessor function */
    char *doc;                                  /*!< documentation string */
    short **table;                              /*!< pointer to table itself */
    short dflt;                                 /*!< default value of entries */
    short lo;                                           /*!< lower bound of tabl
e values */
    short hi;                                   /*!< upper bound of table values
 */
    char index1;                                /*!< type of row indices */
    char index2;                                /*!< type of column indices */
    char valtype;                               /*!< type of data in table */
} TableDefn;

TableDefn tabledefns [] = {{NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0}, NULL};

int numtypes_from_index_type(int x) {return 0;}

void
imfapp_main(void)
{
    init_signal_handlers();

    imf_interp_hook = tk_interp_imf;
    imf_load_hook = tk_load_imf;

      Tk_MainLoop();
}
