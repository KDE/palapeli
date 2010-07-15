/***************************************************************************
 *   Copyright  2010 Johannes Loehnert <loehnert.kde@gmx.de>
 *   Copyright  2010 Stefan Majewsky <majewsky@gmx.net>
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

#ifndef PALAPELISLICERS_GOLDBERG_GRID_H
#define PALAPELISLICERS_GOLDBERG_GRID_H

#include <KLocalizedString>
#include "../../libpala/slicermode.h"

#include "goldberg-engine.h"

class GoldbergMode : public Pala::SlicerMode
{
	public:
		GoldbergMode(const QByteArray& key, const QString& name) : Pala::SlicerMode(key, name) {}
		virtual ~GoldbergMode() {}
		virtual void generateGrid(GoldbergEngine *e, int piece_count) const = 0;
};

class CairoMode : public GoldbergMode
{
	public:
		CairoMode() : GoldbergMode("cairo", i18nc("Puzzle grid type", "Cairo (pentagonal) grid")) {}
		virtual void generateGrid(GoldbergEngine *e, int piece_count) const;
};

class HexMode : public GoldbergMode
{
	public:
		HexMode() : GoldbergMode("hex", i18nc("Puzzle grid type", "Hexagonal grid")) {}
		virtual void generateGrid(GoldbergEngine *e, int piece_count) const;
};

class RectMode : public GoldbergMode
{
	public:
		RectMode() : GoldbergMode("rect", i18nc("Puzzle grid type", "Rectangular grid")) {}
		virtual void generateGrid(GoldbergEngine *e, int piece_count) const;
};

class RotrexMode : public GoldbergMode
{
	public:
		RotrexMode() : GoldbergMode("rotrex", i18nc("Puzzle grid type", "Rotrex (rhombi-trihexagonal) grid")) {}
		virtual void generateGrid(GoldbergEngine *e, int piece_count) const;
};

class IrregularMode : public GoldbergMode
{
	public:
		IrregularMode() : GoldbergMode("irreg", i18nc("Puzzle grid type", "Irregular grid")) {}

		/// Checks if qvoronoi executable is there.
		static bool checkForQVoronoi();

		virtual void generateGrid(GoldbergEngine *e, int piece_count) const;
	private:
		void generateVoronoiGrid(GoldbergEngine *e, QList<QPointF> cell_centers) const;
};

#endif // PALAPELISLICERS_GOLDBERG_GRID_H
