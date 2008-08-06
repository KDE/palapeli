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

#ifndef PALAPELI_PATTERN_HEXAGON_H
#define PALAPELI_PATTERN_HEXAGON_H

#if defined(MAKE_LIBPALAPELIPATTERN)
 #include "../lib/pattern.h"
 #include "../lib/pattern-configuration.h"
 #include "../lib/pattern-plugin.h"
#else
 #include <Palapeli/Pattern>
 #include <Palapeli/PatternConfiguration>
 #include <Palapeli/PatternPlugin>
#endif

namespace Palapeli
{

	/// \internal
	class HexagonalPattern : public Pattern
	{
		public:
			HexagonalPattern(int xCount, int yCount);
			virtual ~HexagonalPattern();
		protected:
			QPoint pieceBasePosition(int x, int y, const QSize& piece, const QSize& image) const;

			virtual void doSlice(const QImage& image);
		private:
			int m_xCount, m_yCount;
	};

	/// \internal
	class HexagonalPatternConfiguration : public PatternConfiguration
	{
		public:
			HexagonalPatternConfiguration(const QString& pluginName, const QString& displayName);
			virtual ~HexagonalPatternConfiguration();
			virtual Pattern* createPattern() const;
	};

	/// \internal
	class HexagonalPatternPlugin : public PatternPlugin
	{
		public:
			HexagonalPatternPlugin(QObject* parent = 0, const QVariantList& args = QVariantList());
			virtual ~HexagonalPatternPlugin();
			virtual QList<PatternConfiguration*> createInstances() const;
	};

}

#endif // PALAPELI_PATTERN_HEXAGON_H
