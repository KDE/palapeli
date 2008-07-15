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

#include "newpuzzledialog.h"
#include "manager.h"
#include "pattern-configuration.h"

#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QStackedLayout>
#include <QVBoxLayout>
#include <KLocalizedString>
#include <KUrlRequester>

namespace Palapeli
{

	class NewPuzzleDialogPrivate
	{
		public:
			NewPuzzleDialogPrivate();
			~NewPuzzleDialogPrivate();

			QVBoxLayout* m_mainLayout;

			QGroupBox* m_generalGroupBox;
			QFormLayout* m_generalLayout;
			KUrlRequester* m_generalImage;
			QComboBox* m_generalPattern;

			QGroupBox* m_patternGroupBox;
			QStackedLayout* m_patternLayout;
	};

}

Palapeli::NewPuzzleDialogPrivate::NewPuzzleDialogPrivate()
	: m_mainLayout(new QVBoxLayout)
	, m_generalGroupBox(new QGroupBox(i18n("General settings")))
	, m_generalLayout(new QFormLayout)
	, m_generalImage(new KUrlRequester)
	, m_generalPattern(new QComboBox)
	, m_patternGroupBox(new QGroupBox(i18n("Pattern settings")))
	, m_patternLayout(new QStackedLayout)
{
	//"General settings" box
	m_generalLayout->addRow(i18n("Image:"), m_generalImage);
	m_generalLayout->addRow(i18n("Pattern:"), m_generalPattern);
	m_generalGroupBox->setLayout(m_generalLayout);
	//"Pattern settings" box
	for (int i = 0; i < ppMgr()->patternConfigCount(); ++i)
	{
		//get pattern config
		Palapeli::PatternConfiguration* configuration = ppMgr()->patternConfig(i);
		QWidget* configWidget = configuration->createConfigurationWidget();
		//add pattern config to UI
		m_generalPattern->addItem(configuration->displayName());
		m_patternLayout->addWidget(configWidget);
	}
	QObject::connect(m_generalPattern, SIGNAL(activated(int)), m_patternLayout, SLOT(setCurrentIndex(int)));
	m_patternGroupBox->setLayout(m_patternLayout);
	//main layout
	m_mainLayout->addWidget(m_generalGroupBox);
	m_mainLayout->addWidget(m_patternGroupBox);
	m_mainLayout->setMargin(0); //margin is added by KDialog's layout
}

Palapeli::NewPuzzleDialogPrivate::~NewPuzzleDialogPrivate()
{
	delete m_mainLayout;
	delete m_patternGroupBox;
	delete m_patternLayout;
	delete m_generalGroupBox;
	delete m_generalLayout;
	delete m_generalImage;
	delete m_generalPattern;
}

Palapeli::NewPuzzleDialog::NewPuzzleDialog()
	: p(new Palapeli::NewPuzzleDialogPrivate)
{
	setCaption(i18n("Create a new jigsaw puzzle"));
	setButtons(KDialog::Ok | KDialog::Cancel);
	setButtonText(KDialog::Ok, i18n("Start game"));
	mainWidget()->setLayout(p->m_mainLayout);
	connect(this, SIGNAL(okClicked()), this, SLOT(okWasClicked()));
}

Palapeli::NewPuzzleDialog::~NewPuzzleDialog()
{
	delete p;
}

void Palapeli::NewPuzzleDialog::okWasClicked()
{
	if (p->m_patternLayout->currentWidget() != 0 || p->m_generalImage->url().isEmpty())
		emit startGame(p->m_generalImage->url(), p->m_patternLayout->currentIndex());
}

#include "newpuzzledialog.moc"
