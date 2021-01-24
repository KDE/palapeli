/*
    SPDX-FileCopyrightText: 2010 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

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
			~TriggerConfigWidget() override;

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
