/*
    SPDX-FileCopyrightText: 2010 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PALAPELI_CONFIGDIALOG_P_H
#define PALAPELI_CONFIGDIALOG_P_H

#include <KComboBox>

namespace Palapeli
{
	class TriggerComboBox : public KComboBox
	{
		Q_OBJECT
		Q_PROPERTY(QString backgroundKey READ backgroundKey WRITE setBackgroundKey NOTIFY backgroundKeyChanged USER true)
		public:
			explicit TriggerComboBox(QWidget* parent = nullptr);

			QString backgroundKey() const;
		public Q_SLOTS:
			void setBackgroundKey(const QString& backgroundKey);
		Q_SIGNALS:
			void backgroundKeyChanged(const QString& backgroundKey);
			void itemTypeChanged(bool isColor);
		private Q_SLOTS:
			void handleCurrentIndexChanged(int index);
	};
}

#endif // PALAPELI_CONFIGDIALOG_P_H
