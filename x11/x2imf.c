/* This program converts a collection of X bitmaps into imf format.
   Copyright (C) 1992, 1993, 1994, 1995, 1999 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "config.h"
#include "misc.h"
#include "lisp.h"
#include "imf.h"
#include "xcutil.h"

char tmpbuf[500];

int
main(int argc, char *argv[])
{
    int i;

    init_lisp();

    for (i = 1; i < argc; ++i) {
	if (!read_any_file(argv[i], NULL)) {
	    fprintf(stderr, "Couldn't read \"%s\"\n", argv[i]);
	}
    }
    sort_all_images();
    /* Now write out all the images that were read. */
    for (i = 0; i < numimages; ++i) {
#if 0
	reverse_rawdata(images[i]);
#endif
	/* Note that the generic image data slots were filled in upon
	   bitmap reading, so the images are ready to write out. */
	fprintf(stderr, "; %s\n", images[i]->name);
	write_imf(stdout, images[i]);
    }
    return 0;
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

