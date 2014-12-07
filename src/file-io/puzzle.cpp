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
#include <QAtomicPointer>
#include <QtCore/QFileInfo>
#include <QtCore/QFutureSynchronizer>
#include <QtCore/QHash>
#include <QtCore/QMutexLocker>
#include <QtCore/QWaitCondition>
#include <QtConcurrentRun>
#include <KGlobal>

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
	QMutex m_allFuturesMutex; //controls access to m_allFutures
	QFutureSynchronizer<void> m_allFutures;
	QMutex m_hashMutex; //controls access to m_components
	QHash<Palapeli::PuzzleComponent::Type, Component*> m_components;
	QAtomicPointer<Palapeli::PuzzleComponent> m_mainComponent;
	QMutex m_locationMutex; //controls access to m_location
	QString m_location;
	QString m_identifier;

	Private(Palapeli::Puzzle* q, Palapeli::PuzzleComponent* mainComponent, const QString& location, const QString& identifier);
	const Palapeli::PuzzleComponent* get(Palapeli::PuzzleComponent::Type type);
};

Palapeli::Puzzle::Puzzle(Palapeli::PuzzleComponent* mainComponent, const QString& location, const QString& identifier)
	: d(new Private(this, mainComponent, location, identifier))
{
	qRegisterMetaType<Palapeli::Puzzle*>();
}

Palapeli::Puzzle::Private::Private(Palapeli::Puzzle* q, Palapeli::PuzzleComponent* mainComponent, const QString& location, const QString& identifier)
	: q(q)
	, m_mainComponent(mainComponent)
	, m_location(location)
	, m_identifier(identifier)
{
	//QT5 m_mainComponent.m_puzzle = q;
	m_components.insert(mainComponent->type(), new Component(mainComponent));
	//mutexes not necessary here because concurrent access is impossible in the ctor
}

Palapeli::Puzzle::~Puzzle()
{
	d->m_allFutures.waitForFinished();
	d->m_hashMutex.lock();
	QHashIterator<Palapeli::PuzzleComponent::Type, Component*> iter(d->m_components);
	while (iter.hasNext())
		delete iter.next().value();
	d->m_hashMutex.unlock();
	delete d;
}

const Palapeli::PuzzleComponent* Palapeli::Puzzle::component(Palapeli::PuzzleComponent::Type type) const
{
	const Component* c = d->m_components.value(type);
	return c ? c->component : 0;
}

QFuture<const Palapeli::PuzzleComponent*> Palapeli::Puzzle::get(Palapeli::PuzzleComponent::Type type)
{
	QFuture<const Palapeli::PuzzleComponent*> future;
	future = QtConcurrent::run(d, &Palapeli::Puzzle::Private::get, type);
	//m_allFutures is used to wait for all running casts to be finished in dtor
	d->m_allFuturesMutex.lock();
	d->m_allFutures.addFuture(future);
	d->m_allFuturesMutex.unlock();
	return future;
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
#if 0 //QT5
		Palapeli::PuzzleComponent* cmp = m_mainComponent->cast(type);
		if (cmp)
			cmp->m_puzzle = q;
		//write access to c need not be mutex-secured because there
		//is only one write access ever per component
		c->component.fetchAndStoreOrdered(cmp);
		c->available.fetchAndStoreOrdered(true);
		//notify other waiting threads that the component is available
		c->wait.wakeAll();
		return cmp;
#endif
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

QString Palapeli::Puzzle::identifier() const
{
	return d->m_identifier;
}

QString Palapeli::Puzzle::location() const
{
	QMutexLocker l(&d->m_locationMutex);
	return d->m_location;
}

void Palapeli::Puzzle::setLocation(const QString& location)
{
	QMutexLocker l(&d->m_locationMutex);
	d->m_location = location;
}

void Palapeli::Puzzle::setMainComponent(Palapeli::PuzzleComponent* component)
{
	if (!component)
		return;
	//add component
	QMutexLocker locker(&d->m_hashMutex);
	Component*& c = d->m_components[component->type()];
	delete c;
	c = new Component(component);
	d->m_mainComponent.fetchAndStoreOrdered(component);
}

void Palapeli::Puzzle::dropComponent(Palapeli::PuzzleComponent::Type type)
{
	//DO NEVER EVER USE THIS FUNCTION! THIS FUNCTION IS PURELY DANGEROUS. STUFF WILL BREAK.
	QMutexLocker locker(&d->m_hashMutex);
	Component*& c = d->m_components[type];
	delete c;
	c = 0;
}

K_GLOBAL_STATIC(QList<QString>, g_usedIdentifiers)

/*static*/ QString Palapeli::Puzzle::fsIdentifier(const QString& location)
{
	QString puzzleName = QFileInfo(location).fileName();
	const char* disallowedChars = "\\:*?\"<>|"; //Windows forbids using these chars in filenames, so we'll strip them
	for (const char* c = disallowedChars; *c; ++c)
		puzzleName.remove(*c);
	const QString identifierPattern = QString::fromLatin1("__FSC_%1_%2_").arg(puzzleName);
	int uniquifier = 0;
	while (g_usedIdentifiers->contains(identifierPattern.arg(uniquifier)))
		++uniquifier;
	const QString identifier = identifierPattern.arg(uniquifier);
	*g_usedIdentifiers << identifier;
	return identifier;
}

//END Palapeli::Puzzle

