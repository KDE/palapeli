/***************************************************************************
 *   Copyright 2009 Stefan Majewsky <majewsky@gmx.net>
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

#include "tabwindow.h"

#include <KXMLGUIBuilder>
#include <KXMLGUIFactory>

Palapeli::TabWindow::TabWindow(const QString& identifier)
	: m_identifier(identifier)
	, m_builder(0)
	, m_factory(0)
{
	setXMLFile(identifier + "ui.rc");
}

Palapeli::TabWindow::~TabWindow()
{
	m_factory->removeClient(this);
	delete m_factory;
	delete m_builder;
}

void Palapeli::TabWindow::setupGUI()
{
	m_builder = new KXMLGUIBuilder(this);
	setClientBuilder(m_builder);
	m_factory = new KXMLGUIFactory(m_builder);
	m_factory->addClient(this);
	setAutoSaveSettings("TabWindow " + m_identifier, false); //false = don't save window size (pointless for subwidgets)
}
