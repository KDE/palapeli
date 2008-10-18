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

#ifndef PALAPELI_VARIANTMAPPER_H
#define PALAPELI_VARIANTMAPPER_H

#include <QObject>
#include <QVariant>

namespace Palapeli
{

	///\internal
	class VariantMapper : public QObject
	{
		Q_OBJECT
		public:
			void addMapping(QObject* sender, const QByteArray& key);
			void removeMappings(QObject* sender);
		public Q_SLOTS:
			void map(const QVariant& value) { mapInternal(sender(), value); }
			void map(const QString& value) { mapInternal(sender(), value); }
			void map(bool value) { mapInternal(sender(), value); }
			void map(int value) { mapInternal(sender(), value); }
			void map(double value) { mapInternal(sender(), value); }
		Q_SIGNALS:
			void mapped(const QByteArray& key, const QVariant& value);
		private:
			void mapInternal(QObject* sender, const QVariant& value);
			QMap<QObject*, QByteArray> m_keyMappings;
	};

}

#endif // PALAPELI_VARIANTMAPPER_H
