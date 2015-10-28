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
#include "view.h" //for minimum/maximum values

#include <QSlider>
#include <QToolButton>
#include <QVBoxLayout>
#include <QIcon>
#include <KLocalizedString>
#include <KStandardShortcut>

Palapeli::ZoomWidget::ZoomWidget(QWidget* parent)
	: QWidget(parent)
	, m_constrainedButton(new QToolButton(this))
	, m_zoomOutButton(new QToolButton(this))
	, m_zoomInButton(new QToolButton(this))
	, m_slider(new QSlider(Qt::Horizontal))
{
	//init buttons
	m_constrainedButton->setIcon(QIcon::fromTheme( QStringLiteral( "select-rectangular" )));
	m_constrainedButton->setToolTip(i18n("Lock the puzzle table area"));
	m_constrainedButton->setCheckable(true);
	connect(m_constrainedButton, &QToolButton::toggled, this, &ZoomWidget::constrainedChanged);
	m_zoomOutButton->setIcon(QIcon::fromTheme( QStringLiteral( "zoom-out" )));
	//QT5 m_zoomOutButton->setShortcut(KStandardShortcut::zoomOut().primary());
	connect(m_zoomOutButton, &QToolButton::pressed, this, &ZoomWidget::zoomOutRequest);
	m_zoomInButton->setIcon(QIcon::fromTheme( QStringLiteral( "zoom-in" )));
	//QT5 m_zoomInButton->setShortcut(KStandardShortcut::zoomIn().primary());
	connect(m_zoomInButton, &QToolButton::pressed, this, &ZoomWidget::zoomInRequest);
	//init slider
	m_slider->setMinimum(Palapeli::View::MinimumZoomLevel);
	m_slider->setMaximum(Palapeli::View::MaximumZoomLevel);
	connect(m_slider, &QSlider::valueChanged, this, &ZoomWidget::levelChanged);
	//init widget layout
	QHBoxLayout* layout = new QHBoxLayout;
	layout->addWidget(m_constrainedButton);
	layout->addWidget(m_zoomOutButton);
	layout->addWidget(m_slider);
	layout->addWidget(m_zoomInButton);
	layout->setContentsMargins(0, 0, 0, 0);
	setLayout(layout);
}

void Palapeli::ZoomWidget::setConstrained(bool constrained)
{
	if (m_constrainedButton->isChecked() != constrained)
	{
		m_constrainedButton->setChecked(constrained);
		emit constrainedChanged(constrained);
	}
}

void Palapeli::ZoomWidget::setLevel(int level)
{
	m_slider->setValue(level);
}


