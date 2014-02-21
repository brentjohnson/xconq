/* X11 image family application for Xconq.
   Copyright (C) 1998-2000 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "config.h"
#include "misc.h"
#include "lisp.h"
#include "imf.h"
#include "xcutil.h"

extern void add_library_path(char *path);
extern void ui_init(void);
extern void imfapp_main(void);

extern char *outdirname;

static void usage(void);

int
main(int argc, char *argv[])
{
    char *arg, *mask_color_name = NULL;
    int i;

    ui_init();

    init_lisp();

    for (i = 1; i < argc; ++i) {
	arg = argv[i];
	if (arg == NULL)
	  continue;
	if (strcmp(arg, "-mc") == 0) {
	    if (i + 1 < argc) {
		mask_color_name = (char *)xmalloc((2 + strlen(argv[i + 1])) * sizeof(char));
		strcpy(mask_color_name, argv[i + 1]);
		argv[i] = NULL;
		argv[i + 1] = NULL;
	    } else {
		low_init_error("No color following -mc");
		usage();
	    }
	} else if ((strcmp(arg, "-help") == 0)
		   || (strcmp(arg, "--help") == 0)) {
	    usage();
	    return 0;
	} else if (strcmp(arg, "-L") == 0) {
	    if (i+1 < argc) {
		if (strcmp(argv[i+1], "-") == 0)
		  add_library_path(NULL);
		else
		  add_library_path(argv[i+1]);
		/* Blast the arg because we'll be scanning the args again
		   and we want to ignore it then. */
		argv[i] = NULL;
		argv[i+1] = NULL;
		++i;
	    } else {
		low_init_error("No directory following -L");
		usage();
	    }
	} else if (strcmp(arg, "-o") == 0) {
	    if (i+1 < argc) {
		outdirname = argv[i+1];
		/* Blast the arg because we'll be scanning the args again
		   and we want to ignore it then. */
		argv[i] = NULL;
		argv[i+1] = NULL;
		++i;
	    } else {
		low_init_error("No output directory following -o");
		usage();
	    }
	}
    }

    /* Any remaining arguments are files, read them. */
    for (i = 1; i < argc; ++i) {
	if (argv[i] != NULL) {
	    /*	    read_suggest = write_suggest = argv[i]; */
	    /* try to guess the format and read the file */
	    if (!read_any_file(argv[i], NULL)) {
		run_warning("Couldn't read \"%s\"", argv[i]);
	    }
	}
    }
    imfapp_main();
    return 0;
}

void
usage(void)
{
    fprintf(stderr,
	    "usage: ximfapp [-mc mask color] [-o outdir] imfile ...\n");
    exit(1);
}
