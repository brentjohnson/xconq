/* C++ Headers Hack for SDL Interface.
   Copyright (C) 2005 Eric A. McDonald.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/* (NOTE: This entire file should go away once we can safely guarantee 
    that enough of the people compiling Xconq have an ANSI C++-compliant 
    standard C++ library and compiler. The present problem is that the 
    headers in this file cannot be included in 'config.h' where they 
    likely belong, but must be included before 'conq.h', hence the name of 
    this file. The reason they cannot be included in 'config.h' is 
    because some older C++ standard C++ lib headers are not wrapped in the 
    'std' namespace, and this causes name conflicts between the C++ lib and 
    certain vars in the kernel.) */

#include <iostream>
