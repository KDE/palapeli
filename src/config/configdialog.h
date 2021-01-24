/*
    SPDX-FileCopyrightText: 2010 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PALAPELI_CONFIGDIALOG_H
#define PALAPELI_CONFIGDIALOG_H

#include "ui_settings.h"

#include <KConfigDialog>

namespace Palapeli
{
	class TriggerConfigWidget;

	class ConfigDialog : public KConfigDialog
	{
		Q_OBJECT
		public:
			explicit ConfigDialog(QWidget* parent = nullptr);

			enum SolutionSpace { Center, None, TopLeft, TopRight,
					     BottomLeft, BottomRight };
		protected:
			bool hasChanged() override;
			bool isDefault() override;
			void updateSettings() override;
			void updateWidgets() override;
			void updateWidgetsDefault() override;
			void showEvent(QShowEvent* event) override;
		private Q_SLOTS:
			void solutionAreaChange(int index);
		private:
			void setupSolutionAreaComboBox();
			Ui::Settings m_generalUi;
			Palapeli::TriggerConfigWidget* m_triggerPage;
			bool m_shownForFirstTime;
	};
}

#endif // PALAPELI_CONFIGDIALOG_H
