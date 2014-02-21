// xConq
// Common Typedefs of C/C++ Primitives

// $Id: primitives.h,v 1.1 2006/04/07 01:03:23 eric_mcdonald Exp $

/*//////////////////////////// COPYRIGHT NOTICE ////////////////////////////////

  Copyright (C) 2005-2006    Eric A. McDonald

//////////////////////////////////// LICENSE ///////////////////////////////////

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

//////////////////////////////////////////////////////////////////////////////*/

/*! \file
    \brief Collection of platform-independent, common typedefs for C/C++ primitives.
    \ingroup grp_types

    \todo Add typedefs for UTF-16 big-endian and little-endian strings.
*/

#ifndef XCONQ_BASETYPE_PRIMITIVES_H
#define XCONQ_BASETYPE_PRIMITIVES_H

#include "basetypes/base.h"

NAMESPACE_XCONQ_BEGIN
NAMESPACE_BASETYPE_BEGIN

/*! \brief Boolean

    Storage: Platform-Dependent\n
    Representation: Boolean
*/
typedef bool			Bool;

/*! \brief Raw Byte

    Storage: 1 Byte\n
    Representation: Raw Byte
*/
typedef unsigned char		Byte;

/*! \brief Array of Raw Bytes

    Storage: Arbitrary-Length Array of Bytes\n
    Representation: Pointer to Raw Bytes
*/
typedef Byte *			Byte_p;

/*! \brief Array of Const Raw Bytes

    Storage: Arbitrary-Length Array of Bytes\n
    Representation: Pointer to Const Raw Bytes
*/
typedef const Byte *		Byte_cp;

/*! \brief Const Array of Const Raw Bytes

    Storage: Arbitrary-Length Array of Bytes\n
    Representation: Const Pointer to Const Raw Bytes
*/
typedef const Byte * const	Byte_cpc;

/*! \brief Const Array of Raw Bytes

    Storage: Arbitrary-Length Array of Bytes\n
    Representation: Const Pointer to Raw Bytes
*/
typedef Byte * const		Byte_pc;

/*! \brief 8-bit 2's-Complement Integer (Signed)

    Storage: 1 Byte\n
    Representation: 2's-Complement Integer (Signed)
*/
typedef signed char		Z8;

/*! \brief 16-bit 2's-Complement Integer (Signed)

    Storage: 2 Bytes\n
    Representation: 2's-Complement Integer (Signed)
*/
typedef short			Z16;

/*! \brief 32-bit 2's-Complement Integer (Signed)

    Storage: 4 Bytes\n
    Representation: 2's-Complement Integer (Signed)\n
    Note: Underlying C/C++ primitive depends on platform.
*/
#if (SIZEOF_INT == 4)
typedef int			Z32;
#else
typedef long			Z32;
#endif

/*! \brief 8-bit 2's-Complement Integer (Unsigned)

    Storage: 1 Byte\n
    Representation: 2's-Complement Integer (Unsigned)
*/
typedef unsigned char		Z8u;

/*! \brief 16-bit 2's-Complement Integer (Unsigned)

    Storage: 2 Bytes\n
    Representation: 2's-Complement Integer (Unsigned)
*/
typedef unsigned short		Z16u;

/*! \brief 32-bit 2's-Complement Integer (Unsigned)

    Storage: 4 Bytes\n
    Representation: 2's-Complement Integer (Unsigned)\n
    Note: Underlying C/C++ primitive depends on platform.
*/
#if (SIZEOF_INT == 4)
typedef unsigned int		Z32u;
#else
typedef unsigned long		Z32u;
#endif

/*! \brief 32-bit Floating-Point Pseudo-Real

    Storage: 4 Bytes\n
    Representation: Floating-Point Pseudo-Real (usually IEEE-754 Format)
*/
typedef float			R32;

/*! \brief 64-bit Floating-Point Pseudo-Real

    Storage: 8 Bytes\n
    Representation: Floating-Point Pseudo-Real (usually IEEE-754 Format)
*/
typedef double			R64;

/*! \brief String of UTF-8 Characters

    Storage: Arbitrary-Length Array of Bytes\n
    Representation: Pointer to UTF-8 Characters
*/
typedef char *			UTF8_p;

/*! \brief String of Const UTF-8 Characters

    Storage: Arbitrary-Length Array of Bytes\n
    Representation: Pointer to Const UTF-8 Characters
*/
typedef const char *		UTF8_cp;

/*! \brief Const String of Const UTF-8 Characters

    Storage: Arbitrary-Length Array of Bytes\n
    Representation: Const Pointer to Const UTF-8 Characters
*/
typedef const char * const	UTF8_cpc;

/*! \brief Const String of UTF-8 Characters

    Storage: Arbitrary-Length Array of Bytes\n
    Representation: Const Pointer to UTF-8 Characters
*/
typedef char * const		UTF8_pc;

NAMESPACE_BASETYPE_END
NAMESPACE_XCONQ_END

#endif // ifndef XCONQ_BASETYPE_PRIMITIVES_H
