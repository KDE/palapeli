/***************************************************************************
 *   Copyright (C) 2008 Stefan Majewsky <majewsky@gmx.net>
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

#ifndef PALADESIGN_PROPERTYMODEL_H
#define PALADESIGN_PROPERTYMODEL_H

#include <QAbstractTableModel>
#include <QObject>

namespace Paladesign
{

	class PropertyModel : public QAbstractTableModel
	{
		Q_OBJECT
		public:
			PropertyModel();

			QObject* object() const;
			void setObject(QObject* object, const char* updateSignal = 0);
			void setShowInheritedProperties(bool showInheritedProperties);
			bool showInheritedProperties() const;

			void addDisplayString(const QByteArray& propertyName, const QString& displayString);
			void addLocalizedCaption(const QByteArray& propertyName, const QString& localizedCaption);

			virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
			virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
			virtual Qt::ItemFlags flags(const QModelIndex& index) const;
			virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
			virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
			virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
		private Q_SLOTS:
			void dataUpdate();
		private:
			QObject* m_object;
			int m_offset;
			bool m_showInheritedProps;

			QMap<QByteArray, QString> m_displayStrings, m_localizedCaptions;
	};

}

#endif // PALADESIGN_PROPERTYMODEL_H
