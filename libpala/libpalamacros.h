/***************************************************************************
 *   Copyright 2009 Stefan Majewsky <majewsky@gmx.net>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public
 *   License as published by the Free Software Foundation; either
 *   version 2 of the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 ***************************************************************************/

#ifndef LIBPALA_MACROS_H
#define LIBPALA_MACROS_H

#include <kdemacros.h>

#ifndef LIBPALA_EXPORT
# if defined(MAKE_LIBPALA)
#  define LIBPALA_EXPORT KDE_EXPORT
# else
#  define LIBPALA_EXPORT KDE_IMPORT
# endif
#endif

#endif // LIBPALA_MACROS_H
