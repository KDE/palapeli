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

#ifndef PALAPELI_PATTERN_CONFIGURATION_H
#define PALAPELI_PATTERN_CONFIGURATION_H

#if defined(MAKE_LIBPALAPELICORE)
 #include "macros.h"
#else
 #include <Palapeli/Macros>
#endif

#include <QObject>
class QWidget;
class KConfigGroup;

//TODO: bool function whether there are configurable things at all (add spacerItem in NewPuzzleDialog to not totally break the layout in that case)

namespace Palapeli
{

	class Pattern;
	class PatternConfigurationPrivate;

	/**
	 * \class PatternConfiguration pattern-configuration.h Palapeli/PatternConfiguration
	 *
	 * This class is not documented yet.
	 *
	 * \author Stefan Majewsky <majewsky@gmx.net>
	 */
	class PALAPELICORE_EXPORT PatternConfiguration : public QObject
	{
		Q_OBJECT
		public:
			enum SizeDefinitionMode
			{
				CustomSizeDefinition = 0,
				CountSizeDefinition
			};

			PatternConfiguration(const QByteArray& patternName, const QString& displayName);
			virtual ~PatternConfiguration();

			//implementation of subclasses (i.e. plugins); interface to Palapeli core
			virtual Pattern* createPattern() const = 0;

			//interface to Palapeli core
			QByteArray patternName() const;
			QString displayName() const;
			QWidget* createConfigurationWidget() const; //DOC: talk about ownership issues
			void readArguments(KConfigGroup* config);
			void writeArguments(KConfigGroup* config) const;
		protected:
			//implementation of subclasses (i.e. plugins); interface to Palapeli core
			virtual void readCustomArguments(KConfigGroup* config);
			virtual void writeCustomArguments(KConfigGroup* config) const;

			//interface to subclasses (i.e. plugins)
			void addWidget(QWidget* widget, const QString& caption);
			void removeWidget(QWidget* widget);
			void setSizeDefinitionMode(SizeDefinitionMode mode);
			int xCount() const;
			int yCount() const;
		private:
			PatternConfigurationPrivate* p;
	};

}

#endif // PALAPELI_PATTERN_CONFIGURATION_H
