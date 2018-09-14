/***************************************************************************
 *   Copyright 2010 Stefan Majewsky <majewsky@gmx.net>
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
			explicit ConfigDialog(QWidget* parent = 0);

			enum SolutionSpace { Center, None, TopLeft, TopRight,
					     BottomLeft, BottomRight };
		protected:
			bool hasChanged() Q_DECL_OVERRIDE;
			bool isDefault() Q_DECL_OVERRIDE;
			void updateSettings() Q_DECL_OVERRIDE;
			void updateWidgets() Q_DECL_OVERRIDE;
			void updateWidgetsDefault() Q_DECL_OVERRIDE;
			void showEvent(QShowEvent* event) Q_DECL_OVERRIDE;
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
