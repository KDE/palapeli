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

#include "zoomwidget.h"

#include <QSlider>
#include <QToolButton>
#include <QVBoxLayout>
#include <KIcon>
#include <KStandardShortcut>

Palapeli::ZoomWidget::ZoomWidget(QWidget* parent)
	: QWidget(parent)
	, m_zoomOutButton(new QToolButton(this))
	, m_zoomInButton(new QToolButton(this))
	, m_slider(new QSlider(Qt::Horizontal))
{
	//init buttons
	m_zoomOutButton->setIcon(KIcon("zoom-out"));
	m_zoomOutButton->setShortcut(KStandardShortcut::zoomOut().primary());
	connect(m_zoomOutButton, SIGNAL(pressed()), this, SIGNAL(zoomOutRequest()));
	m_zoomInButton->setIcon(KIcon("zoom-in"));
	m_zoomInButton->setShortcut(KStandardShortcut::zoomIn().primary());
	connect(m_zoomInButton, SIGNAL(pressed()), this, SIGNAL(zoomInRequest()));
	//init slider
	m_slider->setMinimum(1000);
	m_slider->setMaximum(10 * m_slider->minimum());
	connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(handleValueChanged(int)));
	//init widget layout
	QHBoxLayout* layout = new QHBoxLayout;
	layout->addWidget(m_zoomOutButton);
	layout->addWidget(m_slider);
	layout->addWidget(m_zoomInButton);
	setLayout(layout);
}

void Palapeli::ZoomWidget::setLevel(qreal level)
{
	m_slider->setValue(level * m_slider->minimum());
}

void Palapeli::ZoomWidget::handleValueChanged(int value)
{
	emit levelChanged(qreal(value) / m_slider->minimum());
}

#include "zoomwidget.moc"
