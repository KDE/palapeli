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

#ifndef PALAPELI_SLICERCONFWIDGET_H
#define PALAPELI_SLICERCONFWIDGET_H

class QFormLayout;
#include <QWidget>

namespace Pala
{
	class Slicer;
	class SlicerMode;
	class SlicerProperty;
}

namespace Palapeli
{
	class PropertyWidget;

	class SlicerConfigWidget : public QWidget
	{
		public:
			SlicerConfigWidget(const Pala::Slicer* slicer);

			QMap<QByteArray, QVariant> arguments() const;
			void setMode(const Pala::SlicerMode* mode);
		private:
			struct Entry
			{
				const Pala::SlicerProperty* property;
				Palapeli::PropertyWidget* widget;
			};
			QList<Entry> m_entries;
			QFormLayout* m_layout;
	};
}

#endif // PALAPELI_SLICERCONFWIDGET_H
