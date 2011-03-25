/***************************************************************************
 *   Copyright 2011 Stefan Majewsky <majewsky@gmx.net>
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

#ifndef PALAPELI_PUZZLE_P_H
#define PALAPELI_PUZZLE_P_H

#include "puzzle.h"

#include <QtCore/QHash>
#include <QtCore/QMutexLocker>
#include <QtCore/QtConcurrentRun>

enum ComponentState
{
	Unavailable,
	Requested,
	Available
};
struct Component
{
	ComponentState state;
	Palapeli::PuzzleComponent* component;

	Component() : state(Unavailable), component(0) {}
};

struct Palapeli::Puzzle::Private
{
	//A single mutex for everything is awfully inefficient like the Big Kernel
	//Lock, but I hope to get away with that.
	QMutex m_mutex;
	QHash<Palapeli::PuzzleComponent::Type, Component> m_components;
	Palapeli::PuzzleComponent* m_mainComponent;
};

#endif // PALAPELI_PUZZLE_P_H
