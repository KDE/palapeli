/***************************************************************************
 *   Copyright (C) 2008 Felix Lemke <lemke.felix@ages-skripte.org>
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

#ifndef PALAPELI_STRINGS_H
#define PALAPELI_STRINGS_H

#include <QString>
#include <KStandardDirs>

namespace Palapeli
{

	namespace Strings
	{
		//savegame storage strings and convenience functions
		const QString SavegameDir("savegames");
		const QString ConfigPath("savegames/%1.psg");
		inline QString configPath(const QString& gameName)
			{ return KStandardDirs::locateLocal("appdata", ConfigPath.arg(gameName)); }
		const QString DataPath("savegames/%1"); //used for images
		inline QString dataPath(const QString& dataName)
			{ return KStandardDirs::locateLocal("appdata", DataPath.arg(dataName)); }
		//strings in .psg files
		const QString GeneralGroupKey("Palapeli");
		const QString PatternKey("Pattern");
		const QString ImageFileKey("ImageSource");
		const QString PatternGroupKey("PatternArgs");
		const QString PiecesGroupKey("Pieces");
		const QString PositionKey("Position-%1");
		//strings in palapelirc
		const QString GamesGroupKey("Saved Games");
		const QString GamesListKey("Names");
	};

}

#endif // PALAPELI_STRINGS_H
