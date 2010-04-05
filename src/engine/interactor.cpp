/***************************************************************************
 *   Copyright 2010 Stefan Majewsky <majewsky@gmx.net>
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

#include "interactor.h"

Palapeli::Interactor::Interactor(Palapeli::InteractorTypes types, QGraphicsView* view)
	: m_types(types)
	, m_view(view)
	, m_scene(view->scene())
{
}

Palapeli::Interactor::~Interactor()
{
}

bool Palapeli::Interactor::isMouseInteractor() const
{
	return m_types && Palapeli::MouseInteractor;
}

bool Palapeli::Interactor::isWheelInteractor() const
{
	return m_types && Palapeli::WheelInteractor;
}

Palapeli::InteractorTypes Palapeli::Interactor::interactorTypes() const
{
	return m_types;
}

QString Palapeli::Interactor::description() const
{
	return m_description;
}

QIcon Palapeli::Interactor::icon() const
{
	return m_icon;
}

void Palapeli::Interactor::setMetadata(const QString& description, const QIcon& icon)
{
	m_description = description;
	m_icon = icon;
}

void Palapeli::Interactor::updateScene() //NOTE: Read the scene of the view, and when it has changed, fire a sceneChangeEvent.
{
	QGraphicsScene* oldScene = m_scene;
	QGraphicsScene* newScene = m_view->scene();
	if (oldScene != newScene)
	{
		m_scene = newScene;
		sceneChangeEvent(oldScene, newScene);
	}
}

QGraphicsView* Palapeli::Interactor::view() const
{
	return m_view;
}

QGraphicsScene* Palapeli::Interactor::scene() const
{
	return m_scene;
}

bool Palapeli::Interactor::handleEvent(const Palapeli::MouseEvent& event, QEvent::Type type)
{
	if (!acceptMousePosition(event.pos))
		return false;
	switch (type)
	{
		case QEvent::MouseMove:
			mouseMoveEvent(event);
			return true;
		case QEvent::MouseButtonPress:
		case QEvent::KeyPress:
			mousePressEvent(event);
			return true;
		case QEvent::MouseButtonRelease:
		case QEvent::KeyRelease:
			mouseReleaseEvent(event);
			return true;
		default:
			return false;
	}
}

bool Palapeli::Interactor::handleEvent(const Palapeli::WheelEvent& event)
{
	if (!acceptMousePosition(event.pos))
		return false;
	wheelEvent(event);
	return true;
}
