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

#ifndef PALAPELI_EXPORTWIDGET_H
#define PALAPELI_EXPORTWIDGET_H

#include "onscreendialog.h"

class QListView;
#include <KAction>

namespace Palapeli
{

	class ExportWidget : public OnScreenDialog
	{
		Q_OBJECT
		public:
			static ExportWidget* create(AutoscalingItem* parent = 0);
		public Q_SLOTS:
			void handleButton(int id);
			void handleSelectionChange();
		private:
			ExportWidget(QListView* view, AutoscalingItem* parent);
			QListView* m_view;
	};

	class ExportWidgetAction : public KAction
	{
		Q_OBJECT
		public:
			ExportWidgetAction(QObject* parent);
		public Q_SLOTS:
			void gameCountChanged();
			void trigger();
	};


}

#endif // PALAPELI_EXPORTWIDGET_H
