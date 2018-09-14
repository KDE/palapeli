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
			explicit TriggerComboBox(QWidget* parent = 0);

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
