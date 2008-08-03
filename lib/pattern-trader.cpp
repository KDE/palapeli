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

#include "pattern-trader.h"
#include "pattern-configuration.h"
#include "pattern-plugin.h"

#include <kdemacros.h>
#include <KServiceTypeTrader>

namespace Palapeli
{

	class PatternTraderPrivate
	{
		public:
			QList<PatternConfiguration*> m_configs;
	};

}

Palapeli::PatternTrader::PatternTrader()
	: p(new Palapeli::PatternTraderPrivate)
{
	rescanConfigurations();
}

Palapeli::PatternTrader::~PatternTrader()
{
	delete p;
}

Palapeli::PatternTrader* Palapeli::PatternTrader::self()
{
	static Palapeli::PatternTrader* trader = new Palapeli::PatternTrader;
	return trader;
}

int Palapeli::PatternTrader::configurationCount() const
{
	return p->m_configs.count();
}

Palapeli::PatternConfiguration* Palapeli::PatternTrader::configurationAt(int index) const
{
	if (index < 0 || index >= p->m_configs.count())
		return 0;
	return p->m_configs[index];
}

void Palapeli::PatternTrader::rescanConfigurations()
{
	//flush configuration list
	while (p->m_configs.count() != 0)
		delete p->m_configs.takeFirst();
	//fill configuration list
	KService::List offers = KServiceTypeTrader::self()->query("Palapeli/PatternPlugin");
	foreach (KService::Ptr offer, offers)
	{
		QVariantList args;
		args << offer->property("PluginIdentifier", QVariant::String).toString();
		args << offer->name();
		Palapeli::PatternPlugin* plugin = offer->createInstance<Palapeli::PatternPlugin>(0, args);
		if (plugin == 0)
			continue;
		p->m_configs << plugin->createInstances();
		delete plugin;
	}
}
