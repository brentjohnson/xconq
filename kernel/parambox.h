/* Parameter boxen for Xconq.
   Copyright (C) 2004 Eric A, McDonald

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#ifndef XCONQ_PARAMBOX_H
#define XCONQ_PARAMBOX_H

/* Enumeration of parameter box types. */
enum ParamBoxType {
    PBOX_TYPE_NONE = 0,
    PBOX_TYPE_UNIT,			/* unit.h */
    PBOX_TYPE_UNIT_AT,			/* unit.h */
    PBOX_TYPE_UNIT_UNIT,		/* unit.h */
    PBOX_TYPE_UNIT_SIDE,		/* unit.h */
    PBOX_TYPE_UNIT_SIDE_SEERS,		/* unit.h */
    PBOX_TYPE_UNIT_UNIT_SEERS,		/* unit.h */
    PBOX_TYPE_SIDE,			/* side.h */
    PBOX_TYPE_COUNT			/* (Always last. Not a type.) */
};

/* Base struct for parameter boxen. */
/* (The parameter box is an alternative to using a function with
    an ellipsis and varargs. Having these "abbreviations" available can
    be more useful than parsing a varargs list. These are particularly
    helpful when used with predicate functions in search algorithms.) */
struct ParamBox {
  protected:
    ParamBoxType pboxtype;
    ParamBox() { pboxtype = PBOX_TYPE_NONE; }
  public:
    ParamBoxType get_type() { return pboxtype; }
};

#endif
