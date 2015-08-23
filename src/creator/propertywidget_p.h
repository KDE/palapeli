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

#ifndef PALAPELI_PROPERTYWIDGET_P_H
#define PALAPELI_PROPERTYWIDGET_P_H

#include "propertywidget.h"
#include <QCheckBox>
#include <KComboBox>
#include <KIntSpinBox>
#include <KLineEdit>

namespace Palapeli
{
	class BooleanPropertyWidget : public Palapeli::PropertyWidget
	{
		public:
			BooleanPropertyWidget();
			virtual QVariant propertyValue() const;
		protected:
			virtual void initialize(const Pala::SlicerProperty* property);
		private:
			QCheckBox* m_checkBox;
	};

	class IntegerPropertyWidget : public Palapeli::PropertyWidget
	{
		public:
			IntegerPropertyWidget();
			virtual QVariant propertyValue() const;
		protected:
			virtual void initialize(const Pala::SlicerProperty* property);
		private:
			KComboBox* m_comboBox;
			QSpinBox* m_spinBox;
			QSlider* m_slider;
	};

	class StringPropertyWidget : public Palapeli::PropertyWidget
	{
		public:
			StringPropertyWidget();
			virtual QVariant propertyValue() const;
		protected:
			virtual void initialize(const Pala::SlicerProperty* property);
		private:
			KComboBox* m_comboBox;
			KLineEdit* m_lineEdit;
	};
}

#endif // PALAPELI_PROPERTYWIDGET_P_H
