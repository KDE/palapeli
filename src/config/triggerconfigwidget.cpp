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

#include "triggerconfigwidget.h"
#include "triggerlistview.h"
#include "../engine/interactormanager.h"

#include <KLocalizedString>

Palapeli::TriggerConfigWidget::TriggerConfigWidget(Palapeli::InteractorManager* manager, QWidget* parent)
	: QTabWidget(parent)
	, m_manager(manager)
{
	createTriggerListView(m_mouseView, Palapeli::MouseInteractor);
	createTriggerListView(m_wheelView, Palapeli::WheelInteractor);
	addTab(m_mouseView, i18n("Mouse buttons"));
	addTab(m_wheelView, i18n("Mouse wheel"));
}

void Palapeli::TriggerConfigWidget::createTriggerListView(Palapeli::TriggerListView*& view, Palapeli::InteractorTypes types)
{
	//filter interactors
	QList<Palapeli::Interactor*> interactors = m_manager->interactors();
	QMutableListIterator<Palapeli::Interactor*> it1(interactors);
	while (it1.hasNext())
		if ((it1.next()->interactorTypes() & types) != types)
			it1.remove();
	//filter triggers
	QList<Palapeli::AssociatedInteractorTrigger> triggers = m_manager->triggers();
	QMutableListIterator<Palapeli::AssociatedInteractorTrigger> it2(triggers);
	while (it2.hasNext())
		if (!interactors.contains(it2.next().second))
			it2.remove();
	//create view
	view = new Palapeli::TriggerListView(interactors, triggers, this);
}

void Palapeli::TriggerConfigWidget::writeConfig()
{
	//TODO
}
