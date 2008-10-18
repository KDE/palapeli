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

#include "resetaction.h"
#include "../library/library.h"
#include "../library/librarybase.h"
#include "../library/puzzleinfo.h"
#include "../mainwindow.h"
#include "../manager.h"

#include <QFile>
#include <KActionCollection>
#include <KIcon>
#include <KLocalizedString>
#include <KMessageBox>

Palapeli::ResetAction::ResetAction(QObject* parent)
	: KAction(KIcon("edit-clear-history"), i18n("&Reset"), parent)
{
	setEnabled(false); //will be enabled when a puzzle is started
	setObjectName("palapeli_reset");
	setShortcut(Qt::CTRL + Qt::Key_R);
	setToolTip(i18n("Reset the state of this puzzle"));
	connect(this, SIGNAL(triggered()), this, SLOT(handleTrigger()));

	KActionCollection* collection = qobject_cast<KActionCollection*>(parent);
	if (collection)
		collection->addAction(objectName(), this);
}

void Palapeli::ResetAction::handleTrigger()
{
	//get puzzle info
	const Palapeli::PuzzleInfo* info = ppMgr()->puzzleInfo();
	if (!info)
		return;
	//confirm deletion of state config
	int result = KMessageBox::warningContinueCancel(ppMgr()->window(), i18n("You're about to delete your saved progress for the puzzle \"%1\".", info->name), i18n("Confirm progress deletion"), KStandardGuiItem::cont(), KStandardGuiItem::cancel(), QLatin1String("confirm-state-reset"));
	//perform deletion of state config
	if (result == KMessageBox::Continue)
	{
		const QString stateConfig = ppMgr()->library()->base()->findFile(info->identifier, Palapeli::LibraryBase::StateConfigFile);
		QFile(stateConfig).remove();
		ppMgr()->loadGame(info, true); //reload to lose the progress completely
	}
}

void Palapeli::ResetAction::gameNameWasChanged(const QString& name)
{
	setEnabled(!name.isEmpty() && ppMgr()->puzzleInfo() != 0);
}

#include "resetaction.moc"
