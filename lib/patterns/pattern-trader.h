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

#ifndef PALAPELI_PATTERN_TRADER_H
#define PALAPELI_PATTERN_TRADER_H

#include "../macros.h"

#include <QtCore/QtGlobal>

namespace Palapeli
{

	class PatternConfiguration;
	class PatternTraderPrivate;

	/**
	 * \class PatternTrader pattern-trader.h Palapeli/PatternTrader
	 *
	 * The pattern trader is a thin wrapper class around the KServiceTypeTrader that is used to locate the pattern plugins.
	 *
	 * The following code can be used to fetch a list of all available Palapeli::PatternConfiguration objects from the pattern trader.
\code
QList<Palapeli::PatternConfiguration*> configs;
Palapeli::PatternTrader* trader = Palapeli::PatternTrader::self();

for (int i = 0; i < trader->configurationCount(); ++i)
	configs << trader->configurationAt(i);
\endcode
	 *
	 * \author Stefan Majewsky <majewsky@gmx.net>
	 */
	class PatternTrader
	{
		public:
			/**
			 * Returns a pointer to the global pattern trader object.
			 */
			static PatternTrader* self();
			/**
			 * Returns how many Palapeli::PatternConfiguration instances are available.
			 * \sa configurationAt()
			 */
			int configurationCount() const;
			/**
			 * Returns the index-th Palapeli::PatternConfiguration instance.
			 * \warning The trader will delete this instance automatically when rebuilding its configuration list or on exit.
			 * \sa configurationCount(), configurationFromName()
			 */
			PatternConfiguration* configurationAt(int index) const;
			/**
			 * Searches for a Palapeli::PatternConfiguration instance with the given \a patternName.
			 * \return a pointer to the Palapeli::PatternConfiguration instance, or 0 if no such instance exists
			 * \warning The trader will delete this instance automatically when rebuilding its configuration list or on exit.
			 * \sa configurationAt()
			 */
			PatternConfiguration* configurationFromName(const QString& patternName) const;
			/**
			 * Flushes and rebuilds the list of available Palapeli::PalapeliConfiguration instances.
			 * \warning This method invalidates Palapeli::PatternConfiguration instances previously fetched from the trader.
			 */
			void rescanConfigurations();
		private:
			PatternTrader();
			~PatternTrader();
			Q_DISABLE_COPY(PatternTrader)

			PatternTraderPrivate* const p;
	};

}

#endif // PALAPELI_PATTERN_TRADER_H
