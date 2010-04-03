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

#include "interactormanager.h"
#include "interactors.h"

#include <QGraphicsView>
#include <KConfigGroup>
#include <KGlobal>
#include <KSharedConfig>

Palapeli::InteractorManager::InteractorManager(QGraphicsView* view)
	: QObject(view)
{
	//create interactors, specify default configuration
	m_interactors["MoveViewport"] = new Palapeli::MoveViewportInteractor(view);
	m_defaultConfiguration["MoveViewport"] = QList<int>() << Qt::NoModifier << Qt::RightButton;
	m_interactors["ZoomViewport"] = new Palapeli::ZoomViewportInteractor(view);
	m_defaultConfiguration["ZoomViewport"] = QList<int>() << Qt::NoModifier << Qt::Vertical;
	m_interactors["RubberBand"] = new Palapeli::RubberBandInteractor(view);
	m_defaultConfiguration["RubberBand"] = QList<int>() << Qt::NoModifier << Qt::LeftButton;
	//load configuration
	KConfigGroup group(KGlobal::config(), "Puzzle Table Interaction");
	QMap<QByteArray, Palapeli::Interactor*>::const_iterator it1 = m_interactors.begin(), it2 = m_interactors.end();
	for (; it1 != it2; ++it1)
	{
		const QByteArray interactorKey = it1.key();
		Palapeli::Interactor* interactor = it1.value();
		//read configuration
		const QList<int>& defaultConfiguration = m_defaultConfiguration[interactorKey];
		QList<int> configuration = group.readEntry(interactorKey.data(), defaultConfiguration);
		if (configuration.size() != 2)
			configuration = defaultConfiguration;
		//apply configuration - format: first integer is triggerModifiers, second integer is triggerButton (for mouse interactors) or triggerOrientation (for wheel interactors)
		interactor->setTriggerModifiers((Qt::KeyboardModifiers) configuration[0]);
		switch (interactor->type())
		{
			case Palapeli::MouseInteractor:
				interactor->setTriggerButton((Qt::MouseButton) configuration[1]);
				break;
			case Palapeli::WheelInteractor:
				interactor->setTriggerOrientations((Qt::Orientations) configuration[1]);
				break;
		}
	}
}

bool Palapeli::InteractorManager::handleMouseEvent(QMouseEvent* event)
{
	QMap<QByteArray, Palapeli::Interactor*>::const_iterator it1 = m_interactors.begin(), it2 = m_interactors.end();
	for (; it1 != it2; ++it1)
		if (it1.value()->handleMouseEvent(event))
			return true;
	return false;
}

bool Palapeli::InteractorManager::handleWheelEvent(QWheelEvent* event)
{
	QMap<QByteArray, Palapeli::Interactor*>::const_iterator it1 = m_interactors.begin(), it2 = m_interactors.end();
	for (; it1 != it2; ++it1)
		if (it1.value()->handleWheelEvent(event))
			return true;
	return false;
}
