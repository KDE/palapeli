/***************************************************************************
 *   Copyright 2010 Stefan Majewsky <majewsky@gmx.net>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Library General Public
 *   License as published by the Free Software Foundation; either
 *   version 2 of the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details.
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
***************************************************************************/

#include "elidinglabel.h"

#include <QFontMetrics>
#include <QLabel>

Palapeli::ElidingLabel::ElidingLabel(QWidget* parent)
	: QLabel(parent)
{
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

QString Palapeli::ElidingLabel::fullText() const
{
	return m_fullText;
}

QSize Palapeli::ElidingLabel::minimumSizeHint() const
{
	Palapeli::ElidingLabel* mutableThis = const_cast<Palapeli::ElidingLabel*>(this);
	//calculate size hint from full text
	const QString elidedText = text();
	mutableThis->setText(m_fullText);
	const QSize result = QLabel::minimumSizeHint();
	mutableThis->setText(elidedText);
	//eliding is enabled by decreasing the minimum size hint
	return QSize(result.width() / 3, result.height());
}

QSize Palapeli::ElidingLabel::sizeHint() const
{
	Palapeli::ElidingLabel* mutableThis = const_cast<Palapeli::ElidingLabel*>(this);
	//calculate size hint from full text
	const QString elidedText = text();
	mutableThis->setText(m_fullText);
	const QSize result = QLabel::sizeHint();
	mutableThis->setText(elidedText);
	//eliding is enabled by decreasing the size hint
	return QSize(result.width() / 3, result.height());
}

void Palapeli::ElidingLabel::setFullText(const QString& text)
{
	m_fullText = text;
	resizeEvent(0); //change text in label
}

void Palapeli::ElidingLabel::resizeEvent(QResizeEvent* event)
{
	if (event)
		QLabel::resizeEvent(event);
	//display elided text
	const QFontMetrics fm = fontMetrics();
	const QString text = fm.elidedText(m_fullText, Qt::ElideRight, width());
	setText(text);
	//enable tooltip with fullText if necessary
	setToolTip((text == m_fullText) ? QString() : m_fullText);
}
