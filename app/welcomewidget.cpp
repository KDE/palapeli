/***************************************************************************
 *   Copyright 2008 Stefan Majewsky <majewsky@gmx.net>
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

#include "welcomewidget.h"

//TODO: make an own collection of Palapeli::StandardGuiItems for actions that occur at multiple places (for instance, here and in the toolbar)

#include <QGridLayout>
#include <KIcon>
#include <KLocalizedString>
#include <KPushButton>

Palapeli::WelcomeWidget::WelcomeWidget()
	: m_mainLayout(new QGridLayout)
	, m_libraryButton(new KPushButton(KIcon("document-load"), i18n("Open the puzzle library")))
	, m_importButton(new KPushButton(KIcon("document-import"), i18n("Import a puzzle into your library")))
	, m_createButton(new KPushButton(KIcon("document-new"), i18n("Create a new puzzle")))
{
	m_mainLayout->addWidget(m_libraryButton, 1, 1);
	m_mainLayout->addWidget(m_importButton, 2, 1);
	m_mainLayout->addWidget(m_createButton, 3, 1);
	m_mainLayout->setRowStretch(0, 1);
	m_mainLayout->setRowStretch(4, 1);
	m_mainLayout->setColumnStretch(0, 1);
	m_mainLayout->setColumnStretch(2, 1);
	setLayout(m_mainLayout);
	connect(m_libraryButton, SIGNAL(pressed()), this, SIGNAL(libraryRequest()));
	connect(m_importButton, SIGNAL(pressed()), this, SIGNAL(importRequest()));
	connect(m_createButton, SIGNAL(pressed()), this, SIGNAL(createRequest()));
}

#include "welcomewidget.moc"
