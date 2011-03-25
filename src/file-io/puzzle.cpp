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

#include "puzzle.h"

#include <QtCore/QAtomicInt>
#include <QtCore/QAtomicPointer>
#include <QtCore/QHash>
#include <QtCore/QMutexLocker>
#include <QtCore/QWaitCondition>
#include <QtCore/QtConcurrentRun>

//BEGIN Palapeli::PuzzleComponent

Palapeli::PuzzleComponent::PuzzleComponent()
	: m_puzzle(0)
{
}

Palapeli::PuzzleComponent::~PuzzleComponent()
{
}

Palapeli::PuzzleComponent* Palapeli::PuzzleComponent::cast(Palapeli::PuzzleComponent::Type type) const
{
	Q_UNUSED(type)
	return 0;
}

Palapeli::Puzzle* Palapeli::PuzzleComponent::puzzle() const
{
	return m_puzzle;
}

//END Palapeli::PuzzleComponent
//BEGIN Palapeli::Puzzle

//See Private::get() for the whole story.
struct Component
{
	QAtomicInt available;
	QAtomicPointer<Palapeli::PuzzleComponent> component;
	QWaitCondition wait;

	Component() : available(false), component(0) {}
	explicit Component(Palapeli::PuzzleComponent* component) : available(true), component(component) {}
	~Component() { wait.wakeAll(); delete component; }
};
struct Palapeli::Puzzle::Private
{
	Palapeli::Puzzle* q;
	QMutex m_hashMutex; //controls access to m_components
	QHash<Palapeli::PuzzleComponent::Type, Component*> m_components;
	Palapeli::PuzzleComponent* m_mainComponent;

	Private(Palapeli::Puzzle* q, Palapeli::PuzzleComponent* mainComponent);
	const Palapeli::PuzzleComponent* get(Palapeli::PuzzleComponent::Type type);
};

Palapeli::Puzzle::Puzzle(Palapeli::PuzzleComponent* mainComponent)
	: d(new Private(this, mainComponent))
{
	qRegisterMetaType<Palapeli::Puzzle*>();
}

Palapeli::Puzzle::Private::Private(Palapeli::Puzzle* q, Palapeli::PuzzleComponent* mainComponent)
	: q(q)
	, m_mainComponent(mainComponent)
{
	m_mainComponent->m_puzzle = q;
	m_components.insert(mainComponent->type(), new Component(mainComponent));
	//m_hashMutex not necessary here because concurrent access is impossible in the ctor
}

Palapeli::Puzzle::~Puzzle()
{
	d->m_hashMutex.lock();
	QHashIterator<Palapeli::PuzzleComponent::Type, Component*> iter(d->m_components);
	while (iter.hasNext())
		delete iter.next().value();
	d->m_hashMutex.unlock();
	delete d;
}

const Palapeli::PuzzleComponent* Palapeli::Puzzle::component(Palapeli::PuzzleComponent::Type type) const
{
	return d->m_components.value(type)->component;
}

QFuture<const Palapeli::PuzzleComponent*> Palapeli::Puzzle::get(Palapeli::PuzzleComponent::Type type)
{
	return QtConcurrent::run(d, &Palapeli::Puzzle::Private::get, type);
}

const Palapeli::PuzzleComponent* Palapeli::Puzzle::Private::get(Palapeli::PuzzleComponent::Type type)
{
	bool doRequest = false;
	Component* c = 0;
	//mutex-secured hash access to check state of component
	{
		QMutexLocker l(&m_hashMutex);
		c = m_components.value(type);
		if (!c)
		{
			//this get() is the first to request this component - create Component
			//to mark this component as "work in progress"
			m_components.insert(type, c = new Component);
			//but release mutex before creating the component
			doRequest = true;
		}
		else if (c->available)
			return c->component;
	}
	//start cast() to create component
	if (doRequest)
	{
		Palapeli::PuzzleComponent* cmp = m_mainComponent->cast(type);
		if (cmp)
			cmp->m_puzzle = q;
		//write access to c->component need not be mutex-secured because there
		//is only one write access ever per component
		c->component.fetchAndStoreOrdered(cmp);
		c->available.fetchAndStoreOrdered(true);
		//notify other waiting threads that the component is available
		c->wait.wakeAll();
		return cmp;
	}
	//component has been requested by another worker thread - wait until that
	//thread is done (but come back every 1000 ms in case the other thread
	//jumped through a concurrent loophole and triggered the waitcondition
	//without me listening)
	QMutex mutex;
	mutex.lock();
	while (!c->available)
		c->wait.wait(&mutex, 1000);
	mutex.unlock();
	return c->component;
}

//END Palapeli::Puzzle

#include "puzzle.moc"
