/***************************************************************************
 *   Copyright 2009 Stefan Majewsky <majewsky@gmx.net>
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

#include <QMap>
class KComboBox;
#include <KDialog>
class KLineEdit;
class KUrlRequester;

namespace Pala
{
	class Slicer;
};

namespace Palapeli
{
	class Puzzle;
	class SlicerConfigWidget;

	class PuzzleCreatorDialog : public KDialog
	{
		public:
			PuzzleCreatorDialog();

			Palapeli::Puzzle* result() const;
		private:
			Palapeli::Puzzle* m_result;
			//general information
			KUrlRequester* m_imageSelector;
			KComboBox* m_slicerSelector;
			//metadata
			KLineEdit* m_nameEdit;
			KLineEdit* m_commentEdit;
			KLineEdit* m_authorEdit;
			//NOTE: The keys are retrieved from KService::library(), and should only be used internally.
			QMap<QString, Pala::Slicer*> m_slicers;
			QMap<QString, Palapeli::SlicerConfigWidget*> m_slicerConfigWidgets;
	};
}

#endif // PALAPELI_PUZZLECREATOR_H
