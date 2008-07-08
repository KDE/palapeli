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

#include "action-removerelation.h"
#include "manager.h"
#include "relation.h"

#include <KAction>
#include <KLocalizedString>

Paladesign::RemoveRelationAction::RemoveRelationAction(Paladesign::Manager* manager)
	: KAction(KIcon("list-remove"), i18n("Remove relation"), 0)
	, m_manager(manager)
{
	setEnabled(false);
	connect(this, SIGNAL(triggered()), this, SLOT(remove()));
}

void Paladesign::RemoveRelationAction::remove()
{
	for (int i = 2; i < m_manager->relationCount(); ++i)
	{
		Paladesign::Relation* relation = m_manager->relation(i);
		if (relation->selected())
		{
			m_manager->removeRelation(i);
			break;
		}
	}
}

void Paladesign::RemoveRelationAction::selectedRelationChanged()
{
	setEnabled(false);
	for (int i = 2; i < m_manager->relationCount(); ++i)
	{
		Paladesign::Relation* relation = m_manager->relation(i);
		if (relation->selected())
		{
			setEnabled(true);
			break;
		}
	}
}

#include "action-removerelation.moc"
