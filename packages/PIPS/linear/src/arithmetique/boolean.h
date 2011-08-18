/*

  $Id$

  Copyright 1989-2010 MINES ParisTech

  This file is part of PIPS.

  PIPS is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  any later version.

  PIPS is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.

  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with PIPS.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef BOOLEAN_INCLUDED
#define BOOLEAN_INCLUDED

#include <stdint.h>

#ifdef bool
    #error newgen header not compatible with stdbool.h and linear header have to be compatible with newgen
#endif

typedef intptr_t bool; /* we cannot use an enum or stdbool because we need to be compatible with newgen */

#define false 0
#define true 1

#endif /* BOOLEAN_INCLUDED */
