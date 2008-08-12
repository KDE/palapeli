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
#include "../lib/pattern-configuration.h"
#include "../lib/pattern-trader.h"
#include "manager.h"
#include "puzzlelibrary.h"

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
			NewPuzzleDialogPrivate(Palapeli::NewPuzzleDialog* parent);
			~NewPuzzleDialogPrivate();

			QWidget* m_newPuzzleWidget;
			QVBoxLayout* m_newPuzzleLayout;
			Palapeli::PuzzleLibrary* m_library;

			QGroupBox* m_generalGroupBox;
			QFormLayout* m_generalLayout;
			KUrlRequester* m_generalImage;
			QComboBox* m_generalPattern;

			QGroupBox* m_patternGroupBox;
			QStackedLayout* m_patternLayout;
	};

}

Palapeli::NewPuzzleDialogPrivate::NewPuzzleDialogPrivate(Palapeli::NewPuzzleDialog* parent)
	: m_newPuzzleWidget(new QWidget)
	, m_newPuzzleLayout(new QVBoxLayout)
	, m_library(new Palapeli::PuzzleLibrary)
	, m_generalGroupBox(new QGroupBox(i18n("General settings")))
	, m_generalLayout(new QFormLayout)
	, m_generalImage(new KUrlRequester)
	, m_generalPattern(new QComboBox)
	, m_patternGroupBox(new QGroupBox(i18n("Pattern settings")))
	, m_patternLayout(new QStackedLayout)
{
	//"General settings" box of first page
	m_generalLayout->addRow(i18n("Image:"), m_generalImage);
	m_generalLayout->addRow(i18n("Pattern:"), m_generalPattern);
	m_generalGroupBox->setLayout(m_generalLayout);
	//"Pattern settings" box of first page
	for (int i = 0; i < Palapeli::PatternTrader::self()->configurationCount(); ++i)
	{
		//create configuration widget for PatternConfiguration
		Palapeli::PatternConfiguration* configuration = Palapeli::PatternTrader::self()->configurationAt(i);
		QWidget* configWidget = new QWidget;
		configuration->populateWidget(configWidget);
		//add pattern config to UI
		m_generalPattern->addItem(configuration->property("DisplayName").toString());
		m_patternLayout->addWidget(configWidget);
	}
	QObject::connect(m_generalPattern, SIGNAL(activated(int)), m_patternLayout, SLOT(setCurrentIndex(int)));
	m_patternGroupBox->setLayout(m_patternLayout);
	//setup main widget for first page
	m_newPuzzleLayout->addWidget(m_generalGroupBox);
	m_newPuzzleLayout->addWidget(m_patternGroupBox);
	m_newPuzzleWidget->setLayout(m_newPuzzleLayout);
	//setup puzzle library - emulate "OK" button by double clicking on the puzzle library
	QObject::connect(m_library, SIGNAL(doubleClicked(const QModelIndex&)), parent, SLOT(okWasClicked()));
}

Palapeli::NewPuzzleDialogPrivate::~NewPuzzleDialogPrivate()
{
	delete m_newPuzzleWidget;
	delete m_newPuzzleLayout;
	delete m_patternGroupBox;
	delete m_patternLayout;
	delete m_generalGroupBox;
	delete m_generalLayout;
	delete m_generalImage;
	delete m_generalPattern;
	delete m_library;
}

Palapeli::NewPuzzleDialog::NewPuzzleDialog(QWidget* parent)
	: KPageDialog(parent)
	, p(new Palapeli::NewPuzzleDialogPrivate(this))
{
	//setup KDialog
	setCaption(i18n("New jigsaw puzzle"));
	setButtons(KDialog::Ok | KDialog::Cancel);
	setButtonText(KDialog::Ok, i18n("Start game"));
	connect(this, SIGNAL(okClicked()), this, SLOT(okWasClicked()));
	//setup KPageDialog
	KPageWidgetItem* fromLibPage = new KPageWidgetItem(p->m_library, i18n("From library"));
	fromLibPage->setHeader(i18n("Select a jigsaw puzzle from Palapeli's library"));
	fromLibPage->setIcon(KIcon("document-open"));
	addPage(fromLibPage);
	KPageWidgetItem* fromImagePage = new KPageWidgetItem(p->m_newPuzzleWidget, i18n("From image"));
	fromImagePage->setHeader(i18n("Create a new jigsaw puzzle from an image"));
	fromImagePage->setIcon(KIcon("document-new"));
	addPage(fromImagePage);
	//scale the dialog down to its recommended (i.e. minimum) size
	resize(1, 1);
}

Palapeli::NewPuzzleDialog::~NewPuzzleDialog()
{
	delete p;
}

void Palapeli::NewPuzzleDialog::showDialog()
{
	if (ppMgr()->ensurePersistence(Palapeli::Manager::StartingGame))
	{
		show();
		p->m_generalImage->setFocus(Qt::OtherFocusReason);
	}
}

void Palapeli::NewPuzzleDialog::okWasClicked()
{
	hide();
	if (currentPage()->widget() == p->m_newPuzzleWidget)
	{
		//create new puzzle
		if (p->m_patternLayout->currentWidget() != 0 && !p->m_generalImage->url().isEmpty())
			emit startGame(p->m_generalImage->url(), p->m_patternLayout->currentIndex());
	}
	else
	{
		//choose from library
		QString selectedTemplate = p->m_library->selectedTemplate();
		if (!selectedTemplate.isEmpty())
			emit startGame(selectedTemplate);
	}
}

#include "newpuzzledialog.moc"
