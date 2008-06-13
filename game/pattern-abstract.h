/***************************************************************************
 *   Copyright (C) 2008 Felix Lemke <lemke.felix@ages-skripte.org>
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

#ifndef PALAPELI_PATTERN_ABSTRACT_H
#define PALAPELI_PATTERN_ABSTRACT_H

#include <QImage>
#include <QMap>
#include <QString>
class KConfigGroup;

namespace Palapeli
{

	class Manager;
	class Piece;

	class Pattern
	{
		public:
			Pattern(KConfigGroup* arguments, Manager* manager);
			Pattern(Manager* manager);
			virtual ~Pattern();
	
			virtual QList<Piece*> slice(const QImage& image) = 0;
			virtual QString name() const = 0;
			virtual void writeArguments(KConfigGroup* target) const = 0;
		protected:
			Manager* m_manager;
	};

}

#endif // PALAPELI_PATTERN_ABSTRACT_H
