/*
    SPDX-FileCopyrightText: 2009 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PALAPELI_PROPERTYWIDGET_H
#define PALAPELI_PROPERTYWIDGET_H

#include <QWidget>

namespace Pala
{
	class SlicerProperty;
}

namespace Palapeli
{
	class PropertyWidget : public QWidget
	{
		public:
			virtual QVariant propertyValue() const = 0;
		protected:
			virtual void initialize(const Pala::SlicerProperty* property) = 0;
			friend Palapeli::PropertyWidget* createPropertyWidget(const Pala::SlicerProperty* property);
	};

	Palapeli::PropertyWidget* createPropertyWidget(const Pala::SlicerProperty* property);
}

#endif // PALAPELI_PROPERTYWIDGET_H
