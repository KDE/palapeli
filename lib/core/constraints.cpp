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

#include "constraints.h"

int Palapeli::Constraints::count() const
{
	return m_constraints.count();
}

Palapeli::Constraint Palapeli::Constraints::constraint(int index) const
{
	return m_constraints.value(index);
}

Palapeli::Constraints& Palapeli::Constraints::operator<<(const Palapeli::Constraint& constraint)
{
	add(constraint);
	return *this;
}

void Palapeli::Constraints::add(const Palapeli::Constraint& constraint)
{
	if (!m_constraints.contains(constraint))
		m_constraints << constraint;
	preparePreCache();
}

void Palapeli::Constraints::remove(const Palapeli::Constraint& constraint)
{
	m_constraints.removeAll(constraint);
	preparePreCache();
}

bool Palapeli::Constraints::allows(const QRectF& rect) const
{
	foreach (const Palapeli::Constraint& constraint, m_preCache)
		if (!constraint.allows(rect))
			return false;
	return true;
}

QRectF Palapeli::Constraints::apply(const QRectF& rect) const
{
	QList<Palapeli::Constraint> constraints = constraintsFromCache(rect.size());
	QRectF newRect(rect);
	foreach (const Palapeli::Constraint& constraint, m_constraints)
		newRect = constraint.apply(newRect);
	return newRect;
}

void Palapeli::Constraints::preparePreCache() const
{
	m_preCache.clear();
	m_cache.clear(); //pre-cache is invalidated -> also flush cache
	int count = m_constraints.count();
	for (int i = 0; i < count; ++i)
	{
		const Palapeli::Constraint& constraint = m_constraints[i];
		bool addToPreCache = true; //no conflict or such found yet
		for (int j = 0; j < count; ++j)
		{
			if (j == i)
				continue;
			else if (constraint.conflictsWith(m_constraints[j]) || m_constraints[j].obsoletes(constraint))
			{
				addToPreCache = false;
				break;
			}
		}
		if (addToPreCache)
			m_preCache << constraint;
	}
}

QList<Palapeli::Constraint> Palapeli::Constraints::constraintsFromCache(const QSizeF& size) const
{
	//problem: if RestrictToOutside constraints are inside RestrictToInside constraints, it might be that the rect of this area has to be enlarged. The following graphics illustrates the problem:
	/*
	#########################
	#                       #     description:
	#                       #        # is a RestrictToInside constraint
	#  +----+    PPP        #        - is the RestrictToOutside constraint
	#  |    |    PPP        #        P is a sample piece rect
	#  |    |               #
	#  |    |               #
	#  +----+               #
	#                       #
	#########################
	*/
	//In this example, the piece does not fit below the inner RestrictToOutside constraint, even if the constraint would allow it, because the RestrictToInside constraint conflicts with this placement. The only solution I see is to enlarge the inner constraint to fill the area below it.
	if (m_cache.contains(size))
		return m_cache[size];
	//create cache entry
	QList<Palapeli::Constraint> cacheEntry(m_preCache);
	//look for conflicting RestrictToOutside constraints
	foreach (const Palapeli::Constraint& outerConstraint, cacheEntry)
	{
		if (outerConstraint.type() != Palapeli::Constraint::RestrictToInside)
			continue;
		const QRectF outerRect = outerConstraint.rect();
		for (int i = 0; i < cacheEntry.count(); ++i)
		{
			Palapeli::Constraint& innerConstraint = cacheEntry[i];
			if (innerConstraint.type() != Palapeli::Constraint::RestrictToOutside)
				continue;
			QRectF innerRect = innerConstraint.rect();
			//enlarge to left
			if (innerRect.left() > outerRect.left() && innerRect.left() < outerRect.left() + size.width())
				innerRect.setLeft(outerRect.left());
			//and similar for the other directions
			if (innerRect.right() < outerRect.right() && innerRect.right() > outerRect.right() - size.width())
				innerRect.setRight(outerRect.right());
			if (innerRect.top() > outerRect.top() && innerRect.top() < outerRect.top() + size.height())
				innerRect.setTop(outerRect.top());
			if (innerRect.bottom() < outerRect.bottom() && innerRect.bottom() > outerRect.bottom() - size.height())
				innerRect.setBottom(outerRect.bottom());
			innerConstraint.setRect(innerRect);
		}
	}
	m_cache[size] = cacheEntry;
	return cacheEntry;
}
