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

#ifndef PALAPELI_SLICERSELECTOR_H
#define PALAPELI_SLICERSELECTOR_H

#include <QTreeWidget>

namespace Pala
{
	class Slicer;
	class SlicerMode;
}

namespace Palapeli
{
	struct SlicerSelection
	{
		SlicerSelection() : slicer(nullptr), mode(nullptr) {}
		SlicerSelection(const QString& n, const Pala::Slicer* s, const Pala::SlicerMode* m = nullptr) : slicerPluginName(n), slicer(s), mode(m) {}

		QString slicerPluginName;
		const Pala::Slicer* slicer;
		const Pala::SlicerMode* mode; //== 0 for mode-less slicers
	};

	class SlicerSelector : public QTreeWidget
	{
		Q_OBJECT
		public:
			explicit SlicerSelector(QWidget* parent = nullptr);
			~SlicerSelector() override;

			QList<const Pala::Slicer*> slicers() const;
			//NOTE: The objects pointed to by the SlicerSelection instance will be destroyed when this class is deleted.
			SlicerSelection currentSelection() const;
		Q_SIGNALS:
			void currentSelectionChanged(const Palapeli::SlicerSelection& selection);
		private Q_SLOTS:
			void slotSelectionChanged();
		private:
			QList<Pala::Slicer*> m_slicerInstances;
			QList<Palapeli::SlicerSelection> m_knownSelections;
	};
}

Q_DECLARE_METATYPE(Palapeli::SlicerSelection)

#endif // PALAPELI_SLICERSELECTOR_H
