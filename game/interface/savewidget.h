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

#ifndef PALAPELI_SAVEWIDGET_H
#define PALAPELI_SAVEWIDGET_H

#include "onscreendialog.h"

#include <KAction>
class KLineEdit;

namespace Palapeli
{

	class SaveWidget : public OnScreenDialog
	{
		Q_OBJECT
		public:
			static SaveWidget* create(const QString& gameName, Palapeli::AutoscalingItem* parent = 0);
		public Q_SLOTS:
			void handleButton(int id);
		private:
			SaveWidget(KLineEdit* edit, Palapeli::AutoscalingItem* parent);
			KLineEdit* m_edit;
	};

	class SaveWidgetAction : public KAction
	{
		Q_OBJECT
		public:
			SaveWidgetAction(QObject* parent);
		public Q_SLOTS:
			void setGameName(const QString& name);
			void trigger();
		private:
			QString m_name;
	};

}

#endif // PALAPELI_SAVEWIDGET_H