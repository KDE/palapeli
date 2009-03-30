/***************************************************************************
 *   Copyright 2009 Stefan Majewsky <majewsky@gmx.net>
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

#ifndef PALAPELI_CONSTRAINTS_H
#define PALAPELI_CONSTRAINTS_H

#include "constraint.h"
#include <QHash>
#include <QList>

//necessary for QHash<QSizeF, ...>
inline uint qHash(const QSizeF& sizeF)
{
	const QSize size = sizeF.toSize();
	return qHash(size.width()) ^ !qHash(size.height());
}

namespace Palapeli
{
	class Constraints
	{
		public:
			int count() const;
			Palapeli::Constraint constraint(int index) const;

			Palapeli::Constraints& operator<<(const Palapeli::Constraint& constraint);
			void add(const Palapeli::Constraint& constraint);
			void remove(const Palapeli::Constraint& constraint);

			bool allows(const QRectF& rect) const;
			QRectF apply(const QRectF& rect) const;
		protected:
			void preparePreCache() const;
			QList<Palapeli::Constraint> constraintsFromCache(const QSizeF& size) const;
		private:
			QList<Palapeli::Constraint> m_constraints;
			//The pre-cache contains essentially the same constraints as m_constraints, with the exception that those constraints are deleted that are obsoleted by other constraints in the cache.
			mutable QList<Palapeli::Constraint> m_preCache;
			//The cache contains for several sizes adjusted versions of the constraints.
			mutable QHash<QSizeF, QList<Palapeli::Constraint> > m_cache;
	};
}

#endif // PALAPELI_CONSTRAINTS_H
