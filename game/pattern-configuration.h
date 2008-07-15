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

#include <QObject>
class QWidget;
class KConfigGroup;

namespace Palapeli
{

	class Pattern;
	class PatternConfigurationPrivate;

	class PatternConfiguration : public QObject
	{
		//TODO: documentation (I will include that once this class move into installable headers)
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
			virtual void readArguments(KConfigGroup* config) = 0;
			virtual void writeArguments(KConfigGroup* config) const = 0;
			virtual Pattern* createPattern() const = 0;

			//interface to Palapeli core
			QByteArray patternName() const;
			QString displayName() const;
			QWidget* createConfigurationWidget() const; //DOC: talk about ownership issues
		protected:
			//functionality of base class
			void readArgumentsBase(KConfigGroup* config);
			void writeArgumentsBase(KConfigGroup* config) const;

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
