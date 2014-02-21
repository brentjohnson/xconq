/* Xconq is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.  See the file COPYING.  */

/*! \file missing/xlibstuff.h
 * \brief Provide missing Xlib-related functions.
 *
 * This include file is used to provide function prototypes for 
 * Xlib-related things that may be missing on some systems.
 */

#ifndef _XLIBSTUFF_H_
#define _XLIBSTUFF_H_

#if (defined (__MWERKS__) || defined (_MSC_VER))
#ifndef NEED_XLIB_XSETTILE
#define NEED_XLIB_XSETTILE
#endif
#ifndef NEED_XLIB_XFLUSH
#define NEED_XLIB_XFLUSH
#endif
#ifndef NEED_XLIB_XSYNCHRONIZE
#define NEED_XLIB_XSYNCHRONIZE
#endif
#endif

#ifdef NEED_XLIB_XSETTILE
extern void _XSetTile(Display *dpy, GC gc, Pixmap pm);
#define XSetTile _XSetTile
#endif

#ifdef NEED_XLIB_XFLUSH
extern void _XFlush(Display *dpy);
#define XFlush _XFlush
#endif

#ifdef NEED_XLIB_XSYNCHRONIZE
extern int (*_XSynchronize(Display *dpy, Bool b))(void);
#define XSynchronize _XSynchronize
#endif

#endif /* ifndef _XLIBSTUFF_H_ */
