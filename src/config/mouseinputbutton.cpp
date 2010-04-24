/***************************************************************************
 *   Copyright 2009 Chani Armitage <chani@kde.org>
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

#include "mouseinputbutton.h"
#include "mouseinputbutton_p.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QStyleOptionButton>
#include <KIcon>
#include <KLocalizedString>

const QString Palapeli::MouseInputButton::DefaultToolTip = i18n("Click to change how an action is triggered");

static QIcon clearIcon()
{
	if (QApplication::isLeftToRight())
		return KIcon("edit-clear-locationbar-rtl");
	else
		return KIcon("edit-clear-locationbar-ltr");
}

Palapeli::MouseInputButton::MouseInputButton(QWidget* parent)
	: QPushButton(parent)
	, m_iconLabel(new QLabel)
	, m_mainLabel(new QLabel)
	, m_clearButton(new Palapeli::FlatButton(clearIcon()))
	, m_noButtonAllowed(true)
	, m_requiresValidation(false)
{
	qRegisterMetaType<Palapeli::Trigger>();
	connect(this, SIGNAL(clicked()), SLOT(captureTrigger()));
	connect(m_clearButton, SIGNAL(clicked()), SLOT(clearTrigger()));
	setCheckable(true);
	m_iconLabel->setPixmap(KIcon("input-mouse").pixmap(22)); //TODO: respect global icon size configuration
	//setup child widgets
	m_iconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	m_mainLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	m_mainLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
	m_clearButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	m_clearButton->setToolTip(i18n("Remove this trigger"));
	//setup layout
	QHBoxLayout* layout = new QHBoxLayout;
	setLayout(layout);
	layout->setMargin(0);
	layout->addWidget(m_iconLabel);
	layout->addWidget(m_mainLabel);
	layout->addWidget(m_clearButton);
	updateAppearance();
	//calculate margin for layout from QStyle::sizeFromContents
	QStyleOptionButton opt;
	QPushButton::initStyleOption(&opt);
	const QSize bareSizeHint = layout->sizeHint();
	const QSize fullSizeHint = style()->sizeFromContents(QStyle::CT_PushButton, &opt, bareSizeHint, this).expandedTo(QApplication::globalStrut());
	const int marginX = (fullSizeHint.width() - bareSizeHint.width()) / 2;
	const int marginY = (fullSizeHint.height() - bareSizeHint.height()) / 2;
	layout->setContentsMargins(marginX, marginY, marginX, marginY);
}

QSize Palapeli::MouseInputButton::sizeHint() const
{
	//The layout's size hint is the right one, not the one calculated by QPushButton::sizeHint.
	return QWidget::sizeHint();
}

bool Palapeli::MouseInputButton::isNoButtonAllowed() const
{
	return m_noButtonAllowed;
}

void Palapeli::MouseInputButton::setNoButtonAllowed(bool noButtonAllowed)
{
	m_noButtonAllowed = noButtonAllowed;
}

bool Palapeli::MouseInputButton::requiresValidation() const
{
	return m_requiresValidation;
}

void Palapeli::MouseInputButton::setRequiresValidation(bool requiresValidation)
{
	m_requiresValidation = requiresValidation;
}

Palapeli::Trigger Palapeli::MouseInputButton::trigger() const
{
	return m_trigger;
}

void Palapeli::MouseInputButton::captureTrigger()
{
	setChecked(true);
	m_clearButton->setVisible(false); //while capture is in progress
	m_mainLabel->setText(i18n("Input here..."));
	setToolTip(i18n("Hold down the modifier keys you want, then click a mouse button or scroll a mouse wheel here"));
	setFocus(Qt::MouseFocusReason);
}

void Palapeli::MouseInputButton::clearTrigger()
{
	setTrigger(Palapeli::Trigger());
}

bool Palapeli::MouseInputButton::event(QEvent* event)
{
	const QWheelEvent* wEvent = static_cast<QWheelEvent*>(event);
	const QMouseEvent* mEvent = static_cast<QMouseEvent*>(event);
	const QKeyEvent* kEvent = static_cast<QKeyEvent*>(event);
	if (isChecked())
	{
		//got a trigger or cancel
		switch ((int) event->type())
		{
			case QEvent::Wheel: {
				Palapeli::Trigger newTrigger;
				newTrigger.setModifiers(wEvent->modifiers());
				newTrigger.setWheelDirection(wEvent->orientation());
				setTrigger(newTrigger);
				event->accept();
				return true;
			}
			case QEvent::MouseButtonRelease: {
				Palapeli::Trigger newTrigger;
				newTrigger.setModifiers(mEvent->modifiers());
				newTrigger.setButton(mEvent->button());
				setTrigger(newTrigger);
				event->accept();
				return true;
			}
			case QEvent::MouseButtonPress:
				event->accept();
				return true;
			case QEvent::KeyPress: {
				if (kEvent->key() == Qt::Key_Escape)
				{
					//cancel
					setTrigger(m_trigger);
					event->accept();
					return true;
				}
				if (kEvent->key() == Qt::Key_Space && m_noButtonAllowed)
				{
					//create trigger with NoButton (TODO: make this functionality more user-visible)
					Palapeli::Trigger newTrigger;
					newTrigger.setModifiers(kEvent->modifiers());
					newTrigger.setButton(Qt::NoButton);
					setTrigger(newTrigger);
					event->accept();
					return true;
				}
			}	//fall through
			case QEvent::KeyRelease:
				showModifiers(kEvent->modifiers());
				break;
		}
	}
	bool ret = QPushButton::event(event);
	if (event->type() == QEvent::MouseButtonRelease)
	{
		//fake a tooltip event
		//because otherwise they go away when you click and don't come back until you move the mouse
		QHelpEvent tip(QEvent::ToolTip, mEvent->pos(), mEvent->globalPos());
		QApplication::sendEvent(this, &tip);
	}
	return ret;
}

void Palapeli::MouseInputButton::setTrigger(const Palapeli::Trigger& trigger)
{
	//NOTE: Invalid triggers need not be confirmed (esp. calls to clearTrigger()).
	if (m_requiresValidation && trigger.isValid() && m_trigger != trigger)
	{
		m_stagedTrigger = trigger;
		emit triggerRequest(trigger);
	}
	else
		applyTrigger(trigger);
}

void Palapeli::MouseInputButton::confirmTrigger(const Palapeli::Trigger& trigger)
{
	if (m_stagedTrigger == trigger)
		applyTrigger(m_stagedTrigger);
}

void Palapeli::MouseInputButton::updateAppearance()
{
	//find caption
	static const QString noTriggerString = i18nc("This is used for describing that no mouse action has been assigned to this interaction plugin.", "None");
	QString text = m_trigger.toString();
	if (!m_trigger.isValid())
		text = text.arg(noTriggerString);
	//apply properties
	setChecked(false);
	setToolTip(DefaultToolTip);
	m_mainLabel->setText(text);
	m_clearButton->setVisible(m_trigger.isValid());
}

void Palapeli::MouseInputButton::applyTrigger(const Palapeli::Trigger& trigger)
{
	const bool announceChange = m_trigger != trigger;
	//apply new trigger
	m_trigger = trigger;
	m_stagedTrigger = Palapeli::Trigger();
	updateAppearance();
	//announce change
	if (announceChange)
		emit triggerChanged(trigger);
}

void Palapeli::MouseInputButton::showModifiers(Qt::KeyboardModifiers modifiers)
{
	Palapeli::Trigger dummyTrigger;
	dummyTrigger.setModifiers(modifiers);
	m_mainLabel->setText(dummyTrigger.toString().arg(i18n("Input here...")));
}

#include "mouseinputbutton.moc"
#include "mouseinputbutton_p.moc"
