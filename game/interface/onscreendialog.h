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

#ifndef PALAPELI_ONSCREENDIALOG_H
#define PALAPELI_ONSCREENDIALOG_H

#include "onscreenwidget.h"

class QSignalMapper;
class KGuiItem;
class KIcon;
class KPushButton;

namespace Palapeli
{

	class OnScreenDialog : public OnScreenWidget
	{
		Q_OBJECT
		public:
			OnScreenDialog(QWidget* widget, QList<KGuiItem> buttons, const QString& title = QString(), Palapeli::AutoscalingItem* parent = 0); //takes ownership of widget
			~OnScreenDialog();

			void setButtonGuiItem(int id, const KGuiItem& item);
			void setButtonIcon(int id, const KIcon& icon);
			void setButtonText(int id, const QString& text);
		Q_SIGNALS:
			void buttonPressed(int id);
		private:
			QSignalMapper* m_mapper;
			QList<KPushButton*> m_buttons;
	};

}

#endif // PALAPELI_ONSCREENDIALOG_H
