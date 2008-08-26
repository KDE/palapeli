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

#include "onscreendialog.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QSignalMapper>
#include <QVBoxLayout>
#include <KGuiItem>
#include <KIcon>
#include <KPageWidget>
#include <KPageWidgetItem>
#include <KPushButton>

Palapeli::OnScreenDialog::OnScreenDialog(QWidget* widget, QList<KGuiItem> buttons, const QString& title, Palapeli::AutoscalingItem* parent)
	: Palapeli::OnScreenWidget(0, parent)
	, m_mapper(new QSignalMapper)
{
	//create buttons and compose a button layout
	QHBoxLayout* buttonLayout = 0;
	if (!buttons.isEmpty())
	{
		buttonLayout = new QHBoxLayout;
		buttonLayout->setMargin(0);
		for (int i = 0; i < buttons.count(); ++i)
		{
			KPushButton* button = new KPushButton(buttons[i]);
			buttonLayout->addWidget(button);
			//map pressed() signals of buttons to own buttonPressed(int) signal
			connect(button, SIGNAL(pressed()), m_mapper, SLOT(map()));
			m_mapper->setMapping(button, i);
		}
	}
	connect(m_mapper, SIGNAL(mapped(int)), this, SIGNAL(buttonPressed(int)));
	//compose main layout
	QVBoxLayout* mainLayout = new QVBoxLayout;
	if (!title.isEmpty())
	{
		KPushButton* label = new KPushButton(title);
		label->setEnabled(false);
		label->setDown(true);
		//manipulate palette to let the text color stay like when enabled
		QPalette palette = label->palette();
		palette.setColor(QPalette::Disabled, QPalette::ButtonText, palette.color(QPalette::Active, QPalette::ButtonText));
		label->setPalette(palette);
		//make font bold
		QFont font = label->font();
		font.setWeight(QFont::Bold);
		label->setFont(font);
		mainLayout->addWidget(label);
	}
	if (widget)
		mainLayout->addWidget(widget);
	if (buttonLayout)
		mainLayout->addLayout(buttonLayout);
	mainLayout->setMargin(0);
	//create container for main layout
	QWidget* container = new QWidget;
	container->setLayout(mainLayout);
	setWidget(container);
	//ownership for all objects created in this function is taken by Palapeli::OnScreenDialog (or, to be more precise, its direct base Palapeli::OnScreenWidget)
}

Palapeli::OnScreenDialog::~OnScreenDialog()
{
	delete m_mapper; //everything else is deleted together with the container widget
}

void Palapeli::OnScreenDialog::setButtonGuiItem(int id, const KGuiItem& item)
{
	if (id < 0 || id >= m_buttons.count())
		return;
	m_buttons[id]->setGuiItem(item);
	updateGeometry();
	setGeometry(QRectF(QPointF(0.0, 0.0), effectiveSizeHint(Qt::PreferredSize)));
}

void Palapeli::OnScreenDialog::setButtonIcon(int id, const KIcon& icon)
{
	if (id < 0 || id >= m_buttons.count())
		return;
	m_buttons[id]->setIcon(icon);
	updateGeometry();
	setGeometry(QRectF(QPointF(0.0, 0.0), effectiveSizeHint(Qt::PreferredSize)));
}

void Palapeli::OnScreenDialog::setButtonText(int id, const QString& text)
{
	if (id < 0 || id >= m_buttons.count())
		return;
	m_buttons[id]->setText(text);
	updateGeometry();
	setGeometry(QRectF(QPointF(0.0, 0.0), effectiveSizeHint(Qt::PreferredSize)));
}

#include "onscreendialog.moc"
