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

#include "constraint.h"

Palapeli::Constraint::Constraint()
	: m_type(RestrictToInside)
	, m_rect(-100000.0, -100000.0, 200000.0, 200000.0)
{
}

Palapeli::Constraint::Constraint(Palapeli::Constraint::Type type, const QRectF& rect)
	: m_type(type)
	, m_rect(rect.normalized())
{
}

Palapeli::Constraint::Type Palapeli::Constraint::type() const
{
	return m_type;
}

QRectF Palapeli::Constraint::rect() const
{
	return m_rect;
}

void Palapeli::Constraint::setRect(const QRectF& rect)
{
	m_rect = rect;
}

bool Palapeli::Constraint::operator==(const Palapeli::Constraint& other) const
{
	return m_type == other.m_type && m_rect == other.m_rect;
}

bool Palapeli::Constraint::operator!=(const Palapeli::Constraint& other) const
{
	return m_type != other.m_type || m_rect != other.m_rect;
}

bool Palapeli::Constraint::allows(const QRectF& rect) const
{
	switch (m_type)
	{
		case RestrictToInside:
			return m_rect.contains(rect);
		case RestrictToOutside:
			return !m_rect.intersects(rect);
		default: //or GCC will drop a warning that "control reaches end of non-void function"
			return false;
	}
}

QRectF Palapeli::Constraint::apply(const QRectF& rect) const
{
	QRectF newRect(rect);
	switch (m_type)
	{
		case RestrictToInside:
			//X axis
			if (newRect.width() < m_rect.width()) //constraint rect too small -> center on constraint rect
				newRect.moveCenter(QPointF(m_rect.center().x(), newRect.center().y()));
			else if (newRect.left() < m_rect.left())
				newRect.moveLeft(m_rect.left());
			else if (newRect.right() > m_rect.right())
				newRect.moveRight(m_rect.right());
			//Y axis
			if (newRect.height() < m_rect.height())
				newRect.moveCenter(QPointF(newRect.center().x(), m_rect.center().y()));
			else if (newRect.top() < m_rect.top())
				newRect.moveTop(m_rect.top());
			else if (newRect.bottom() > m_rect.bottom())
				newRect.moveBottom(m_rect.bottom());
			break;
		case RestrictToOutside:
			//X axis
			if (newRect.right() > m_rect.left() && newRect.left() < m_rect.left())
				newRect.moveRight(m_rect.left());
			else if (newRect.left() < m_rect.right() && newRect.right() > m_rect.right())
				newRect.moveLeft(m_rect.right());
			//Y axis
			if (newRect.bottom() > m_rect.top() && newRect.top() < m_rect.top())
				newRect.moveBottom(m_rect.top());
			else if (newRect.top() < m_rect.bottom() && newRect.bottom() > m_rect.bottom())
				newRect.moveTop(m_rect.bottom());
			break;
	}
	return newRect;
}

bool Palapeli::Constraint::obsoletes(const Palapeli::Constraint& constraint) const
{
	if (m_type == constraint.m_type)
	{
		switch (m_type)
		{
			case RestrictToInside:
				return m_rect.contains(constraint.m_rect);
			case RestrictToOutside:
				return constraint.m_rect.contains(m_rect);
		}
	}
	return false;
}

bool Palapeli::Constraint::conflictsWith(const Palapeli::Constraint& constraint) const
{
	//Note that RestrictToOutside constraints may conflict with RestrictToInside constraints, but not the other way around. This priorization ensures that only one of two constraints is killed by Palapeli::Constraints::preparePreCache() in a conflict case.
	return m_type == RestrictToOutside && constraint.m_type == RestrictToInside && m_rect.contains(constraint.m_rect);
}
