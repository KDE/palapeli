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
#include "../engine/interactor.h"
#include "../engine/triggermapper.h"

#include <KLocalizedString>

Palapeli::TriggerConfigWidget::TriggerConfigWidget(QWidget* parent)
	: QTabWidget(parent)
	, m_interactors(Palapeli::TriggerMapper::createInteractors(0)) //these interactors are just for reading metadata
{
	createTriggerListView(m_mouseView, Palapeli::MouseInteractor);
	createTriggerListView(m_wheelView, Palapeli::WheelInteractor);
	addTab(m_mouseView, i18n("Mouse buttons"));
	addTab(m_wheelView, i18n("Mouse wheel"));
}

Palapeli::TriggerConfigWidget::~TriggerConfigWidget()
{
	qDeleteAll(m_interactors);
}

void Palapeli::TriggerConfigWidget::createTriggerListView(Palapeli::TriggerListView*& view, int interactorType)
{
	//filter interactors
	QMap<QByteArray, Palapeli::Interactor*> interactors(m_interactors);
	QMutableMapIterator<QByteArray, Palapeli::Interactor*> iter1(interactors);
	while (iter1.hasNext())
		if (iter1.next().value()->interactorType() != interactorType)
			iter1.remove();
	//filter associations
	QMap<QByteArray, Palapeli::Trigger> associations = Palapeli::TriggerMapper::instance()->associations();
	QMutableMapIterator<QByteArray, Palapeli::Trigger> iter2(associations);
	while (iter2.hasNext())
		if (!interactors.contains(iter2.next().key()))
			iter2.remove();
	//create view
	view = new Palapeli::TriggerListView(interactors, associations, this);
}

void Palapeli::TriggerConfigWidget::writeConfig()
{
	//TODO
}
