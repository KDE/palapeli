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

#include "autosaver.h"
#include "manager.h"
#include "settings.h"

#include <QTimer>
#include <KLocalizedString>

namespace Palapeli
{

	class AutosaverPrivate
	{
		public:
			AutosaverPrivate();

			QTimer m_timer;
			int m_moveCounter, m_moveCount;
			bool m_enabled;
	};

}

Palapeli::AutosaverPrivate::AutosaverPrivate()
	: m_moveCounter(0)
	, m_moveCount(0)
	, m_enabled(false)
{
	QObject::connect(&m_timer, SIGNAL(timeout()), ppMgr(), SLOT(autosaveGame()));
}

Palapeli::Autosaver::Autosaver()
	: p(new Palapeli::AutosaverPrivate)
{
	//restore settings
	setTimeInterval(Settings::autosaveTime(), false);
	setMoveInterval(Settings::autosaveMoves(), false);
}

Palapeli::Autosaver::~Autosaver()
{
	delete p;
}

void Palapeli::Autosaver::setTimeInterval(int minutes, bool saveChange)
{
	p->m_timer.setInterval(minutes * 60000); //interval is given in milliseconds
	if (minutes == 0)
		p->m_timer.stop(); //disable timer permanently
	if (saveChange)
	{
		Settings::setAutosaveTime(minutes);
		Settings::self()->writeConfig();
	}
}

void Palapeli::Autosaver::setMoveInterval(int moveCount, bool saveChange)
{
	p->m_moveCount = moveCount;
	if (saveChange)
	{
		Settings::setAutosaveMoves(moveCount);
		Settings::self()->writeConfig();
	}
}

void Palapeli::Autosaver::countMove()
{
	if (!p->m_enabled || p->m_moveCount == 0) //== 0 means: move-based autosave is disabled
		return;
	if (++p->m_moveCounter >= p->m_moveCount)
	{
		p->m_moveCounter = 0;
		ppMgr()->autosaveGame();
	}
}

void Palapeli::Autosaver::reset()
{
	//reset move counter
	p->m_moveCounter = 0;
	//restart timer
	p->m_timer.stop();
	if (p->m_enabled && p->m_timer.interval() != 0)
		p->m_timer.start();
}

void Palapeli::Autosaver::setEnabled(bool enabled)
{
	p->m_enabled = enabled;
	if (enabled)
	{
		if (p->m_timer.interval() != 0) //== 0 means: time-based autosave is disabled
			p->m_timer.start();
	}
	else
		p->m_timer.stop();
}

#include "autosaver.moc"
