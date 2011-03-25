/***************************************************************************
 *   Copyright 2009, 2010 Stefan Majewsky <majewsky@gmx.net>
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

#ifndef PALAPELI_PUZZLECREATOR_H
#define PALAPELI_PUZZLECREATOR_H

#include "slicerselector.h"

#include <QMap>
class QStackedWidget;
#include <KAssistantDialog>
class KComboBox;
class KLineEdit;
class KUrlRequester;

namespace Pala
{
	class Slicer;
}

namespace Palapeli
{
	class OldPuzzle;
	class SlicerConfigWidget;

	class PuzzleCreatorDialog : public KAssistantDialog
	{
		Q_OBJECT
		public:
			PuzzleCreatorDialog();

			Palapeli::OldPuzzle* result() const;
		private Q_SLOTS:
			void checkData();
			void updateSlicerSelection(const Palapeli::SlicerSelection& selection);
			void createPuzzle();
		private:
			Palapeli::OldPuzzle* m_result;
			//page items
			KPageWidgetItem* m_sourcePage;
			KPageWidgetItem* m_slicerPage;
			KPageWidgetItem* m_slicerConfigPage;
			//first page
			KUrlRequester* m_imageSelector;
			KLineEdit* m_nameEdit;
			KLineEdit* m_commentEdit;
			KLineEdit* m_authorEdit;
			//second/third page
			Palapeli::SlicerSelector* m_slicerSelector;
			QMap<const Pala::Slicer*, Palapeli::SlicerConfigWidget*> m_slicerConfigWidgets;
			QStackedWidget* m_slicerConfigMasterWidget;
	};
}

#endif // PALAPELI_PUZZLECREATOR_H
