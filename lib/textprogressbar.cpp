/***************************************************************************
 *   Copyright 2008 Stefan Majewsky <majewsky@gmx.net>
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

#include "textprogressbar.h"

Palapeli::TextProgressBar::TextProgressBar(QWidget* parent)
	: QProgressBar(parent)
{
	connect(&m_flushTimer, SIGNAL(timeout()), this, SLOT(reset()));
}

QString Palapeli::TextProgressBar::text() const
{
	return m_text;
}

void Palapeli::TextProgressBar::setText(const QString& text)
{
	m_flushTimer.stop();
	m_text = text;
	update();
}

void Palapeli::TextProgressBar::flush(int secondsDelay)
{
	if (secondsDelay == 0)
		reset();
	else
		m_flushTimer.start(1000 * secondsDelay);
}

void Palapeli::TextProgressBar::reset()
{
	m_flushTimer.stop();
	m_text.clear();
	QProgressBar::reset();
}

#include "textprogressbar.moc"
