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
	: KTabWidget(parent)
	, m_interactors(Palapeli::TriggerMapper::createInteractors(0)) //these interactors are just for reading metadata
	, m_mouseView(new Palapeli::TriggerListView(m_interactors, Palapeli::MouseInteractor, this))
	, m_wheelView(new Palapeli::TriggerListView(m_interactors, Palapeli::WheelInteractor, this))
{
	addTab(m_mouseView, i18n("Mouse buttons"));
	addTab(m_wheelView, i18n("Mouse wheel"));
	connect(m_mouseView, SIGNAL(associationsChanged()), SIGNAL(associationsChanged()));
	connect(m_wheelView, SIGNAL(associationsChanged()), SIGNAL(associationsChanged()));
}

Palapeli::TriggerConfigWidget::~TriggerConfigWidget()
{
	qDeleteAll(m_interactors);
}

bool Palapeli::TriggerConfigWidget::hasChanged() const
{
	QMap<QByteArray, Palapeli::Trigger> associations;
	m_mouseView->getAssociations(associations);
	m_wheelView->getAssociations(associations);
	return associations != Palapeli::TriggerMapper::instance()->associations();
}

bool Palapeli::TriggerConfigWidget::isDefault() const
{
	QMap<QByteArray, Palapeli::Trigger> associations;
	m_mouseView->getAssociations(associations);
	m_wheelView->getAssociations(associations);
	return associations == Palapeli::TriggerMapper::defaultAssociations();
}

void Palapeli::TriggerConfigWidget::updateSettings()
{
	QMap<QByteArray, Palapeli::Trigger> associations;
	m_mouseView->getAssociations(associations);
	m_wheelView->getAssociations(associations);
	Palapeli::TriggerMapper::instance()->setAssociations(associations);
}

void Palapeli::TriggerConfigWidget::updateWidgets()
{
	const QMap<QByteArray, Palapeli::Trigger> associations = Palapeli::TriggerMapper::instance()->associations();
	m_mouseView->setAssociations(associations);
	m_wheelView->setAssociations(associations);
	emit associationsChanged();
}

void Palapeli::TriggerConfigWidget::updateWidgetsDefault()
{
	const QMap<QByteArray, Palapeli::Trigger> associations = Palapeli::TriggerMapper::defaultAssociations();
	m_mouseView->setAssociations(associations);
	m_wheelView->setAssociations(associations);
	emit associationsChanged();
}

#include "triggerconfigwidget.moc"
