/*
    SPDX-FileCopyrightText: 2009 Chani Armitage <chani@kde.org>
    SPDX-FileCopyrightText: 2010 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mouseinputbutton.h"
#include "mouseinputbutton_p.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QStyleOptionButton>
#include <QIcon>
#include <KLocalizedString>

static QIcon clearIcon()
{
	if (QApplication::isLeftToRight())
		return QIcon::fromTheme( QStringLiteral( "edit-clear-locationbar-rtl" ));
	else
		return QIcon::fromTheme( QStringLiteral( "edit-clear-locationbar-ltr" ));
}

Palapeli::MouseInputButton::MouseInputButton(QWidget* parent)
	: QPushButton(parent)
	, m_iconLabel(new QLabel)
	, m_mainLabel(new QLabel)
	, m_clearButton(new Palapeli::FlatButton(clearIcon()))
	, m_mouseAllowed(true)
	, m_wheelAllowed(true)
	, m_noButtonAllowed(true)
	, m_requiresValidation(false)
{
	qRegisterMetaType<Palapeli::Trigger>();
	connect(this, &QAbstractButton::clicked, this, &MouseInputButton::captureTrigger);
	connect(m_clearButton, &FlatButton::clicked, this, &MouseInputButton::clearTrigger);
	setCheckable(true);
	m_iconLabel->setPixmap(QIcon::fromTheme( QStringLiteral( "input-mouse" )).pixmap(22)); //TODO: respect global icon size configuration
	//setup child widgets
	m_iconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	m_mainLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	m_mainLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
	m_clearButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	m_clearButton->setToolTip(i18n("Remove this trigger"));
	//setup layout
	QHBoxLayout* layout = new QHBoxLayout;
	setLayout(layout);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(m_iconLabel);
	layout->addWidget(m_mainLabel);
	layout->addWidget(m_clearButton);
	updateAppearance();
	//calculate margin for layout from QStyle::sizeFromContents
	QStyleOptionButton opt;
	QPushButton::initStyleOption(&opt);
	const QSize bareSizeHint = layout->sizeHint();
	const QSize fullSizeHint = style()->sizeFromContents(QStyle::CT_PushButton, &opt, bareSizeHint, this);
	const int marginX = (fullSizeHint.width() - bareSizeHint.width()) / 2;
	const int marginY = (fullSizeHint.height() - bareSizeHint.height()) / 2;
	layout->setContentsMargins(marginX, marginY, marginX, marginY);
}

QSize Palapeli::MouseInputButton::sizeHint() const
{
	//The layout's size hint is the right one, not the one calculated by QPushButton::sizeHint.
	return QWidget::sizeHint();
}

bool Palapeli::MouseInputButton::isMouseAllowed() const
{
	return m_mouseAllowed;
}

void Palapeli::MouseInputButton::setMouseAllowed(bool mouseAllowed)
{
	m_mouseAllowed = mouseAllowed;
}

bool Palapeli::MouseInputButton::isWheelAllowed() const
{
	return m_wheelAllowed;
}

void Palapeli::MouseInputButton::setWheelAllowed(bool wheelAllowed)
{
	m_wheelAllowed = wheelAllowed;
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

	if (isChecked())
	{
		//got a trigger or cancel
		switch ((int) event->type())
		{
			case QEvent::Wheel: {
				if (!m_wheelAllowed)
					return false;
				const QWheelEvent* wEvent = static_cast<QWheelEvent*>(event);
				Palapeli::Trigger newTrigger;
				newTrigger.setModifiers(wEvent->modifiers());
				const QPoint angleDelta = wEvent->angleDelta();
				const Qt::Orientation orientation = (qAbs(angleDelta.x()) > qAbs(angleDelta.y()) ? Qt::Horizontal : Qt::Vertical);
				newTrigger.setWheelDirection(orientation);
				setTrigger(newTrigger);
				event->accept();
				return true;
			}
			case QEvent::MouseButtonRelease: {
				if (!m_mouseAllowed)
					return false;
				const QMouseEvent* mEvent = static_cast<QMouseEvent*>(event);
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
				const QKeyEvent* kEvent = static_cast<QKeyEvent*>(event);
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
			case QEvent::KeyRelease: {
				const QKeyEvent* kEvent = static_cast<QKeyEvent*>(event);
				showModifiers(kEvent->modifiers());
				break;
			}
		}
	}
	bool ret = QPushButton::event(event);
	if (event->type() == QEvent::MouseButtonRelease)
	{
		const QMouseEvent* mEvent = static_cast<QMouseEvent*>(event);
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
		Q_EMIT triggerRequest(trigger);
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
	setToolTip(i18n("Click to change how an action is triggered"));
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
		Q_EMIT triggerChanged(trigger);
}

void Palapeli::MouseInputButton::showModifiers(Qt::KeyboardModifiers modifiers)
{
	Palapeli::Trigger dummyTrigger;
	dummyTrigger.setModifiers(modifiers);
	m_mainLabel->setText(dummyTrigger.toString().arg(i18n("Input here...")));
}


//
