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

#include "pattern-plugin.h"
#include "pattern-configuration.h"

namespace Palapeli
{

	class PatternPluginPrivate
	{
		public:
			PatternPluginPrivate(const QVariantList& args);

			QString m_pluginName, m_displayName, m_iconName;
	};
	
}

//BEGIN Palapeli::PatternPluginPrivate

Palapeli::PatternPluginPrivate::PatternPluginPrivate(const QVariantList& args)
	: m_pluginName(args.value(0, QVariant()).toString())
	, m_displayName(args.value(1, QVariant()).toString())
	, m_iconName(args.value(2, QVariant()).toString())
{
}

//END Palapeli::PatternPluginPrivate

//BEGIN Palapeli::PatternPlugin

Palapeli::PatternPlugin::PatternPlugin(QObject* parent, const QVariantList& args)
	: p(new Palapeli::PatternPluginPrivate(args))
{
	Q_UNUSED(parent)
}

Palapeli::PatternPlugin::~PatternPlugin()
{
	delete p;
}

QString Palapeli::PatternPlugin::pluginName() const
{
	return p->m_pluginName;
}

QString Palapeli::PatternPlugin::displayName() const
{
	return p->m_displayName;
}

QString Palapeli::PatternPlugin::iconName() const
{
	return p->m_iconName;
}

//END Palapeli::PatternPlugin

#include "pattern-plugin.moc"
