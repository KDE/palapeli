/***************************************************************************
 *   Copyright 2010 Stefan Majewsky <majewsky@gmx.net>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Library General Public
 *   License as published by the Free Software Foundation; either
 *   version 2 of the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details.
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
***************************************************************************/

#include "interactorwidget.h"
#include "mouseinputbutton.h"
#include "../engine/interactor.h"
#include "../engine/interactormanager.h"

#include <QComboBox>
#include <QGridLayout>
#include <QTimer>
#include <QToolButton>
#include <KIcon>

Palapeli::InteractorWidget::InteractorWidget(Palapeli::InteractorManager* manager, int index, QGridLayout* parent)
	: m_manager(manager)
	, m_index(index)
	, m_layout(parent)
	, m_inputButton(new Palapeli::MouseInputButton)
	, m_pluginList(new QComboBox)
	, m_removeButton(new QToolButton)
{
	Q_ASSERT(m_index >= 0 && m_index < m_manager->triggers().size());
	//read trigger, load interactor list
	const Palapeli::AssociatedInteractorTrigger trigger = manager->triggers()[m_index];
	foreach (Palapeli::Interactor* interactor, manager->interactors())
	{
		m_pluginList->addItem(interactor->icon(), interactor->description(), qVariantFromValue(interactor));
		if (interactor == trigger.second)
			//select current interactor
			m_pluginList->setCurrentIndex(m_pluginList->count() - 1);
	}
	//configure buttons
	m_inputButton->setTrigger(trigger.first);
	m_removeButton->setIcon(KIcon("list-remove"));
	//add widgets to layout (WARNING: When changing this, also adjust triggerRemovedFromManager().)
	m_layout->addWidget(m_inputButton, m_index, 0);
	m_layout->addWidget(m_pluginList, m_index, 1);
	m_layout->addWidget(m_removeButton, m_index, 2);
	//wire up stuff
	connect(m_manager, SIGNAL(triggerRemoved(int)), SLOT(triggerRemovedFromManager(int)));
	connect(m_inputButton, SIGNAL(triggerChanged(const Palapeli::InteractorTrigger&)), SLOT(triggerChanged(const Palapeli::InteractorTrigger&)));
	connect(m_pluginList, SIGNAL(currentIndexChanged(int)), SLOT(interactorChanged(int)));
	connect(m_removeButton, SIGNAL(clicked()), SLOT(removeRequest()));
	//start trigger capture if necessary
	if (!trigger.first.isValid())
		QTimer::singleShot(0, m_inputButton, SLOT(captureTrigger())); //NOTE: We have to delay this operation because the widget needs to be displayed first, in order to be able to gain focus.
}

Palapeli::InteractorWidget::~InteractorWidget()
{
	delete m_inputButton;
	delete m_pluginList;
	delete m_removeButton;
}

void Palapeli::InteractorWidget::triggerChanged(const Palapeli::InteractorTrigger& trigger)
{
	Palapeli::AssociatedInteractorTrigger aTrigger = m_manager->triggers()[m_index];
	aTrigger.first = trigger;
	m_manager->changeTrigger(m_index, aTrigger);
}

void Palapeli::InteractorWidget::interactorChanged(int interactorIndex)
{
	Palapeli::AssociatedInteractorTrigger trigger = m_manager->triggers()[m_index];
	trigger.second = m_pluginList->itemData(interactorIndex).value<Palapeli::Interactor*>();
	m_manager->changeTrigger(m_index, trigger);
}

void Palapeli::InteractorWidget::removeRequest()
{
	m_manager->removeTrigger(m_index);
	//NOTE: The rest is done in triggerRemovedFromManager().
}

void Palapeli::InteractorWidget::triggerRemovedFromManager(int triggerIndex)
{
	if (triggerIndex == m_index)
		deleteLater();
	else if (triggerIndex < m_index)
	{
		//This widget has moved up one position in the list because something before it was removed.
		--m_index;
		m_layout->removeWidget(m_inputButton);
		m_layout->removeWidget(m_pluginList);
		m_layout->removeWidget(m_removeButton);
		m_layout->addWidget(m_inputButton, m_index, 0);
		m_layout->addWidget(m_pluginList, m_index, 1);
		m_layout->addWidget(m_removeButton, m_index, 2);
	}
}

#include "interactorwidget.moc"
