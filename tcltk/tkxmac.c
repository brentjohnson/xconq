/* Improved support for various X drawing functions in TclTk 8.3.4 
   for MacOS Classic. Copyright (C) 2002-2003 Hans Ronne.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#ifdef __cplusplus
extern "C" {
#endif

/* For tcltk 8.4.0 source compatibility. */
#define USE_NON_CONST

#include <tclMacCommonPch.h>
#include <tclMac.h>			/* Includes tcl.h. */
#include <tkMac.h>			/* Includes tk.h. */
#include <tkMacInt.h>			/* Needed for custom-made XCopyArea below. */

#ifdef __cplusplus
}
#endif

#define QDPat(whatever) (&(qd.##whatever))

#define PI 3.14159265358979

extern Tcl_Interp *interp;

extern void XSetTile(Display *dpy, GC gc, Pixmap pm);

/* This is a rewritten version of XCopyArea for which the use of clip masks works, 
unlike the XCopyArea in TclTk 8.3.4. The new code is mostly from XCopyPlane, 
which does have working support for clip masks. The stuff that is commented out is 
code from the original XCopyArea that does not seem to have any effect whether it is 
included or not. */

#if 0
static RgnHandle tmpRgn = NULL;
static RgnHandle tmpRgn2 = NULL;
#endif

void 
XCopyArea(
    Display* display,		/* Display. */
    Drawable src,			/* Source drawable. */
    Drawable dest,		/* Destination drawable. */
    GC gc,				/* GC to use. */
    int src_x,			/* X & Y, width & height */
    int src_y,			/* define the source rectangle */
    unsigned int width,		/* the will be copied. */
    unsigned int height,
    int dest_x,			/* Dest X & Y on dest rect. */
    int dest_y)
{
    Rect srcRect, destRect;
    BitMapPtr srcBit, destBit, maskBit;
    MacDrawable *srcDraw = (MacDrawable *) src;
    MacDrawable *destDraw = (MacDrawable *) dest;
    TkpClipMask *clipPtr = (TkpClipMask*)gc->clip_mask;
    GWorldPtr srcPort, destPort, maskPort;
    CGrafPtr saveWorld;
    GDHandle saveDevice;
    short tmode;

#if 0
    RGBColor origForeColor, origBackColor, whiteColor, blackColor;
#endif

    destPort = TkMacGetDrawablePort(dest);
    srcPort = TkMacGetDrawablePort(src);

    display->request++;
    GetGWorld(&saveWorld, &saveDevice);
    SetGWorld(destPort, NULL);

#if 0
    GetForeColor(&origForeColor);
    GetBackColor(&origBackColor);
    whiteColor.red = 0;
    whiteColor.blue = 0;
    whiteColor.green = 0;
    RGBForeColor(&whiteColor);
    blackColor.red = 0xFFFF;
    blackColor.blue = 0xFFFF;
    blackColor.green = 0xFFFF;
    RGBBackColor(&blackColor);
#endif
    
    TkMacSetUpClippingRgn(dest);

#if 0
    /*
     *  We will change the clip rgn in this routine, so we need to 
     *  be able to restore it when we exit.
     */
     
    if (tmpRgn2 == NULL) {
        tmpRgn2 = NewRgn();
    }
    GetClip(tmpRgn2);

    if (((TkpClipMask*)gc->clip_mask)->type == TKP_CLIP_REGION) {
	RgnHandle clipRgn = (RgnHandle)
	        ((TkpClipMask*)gc->clip_mask)->value.region;
	
	int xOffset, yOffset;
	
	if (tmpRgn == NULL) {
	    tmpRgn = NewRgn();
	}
	
	xOffset = destDraw->xOff + gc->clip_x_origin;
	yOffset = destDraw->yOff + gc->clip_y_origin;
	
	OffsetRgn(clipRgn, xOffset, yOffset);
	
	GetClip(tmpRgn);
	SectRgn(tmpRgn, clipRgn, tmpRgn);
	
	SetClip(tmpRgn);
	
	OffsetRgn(clipRgn, -xOffset, -yOffset);
    }
#endif
    
    srcBit = &((GrafPtr) srcPort)->portBits;
    destBit = &((GrafPtr) destPort)->portBits;
    SetRect(&srcRect, (short) (srcDraw->xOff + src_x),
	    (short) (srcDraw->yOff + src_y),
	    (short) (srcDraw->xOff + src_x + width),
	    (short) (srcDraw->yOff + src_y + height));	
    SetRect(&destRect, (short) (destDraw->xOff + dest_x),
	    (short) (destDraw->yOff + dest_y), 
	    (short) (destDraw->xOff + dest_x + width),
	    (short) (destDraw->yOff + dest_y + height));	

    if (clipPtr == NULL || clipPtr->type == TKP_CLIP_REGION) {

	/* Case 1: opaque bitmaps. */
	tmode = srcCopy;
	CopyBits(srcBit, destBit, &srcRect, &destRect, tmode, NULL);
    } else if (clipPtr->type == TKP_CLIP_PIXMAP) {
	if (clipPtr->value.pixmap == src) {
	
	    /* Case 2: transparent bitmaps.  If it's color we ignore the forecolor. */
	    if ((**(srcPort->portPixMap)).pixelSize == 1) {
		tmode = srcOr;
	    } else {
		tmode = transparent;
	    }
	    CopyBits(srcBit, destBit, &srcRect, &destRect, tmode, NULL);
	} else {
	
	    /* Case 3: two arbitrary bitmaps. */
	    tmode = srcCopy;
	    maskPort = TkMacGetDrawablePort(clipPtr->value.pixmap);
	    maskBit = &((GrafPtr) maskPort)->portBits;
	    CopyDeepMask(srcBit, maskBit, destBit, &srcRect, &srcRect, &destRect, tmode, NULL);
	}
    }

#if 0
    RGBForeColor(&origForeColor);
    RGBBackColor(&origBackColor);
    SetClip(tmpRgn2);
#endif

    SetGWorld(saveWorld, saveDevice);
}

/* Using a static PixPatHandle also for the tiled drawing slowed down things
enormously and caused out of memory errors. Should find out why. */

static PixPatHandle gPenPat = NULL;

/* This is a hacked version of XFillRectangles where I have added support for clipping 
masks and stippled drawing. The code from TkMacSetUpGraphicsPort has been integrated 
since it uses a static PixPatHandle that is shared with XFillRectangles. */

/* 030517 - Support for tiled drawing added. */

void 
XFillRectangles(
    Display* display,		/* Display. */
    Drawable d,			/* Draw on this. */
    GC gc,				/* Use this GC. */
    XRectangle *rectangles,	/* Rectangle array. */
    int n_rectangels)		/* Number of rectangles. */
{
    TkpClipMask *clipPtr = (TkpClipMask*)gc->clip_mask;
    MacDrawable *macWin = (MacDrawable *) d;
    Tk_Window rootwin = Tk_MainWindow(interp);
    CGrafPtr saveWorld;
    GDHandle saveDevice;
    GWorldPtr destPort;
    RGBColor macColor;
    Pattern thePat;
    Rect theRect;
    int i;

    destPort = TkMacGetDrawablePort(d);

    display->request++;
    GetGWorld(&saveWorld, &saveDevice);
    SetGWorld(destPort, NULL);

    TkMacSetUpClippingRgn(d);

    /* Start of code from TkMacSetUpGraphicsPort. */

    if (gPenPat == NULL) {
	gPenPat = NewPixPat();
    }
    
    if (TkSetMacColor(gc->foreground, &macColor) == true) {
        /* TODO: cache RGBPats for preformace - measure gains...  */
	MakeRGBPat(gPenPat, &macColor);
    }
    
    PenNormal();
    if(gc->function == GXxor) {
	PenMode(patXor);
    }
    if (gc->line_width > 1) {
	PenSize(gc->line_width, gc->line_width);
    }
    if (gc->line_style != LineSolid) {
	unsigned char *p = (unsigned char *) &(gc->dashes);
    }

    /* End of code from TkMacSetUpGraphicsPort. */

    for (i=0; i<n_rectangels; i++) {
	theRect.left = (short) (macWin->xOff + rectangles[i].x);
	theRect.top = (short) (macWin->yOff + rectangles[i].y);
	theRect.right = (short) (theRect.left + rectangles[i].width);
	theRect.bottom = (short) (theRect.top + rectangles[i].height);

	/* If we have a clip mask, we convert both the mask and the rect into
	regions and color the intersection. */
        	if (clipPtr && clipPtr->type == TKP_CLIP_PIXMAP) {
    		BitMapPtr maskBit;
    		GWorldPtr maskPort;
   		RgnHandle rectRgn = NewRgn();
   		RgnHandle maskRgn = NewRgn();
		
		RectRgn(rectRgn, &theRect);
		maskPort = TkMacGetDrawablePort(clipPtr->value.pixmap);
	    	maskBit = &((GrafPtr) maskPort)->portBits;
		BitMapToRegion(maskRgn, (const BitMap *) maskBit);
		OffsetRgn(maskRgn, gc->clip_x_origin, gc->clip_y_origin);	
		SectRgn(rectRgn, maskRgn, rectRgn);
		/* Support the predefined stipple styles in Tk. */
		if (gc->fill_style == FillStippled) {
			PenMode(patOr);
			RGBForeColor(&macColor);
			/* Tk gray12 does not have a corresponding QuickDraw global. */
			if (gc->stipple == Tk_GetBitmap(interp, rootwin, "gray12")) {
				GetIndPattern(&thePat, sysPatListID, 22);
				PenPat(&thePat);
			} else if (gc->stipple == Tk_GetBitmap(interp, rootwin, "gray25")) {
				PenPat(QDPat(ltGray));
			} else if (gc->stipple == Tk_GetBitmap(interp, rootwin, "gray50")) {
				PenPat(QDPat(gray));
			} else if (gc->stipple == Tk_GetBitmap(interp, rootwin, "gray75")) {
				PenPat(QDPat(dkGray));
			} else if (gc->stipple == Tk_GetBitmap(interp, rootwin, "black")) {
				PenPat(QDPat(black));
			}
			PaintRgn(rectRgn);
			ForeColor(blackColor);
			PenNormal();
		/* Support tiled drawing. */
		} else if (gc->fill_style == FillTiled) {
			/* Adapted from Tk_FreePixmap in tkMacSubwindows.c. */
			MacDrawable *macPix = (MacDrawable *) gc->tile;
			PixMapHandle pixels = GetGWorldPixMap(macPix->portPtr);
			PixPatHandle tilePat = NewPixPat();		
			int numBytes;

			/* Adapted from Pict2PPat.c. */
			numBytes = (**pixels).rowBytes & 0x7FFF;
			numBytes *= (**pixels).bounds.bottom - (**pixels).bounds.top;
			(**tilePat).patData = NewHandleClear(numBytes);
			LockPixels(pixels);
			CopyPixMap(pixels, (**tilePat).patMap);
			BlockMove(GetPixBaseAddr(pixels), *(**tilePat).patData, numBytes);
			UnlockPixels(pixels);
			PixPatChanged(tilePat);
			FillCRgn(rectRgn, tilePat);
			DisposePixPat(tilePat);
		/* Normal drawing. */
		} else {
			FillCRgn(rectRgn, gPenPat);
		}
		DisposeRgn(maskRgn);
		DisposeRgn(rectRgn);
	/* No clip mask. */
	} else {
		/* Support the predefined stipple styles in Tk. */
		if (gc->fill_style == FillStippled) {
			PenMode(patOr);
			RGBForeColor(&macColor);
			/* Tk gray12 does not have a corresponding QuickDraw global. */
			if (gc->stipple == Tk_GetBitmap(interp, rootwin, "gray12")) {				
				GetIndPattern(&thePat, sysPatListID, 22);
				PenPat(&thePat);
			} else if (gc->stipple == Tk_GetBitmap(interp, rootwin, "gray25")) {
				PenPat(QDPat(ltGray));
			} else if (gc->stipple == Tk_GetBitmap(interp, rootwin, "gray50")) {
				PenPat(QDPat(gray));
			} else if (gc->stipple == Tk_GetBitmap(interp, rootwin, "gray75")) {
				PenPat(QDPat(dkGray));
			} else if (gc->stipple == Tk_GetBitmap(interp, rootwin, "black")) {
				PenPat(QDPat(black));
			}
			PaintRect(&theRect);
			ForeColor(blackColor);
			PenNormal();
		/* Support tiled drawing. */
		} else if (gc->fill_style == FillTiled) {
			/* Adapted from Tk_FreePixmap in tkMacSubwindows.c. */
			MacDrawable *macPix = (MacDrawable *) gc->tile;
			PixMapHandle pixels = GetGWorldPixMap(macPix->portPtr);			
			PixPatHandle tilePat = NewPixPat();		
			int numBytes;

			/* Adapted from Pict2PPat.c. */
			numBytes = (**pixels).rowBytes & 0x7FFF;
			numBytes *= (**pixels).bounds.bottom - (**pixels).bounds.top;
			(**tilePat).patData = NewHandleClear(numBytes);
			LockPixels(pixels);
			CopyPixMap(pixels, (**tilePat).patMap);
			BlockMove(GetPixBaseAddr(pixels), *(**tilePat).patData, numBytes);
			UnlockPixels(pixels);
			PixPatChanged(tilePat);
			FillCRect(&theRect, tilePat);
			DisposePixPat(tilePat);
		/* Normal drawing. */
		} else {
			FillCRect(&theRect, gPenPat);
		}
	}
    }
    SetGWorld(saveWorld, saveDevice);
}

/* This is a hacked version of XFillArc where I have added support for stippled 
drawing. The code from TkMacSetUpGraphicsPort has been integrated since it 
uses a static PixPatHandle that is shared with XFillRectangles. */

/* 030517 - Support for tiled drawing added. */

void
XFillArc(
    Display* display,		/* Display. */
    Drawable d,			/* Draw on this. */
    GC gc,			/* Use this GC. */
    int x,			/* Upper left of */
    int y,			/* bounding rect. */
    unsigned int width,		/* Width & height. */
    unsigned int height,
    int angle1,			/* Staring angle of arc. */
    int angle2)			/* Ending angle of arc. */
{
    MacDrawable *macWin = (MacDrawable *) d;
    Tk_Window rootwin = Tk_MainWindow(interp);
    Rect theRect;
    short start, extent;
    PolyHandle polygon;
    double sin1, cos1, sin2, cos2, angle;
    double boxWidth, boxHeight;
    double vertex[2], center1[2], center2[2];
    CGrafPtr saveWorld;
    GDHandle saveDevice;
    GWorldPtr destPort;
    RGBColor macColor;
    Pattern thePat;

    destPort = TkMacGetDrawablePort(d);

    display->request++;
    GetGWorld(&saveWorld, &saveDevice);
    SetGWorld(destPort, NULL);

    TkMacSetUpClippingRgn(d);

    /* Start of code from TkMacSetUpGraphicsPort. */

    if (gPenPat == NULL) {
	gPenPat = NewPixPat();
    }
    
    if (TkSetMacColor(gc->foreground, &macColor) == true) {
        /* TODO: cache RGBPats for preformace - measure gains...  */
	MakeRGBPat(gPenPat, &macColor);
    }
    
    PenNormal();
    if(gc->function == GXxor) {
	PenMode(patXor);
    }
    if (gc->line_width > 1) {
	PenSize(gc->line_width, gc->line_width);
    }
    if (gc->line_style != LineSolid) {
	unsigned char *p = (unsigned char *) &(gc->dashes);
    }

    /* End of code from TkMacSetUpGraphicsPort. */

    theRect.left = (short) (macWin->xOff + x);
    theRect.top = (short) (macWin->yOff + y);
    theRect.right = (short) (theRect.left + width);
    theRect.bottom = (short) (theRect.top + height);
    start = (short) (90 - (angle1 / 64));
    extent = (short) (- (angle2 / 64));

    if (gc->arc_mode == ArcChord) {
    	boxWidth = theRect.right - theRect.left;
    	boxHeight = theRect.bottom - theRect.top;
    	angle = -(angle1/64.0)*PI/180.0;
    	sin1 = sin(angle);
    	cos1 = cos(angle);
    	angle -= (angle2/64.0)*PI/180.0;
    	sin2 = sin(angle);
    	cos2 = cos(angle);
    	vertex[0] = (theRect.left + theRect.right)/2.0;
    	vertex[1] = (theRect.top + theRect.bottom)/2.0;
    	center1[0] = vertex[0] + cos1*boxWidth/2.0;
    	center1[1] = vertex[1] + sin1*boxHeight/2.0;
    	center2[0] = vertex[0] + cos2*boxWidth/2.0;
    	center2[1] = vertex[1] + sin2*boxHeight/2.0;

	polygon = OpenPoly();
	MoveTo((short) ((theRect.left + theRect.right)/2),
		(short) ((theRect.top + theRect.bottom)/2));
	
	LineTo((short) (center1[0] + 0.5), (short) (center1[1] + 0.5));
	LineTo((short) (center2[0] + 0.5), (short) (center2[1] + 0.5));
	ClosePoly();

	ShowPen();
	/* Support the four predefined stipple styles in Tk. */
	if (gc->fill_style == FillStippled) {
		PenMode(patOr);
		RGBForeColor(&macColor);
		/* Tk gray12 does not have a corresponding QuickDraw global. */
		if (gc->stipple == Tk_GetBitmap(interp, rootwin, "gray12")) {
			GetIndPattern(&thePat, sysPatListID, 22);
			PenPat(&thePat);
		} else if (gc->stipple == Tk_GetBitmap(interp, rootwin, "gray25")) {
			PenPat(QDPat(ltGray));
		} else if (gc->stipple == Tk_GetBitmap(interp, rootwin, "gray50")) {
			PenPat(QDPat(gray));
		} else if (gc->stipple == Tk_GetBitmap(interp, rootwin, "gray75")) {
			PenPat(QDPat(dkGray));
		} else if (gc->stipple == Tk_GetBitmap(interp, rootwin, "black")) {
			PenPat(QDPat(black));
		}
		PaintArc(&theRect, start, extent);
		PaintPoly(polygon);
		ForeColor(blackColor);
		PenNormal();
	} else if (gc->fill_style == FillTiled) {
		/* Adapted from Tk_FreePixmap in tkMacSubwindows.c. */
		MacDrawable *macPix = (MacDrawable *) gc->tile;
		PixMapHandle pixels = GetGWorldPixMap(macPix->portPtr);			
		PixPatHandle tilePat = NewPixPat();		
		int numBytes;

		/* Adapted from Pict2PPat.c. */
		numBytes = (**pixels).rowBytes & 0x7FFF;
		numBytes *= (**pixels).bounds.bottom - (**pixels).bounds.top;
		(**tilePat).patData = NewHandleClear(numBytes);
		LockPixels(pixels);
		CopyPixMap(pixels, (**tilePat).patMap);
		BlockMove(GetPixBaseAddr(pixels), *(**tilePat).patData, numBytes);
		UnlockPixels(pixels);
		PixPatChanged(tilePat);
		FillCArc(&theRect, start, extent, tilePat);
		FillCPoly(polygon, tilePat);
		DisposePixPat(tilePat);
	/* Normal drawing. */
	} else {
		FillCArc(&theRect, start, extent, gPenPat);
		FillCPoly(polygon, gPenPat);
	}
	KillPoly(polygon);
    } else {
	ShowPen();
	/* Support the four predefined stipple styles in Tk. */
	if (gc->fill_style == FillStippled) {
		PenMode(patOr);
		RGBForeColor(&macColor);
		/* Tk gray12 does not have a corresponding QuickDraw global. */
		if (gc->stipple == Tk_GetBitmap(interp, rootwin, "gray12")) {				
			GetIndPattern(&thePat, sysPatListID, 22);
			PenPat(&thePat);
		} else if (gc->stipple == Tk_GetBitmap(interp, rootwin, "gray25")) {
			PenPat(QDPat(ltGray));
		} else if (gc->stipple == Tk_GetBitmap(interp, rootwin, "gray50")) {
			PenPat(QDPat(gray));
		} else if (gc->stipple == Tk_GetBitmap(interp, rootwin, "gray75")) {
			PenPat(QDPat(dkGray));
		} else if (gc->stipple == Tk_GetBitmap(interp, rootwin, "black")) {
			PenPat(QDPat(black));
		}
		PaintArc(&theRect, start, extent);
		ForeColor(blackColor);
		PenNormal();
	/* Support tiled drawing. */
	} else if (gc->fill_style == FillTiled) {
		/* Adapted from Tk_FreePixmap in tkMacSubwindows.c. */
		MacDrawable *macPix = (MacDrawable *) gc->tile;
		PixMapHandle pixels = GetGWorldPixMap(macPix->portPtr);			
		PixPatHandle tilePat = NewPixPat();		
		int numBytes;

		/* Adapted from Pict2PPat.c. */
		numBytes = (**pixels).rowBytes & 0x7FFF;
		numBytes *= (**pixels).bounds.bottom - (**pixels).bounds.top;
		(**tilePat).patData = NewHandleClear(numBytes);
		LockPixels(pixels);
		CopyPixMap(pixels, (**tilePat).patMap);
		BlockMove(GetPixBaseAddr(pixels), *(**tilePat).patData, numBytes);
		UnlockPixels(pixels);
		PixPatChanged(tilePat);
		FillCArc(&theRect, start, extent, tilePat);
		DisposePixPat(tilePat);
	/* Normal drawing. */
	} else {
		FillCArc(&theRect, start, extent, gPenPat);
    	}
    }

    SetGWorld(saveWorld, saveDevice);
}

/* This version of XMapWindow (from tkMacSubwindows.c in Tcl/Tk 8.3.4)
has been hacked in a minor but very important way: a call to SelectWindow has
been added after ShowWindow. This fixes a bug that prevented deiconified toplevel 
MacTcl windows from ever regaining the focus (even focus -force failed). */

void 
XMapWindow(
    Display* display,		/* Display. */
    Window window)		/* Window. */
{
    MacDrawable *macWin = (MacDrawable *) window;
    XEvent event;
    GWorldPtr destPort;

    /*
     * Under certain situations it's possible for this function to be
     * called before the toplevel window it's associated with has actually
     * been mapped.  In that case we need to create the real Macintosh
     * window now as this function as well as other X functions assume that
     * the portPtr is valid.
     */
    if (!TkMacHostToplevelExists(macWin->toplevel->winPtr)) {
	TkMacMakeRealWindowExist(macWin->toplevel->winPtr);
    }
    destPort = TkMacGetDrawablePort(window);

    display->request++;
    macWin->winPtr->flags |= TK_MAPPED;
    if (Tk_IsTopLevel(macWin->winPtr)) {
	if (!Tk_IsEmbedded(macWin->winPtr)) {
		ShowWindow((WindowRef) destPort);		
		SelectWindow((WindowRef) destPort);
	}

	/* 
	 * We only need to send the MapNotify event
	 * for toplevel windows.
	 */
	event.xany.serial = display->request;
	event.xany.send_event = False;
	event.xany.display = display;
	
	event.xmap.window = window;
	event.xmap.type = MapNotify;
	event.xmap.event = window;
	event.xmap.override_redirect = macWin->winPtr->atts.override_redirect;
	Tk_QueueWindowEvent(&event, TCL_QUEUE_TAIL);
    } else {
	TkMacInvalClipRgns(macWin->winPtr->parentPtr);
    }

    /* 
     * Generate damage for that area of the window 
     */
    SetGWorld(destPort, NULL);
    TkMacUpdateClipRgn(macWin->winPtr);
    TkMacInvalidateWindow(macWin, TK_PARENT_WINDOW);
}

/* XFillPolygon from 8.4.2 in which PenPixPat is set correctly. The code from 
TkMacSetUpGraphicsPort has been integrated since it uses a static PixPatHandle 
that is shared with XFillRectangles. */

void 
XFillPolygon(
    Display* display,		/* Display. */
    Drawable d,			/* Draw on this. */
    GC gc,				/* Use this GC. */
    XPoint* points,		/* Array of points. */
    int npoints,			/* Number of points. */
    int shape,			/* Shape to draw. */
    int mode)			/* Drawing mode. */
{
    MacDrawable *macWin = (MacDrawable *) d;
    Tk_Window rootwin = Tk_MainWindow(interp);
    PolyHandle polygon;
    CGrafPtr saveWorld;
    GDHandle saveDevice;
    GWorldPtr destPort;
    int i;
    RGBColor macColor;
    Pattern thePat;

    destPort = TkMacGetDrawablePort(d);

    display->request++;
    GetGWorld(&saveWorld, &saveDevice);
    SetGWorld(destPort, NULL);

    TkMacSetUpClippingRgn(d);
    
    /* Start of code from TkMacSetUpGraphicsPort. */

    if (gPenPat == NULL) {
	gPenPat = NewPixPat();
    }
    
    if (TkSetMacColor(gc->foreground, &macColor) == true) {
        /* TODO: cache RGBPats for preformace - measure gains...  */
	MakeRGBPat(gPenPat, &macColor);
    }
    
    PenNormal();
    if(gc->function == GXxor) {
	PenMode(patXor);
    }
    if (gc->line_width > 1) {
	PenSize(gc->line_width, gc->line_width);
    }
    if (gc->line_style != LineSolid) {
	unsigned char *p = (unsigned char *) &(gc->dashes);
    }

    /* End of code from TkMacSetUpGraphicsPort. */

    TkMacSetUpGraphicsPort(gc);

    PenSize(0, 0);    
    
    polygon = OpenPoly();

    MoveTo((short) (macWin->xOff + points[0].x),
	    (short) (macWin->yOff + points[0].y));
    for (i = 1; i < npoints; i++) {
	if (mode == CoordModePrevious) {
	    Line((short) (macWin->xOff + points[i].x),
		    (short) (macWin->yOff + points[i].y));
	} else {
	    LineTo((short) (macWin->xOff + points[i].x),
		    (short) (macWin->yOff + points[i].y));
	}
    }

    ClosePoly();

    /* Support the predefined stipple styles in Tk. */
    if (gc->fill_style == FillStippled) {
	PenMode(patOr);
	RGBForeColor(&macColor);
	/* Tk gray12 does not have a corresponding QuickDraw global. */
	if (gc->stipple == Tk_GetBitmap(interp, rootwin, "gray12")) {
		GetIndPattern(&thePat, sysPatListID, 22);
		PenPat(&thePat);
	} else if (gc->stipple == Tk_GetBitmap(interp, rootwin, "gray25")) {
		PenPat(QDPat(ltGray));
	} else if (gc->stipple == Tk_GetBitmap(interp, rootwin, "gray50")) {
		PenPat(QDPat(gray));
	} else if (gc->stipple == Tk_GetBitmap(interp, rootwin, "gray75")) {
		PenPat(QDPat(dkGray));
	} else if (gc->stipple == Tk_GetBitmap(interp, rootwin, "black")) {
		PenPat(QDPat(black));
	}
	PaintPoly(polygon);
	ForeColor(blackColor);
    /* Support tiled drawing. */
    } else if (gc->fill_style == FillTiled) {
	/* Adapted from Tk_FreePixmap in tkMacSubwindows.c. */
	MacDrawable *macPix = (MacDrawable *) gc->tile;
	PixMapHandle pixels = GetGWorldPixMap(macPix->portPtr);			
	PixPatHandle tilePat = NewPixPat();		
	int numBytes;

	/* Adapted from Pict2PPat.c. */
	numBytes = (**pixels).rowBytes & 0x7FFF;
	numBytes *= (**pixels).bounds.bottom - (**pixels).bounds.top;
	(**tilePat).patData = NewHandleClear(numBytes);
	LockPixels(pixels);
	CopyPixMap(pixels, (**tilePat).patMap);
	BlockMove(GetPixBaseAddr(pixels), *(**tilePat).patData, numBytes);
	UnlockPixels(pixels);
	PixPatChanged(tilePat);
	FillCPoly(polygon, tilePat);
	DisposePixPat(tilePat);
    /* Normal drawing. */
    } else {
    	FillCPoly(polygon, gPenPat);
    }    

    PenNormal();
    KillPoly(polygon);
    SetGWorld(saveWorld, saveDevice);
}

/* 030517 - Support for tiled drawing added. */

void XSetTile(Display *dpy, GC gc, Pixmap pm)
{
	gc->tile = pm;
}
