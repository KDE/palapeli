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

#include "createaction.h"
#include "commonaction.h"
#include "../../lib/library/library.h"
#include "../../lib/library/librarybase.h"
#include "../../lib/library/puzzleinfo.h"
#include "../../lib/patterns/pattern.h"
#include "../../lib/patterns/pattern-configuration.h"
#include "../../lib/patterns/pattern-trader.h"

#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QStackedLayout>
#include <QUuid>
#include <QVBoxLayout>
#include <KActionCollection>
#include <KConfig>
#include <KConfigGroup>
#include <KIO/NetAccess>
#include <KLineEdit>
#include <KLocalizedString>
#include <KMessageBox>
#include <KUrlRequester>
#include <KUser>

namespace Palapeli
{

	class CreateDialogPrivate
	{
		public:
			CreateDialogPrivate();

			QVBoxLayout* m_layout;

			QGroupBox* m_generalGroupBox;
			QFormLayout* m_generalLayout;
			KUrlRequester* m_generalImage;
			QComboBox* m_generalPattern;

			QGroupBox* m_patternGroupBox;
			QStackedLayout* m_patternLayout;

			QGroupBox* m_metaGroupBox;
			QFormLayout* m_metaLayout;
			KLineEdit* m_metaName;
			KLineEdit* m_metaComment;
			KLineEdit* m_metaAuthor;
	};

}

//BEGIN Palapeli::CreateDialogPrivate

Palapeli::CreateDialogPrivate::CreateDialogPrivate()
	: m_layout(new QVBoxLayout)
	, m_generalGroupBox(new QGroupBox(i18n("General settings")))
	, m_generalLayout(new QFormLayout)
	, m_generalImage(new KUrlRequester)
	, m_generalPattern(new QComboBox)
	, m_patternGroupBox(new QGroupBox(i18n("Pattern settings")))
	, m_patternLayout(new QStackedLayout)
	, m_metaGroupBox(new QGroupBox(i18n("Metadata"))) //TODO: better caption
	, m_metaLayout(new QFormLayout)
	, m_metaName(new KLineEdit)
	, m_metaComment(new KLineEdit)
	, m_metaAuthor(new KLineEdit)
{
	//"General settings" box
	m_generalLayout->addRow(i18n("Image:"), m_generalImage);
	m_generalLayout->addRow(i18n("Pattern:"), m_generalPattern);
	m_generalGroupBox->setLayout(m_generalLayout);
	//"Pattern settings" box
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
	//"Metadata" box
	m_metaLayout->addRow(i18n("Puzzle name:"), m_metaName);
	m_metaLayout->addRow(i18n("Comment:"), m_metaComment);
	m_metaLayout->addRow(i18n("Image author:"), m_metaAuthor);
	m_metaAuthor->setText(KUser().property(KUser::FullName).toString());
	m_metaGroupBox->setLayout(m_metaLayout);
	//setup main layout
	m_layout->addWidget(m_generalGroupBox);
	m_layout->addWidget(m_patternGroupBox);
	m_layout->addWidget(m_metaGroupBox);
}

//END Palapeli::CreateDialogPrivate

//BEGIN Palapeli::CreateDialog

Palapeli::CreateDialog::CreateDialog()
	: KDialog(Palapeli::Actions::dialogParent())
	, p(new Palapeli::CreateDialogPrivate)
{
	setCaption(i18n("Create a puzzle"));
	setButtons(KDialog::Ok | KDialog::Cancel);
	mainWidget()->setLayout(p->m_layout);
	connect(this, SIGNAL(okClicked()), this, SLOT(handleOkButton()));
	connect(p->m_generalImage, SIGNAL(textChanged(const QString&)), this, SLOT(handleInput()));
	connect(p->m_metaName, SIGNAL(textChanged(const QString&)), this, SLOT(handleInput()));
	connect(p->m_metaAuthor, SIGNAL(textChanged(const QString&)), this, SLOT(handleInput()));
	handleInput();
}

Palapeli::CreateDialog::~CreateDialog()
{
}

void Palapeli::CreateDialog::handleInput()
{
	const bool incompleteInput = p->m_metaName->text().isEmpty() || p->m_metaAuthor->text().isEmpty() || p->m_generalImage->url().isEmpty();
	enableButtonOk(!incompleteInput);
}

void Palapeli::CreateDialog::handleOkButton()
{
	hide();
	Palapeli::LibraryStandardBase* base = Palapeli::LibraryStandardBase::self();
	//get image file name, extract extension, create unique filename (to avoid problems with generic image names like "test.jpg")
	const KUrl imageUrl = p->m_generalImage->url();
	static QRegExp extensionExtractor("(\\..*)$");
	QString extension = imageUrl.fileName();
	if (extensionExtractor.indexIn(extension, 0) >= 0)
		extension = extensionExtractor.cap(1);
	const QString imageName = QUuid::createUuid().toString() + extension;
	QString imagePath = base->findFile(imageName, Palapeli::LibraryBase::ImageFile, true);
	//download image file
	if (imageUrl.isLocalFile())
	{
		QFile imageFile(imageUrl.path());
		if (!imageFile.copy(imagePath))
		{
			KMessageBox::error(Palapeli::Actions::dialogParent(), i18n("Image could not be copied to the puzzle library."));
			return;
		}
	}
	else
	{
		if (!KIO::NetAccess::download(imageUrl, imagePath, 0))
		{
			KMessageBox::error(Palapeli::Actions::dialogParent(), KIO::NetAccess::lastErrorString());
			return;
		}
	}
	//create main config file
	const QString identifier = QUuid::createUuid().toString();
	KConfig config(base->findFile(identifier, Palapeli::LibraryBase::MainConfigFile, true));
	KConfigGroup desktopGroup(&config, "Desktop Entry");
	desktopGroup.writeEntry("Name", p->m_metaName->text());
	desktopGroup.writeEntry("Comment", p->m_metaComment->text());
	desktopGroup.writeEntry("Icon", imageName);
	desktopGroup.writeEntry("Type", "X-Palapeli-Puzzle");
	desktopGroup.writeEntry("X-KDE-PluginInfo-Author", p->m_metaAuthor->text());
	//write pattern arguments
	KConfigGroup palapeliGroup(&config, "X-Palapeli");
	Palapeli::PatternConfiguration* patternConfig = Palapeli::PatternTrader::self()->configurationAt(p->m_patternLayout->currentIndex());
	patternConfig->writeArguments(&palapeliGroup);
	//simulate puzzle creation to find piece count
	Palapeli::Pattern* pattern = patternConfig->createPattern();
	pattern->setMode(Palapeli::Pattern::SimulateCreation);
	pattern->slice(QImage(imagePath));
	palapeliGroup.writeEntry("PieceCount", pattern->pieceCount());
	//config file done - report to LibraryBase about new item
	config.sync();
	base->reportNewEntry(identifier);
	emit gameCreated(Palapeli::standardLibrary()->infoForPuzzle(identifier));
}

//END Palapeli::CreateDialog

#include "createaction.moc"
