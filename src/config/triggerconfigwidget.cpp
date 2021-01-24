/*
    SPDX-FileCopyrightText: 2010 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "triggerconfigwidget.h"
#include "triggerlistview.h"
#include "../engine/interactor.h"
#include "../engine/triggermapper.h"

#include <KLocalizedString>

Palapeli::TriggerConfigWidget::TriggerConfigWidget(QWidget* parent)
	: QTabWidget(parent)
	, m_interactors(Palapeli::TriggerMapper::createInteractors(nullptr)) //these interactors are just for reading metadata
	, m_mouseView(new Palapeli::TriggerListView(m_interactors, Palapeli::MouseInteractor, this))
	, m_wheelView(new Palapeli::TriggerListView(m_interactors, Palapeli::WheelInteractor, this))
{
	addTab(m_mouseView, i18n("Mouse buttons"));
	addTab(m_wheelView, i18n("Mouse wheel"));
	connect(m_mouseView, &Palapeli::TriggerListView::associationsChanged, this, &TriggerConfigWidget::associationsChanged);
	connect(m_wheelView, &Palapeli::TriggerListView::associationsChanged, this, &TriggerConfigWidget::associationsChanged);
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
	Q_EMIT associationsChanged();
}

void Palapeli::TriggerConfigWidget::updateWidgetsDefault()
{
	const QMap<QByteArray, Palapeli::Trigger> associations = Palapeli::TriggerMapper::defaultAssociations();
	m_mouseView->setAssociations(associations);
	m_wheelView->setAssociations(associations);
	Q_EMIT associationsChanged();
}


