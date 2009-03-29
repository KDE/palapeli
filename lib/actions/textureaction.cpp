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

#include "textureaction.h"
#include "../core/viewmenu.h"

#include <KActionCollection>
#include <KMessageBox>
#include <KLocalizedString>

Palapeli::TextureAction::TextureAction(Palapeli::ViewMenu* menu, QObject* parent)
	: KAction(KIcon("games-config-board"), i18n("Choose puzzle table &texture..."), parent)
	, m_menu(menu)
{
	setObjectName("palapeli_texture");
	setToolTip(i18n("Select a texture for the puzzle table"));
	connect(this, SIGNAL(triggered()), this, SLOT(handleTrigger()));

	KActionCollection* collection = qobject_cast<KActionCollection*>(parent);
	if (collection)
		collection->addAction(objectName(), this);
}

void Palapeli::TextureAction::handleTrigger()
{
	m_menu->showAtCursorPosition();
	connect(m_menu, SIGNAL(hidden()), this, SLOT(handleHidden()));
}

void Palapeli::TextureAction::handleHidden()
{
	disconnect(m_menu, SIGNAL(hidden()), this, SLOT(handleHidden()));
	KMessageBox::information(0, i18n("You can also open the puzzle table texture menu by right-clicking on the view."), QString(), "palapeli_texture_alternative"); //the last one is the dontShowAgainName
}

#include "textureaction.moc"
