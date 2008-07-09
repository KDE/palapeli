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

#include "action-addrelation.h"
#include "manager.h"
#include "relation.h"

#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <KIcon>
#include <KIntSpinBox>
#include <KLocalizedString>
#include <KMenu>

Paladesign::AddRelationAction::AddRelationAction(Paladesign::Manager* manager)
	: KActionMenu(KIcon("list-add"), i18n("Add logical relation"), 0)
	, m_manager(manager)
	, m_relation1StepsCaption(new QLabel(i18nc("PR = Physical relation, please translate the acronym", "Steps along PR 1")))
	, m_relation1Steps(new KIntSpinBox(-5, 5, 1, 1, 0))
	, m_relation2StepsCaption(new QLabel(i18nc("PR = Physical relation, please translate the acronym", "Steps along PR 2")))
	, m_relation2Steps(new KIntSpinBox(-5, 5, 1, 1, 0))
	, m_actionButton(new QPushButton(i18n("Add relation")))
	, m_containerLayout(new QGridLayout)
	, m_container(new QWidget)
	, m_containerAct(new KAction(0))
{
	//widget layout
	m_containerLayout->addWidget(m_relation1StepsCaption, 0, 0, 1, 1, Qt::AlignRight | Qt::AlignVCenter);
	m_containerLayout->addWidget(m_relation1Steps, 0, 1, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
	m_containerLayout->addWidget(m_relation2StepsCaption, 1, 0, 1, 1, Qt::AlignRight | Qt::AlignVCenter);
	m_containerLayout->addWidget(m_relation2Steps, 1, 1, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
	m_containerLayout->addWidget(m_actionButton, 2, 0, 1, 2, Qt::AlignCenter);
	//TODO: Make a KIntSpinBox with returnPressed() signal.
	//connect(m_relation1Steps, SIGNAL(editingFinished()), this, SLOT(addRelation()));
	//connect(m_relation2Steps, SIGNAL(editingFinished()), this, SLOT(addRelation()));
	connect(m_actionButton, SIGNAL(clicked()), this, SLOT(addRelation()));
	//container widget
	m_container->setLayout(m_containerLayout);
	m_containerAct->setDefaultWidget(m_container);
	addAction(m_containerAct);
	//menu
	setDelayed(false);
	setStickyMenu(true);
	setToolTip(i18n("Add a new logical relation"));
}

Paladesign::AddRelationAction::~AddRelationAction()
{
	delete m_containerAct;
	delete m_container;
	delete m_containerLayout;
	delete m_actionButton;
	delete m_relation1StepsCaption;
	delete m_relation1Steps;
	delete m_relation2StepsCaption;
	delete m_relation2Steps;
}

void Paladesign::AddRelationAction::addRelation()
{
	//TODO: check if a relation with such values does already exist
	m_manager->addRelation(new Paladesign::LogicalRelation(m_relation1Steps->value(), m_relation2Steps->value(), m_manager));
	menu()->hide();
}

#include "action-addrelation.moc"
