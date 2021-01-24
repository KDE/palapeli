/*
    SPDX-FileCopyrightText: 2011 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "puzzle.h"

#include <QFileInfo>
#include <QHash>

//BEGIN Palapeli::PuzzleComponent

Palapeli::PuzzleComponent::PuzzleComponent()
	: m_puzzle(nullptr)
{
}

Palapeli::PuzzleComponent::~PuzzleComponent()
{
}

Palapeli::PuzzleComponent* Palapeli::PuzzleComponent::cast(Palapeli::PuzzleComponent::Type type) const
{
	Q_UNUSED(type)
	return nullptr;
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
	Palapeli::PuzzleComponent *component = nullptr;

	Component() {}
	explicit Component(Palapeli::PuzzleComponent* component) : component(component) {}
	~Component() { delete component; }
};
struct Palapeli::Puzzle::Private
{
	Palapeli::Puzzle* q;
	QHash<Palapeli::PuzzleComponent::Type, Component*> m_components;
	Palapeli::PuzzleComponent *m_mainComponent;
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
	m_mainComponent->m_puzzle = q;
	m_components.insert(mainComponent->type(), new Component(mainComponent));
}

Palapeli::Puzzle::~Puzzle()
{
	QHashIterator<Palapeli::PuzzleComponent::Type, Component*> iter(d->m_components);
	while (iter.hasNext())
		delete iter.next().value();
	delete d;
}

const Palapeli::PuzzleComponent* Palapeli::Puzzle::component(Palapeli::PuzzleComponent::Type type) const
{
	const Component* c = d->m_components.value(type);
	return c ? c->component : nullptr;
}

const Palapeli::PuzzleComponent* Palapeli::Puzzle::get(Palapeli::PuzzleComponent::Type type)
{
	return d->get (type);
}

const Palapeli::PuzzleComponent* Palapeli::Puzzle::Private::get(Palapeli::PuzzleComponent::Type type)
{
	Component* c = m_components.value(type);
	if (c)
		return c->component;

	m_components.insert(type, c = new Component);
	Palapeli::PuzzleComponent* cmp = m_mainComponent->cast(type);
	if (cmp)
		cmp->m_puzzle = q;
	c->component = cmp;
	return cmp;
}

QString Palapeli::Puzzle::identifier() const
{
	return d->m_identifier;
}

QString Palapeli::Puzzle::location() const
{
	return d->m_location;
}

void Palapeli::Puzzle::setLocation(const QString& location)
{
	d->m_location = location;
}

void Palapeli::Puzzle::dropComponent(Palapeli::PuzzleComponent::Type type)
{
	//DO NEVER EVER USE THIS FUNCTION! THIS FUNCTION IS PURELY DANGEROUS. STUFF WILL BREAK.
	Component*& c = d->m_components[type];
	delete c;
	c = nullptr;
}

Q_GLOBAL_STATIC(QList<QString>, g_usedIdentifiers)

/*static*/ QString Palapeli::Puzzle::fsIdentifier(const QString& location)
{
	QString puzzleName = QFileInfo(location).fileName();
	const char* disallowedChars = "\\:*?\"<>|"; //Windows forbids using these chars in filenames, so we'll strip them
	for (const char* c = disallowedChars; *c; ++c)
		puzzleName.remove(QLatin1Char(*c));
	const QString identifierPattern = QString::fromLatin1("__FSC_%1_%2_").arg(puzzleName);
	int uniquifier = 0;
	while (g_usedIdentifiers->contains(identifierPattern.arg(uniquifier)))
		++uniquifier;
	const QString identifier = identifierPattern.arg(uniquifier);
	*g_usedIdentifiers << identifier;
	return identifier;
}

//END Palapeli::Puzzle

