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

#if defined(MAKE_LIBPALAPELIPATTERN)
 #include "macros.h"
#else
 #include <Palapeli/Macros>
#endif

#include <QtGlobal>

namespace Palapeli
{

	class PatternConfiguration;
	class PatternTraderPrivate;

	class PALAPELIPATTERN_EXPORT PatternTrader
	{
		public:
			static PatternTrader* self();

			int configurationCount() const;
			PatternConfiguration* configurationAt(int index) const;
		private:
			PatternTrader();
			~PatternTrader();
			Q_DISABLE_COPY(PatternTrader)

			PatternTraderPrivate* p;
	};

}

#endif // PALAPELI_PATTERN_TRADER_H
