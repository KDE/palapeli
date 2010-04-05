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

#include <QGraphicsItem>
#include <QMouseEvent>
#include <QWheelEvent>

Palapeli::InteractorMouseEvent::InteractorMouseEvent(QGraphicsView* view, const QPoint& pos_, const QPoint& lastPos_)
	: pos(pos_)
	, lastPos(lastPos_)
	, scenePos(view->mapToScene(pos_))
	, lastScenePos(view->mapToScene(lastPos_))
{
}

Palapeli::InteractorWheelEvent::InteractorWheelEvent(QGraphicsView* view, const QPoint& pos_, int delta_)
	: delta(delta_)
	, pos(pos_)
	, scenePos(view->mapToScene(pos_))
{
}

Palapeli::Interactor::Interactor(Palapeli::InteractorType type, QGraphicsView* view)
	: m_type(type)
	, m_view(view)
	, m_scene(view->scene())
{
}

//BEGIN getters/setters

Palapeli::InteractorType Palapeli::Interactor::type() const
{
	return m_type;
}

QGraphicsView* Palapeli::Interactor::view() const
{
	return m_view;
}

QGraphicsScene* Palapeli::Interactor::scene() const
{
	return m_scene;
}

void Palapeli::Interactor::setScene(QGraphicsScene* scene)
{
	if (m_scene == scene)
		return;
	QGraphicsScene* oldScene = m_scene;
	m_scene = scene;
	sceneChangeEvent(oldScene, scene);
}

Qt::KeyboardModifiers Palapeli::Interactor::triggerModifiers() const
{
	return m_triggerModifiers;
}

void Palapeli::Interactor::setTriggerModifiers(Qt::KeyboardModifiers modifiers)
{
	m_triggerModifiers = modifiers;
}

Qt::MouseButton Palapeli::Interactor::triggerButton() const
{
	return m_triggerButton;
}

void Palapeli::Interactor::setTriggerButton(Qt::MouseButton button)
{
	m_triggerButton = button;
}

Qt::Orientations Palapeli::Interactor::triggerOrientations() const
{
	return m_triggerOrientations;
}

void Palapeli::Interactor::setTriggerOrientations(Qt::Orientations orientations)
{
	m_triggerOrientations = orientations;
}

//END getters/setters

bool Palapeli::Interactor::handleMouseEvent(QMouseEvent* event)
{
	if (m_type != Palapeli::MouseInteractor)
		return false;
	//find and save mouse position
	const QPoint lastPos = m_lastPos;
	const QPoint pos = event->pos();
	m_lastPos = pos;
	//check trigger
	const bool checkModifiers = event->modifiers() == m_triggerModifiers;
	const bool checkButton = (event->type() == QEvent::MouseMove) ? (event->buttons() & m_triggerButton) : (event->button() == m_triggerButton);
	if (!(checkModifiers && checkButton))
		return false;
	//check custom conditions
	if (!acceptMousePosition(pos))
		return false;
	QList<QGraphicsItem*> itemsUnderMouse = m_view->scene()->items(m_view->mapToScene(pos), Qt::IntersectsItemShape, Qt::DescendingOrder, m_view->viewportTransform());
	QGraphicsItem* itemUnderMouse = 0;
	foreach (QGraphicsItem* someItemUnderMouse, itemsUnderMouse)
	{
		//We only want those items that accept the same mouse buttons as we do. (TODO: Is that consistent with Qt?)
		if (someItemUnderMouse->acceptedMouseButtons() & m_triggerButton)
		{
			itemUnderMouse = someItemUnderMouse;
			break;
		}
	}
	if (!acceptItemUnderMouse(itemUnderMouse))
		return false;
	//handle event
	Palapeli::InteractorMouseEvent imEvent(m_view, pos, lastPos);
	switch (event->type())
	{
		case QEvent::MouseMove:
			mouseMoveEvent(imEvent);
			return true;
		case QEvent::MouseButtonPress:
			mousePressEvent(imEvent);
			return true;
		case QEvent::MouseButtonRelease:
			mouseReleaseEvent(imEvent);
			return true;
		default:
			return false;
	}
}

bool Palapeli::Interactor::handleWheelEvent(QWheelEvent* event)
{
	if (m_type != Palapeli::WheelInteractor)
		return false;
	const QPoint pos = event->pos();
	//check trigger
	const bool checkModifiers = event->modifiers() == m_triggerModifiers;
	const bool checkOrientation = (event->orientation() & m_triggerOrientations);
	if (!(checkModifiers && checkOrientation))
		return false;
	//check custom conditions
	if (!acceptMousePosition(pos))
		return false;
	QList<QGraphicsItem*> itemsUnderMouse = m_view->scene()->items(m_view->mapToScene(pos), Qt::IntersectsItemShape, Qt::DescendingOrder, m_view->viewportTransform());
	if (!acceptItemUnderMouse(itemsUnderMouse.value(0, 0)))
		return false;
	//handle event
	Palapeli::InteractorWheelEvent iwEvent(m_view, pos, event->delta());
	wheelEvent(iwEvent);
	return true;
}

//BEGIN empty base class implementations

bool Palapeli::Interactor::acceptItemUnderMouse(QGraphicsItem* item)
{
	Q_UNUSED(item)
	return true;
}

bool Palapeli::Interactor::acceptMousePosition(const QPoint& pos)
{
	Q_UNUSED(pos)
	return true;
}

void Palapeli::Interactor::mousePressEvent(const Palapeli::InteractorMouseEvent& event)
{
	Q_UNUSED(event)
}

void Palapeli::Interactor::mouseMoveEvent(const Palapeli::InteractorMouseEvent& event)
{
	Q_UNUSED(event)
}

void Palapeli::Interactor::mouseReleaseEvent(const Palapeli::InteractorMouseEvent& event)
{
	Q_UNUSED(event)
}

void Palapeli::Interactor::wheelEvent(const Palapeli::InteractorWheelEvent& event)
{
	Q_UNUSED(event)
}

void Palapeli::Interactor::sceneChangeEvent(QGraphicsScene* oldScene, QGraphicsScene* newScene)
{
	Q_UNUSED(oldScene)
	Q_UNUSED(newScene)
}

//END empty base class implementations
