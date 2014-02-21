/* Xconq is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.  See the file COPYING.  */

/*! \file missing/util/xlibstuff.c
 * \brief Provide missing Xlib-related functions.
 *
 * This file is used to provide substitute implementations of 
 * Xlib-related functions that may be missing on some systems.
 */

#ifdef HAVE_ACDEFS_H
#include "../kernel/acdefs.h"
#endif
#include <tk.h>
#include "xlibstuff.h"

#ifdef NEED_XLIB_XSETTILE
void _XSetTile(Display *dpy, GC gc, Pixmap pm) {}
#endif

#ifdef NEED_XLIB_XFLUSH
void _XFlush(Display *dpy) {}
#endif

#ifdef NEED_XLIB_XSYNCHRONIZE
int (*_XSynchronize(Display *dpy, Bool b))(void) { return NULL; }
#endif
