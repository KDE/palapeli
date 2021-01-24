/*
    SPDX-FileCopyrightText: 2009 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PALAPELI_PROPERTYWIDGET_P_H
#define PALAPELI_PROPERTYWIDGET_P_H

#include "propertywidget.h"
#include <QCheckBox>
#include <QSpinBox>
#include <KComboBox>
#include <KLineEdit>

namespace Palapeli
{
	class BooleanPropertyWidget : public Palapeli::PropertyWidget
	{
		public:
			BooleanPropertyWidget();
			QVariant propertyValue() const override;
		protected:
			void initialize(const Pala::SlicerProperty* property) override;
		private:
			QCheckBox* m_checkBox;
	};

	class IntegerPropertyWidget : public Palapeli::PropertyWidget
	{
		public:
			IntegerPropertyWidget();
			QVariant propertyValue() const override;
		protected:
			void initialize(const Pala::SlicerProperty* property) override;
		private:
			KComboBox* m_comboBox;
			QSpinBox* m_spinBox;
			QSlider* m_slider;
	};

	class StringPropertyWidget : public Palapeli::PropertyWidget
	{
		public:
			StringPropertyWidget();
			QVariant propertyValue() const override;
		protected:
			void initialize(const Pala::SlicerProperty* property) override;
		private:
			KComboBox* m_comboBox;
			KLineEdit* m_lineEdit;
	};
}

#endif // PALAPELI_PROPERTYWIDGET_P_H
