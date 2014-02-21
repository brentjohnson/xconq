/* Definitions of image families for the Tk interface to Xconq.
   Copyright (C) 1998-2000 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/* Structure for Tk-specific data. */

/* Note that this structure has an implicit window and therefore player. */

typedef struct a_tk_image_family {
    Tk_Window rootwin;		/* Root window of family's pixmaps */
} TkImageFamily;

typedef struct a_tk_image {
    Image *generic;		/* pointer to generic image */
    Pixmap mono;
    Pixmap colr;
    Pixmap mask;
    XColor *solid;
    XColor **cmap;
} TkImage;

/* tkimf.h gets included everywhere (directly or through tkconq.h) so this is a good
place to put this platform-specific stuff. */

#ifdef MAC
#undef GXinvert
#define GXinvert GXxor	/* TkMacSetUpGraphicsPort in MacTcl uses GXxor instead 
					of GXinvert for line inversion (patXor). */ 
#endif

#if (!defined (MAC) && !defined (UNIX) && !defined (__CYGWIN32__) && !defined(__MINGW32__))
extern void XFlush(Display *dpy);
extern int (*XSynchronize(Display *dpy, Bool b))(void);
#endif

#if (!defined (UNIX) && !defined (__CYGWIN32__) && !defined(__MINGW32__))
extern void XSetTile(Display *dpy, GC gc, Pixmap pm);
#endif

extern TkImage *init_tk_image(Image *img);
extern TkImage *get_tk_image(Image *img);
extern ImageFamily *tk_load_imf(ImageFamily *imf);
extern ImageFamily *tk_interp_imf(ImageFamily *imf, Image *img, int force);
extern void reverse_bit_endianness(char *rawdata, int numbytes);
extern ImageFamily *tk_find_imf(char *name);
extern int imfsample_cmd(ClientData cldata, Tcl_Interp *interp,
		  int argc, char *argv[]);

