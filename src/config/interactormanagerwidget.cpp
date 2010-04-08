/***************************************************************************
 *   Copyright 2010 Stefan Majewsky <majewsky@gmx.net>
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

#include "interactormanagerwidget.h"
#include "interactorwidget.h"
#include "../engine/interactormanager.h"

#include <QGridLayout>
#include <QLabel>//FIXME: remove
#include <QPushButton>
#include <QSpacerItem>
#include <QVBoxLayout>
#include <KIcon>
#include <KLocalizedString>

Palapeli::InteractorManagerWidget::InteractorManagerWidget(Palapeli::InteractorManager* manager, QWidget* parent)
	: QWidget(parent)
	, m_manager(manager)
	, m_contentsLayout(new QGridLayout)
	, m_addButton(0)
{
	//setup layouts
	QVBoxLayout* mainLayout = new QVBoxLayout;
	setLayout(mainLayout);
	mainLayout->addLayout(m_contentsLayout);
	mainLayout->addWidget(new QLabel(QLatin1String("Changes are applied immediately. (TODO: Change before release)")));
	mainLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));
	m_contentsLayout->setContentsMargins(0, 0, 0, 0);
	//create interactor widgets for existing triggers
	for (int i = 0; i < m_manager->triggers().size(); ++i)
		triggerAdded(i);
	connect(m_manager, SIGNAL(triggerAdded(int)), SLOT(triggerAdded(int)));
	//add "Add action" button
	m_addButton = new QPushButton(KIcon("list-add"), i18n("Add Action..."));
	m_addButton->setToolTip(i18n("Add another mouse action"));
	m_contentsLayout->addWidget(m_addButton, m_manager->triggers().size() + 1, 0);
	connect(m_addButton, SIGNAL(clicked()), SLOT(addRequest()));
}

Palapeli::InteractorManagerWidget::~InteractorManagerWidget()
{
	//NOTE: We cannot use QObject::setParent to delete the interactor widgets, because when QObjectPrivate::deleteChildren is invoked by this->~QObject(), the subwidgets of the interactor widgets have already been destroyed by this->~QWidget().
	qDeleteAll(m_interactorWidgets);
}

void Palapeli::InteractorManagerWidget::addRequest()
{
	//create new empty trigger
	Palapeli::AssociatedInteractorTrigger dummyTrigger;
	dummyTrigger.second = m_manager->interactors()[0];
	m_manager->addTrigger(dummyTrigger);
	//This will send a triggerAdded() signal.
}

void Palapeli::InteractorManagerWidget::triggerAdded(int index)
{
	Palapeli::InteractorWidget* iWidget = new Palapeli::InteractorWidget(m_manager, index, m_contentsLayout);
	m_interactorWidgets << iWidget;
	connect(iWidget, SIGNAL(destroyed(QObject*)), SLOT(interactorWidgetDestroyed(QObject*)));
	if (m_addButton)
	{
		m_contentsLayout->removeWidget(m_addButton);
		m_contentsLayout->addWidget(m_addButton, m_manager->triggers().size() + 1, 0);
	}
}

void Palapeli::InteractorManagerWidget::interactorWidgetDestroyed(QObject* object)
{
	m_interactorWidgets.removeAll(reinterpret_cast<Palapeli::InteractorWidget*>(object));
	m_contentsLayout->removeWidget(m_addButton);
	m_contentsLayout->addWidget(m_addButton, m_manager->triggers().size(), 0);
}

#include "interactormanagerwidget.moc"
