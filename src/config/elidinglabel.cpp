/*
    SPDX-FileCopyrightText: 2010 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "elidinglabel.h"

#include <QFontMetrics>

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
	resizeEvent(nullptr); //change text in label
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
