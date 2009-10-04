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
