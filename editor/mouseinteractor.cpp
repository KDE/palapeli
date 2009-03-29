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

#include "mouseinteractor.h"

Paladesign::MouseInteractor::MouseInteractor()
	: m_hovered(false)
	, m_selected(false)
	, m_clicked(false)
{
}

bool Paladesign::MouseInteractor::hovered() const
{
	return m_hovered;
}

void Paladesign::MouseInteractor::setHovered(bool hovered)
{
	if (m_hovered != hovered)
	{
		m_hovered = hovered;
		emit mouseStateChanged();
	}
}

bool Paladesign::MouseInteractor::selected() const
{
	return m_selected;
}

void Paladesign::MouseInteractor::setSelected(bool selected)
{
	if (m_selected != selected)
	{
		m_selected = selected;
		emit mouseStateChanged();
	}
}

bool Paladesign::MouseInteractor::clicked() const
{
	return m_clicked;
}

void Paladesign::MouseInteractor::setClicked(bool clicked)
{
	if (m_clicked != clicked)
	{
		m_clicked = clicked;
		m_startPosition = m_currentPosition;
		m_clicked ? mouseDown() : mouseUp();
		emit mouseStateChanged();
	}
}

QPointF Paladesign::MouseInteractor::mousePosition() const
{
	return m_currentPosition;
}

QPointF Paladesign::MouseInteractor::mouseStartPosition() const
{
	return m_startPosition;
}

void Paladesign::MouseInteractor::setMousePosition(const QPointF& point)
{
	if (m_currentPosition != point)
	{
		m_currentPosition = point;
		if (m_clicked)
			mouseMove();
	}
}

bool Paladesign::MouseInteractor::clickAreaContains(const QPointF& point)
{
	return hoverAreaContains(point);
}

void Paladesign::MouseInteractor::announceInteractorChanges()
{
	emit interactorChanged();
}

#include "mouseinteractor.moc"
