/*
    SPDX-FileCopyrightText: 2010 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "configdialog.h"
#include "configdialog_p.h"
#include "triggerconfigwidget.h"
#include "../engine/texturehelper.h"
#include "settings.h"
#include <QIcon>
#include <QTimer>

//BEGIN Palapeli::TriggerComboBox

Palapeli::TriggerComboBox::TriggerComboBox(QWidget* parent)
	: KComboBox(parent)
{
	connect(this, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &TriggerComboBox::handleCurrentIndexChanged);
}

QString Palapeli::TriggerComboBox::backgroundKey() const
{
	return itemData(currentIndex(), Palapeli::TextureHelper::IdentifierRole).toString();
}

void Palapeli::TriggerComboBox::setBackgroundKey(const QString& backgroundKey)
{
	int index = findData(backgroundKey, Palapeli::TextureHelper::IdentifierRole);
	if (index > -1)
		setCurrentIndex(index);
}

void Palapeli::TriggerComboBox::handleCurrentIndexChanged(int index)
{
	Q_UNUSED(index)
	const QString key = backgroundKey();
	Q_EMIT backgroundKeyChanged(key);
	Q_EMIT itemTypeChanged(key == QLatin1String("__color__"));
}

//END Palapeli::TriggerComboBox
//BEGIN Palapeli::ConfigDialog

Palapeli::ConfigDialog::ConfigDialog(QWidget* parent)
	: KConfigDialog(parent, QString(), Settings::self())
	, m_triggerPage(new Palapeli::TriggerConfigWidget)
	, m_shownForFirstTime(false)
{
	//setup page "General settings"
	QWidget* generalPage = new QWidget;
	m_generalUi.setupUi(generalPage);

	// Construction of the TextureHelper singleton also loads all Palapeli
	// settings or defaults and the combobox contents for ViewBackground.
	m_generalUi.kcfg_ViewBackground->setModel(
					Palapeli::TextureHelper::instance());
	setupSolutionAreaComboBox();

	addPage(generalPage, i18n("General settings"))->
				setIcon(QIcon::fromTheme( QStringLiteral( "configure" )));
	//setup page "Mouse interaction"
	addPage(m_triggerPage, i18n("Mouse interaction"))->
				setIcon(QIcon::fromTheme( QStringLiteral( "input-mouse" )));
	connect(m_triggerPage, &TriggerConfigWidget::associationsChanged,
	        this, &ConfigDialog::updateButtons);
}

bool Palapeli::ConfigDialog::hasChanged()
{
	return m_triggerPage->hasChanged();
}

bool Palapeli::ConfigDialog::isDefault()
{
	return m_triggerPage->isDefault();
}

void Palapeli::ConfigDialog::updateSettings()
{
	//schedule update of TextureHelper (but only after KConfigDialog has written the settings, which might happen after this slot call)
	QTimer::singleShot(0, Palapeli::TextureHelper::instance(), &TextureHelper::readSettings);
	m_triggerPage->updateSettings();
}

void Palapeli::ConfigDialog::updateWidgets()
{
	m_triggerPage->updateWidgets();
}

void Palapeli::ConfigDialog::updateWidgetsDefault()
{
	m_triggerPage->updateWidgetsDefault();
}

void Palapeli::ConfigDialog::showEvent(QShowEvent* event)
{
	KConfigDialog::showEvent(event);
	//the dialog is usually created a bit small
	if (!m_shownForFirstTime)
	{
		resize(minimumSize().expandedTo(geometry().size()) + QSize(50, 100));
		m_shownForFirstTime = true;
	}
}

void Palapeli::ConfigDialog::setupSolutionAreaComboBox()
{
	QComboBox* b = m_generalUi.kcfg_SolutionArea;
	b->insertItem(Center,      i18n("Center"),       Center);
	b->insertItem(None,        i18n("None"),         None);
	b->insertItem(TopLeft,     i18n("Top Left"),     TopLeft);
	b->insertItem(TopRight,    i18n("Top Right"),    TopRight);
	b->insertItem(BottomLeft,  i18n("Bottom Left"),  BottomLeft);
	b->insertItem(BottomRight, i18n("Bottom Right"), BottomRight);
	b->setCurrentIndex(Settings::solutionArea());
	connect(b,    QOverload<int>::of(&QComboBox::currentIndexChanged),
		this, &ConfigDialog::solutionAreaChange);
}

void Palapeli::ConfigDialog::solutionAreaChange(int index)
{
	// Play safe by providing this slot. KConfigSkeleton seems to save the
	// index setting if .kcfg says "Int", but it does not officially support
	// the QComboBox type.
	Settings::setSolutionArea(index);
	Settings::self()->save();
}

//END Palapeli::ConfigDialog


//
