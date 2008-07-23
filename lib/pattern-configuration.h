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

#if defined(MAKE_LIBPALAPELIPATTERN)
 #include "macros.h"
#else
 #include <Palapeli/Macros>
#endif

#include <QObject>
#include <QVariant>
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
	 * This class is not documented yet because its design is not yet finalized.
	 *
	 * \author Stefan Majewsky <majewsky@gmx.net>
	 */
	class PALAPELIPATTERN_EXPORT PatternConfiguration : public QObject
	{
		Q_OBJECT
		public:
			enum DataType
			{
				Variant = 0,
				String,
				Integer
			};

			PatternConfiguration(QObject* parent = 0, const QVariantList& args = QVariantList());
			virtual ~PatternConfiguration();
			//interface to Palapeli core
			virtual Pattern* createPattern() const = 0;
			QVariant property(const QByteArray& key) const;
			void readArguments(KConfigGroup* config);
			void writeArguments(KConfigGroup* config) const;
			void populateWidget(QWidget* widget);
		public Q_SLOTS:
			void setProperty(const QByteArray& key, const QVariant& value);
		Q_SIGNALS:
			void propertyChanged(const QByteArray& key, const QVariant& value);
		protected:
			virtual void readCustomArguments(KConfigGroup* config);
			virtual void writeCustomArguments(KConfigGroup* config) const;
			//interface to subclasses
			void addProperty(const QByteArray& key, DataType type, const QString& caption);
			void addPropertyParameters(const QByteArray& key, const QVariantList& parameters);
			void removeProperty(const QByteArray& key);
		private:
			PatternConfigurationPrivate* const p;
	};

}

#endif // PALAPELI_PATTERN_CONFIGURATION_H
