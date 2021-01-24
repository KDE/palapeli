/*
    SPDX-FileCopyrightText: 2010 Johannes Loehnert <loehnert.kde@gmx.de>
    SPDX-FileCopyrightText: 2010 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PALAPELISLICERS_GOLDBERG_GRID_H
#define PALAPELISLICERS_GOLDBERG_GRID_H

#include <KLocalizedString>
#include <Pala/SlicerMode>

#include "goldberg-engine.h"

class GoldbergMode : public Pala::SlicerMode
{
	public:
		GoldbergMode(const QByteArray& key, const QString& name) : Pala::SlicerMode(key, name) {}
		~GoldbergMode() override {}
		virtual void generateGrid(GoldbergEngine *e, int piece_count) const = 0;
};

class PresetMode : public GoldbergMode
{
	public:
		PresetMode() : GoldbergMode("preset", i18nc("Puzzle grid type", "Predefined settings")) {}
		void generateGrid(GoldbergEngine *e, int piece_count) const override;
};

class CairoMode : public GoldbergMode
{
	public:
		CairoMode() : GoldbergMode("cairo", i18nc("Puzzle grid type", "Cairo (pentagonal) grid")) {}
		void generateGrid(GoldbergEngine *e, int piece_count) const override;
};

class HexMode : public GoldbergMode
{
	public:
		HexMode() : GoldbergMode("hex", i18nc("Puzzle grid type", "Hexagonal grid")) {}
		void generateGrid(GoldbergEngine *e, int piece_count) const override;
};

class RectMode : public GoldbergMode
{
	public:
		RectMode() : GoldbergMode("rect", i18nc("Puzzle grid type", "Rectangular grid")) {}
		void generateGrid(GoldbergEngine *e, int piece_count) const override;
};

class RotrexMode : public GoldbergMode
{
	public:
		RotrexMode() : GoldbergMode("rotrex", i18nc("Puzzle grid type", "Rotrex (rhombi-trihexagonal) grid")) {}
		void generateGrid(GoldbergEngine *e, int piece_count) const override;
};

class IrregularMode : public GoldbergMode
{
	public:
		IrregularMode() : GoldbergMode("irreg", i18nc("Puzzle grid type", "Irregular grid")) {}

		/// Checks if qvoronoi executable is there.
		static bool checkForQVoronoi();

		void generateGrid(GoldbergEngine *e, int piece_count) const override;
	private:
		void generateVoronoiGrid(GoldbergEngine *e, QList<QPointF> cell_centers) const;
};

#endif // PALAPELISLICERS_GOLDBERG_GRID_H
