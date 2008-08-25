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

#include "onscreenwidget.h"
#include "onscreenanimator.h"

#include <QGraphicsItemAnimation>
#include <QGraphicsProxyWidget>
#include <QGraphicsView>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QWidget>

const qreal ItemPadding = 5.0;

Palapeli::OnScreenWidget::OnScreenWidget(QWidget* widget, QGraphicsItem* parent)
	: QGraphicsWidget(parent)
	, m_proxy(new QGraphicsProxyWidget(this))
	, m_animator(new Palapeli::OnScreenAnimator(this))
{
	setWidget(widget);
	hide(); //start hidden, waiting for a showAnimated() call
	m_proxy->hide();
}

Palapeli::OnScreenWidget::~OnScreenWidget()
{
	delete m_proxy;
	delete m_animator;
}

const Palapeli::OnScreenAnimator* Palapeli::OnScreenWidget::animator() const
{
	return m_animator;
}

void Palapeli::OnScreenWidget::setWidget(QWidget* widget)
{
	if (widget)
	{
		widget->setAttribute(Qt::WA_NoSystemBackground); //do not overdraw the nice background
		widget->setAttribute(Qt::WA_WState_ExplicitShowHide); //QGraphicsProxyWidget would break widget visibility during its setWidget() call if this attribute is not set
	}
	m_proxy->setWidget(widget);
	updateGeometry();
	setGeometry(QRectF(QPointF(0.0, 0.0), effectiveSizeHint(Qt::PreferredSize)));
}

#include <QStyleOption>

void Palapeli::OnScreenWidget::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
//	Q_UNUSED(widget)
//	painter->fillRect(option->rect, QColor(255, 128, 128));
	//construct QStyleOption
	QStyleOptionButton opt;
	opt.initFrom(widget);
	opt.rect = option->rect;
	opt.state = QStyle::State_None;
	widget->style()->drawControl(QStyle::CE_PushButton, &opt, painter);
}

void Palapeli::OnScreenWidget::setGeometry(const QRectF& rect)
{
	QRectF innerRect(ItemPadding, ItemPadding, rect.width() - 2.0 * ItemPadding, rect.height() - 2.0 * ItemPadding);
	m_proxy->setGeometry(innerRect);
	QGraphicsWidget::setGeometry(rect);
}

void Palapeli::OnScreenWidget::showAnimated()
{
	m_animator->start(Palapeli::OnScreenAnimator::ShowDirection);
}

void Palapeli::OnScreenWidget::hideAnimated()
{
	m_animator->start(Palapeli::OnScreenAnimator::HideDirection);
}

QSizeF Palapeli::OnScreenWidget::sizeHint(Qt::SizeHint which, const QSizeF& constraint) const
{
	static const QSizeF paddingSize(2.0 * ItemPadding, 2.0 * ItemPadding);
	return m_proxy->effectiveSizeHint(which, constraint) + paddingSize;
}

void Palapeli::OnScreenWidget::hideEvent(QHideEvent* event)
{
	Q_UNUSED(event)
	m_proxy->hide();
}

void Palapeli::OnScreenWidget::showEvent(QShowEvent* event)
{
	Q_UNUSED(event)
	m_proxy->show();
}

#include "onscreenwidget.moc"
