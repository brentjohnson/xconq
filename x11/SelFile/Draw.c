/*
 * Copyright 1989 Software Research Associates, Inc., Tokyo, Japan
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Software Research Associates not be used
 * in advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.  Software Research Associates
 * makes no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * SOFTWARE RESEARCH ASSOCIATES DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
 * IN NO EVENT SHALL SOFTWARE RESEARCH ASSOCIATES BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Erik M. van der Poel
 *         Software Research Associates, Inc., Tokyo, Japan
 *         erik@sra.co.jp
 */

#include <stdio.h>
#include "SFinternal.h"
#include "xstat.h"
#include <X11/StringDefs.h>
#include <X11/Xaw/Scrollbar.h>

#define SF_DEFAULT_FONT "9x15"

#define ABS(x) (((x) < 0) ? (-(x)) : (x))

typedef struct {
	char *fontname;
} TextData, *textPtr;

int SFcharWidth, SFcharAscent, SFcharHeight;

int SFcurrentInvert[3] = { -1, -1, -1 };

static GC SFlineGC, SFscrollGC, SFinvertGC, SFtextGC;

static XtResource textResources[] = {
	{XtNfont, XtCFont, XtRString, sizeof (char *),
		XtOffset(textPtr, fontname), XtRString, (void*)SF_DEFAULT_FONT},
};

static XFontStruct *SFfont;

static int SFcurrentListY;

static XtIntervalId SFscrollTimerId;

int SFchdir(char *);

void
SFinitFont(void)
{
	TextData	*data;

	data = XtNew(TextData);

	XtGetApplicationResources(selFileForm, (XtPointer) data, textResources,
		XtNumber(textResources), (Arg *) NULL, 0);

	SFfont = XLoadQueryFont(SFdisplay, data->fontname);
	if (!SFfont) {
		SFfont = XLoadQueryFont(SFdisplay, SF_DEFAULT_FONT);
		if (!SFfont) {
			char	sbuf[256];

			(void) sprintf(sbuf, "XsraSelFile: can't get font %s",
				SF_DEFAULT_FONT);

			XtAppError(SFapp, sbuf);
		}
	}

	SFcharWidth = (SFfont->max_bounds.width + SFfont->min_bounds.width) / 2;
	SFcharAscent = SFfont->max_bounds.ascent;
	SFcharHeight = SFcharAscent + SFfont->max_bounds.descent;
}

void
SFcreateGC(void)
{
	XGCValues	gcValues;
	XRectangle	rectangles[1];

	gcValues.foreground = SFfore;

	SFlineGC = XtGetGC(
		selFileLists[0],
		(XtGCMask)
			GCForeground		|
			0,
		&gcValues
	);

	SFscrollGC = XtGetGC(
		selFileLists[0],
		(XtGCMask)
			0,
		&gcValues
	);

	gcValues.function = GXinvert;
	gcValues.plane_mask = (SFfore ^ SFback);

	SFinvertGC = XtGetGC(
		selFileLists[0],
		(XtGCMask)
			GCFunction		|
			GCPlaneMask		|
			0,
		&gcValues
	);

	gcValues.foreground = SFfore;
	gcValues.background = SFback;
	gcValues.font = SFfont->fid;

	SFtextGC = XCreateGC(
		SFdisplay,
		XtWindow(selFileLists[0]),
		(unsigned long)
			GCForeground		|
			GCBackground		|
			GCFont			|
			0,
		&gcValues
	);

	rectangles[0].x = SFlineToTextH + SFbesideText;
	rectangles[0].y = 0;
	rectangles[0].width = SFcharsPerEntry * SFcharWidth;
	rectangles[0].height = SFupperY + 1;

	XSetClipRectangles(
		SFdisplay,
		SFtextGC,
		0,
		0,
		rectangles,
		1,
		Unsorted
	);
}

void
SFclearList(int n, int doScroll)
{
	SFDir	*dir;

	SFcurrentInvert[n] = -1;

	XClearWindow(SFdisplay, XtWindow(selFileLists[n]));

	XDrawSegments(SFdisplay, XtWindow(selFileLists[n]), SFlineGC, SFsegs,
		2);

	if (doScroll) {
		dir = &(SFdirs[SFdirPtr + n]);

		if ((SFdirPtr + n < SFdirEnd) && dir->nEntries && dir->nChars) {
			XawScrollbarSetThumb(
				selFileVScrolls[n],
				(float) (((double) dir->vOrigin) /
					dir->nEntries),
				(float) (((double) ((dir->nEntries < SFlistSize)
					? dir->nEntries : SFlistSize)) /
					dir->nEntries)
			);

			XawScrollbarSetThumb(
				selFileHScrolls[n],
				(float) (((double) dir->hOrigin) / dir->nChars),
				(float) (((double) ((dir->nChars <
					SFcharsPerEntry) ? dir->nChars :
					SFcharsPerEntry)) / dir->nChars)
			);
		} else {
			XawScrollbarSetThumb(selFileVScrolls[n], (float) 0.0,
				(float) 1.0);
			XawScrollbarSetThumb(selFileHScrolls[n], (float) 0.0,
				(float) 1.0);
		}
	}
}

static void
SFdeleteEntry(SFDir *dir, SFEntry *entry)
{
	register SFEntry	*e;
	register SFEntry	*end;
	int			n;
	int			idx;

	idx = entry - dir->entries;

	if (idx < dir->beginSelection) {
		dir->beginSelection--;
	}
	if (idx <= dir->endSelection) {
		dir->endSelection--;
	}
	if (dir->beginSelection > dir->endSelection) {
		dir->beginSelection = dir->endSelection = -1;
	}

	if (idx < dir->vOrigin) {
		dir->vOrigin--;
	}

	XtFree(entry->real);

	end = &(dir->entries[dir->nEntries - 1]);

	for (e = entry; e < end; e++) {
		*e = *(e + 1);
	}

	if (!(--dir->nEntries)) {
		return;
	}

	n = dir - &(SFdirs[SFdirPtr]);
	if ((n < 0) || (n > 2)) {
		return;
	}

	XawScrollbarSetThumb(
		selFileVScrolls[n],
		(float) (((double) dir->vOrigin) / dir->nEntries),
		(float) (((double) ((dir->nEntries < SFlistSize) ?
			dir->nEntries : SFlistSize)) / dir->nEntries)
	);
}

static void
SFwriteStatChar(char *name, int last, struct stat *statBuf)
{
	name[last] = SFstatChar(statBuf);
}

static int
SFstatAndCheck(SFDir *dir, SFEntry *entry)
{
	struct stat	statBuf;
	char		save;
	int		last;

	/*
	 * must be restored before returning
	 */
	save = *(dir->path);
	*(dir->path) = 0;

	if (!SFchdir(SFcurrentPath)) {
		last = strlen(entry->real) - 1;
		entry->real[last] = 0;
		entry->statDone = 1;
		if (
			(!stat(entry->real, &statBuf))

#ifdef S_IFLNK

		     || (!lstat(entry->real, &statBuf))

#endif /* ndef S_IFLNK */

		) {
			if (SFfunc) {
				char *shown;

				shown = NULL;
				if (SFfunc(entry->real, &shown, &statBuf)) {
					if (shown) {
						int len;

						len = strlen(shown);
						entry->shown = XtMalloc(
							(unsigned) (len + 2)
						);
						(void) strcpy(entry->shown,
							shown);
						SFwriteStatChar(
							entry->shown,
							len,
							&statBuf
						);
						entry->shown[len + 1] = 0;
					}
				} else {
					SFdeleteEntry(dir, entry);

					*(dir->path) = save;
					return 1;
				}
			}
			SFwriteStatChar(entry->real, last, &statBuf);
		} else {
			entry->real[last] = ' ';
		}
	}

	*(dir->path) = save;
	return 0;
}

static void
SFdrawStrings(register Window w, register SFDir *dir, register int from, register int to)
{
	register int		i;
	register SFEntry	*entry;
	int			x;

	x = SFtextX - dir->hOrigin * SFcharWidth;

	if (dir->vOrigin + to >= dir->nEntries) {
		to = dir->nEntries - dir->vOrigin - 1;
	}
	for (i = from; i <= to; i++) {
		entry = &(dir->entries[dir->vOrigin + i]);
		if (!(entry->statDone)) {
			if (SFstatAndCheck(dir, entry)) {
				if (dir->vOrigin + to >= dir->nEntries) {
					to = dir->nEntries - dir->vOrigin - 1;
				}
				i--;
				continue;
			}
		}
		XDrawImageString(
			SFdisplay,
			w,
			SFtextGC,
			x,
			SFtextYoffset + i * SFentryHeight,
			entry->shown,
			strlen(entry->shown)
		);
		if (dir->vOrigin + i == dir->beginSelection) {
			XDrawLine(
				SFdisplay,
				w,
				SFlineGC,
				SFlineToTextH + 1,
				SFlowerY + i * SFentryHeight,
				SFlineToTextH + SFentryWidth - 2,
				SFlowerY + i * SFentryHeight
			);
		}
		if (
			(dir->vOrigin + i >= dir->beginSelection) &&
			(dir->vOrigin + i <= dir->endSelection)
		) {
			SFcompletionSegs[0].y1 = SFcompletionSegs[1].y1 =
				SFlowerY + i * SFentryHeight;
			SFcompletionSegs[0].y2 = SFcompletionSegs[1].y2 =
				SFlowerY + (i + 1) * SFentryHeight - 1;
			XDrawSegments(
				SFdisplay,
				w,
				SFlineGC,
				SFcompletionSegs,
				2
			);
		}
		if (dir->vOrigin + i == dir->endSelection) {
			XDrawLine(
				SFdisplay,
				w,
				SFlineGC,
				SFlineToTextH + 1,
				SFlowerY + (i + 1) * SFentryHeight - 1,
				SFlineToTextH + SFentryWidth - 2,
				SFlowerY + (i + 1) * SFentryHeight - 1
			);
		}
	}
}

void
SFdrawList(int n, int doScroll)
{
	SFDir	*dir;
	Window	w;

	SFclearList(n, doScroll);

	if (SFdirPtr + n < SFdirEnd) {
		dir = &(SFdirs[SFdirPtr + n]);
		w = XtWindow(selFileLists[n]);
		XDrawImageString(
			SFdisplay,
			w,
			SFtextGC,
			SFtextX - dir->hOrigin * SFcharWidth,
			SFlineToTextV + SFaboveAndBelowText + SFcharAscent,
			dir->dir,
			strlen(dir->dir)
		);
		SFdrawStrings(w, dir, 0, SFlistSize - 1);
	}
}

void
SFdrawLists(int doScroll)
{
	int	i;

	for (i = 0; i < 3; i++) {
		SFdrawList(i, doScroll);
	}
}

static void
SFinvertEntry(register int n)
{
	XFillRectangle(
		SFdisplay,
		XtWindow(selFileLists[n]),
		SFinvertGC,
		SFlineToTextH,
		SFcurrentInvert[n] * SFentryHeight + SFlowerY,
		SFentryWidth,
		SFentryHeight
	);
}

/* Didn't match signature in X11/Intrinsic.h */
static unsigned long
SFscrollTimerInterval()
{
	static int	maxVal = 200;
	static int	varyDist = 50;
	static int	minDist = 50;
	int		t;
	int		dist;

	if (SFcurrentListY < SFlowerY) {
		dist = SFlowerY - SFcurrentListY;
	} else if (SFcurrentListY > SFupperY) {
		dist = SFcurrentListY - SFupperY;
	} else {
		return (unsigned long) 1;
	}

	t = maxVal - ((maxVal / varyDist) * (dist - minDist));

	if (t < 1) {
		t = 1;
	}

	if (t > maxVal) {
		t = maxVal;
	}

	return (unsigned long) t;
}

static void
SFscrollTimer(XtPointer xp, XtIntervalId *id)
{
	SFDir	*dir;
	int	save;
	int n = (int)xp;

	dir = &(SFdirs[SFdirPtr + n]);
	save = dir->vOrigin;

	if (SFcurrentListY < SFlowerY) {
		if (dir->vOrigin > 0) {
			SFvSliderMovedCallback(selFileVScrolls[n], n,
				dir->vOrigin - 1);
		}
	} else if (SFcurrentListY > SFupperY) {
		if (dir->vOrigin < dir->nEntries - SFlistSize) {
			SFvSliderMovedCallback(selFileVScrolls[n], n,
				dir->vOrigin + 1);
		}
	}

	if (dir->vOrigin != save) {
		if (dir->nEntries) {
		    XawScrollbarSetThumb(
			selFileVScrolls[n],
			(float) (((double) dir->vOrigin) / dir->nEntries),
			(float) (((double) ((dir->nEntries < SFlistSize) ?
				dir->nEntries : SFlistSize)) / dir->nEntries)
		    );
		}
	}

	if (SFbuttonPressed) {
		SFscrollTimerId = XtAppAddTimeOut(SFapp,
			SFscrollTimerInterval(), SFscrollTimer, (XtPointer) n);
	}
}

static int
SFnewInvertEntry(register int n, register XMotionEvent *event)
     /*register int		n;
       register XMotionEvent	*event;*/
{
	register int	x, y;
	register int	neww;
	static int	SFscrollTimerAdded = 0;

	x = event->x;
	y = event->y;

	if (SFdirPtr + n >= SFdirEnd) {
		return -1;
	} else if (
		(x >= 0)	&& (x <= SFupperX) &&
		(y >= SFlowerY)	&& (y <= SFupperY)
	) {
		register SFDir *dir = &(SFdirs[SFdirPtr + n]);

		if (SFscrollTimerAdded) {
			SFscrollTimerAdded = 0;
			XtRemoveTimeOut(SFscrollTimerId);
		}

		neww = (y - SFlowerY) / SFentryHeight;
		if (dir->vOrigin + neww >= dir->nEntries) {
			return -1;
		}
		return neww;
	} else {
		if (SFbuttonPressed) {
			SFcurrentListY = y;
			if (!SFscrollTimerAdded) {
				SFscrollTimerAdded = 1;
				SFscrollTimerId = XtAppAddTimeOut(SFapp,
					SFscrollTimerInterval(), SFscrollTimer,
					(XtPointer) n);
			}
		}

		return -1;
	}
}

/* ARGSUSED */
void
SFenterList(Widget w, register XtPointer n, register XEvent *event, Boolean *cont)
     /*Widget				w;
	register int			n;
	register XEnterWindowEvent	*event;*/
{
	register int	neww;

	/* sanity */
	if (SFcurrentInvert[(int)n] != -1) {
		SFinvertEntry((int)n);
		SFcurrentInvert[(int)n] = -1;
	}

	neww = SFnewInvertEntry((int)n, (XMotionEvent *) event);
	if (neww != -1) {
		SFcurrentInvert[(int)n] = neww;
		SFinvertEntry((int)n);
	}
}

/* ARGSUSED */
void
SFleaveList(Widget w, register XtPointer n, XEvent *event, Boolean *cont)
     /*Widget		w;
	register int	n;
	XEvent		*event;*/
{
	if (SFcurrentInvert[(int)n] != -1) {
		SFinvertEntry((int)n);
		SFcurrentInvert[(int)n] = -1;
	}
}

/* ARGSUSED */
void
SFmotionList(Widget w, register XtPointer n, register XEvent *event, Boolean *cont)
     /*Widget			w;
	register int		n;
	register XMotionEvent	*event;*/
{
	register int	neww;

	neww = SFnewInvertEntry((int)n, (XMotionEvent *)event);

	if (neww != SFcurrentInvert[(int)n]) {
		if (SFcurrentInvert[(int)n] != -1) {
			SFinvertEntry((int)n);
		}
		SFcurrentInvert[(int)n] = neww;
		if (neww != -1) {
			SFinvertEntry((int)n);
		}
	}
}

/* ARGSUSED */
void
SFvFloatSliderMovedCallback(Widget w, XtPointer n, XtPointer fnew)
     /*Widget	w;
	int	n;
	float	*fnew;*/
{
	int	neww;

	neww = (int)(*((float *)fnew) * SFdirs[SFdirPtr + (int)n].nEntries);

	SFvSliderMovedCallback(w, (int)n, neww);
}

/* ARGSUSED */
void
SFvSliderMovedCallback(Widget w, int n, int neww)
     /*Widget	w;
	int	n;
	int	neww;*/
{
	int		old;
	register Window	win;
	SFDir		*dir;

	dir = &(SFdirs[SFdirPtr + n]);

	old = dir->vOrigin;
	dir->vOrigin = neww;

	if (old == neww) {
		return;
	}

	win = XtWindow(selFileLists[n]);

	if (ABS(neww - old) < SFlistSize) {
		if (neww > old) {
			XCopyArea(
				SFdisplay,
				win,
				win,
				SFscrollGC,
				SFlineToTextH,
				SFlowerY + (neww - old) * SFentryHeight,
				SFentryWidth + SFlineToTextH,
				(SFlistSize - (neww - old)) * SFentryHeight,
				SFlineToTextH,
				SFlowerY
			);
			XClearArea(
				SFdisplay,
				win,
				SFlineToTextH,
				SFlowerY + (SFlistSize - (neww - old)) *
					SFentryHeight,
				SFentryWidth + SFlineToTextH,
				(neww - old) * SFentryHeight,
				False
			);
			SFdrawStrings(win, dir, SFlistSize - (neww - old),
				SFlistSize - 1);
		} else {
			XCopyArea(
				SFdisplay,
				win,
				win,
				SFscrollGC,
				SFlineToTextH,
				SFlowerY,
				SFentryWidth + SFlineToTextH,
				(SFlistSize - (old - neww)) * SFentryHeight,
				SFlineToTextH,
				SFlowerY + (old - neww) * SFentryHeight
			);
			XClearArea(
				SFdisplay,
				win,
				SFlineToTextH,
				SFlowerY,
				SFentryWidth + SFlineToTextH,
				(old - neww) * SFentryHeight,
				False
			);
			SFdrawStrings(win, dir, 0, old - neww);
		}
	} else {
		XClearArea(
			SFdisplay,
			win,
			SFlineToTextH,
			SFlowerY,
			SFentryWidth + SFlineToTextH,
			SFlistSize * SFentryHeight,
			False
		);
		SFdrawStrings(win, dir, 0, SFlistSize - 1);
	}
}

/* ARGSUSED */
void
SFvAreaSelectedCallback(Widget w, XtPointer n, XtPointer pnew)
     /*Widget	w;
	int	n;
	int	pnew;*/
{
	SFDir	*dir;
	int	neww;

	dir = &(SFdirs[SFdirPtr + (int)n]);

	neww = (int)(dir->vOrigin +
		(((double)((int)pnew)) / SFvScrollHeight) * dir->nEntries);

	if (neww > dir->nEntries - SFlistSize) {
		neww = dir->nEntries - SFlistSize;
	}

	if (neww < 0) {
		neww = 0;
	}

	if (dir->nEntries) {
		float f = ((double) neww) / dir->nEntries;

		XawScrollbarSetThumb(
			w,
			f,
			(float) (((double) ((dir->nEntries < SFlistSize) ?
				dir->nEntries : SFlistSize)) / dir->nEntries)
		);
	}

	SFvSliderMovedCallback(w, (int)n, neww);
}

/* ARGSUSED */
void
SFhSliderMovedCallback(Widget w, XtPointer n, XtPointer neww)
     /*Widget	w;
	int	n;
	float	*neww;*/
{
	SFDir	*dir;
	int	save;

	dir = &(SFdirs[SFdirPtr + (int)n]);
	save = dir->hOrigin;
	dir->hOrigin = (int)((*((float *)neww)) * dir->nChars);
	if (dir->hOrigin == save) {
		return;
	}

	SFdrawList((int)n, SF_DO_NOT_SCROLL);
}

/* ARGSUSED */
void
SFhAreaSelectedCallback(Widget w, XtPointer n, XtPointer pnew)
     /*Widget	w;
	int	n;
	int	pnew;*/
{
	SFDir	*dir;
	int	neww;

	dir = &(SFdirs[SFdirPtr + (int)n]);

	neww = (int)(dir->hOrigin +
		(((double)*((float *)pnew)) / SFhScrollWidth) * dir->nChars);

	if (neww > dir->nChars - SFcharsPerEntry) {
		neww = dir->nChars - SFcharsPerEntry;
	}

	if (neww < 0) {
		neww = 0;
	}

	if (dir->nChars) {
		float	f;

		f = ((double) neww) / dir->nChars;

		XawScrollbarSetThumb(
			w,
			f,
			(float) (((double) ((dir->nChars < SFcharsPerEntry) ?
				dir->nChars : SFcharsPerEntry)) / dir->nChars)
		);

		SFhSliderMovedCallback(w, n, (XtPointer)&f);
	}
}

/* ARGSUSED */
void
SFpathSliderMovedCallback(Widget w, XtPointer client_data, XtPointer neww)
     /*Widget		w;
	XtPointer	client_data;
	float	*neww;*/
{
	SFDir		*dir;
	int		n;
	XawTextPosition	pos;
	int	SFdirPtrSave;

	SFdirPtrSave = SFdirPtr;
	SFdirPtr = (int)((*((float *)neww)) * SFdirEnd);
	if (SFdirPtr == SFdirPtrSave) {
		return;
	}

	SFdrawLists(SF_DO_SCROLL);

	n = 2;
	while (SFdirPtr + n >= SFdirEnd) {
		n--;
	}

	dir = &(SFdirs[SFdirPtr + n]);

	pos = dir->path - SFcurrentPath;

	if (!strncmp(SFcurrentPath, SFstartDir, strlen(SFstartDir))) {
		pos -= strlen(SFstartDir);
		if (pos < 0) {
			pos = 0;
		}
	}

	XawTextSetInsertionPoint(selFileField, pos);
}

/* ARGSUSED */

void
SFpathAreaSelectedCallback(Widget w, XtPointer client_data, XtPointer pnew)
{
	int	neww;
	float	f;

	neww = (int)(SFdirPtr + (((double)((int)pnew)) / SFpathScrollWidth) * SFdirEnd);

	if (neww > SFdirEnd - 3) {
		neww = SFdirEnd - 3;
	}

	if (neww < 0) {
		neww = 0;
	}

	f = ((double) neww) / SFdirEnd;

	XawScrollbarSetThumb(
		w,
		f,
		(float) (((double) ((SFdirEnd < 3) ? SFdirEnd : 3)) /
			SFdirEnd)
	);

	SFpathSliderMovedCallback(w, (XtPointer) NULL, (XtPointer)&f);
}

Boolean
SFworkProc(void *p)
{
	register SFDir		*dir;
	register SFEntry	*entry;

	for (dir = &(SFdirs[SFdirEnd - 1]); dir >= SFdirs; dir--) {
		if (!(dir->nEntries)) {
			continue;
		}
		for (
			entry = &(dir->entries[dir->nEntries - 1]);
			entry >= dir->entries;
			entry--
		) {
			if (!(entry->statDone)) {
				(void) SFstatAndCheck(dir, entry);
				return False;
			}
		}
	}

	SFworkProcAdded = 0;

	return True;
}
