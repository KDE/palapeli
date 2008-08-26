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

#include "interfacemanager.h"
#include "autoscalingitem.h"
#include "exportwidget.h"
#include "../manager.h"
#include "loadwidget.h"
#include "onscreenanimator.h"
#include "onscreenwidget.h"
#include "savewidget.h"
#include "../view.h"

//FIXME: give focus to widget

Palapeli::InterfaceManager::InterfaceManager()
	: m_autoscaler(new Palapeli::AutoscalingItem(ppMgr()->view()))
	, m_currentWidget(0)
	, m_currentWidgetType(NoWidget)
	, m_nextWidget(0)
	, m_nextWidgetType(NoWidget)
{
}

Palapeli::InterfaceManager::~InterfaceManager()
{
	//This causes a crash, as we do not know when the Manager or the InterfaceManager (or, the scene or the widget items) are deleted.
//	delete m_autoscaler;
//	delete m_currentWidget;
//	delete m_nextWidget;
}

Palapeli::InterfaceManager* Palapeli::InterfaceManager::self()
{
	static Palapeli::InterfaceManager theOneAndOnly;
	return &theOneAndOnly;
}

Palapeli::OnScreenWidget* Palapeli::InterfaceManager::show(Palapeli::InterfaceManager::WidgetType type, const QVariantList& args)
{
	if (type == m_currentWidgetType) //already shown
		return m_currentWidget;
	if (type == m_nextWidgetType) //already scheduled to be shown next
		return m_nextWidget;
	//create widget
	Palapeli::OnScreenWidget* widget = 0;
	switch (type)
	{
		case NoWidget: //invalid input
			break;
		case ExportWidget:
			widget = Palapeli::ExportWidget::create(m_autoscaler);
			break;
		case LoadWidget:
			widget = Palapeli::LoadWidget::create(m_autoscaler);
			break;
		case SaveWidget:
			if (args.count() < 1) //not enough arguments
				break;
			widget = Palapeli::SaveWidget::create(args[0].toString(), m_autoscaler);
			break;
		default: //for unimplemented widgets
			break;
	}
	if (!widget)
		return 0;
	//set this widget as the one to be shown next
	if (m_nextWidget)
		m_nextWidget->deleteLater();
	m_nextWidget = widget;
	m_nextWidgetType = type;
	if (m_currentWidget)
		hide(m_currentWidgetType); //another widget shown - hide the one that is shown currently
	else
		next(); //nothing shown currently - directly proceed with showing this one
	return widget;
}

void Palapeli::InterfaceManager::hide(Palapeli::InterfaceManager::WidgetType type)
{
	if (type == m_currentWidgetType)
	{
		//widget is being shown currently - hide it
		m_currentWidget->hideAnimated();
		connect(m_currentWidget->animator(), SIGNAL(finished()), this, SLOT(next()));
	}
	else if (type == m_nextWidgetType)
	{
		//widget is scheduled to be shown next - remove from the schedule
		m_nextWidget->deleteLater();
		m_nextWidget = 0;
		m_nextWidgetType = NoWidget;
	}
}

void Palapeli::InterfaceManager::next()
{
	//delete current widget
	if (m_currentWidget)
		m_currentWidget->deleteLater();
	//move next widget to the place of the current widget
	m_currentWidget = m_nextWidget;
	m_currentWidgetType = m_nextWidgetType;
	//mark queue as vacant
	m_nextWidget = 0;
	m_nextWidgetType = NoWidget;
	//start to show new current widget if there is one
	if (m_currentWidget)
	{
		m_currentWidget->showAnimated();
		ppMgr()->view()->moveToTop(m_autoscaler);
	}
}

#include "interfacemanager.moc"
