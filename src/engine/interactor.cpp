/*
    SPDX-FileCopyrightText: 2010 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "interactor.h"

#include <QGraphicsView>

//BEGIN events

Palapeli::MouseEvent::MouseEvent(QGraphicsView* view, const QPoint& pos_)
	: pos(pos_)
	, scenePos(view->mapToScene(pos_))
{
}

Palapeli::MouseEvent::MouseEvent()
{
}

Palapeli::WheelEvent::WheelEvent(QGraphicsView* view, const QPoint& pos_, int delta_)
	: pos(pos_)
	, scenePos(view->mapToScene(pos_))
	, delta(delta_)
{
}

//END events

Palapeli::Interactor::Interactor(int priority, Palapeli::InteractorType type, QGraphicsView* view)
	: m_type(type)
	, m_view(view)
	, m_scene(view ? view->scene() : nullptr)
	, m_active(false)
	, m_category(NoCategory)
	, m_priority(priority)
{
}

Palapeli::Interactor::~Interactor()
{
}

Palapeli::InteractorType Palapeli::Interactor::interactorType() const
{
	return m_type;
}

int Palapeli::Interactor::priority() const
{
	return m_priority;
}

Palapeli::Interactor::Category Palapeli::Interactor::category() const
{
	return m_category;
}

QString Palapeli::Interactor::description() const
{
	return m_description;
}

QIcon Palapeli::Interactor::icon() const
{
	return m_icon;
}

void Palapeli::Interactor::setMetadata(Palapeli::Interactor::Category category, const QString& description, const QIcon& icon)
{
	m_category = category;
	m_description = description;
	m_icon = icon;
}

void Palapeli::Interactor::updateScene()
{
	QGraphicsScene* oldScene = m_scene;
	QGraphicsScene* newScene = m_view ? m_view->scene() : nullptr;
	if (oldScene != newScene)
	{
		setInactive();
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

bool Palapeli::Interactor::isActive() const
{
	return m_active;
}

void Palapeli::Interactor::setInactive()
{
	//resend last event as release event
	if (m_active)
	{
		stopInteraction(m_lastMouseEvent);
		m_active = false;
	}
}

void Palapeli::Interactor::sendEvent(const Palapeli::MouseEvent& event, Palapeli::EventProcessingFlags flags)
{
	//conclude interaction
	if (flags & Palapeli::EventConcludesInteraction || !(flags & Palapeli::EventMatches))
	{
		setInactive();
		return;
	}
	//check if caller attempts to start new interaction chain while an old one is still in progress
	if (flags & Palapeli::EventStartsInteraction)
		setInactive();
	//handle event, thereby starting a new interaction if necessary
	if (m_active)
		continueInteraction(event);
	else
		m_active = startInteraction(event);
}

void Palapeli::Interactor::sendEvent(const Palapeli::WheelEvent& event)
{
	doInteraction(event);
}
