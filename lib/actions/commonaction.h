/***************************************************************************
 *   Copyright (C) 2008 Stefan Majewsky <majewsky@gmx.net>
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

#ifndef PALAPELI_COMMONACTION_H
#define PALAPELI_COMMONACTION_H

#include "../macros.h"

class QWidget;
#include "../../lib/library/puzzleinfo.h"

namespace Palapeli
{

	typedef const Palapeli::PuzzleInfo*(*InfoGetter)();

	namespace Actions
	{
		PALAPELIBASE_EXPORT QWidget* dialogParent();
		PALAPELIBASE_EXPORT void setDialogParent(QWidget* dialogParent);
		PALAPELIBASE_EXPORT const Palapeli::PuzzleInfo* puzzleInfo();
		PALAPELIBASE_EXPORT void setInfoGetter(InfoGetter infoGetter);
	}

}

#endif // PALAPELI_COMMONACTION_H