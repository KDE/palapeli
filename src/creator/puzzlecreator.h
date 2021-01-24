/*
    SPDX-FileCopyrightText: 2009, 2010 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PALAPELI_PUZZLECREATOR_H
#define PALAPELI_PUZZLECREATOR_H

#include "slicerselector.h"

#include <QMap>
class QStackedWidget;
#include <KAssistantDialog>
class KLineEdit;
class KUrlRequester;

namespace Pala
{
	class Slicer;
}

namespace Palapeli
{
	class Puzzle;
	class SlicerConfigWidget;

	class PuzzleCreatorDialog : public KAssistantDialog
	{
		Q_OBJECT
		public:
			PuzzleCreatorDialog();

			Palapeli::Puzzle* result() const;
		private Q_SLOTS:
			void checkData();
			void updateSlicerSelection(const Palapeli::SlicerSelection& selection);
			void createPuzzle();
		private:
			Palapeli::Puzzle* m_result;
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
