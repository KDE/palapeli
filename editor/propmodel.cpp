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

#include "propmodel.h"

#include <QByteArray>
#include <QMetaProperty>
#include <KLocalizedString>

Paladesign::PropertyModel::PropertyModel()
	: QAbstractTableModel()
	, m_object(0)
	, m_offset(0)
	, m_showInheritedProps(true)
{
}

QObject* Paladesign::PropertyModel::object() const
{
	return m_object;
}

void Paladesign::PropertyModel::setObject(QObject* object, const char* updateSignal)
{
	if (m_object == object)
		return;
	disconnect(this, SLOT(dataUpdate()));
	m_object = object;
	if (m_object == 0)
		m_offset = 0;
	else
	{
		m_offset = m_showInheritedProps ? 0 : m_object->metaObject()->propertyOffset();
		if (updateSignal != 0)
			connect(m_object, updateSignal, this, SLOT(dataUpdate()));
	}
	reset();
}

void Paladesign::PropertyModel::setShowInheritedProperties(bool showInheritedProperties)
{
	if (m_showInheritedProps == showInheritedProperties)
		return;
	m_showInheritedProps = showInheritedProperties;
	m_offset = (m_object == 0) ? 0 : (showInheritedProperties ? 0 : m_object->metaObject()->propertyOffset());
	reset();
}

bool Paladesign::PropertyModel::showInheritedProperties() const
{
	return m_showInheritedProps;
}

void Paladesign::PropertyModel::addDisplayString(const QByteArray& propertyName, const QString& displayString)
{
	m_displayStrings[propertyName] = displayString;
}

void Paladesign::PropertyModel::addLocalizedCaption(const QByteArray& propertyName, const QString& localizedCaption)
{
	m_localizedCaptions[propertyName] = localizedCaption;
}

int Paladesign::PropertyModel::columnCount(const QModelIndex&) const
{
	return 1;
}

QVariant Paladesign::PropertyModel::data(const QModelIndex& index, int role) const
{
	if (m_object == 0 || !index.isValid() || index.column() != 0)
		return QVariant();
	const int row = index.row();
	const int propCount = m_object->metaObject()->propertyCount() - m_offset;
	if (row >= propCount)
		return QVariant();
	QMetaProperty property = m_object->metaObject()->property(row + m_offset);
	QVariant value = property.read(m_object);
	static const QString simpleValueString("%1");
	switch (role)
	{
		case Qt::DisplayRole:
			return m_displayStrings.value(property.name(), simpleValueString).arg(value.toString());
		case Qt::EditRole:
			return value;
		default:
			return QVariant();
	}
}

Qt::ItemFlags Paladesign::PropertyModel::flags(const QModelIndex& index) const
{
	if (!index.isValid() || index.column() != 0 || m_object == 0)
		return 0;
	const int row = index.row();
	const int propCount = m_object->metaObject()->propertyCount() - m_offset;
	if (row >= propCount)
		return 0;
	else if (m_object->metaObject()->property(row + m_offset).isWritable())
		return Qt::ItemIsEnabled | Qt::ItemIsEditable;
	else
		return Qt::ItemIsEnabled;
}

QVariant Paladesign::PropertyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole || m_object == 0 || section < 0)
		return QVariant();
	if (orientation == Qt::Horizontal)
		return i18n("Value");
	const int propCount = m_object->metaObject()->propertyCount() - m_offset;
	if (section >= propCount)
		return QVariant();
	QByteArray propertyName(m_object->metaObject()->property(section + m_offset).name());
	return m_localizedCaptions.value(propertyName, QLatin1String(propertyName));
}

int Paladesign::PropertyModel::rowCount(const QModelIndex& parent) const
{
	if (parent.isValid() || m_object == 0)
		return 0;
	return m_object->metaObject()->propertyCount() - m_offset;
}

bool Paladesign::PropertyModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if (role != Qt::EditRole || m_object == 0 || !index.isValid() || index.column() != 0)
		return false;
	const int row = index.row();
	const int propCount = m_object->metaObject()->propertyCount() - m_offset;
	if (row >= propCount)
		return false;
	if (m_object->metaObject()->property(row + m_offset).write(m_object, value))
	{
		emit dataChanged(index, index);
		return true;
	}
	return false;
}

void Paladesign::PropertyModel::dataUpdate()
{
	emit dataChanged(index(0, 0), index(0, m_object->metaObject()->propertyCount() - m_offset - 1));
}
