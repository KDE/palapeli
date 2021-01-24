/*
    SPDX-FileCopyrightText: 2009 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
			explicit SlicerConfigWidget(const Pala::Slicer* slicer);

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
