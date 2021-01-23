/***************************************************************************
 *   Copyright 2010 Stefan Majewsky <majewsky@gmx.net>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Library General Public
 *   License as published by the Free Software Foundation; either
 *   version 2 of the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details.
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
***************************************************************************/

#ifndef PALAPELI_TRIGGERCONFIGWIDGET_H
#define PALAPELI_TRIGGERCONFIGWIDGET_H

#include <QMap>
#include <QTabWidget>

namespace Palapeli
{
	class Interactor;
	class TriggerListView;

	class TriggerConfigWidget : public QTabWidget
	{
		Q_OBJECT
		public:
			//TODO: Provide signal interface for changes (to enable "Apply" button in config dialog.)
			explicit TriggerConfigWidget(QWidget* parent = nullptr);
			virtual ~TriggerConfigWidget();

			bool hasChanged() const;
			bool isDefault() const;
			void updateSettings();
			void updateWidgets();
			void updateWidgetsDefault();
		Q_SIGNALS:
			void associationsChanged();
		private:
			QMap<QByteArray, Palapeli::Interactor*> m_interactors;
			Palapeli::TriggerListView* m_mouseView;
			Palapeli::TriggerListView* m_wheelView;
	};
}

#endif // PALAPELI_TRIGGERCONFIGWIDGET_H
