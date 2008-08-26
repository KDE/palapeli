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

#ifndef PALAPELI_LOADWIDGET_H
#define PALAPELI_LOADWIDGET_H

#include "onscreendialog.h"

#include <KAction>
class QListView;

namespace Palapeli
{

	class LoadWidget : public OnScreenDialog
	{
		Q_OBJECT
		public:
			static LoadWidget* create(Palapeli::AutoscalingItem* parent = 0);
		public Q_SLOTS:
			void handleButton(int id);
			void handleSelectionChange();
			void load();
		private:
			LoadWidget(QListView* view, Palapeli::AutoscalingItem* parent);
			QListView* m_view;
	};

	class LoadWidgetAction : public KAction
	{
		Q_OBJECT
		public:
			LoadWidgetAction(QObject* parent);
		public Q_SLOTS:
			void trigger();
	};

}

#endif // PALAPELI_LOADWIDGET_H
