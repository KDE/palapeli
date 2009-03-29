/***************************************************************************
 *   Copyright 2008 Stefan Majewsky <majewsky@gmx.net>
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

#include "variantmapper.h"

void Palapeli::VariantMapper::addMapping(QObject* sender, const QByteArray& key)
{
	m_keyMappings[sender] = key;
}

void Palapeli::VariantMapper::removeMappings(QObject* sender)
{
	m_keyMappings.take(sender);
}

void Palapeli::VariantMapper::mapInternal(QObject* sender, const QVariant& value)
{
	emit mapped(m_keyMappings.value(sender, QByteArray()), value);
}

#include "variantmapper.moc"
