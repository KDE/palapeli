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

#ifndef PALAPELI_PATTERN_JIGSAW_H
#define PALAPELI_PATTERN_JIGSAW_H

#if defined(MAKE_LIBPALAPELIPATTERN)
 #include "../lib/pattern.h"
 #include "../lib/pattern-configuration.h"
 #include "../lib/pattern-plugin.h"
#else
 #include <Palapeli/Pattern>
 #include <Palapeli/PatternConfiguration>
 #include <Palapeli/PatternPlugin>
#endif

#include <QStringList>
class KSvgRenderer;

namespace Palapeli
{

	/// \internal
	class JigsawPattern : public Pattern
	{
		public:
			JigsawPattern(int xCount, int yCount, const QString& name, int seed);
			virtual ~JigsawPattern();
		protected:
			virtual void doSlice(const QImage& image);
		private:
			int m_xCount, m_yCount;
			int m_shapeCount, m_seed;
			QList<KSvgRenderer*> m_shapes;
	};

	/// \internal
	class JigsawPatternConfiguration : public PatternConfiguration
	{
		public:
			JigsawPatternConfiguration(const QString& themeName, const QString& pluginName, const QString& displayName);
			virtual ~JigsawPatternConfiguration();
			virtual Pattern* createPattern() const;
		protected:
			virtual void readCustomArguments(KConfigGroup* config);
			virtual void writeCustomArguments(KConfigGroup* config) const;
		private:
			int m_seed;
			QString m_themeName;
	};

	/// \internal
	class JigsawPatternPlugin : public PatternPlugin
	{
		public:
			explicit JigsawPatternPlugin(QObject* parent = 0, const QVariantList& args = QVariantList());
			virtual ~JigsawPatternPlugin();
			virtual QList<PatternConfiguration*> createInstances() const;
		private:
			QStringList m_themeNames;
	};

}

#endif // PALAPELI_PATTERN_JIGSAW_H
