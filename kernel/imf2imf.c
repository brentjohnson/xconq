/* Manipulate Xconq image format files.
   Copyright (C) 1995, 1999 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "config.h"
#include "misc.h"
#include "lisp.h"
#include "imf.h"

char *indirname;

char *outdirname;

char imdirname[BUFSIZE];

int showprogress = 0;

int explode = 0;

char *xconqlib = "";

char spbuf[BUFSIZE];

char readerrbuf[1000];

static void usage(void);

int
main(int argc, char *argv[])
{
    int i, rslt;
    char *arg;
    FILE *ifp, *ofp, *fp;
    Obj *form;
    int startlineno = 0, endlineno = 0;

    init_lisp();

    if (argc == 1) usage();
    for (i = 1; i < argc; ++i) {
	arg = argv[i];
	if (strcmp(arg, "-i") == 0) {
	    if (i + 1 < argc) {
		indirname = argv[i + 1];
		/* Blast the arg because we'll be scanning the args again
		   and we want to ignore it then. */
		argv[i] = NULL;
		argv[i + 1] = NULL;
		++i;
	    } else {
		init_error("No input directory following -i");
	    }
	} else if (strcmp(arg, "-o") == 0) {
	    if (i + 1 < argc) {
		outdirname = argv[i + 1];
		/* Blast the arg because we'll be scanning the args again
		   and we want to ignore it then. */
		argv[i] = NULL;
		argv[i + 1] = NULL;
		++i;
	    } else {
		init_error("No output directory following -o");
	    }
	} else if (strcmp(arg, "-p") == 0) {
	    showprogress = 1;
	    argv[i] = NULL;
	} else if (strcmp(arg, "--help") == 0) {
	    usage();
	    argv[i] = NULL;
	} else if (strcmp(arg, "--explode") == 0) {
	    explode = 1;
	    argv[i] = NULL;
	}
    }
    /* Open and read an image directory file (and associated files)
       if supplied. */
    if (indirname != NULL) {
	ifp = fopen(indirname, "r");
	if (ifp != NULL) {
	    load_image_families(ifp, TRUE, NULL);
	} else {
	    run_warning("Couldn't open \"%s\", ignoring", indirname);
	}
    }
    /* Now append contents of random imf files. */
    for (i = 1; i < argc; ++i) {
	if (argv[i] != NULL) {
	    rslt = load_imf_file(argv[i], NULL);
	    if (!rslt)
	      run_warning("Couldn't open \"%s\", ignoring", argv[i]);
	}
    }
    sort_all_images();
    if (outdirname) {
	/* Write the image directory file. */
	sprintf(imdirname, "%s%s", outdirname, "imf.dir");
	ofp = fopen(imdirname, "w");
    } else {
	ofp = NULL;
    }
    for (i = 0; i < numimages; ++i) {
	if (showprogress)
	  fprintf(stderr, "; %s\n", images[i]->name);
	if (explode && outdirname) {
	    sprintf(spbuf, "%s%s", outdirname, images[i]->name);
	    fp = fopen(spbuf, "w");
	    if (fp != NULL) {
		write_imf(fp, images[i]);
		fclose(fp);
	    } else {
		fprintf(stderr, "could not open \"%s\"\n", spbuf);
	    }
	} else {
	    write_imf(stdout, images[i]);
	}
    }
    if (ofp != NULL) {
	fclose(ofp);
    }
    return 0;
}

static void
usage(void)
{
    fprintf(stderr,
	    "usage: imf2imf [ -i indir ] [ -o outdir ] [ -p ] [ files ... ]\n");
    exit(1);
}

void
close_displays(void)
{
}

int
write_entire_game_state(char *fname)
{
    return 0;
}

/* Need these so we can link in Lisp reader code. */

void
announce_read_progress()
{
}

void  
syntax_error(Obj *x, char *msg)
{
    sprintlisp(readerrbuf, x, BUFSIZE);
    init_warning("syntax error in `%s' - %s", readerrbuf, msg);
}

void
low_init_error(const char *str)
{
    fprintf(stderr, "Error: %s.\n", str);
    fflush(stderr);
}

/* A warning just gets displayed, no other action is taken. */

void
low_init_warning(char *str)
{
    fprintf(stdout, "Warning: %s.\n", str);
    fflush(stdout);
}

/* A run error is fatal. */

void
low_run_error(char *str)
{
    fprintf(stderr, "Error: %s.\n", str);
    fflush(stderr);
    exit(1);
}

void
low_run_warning(char *str)
{
    fprintf(stdout, "Warning: %s.\n", str);
    fflush(stdout);
}

/* Fake definitions of unneeded routines called by lisp.c. */

int
keyword_code(char *str)
{
    run_warning("fake keyword_code being called");
    return 0;
}

/* Make the table so keyword lookup works. */

struct a_key {
    char *name;
} keywordtable[] = {

#undef  DEF_KWD
#define DEF_KWD(NAME,code)  { NAME },

#include "keyword.def"

    { NULL }
};

char *
keyword_name(enum keywords k)
{
    return keywordtable[k].name;
}

void
init_predefined_symbols(void)
{
}

int
lazy_bind(Obj *sym)
{
    run_warning("fake lazy_bind being called");
    return FALSE;
}

void
prealloc_debug(void)
{
}
