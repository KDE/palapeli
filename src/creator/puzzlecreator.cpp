/***************************************************************************
 *   Copyright 2009 Stefan Majewsky <majewsky@gmx.net>
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

#include "puzzlecreator.h"
#include "slicerconfwidget.h"
#include "file-io/puzzle.h"
#include "file-io/puzzlestructs.h"
#include "../libpala/slicer.h"
#include "../libpala/slicerjob.h"

#include <QBoxLayout>
#include <QFormLayout>
#include <QStackedLayout>
#include <KComboBox>
#include <KLineEdit>
#include <KLocalizedString>
#include <KMessageBox>
#include <KServiceTypeTrader>
#include <KUrlRequester>

Palapeli::PuzzleCreatorDialog::PuzzleCreatorDialog()
	: m_result(0)
	, m_imageSelector(new KUrlRequester)
	, m_slicerSelector(new KComboBox)
	, m_nameEdit(new KLineEdit)
	, m_commentEdit(new KLineEdit)
	, m_authorEdit(new KLineEdit)
	, m_slicerConfigLayout(new QStackedLayout)
{
	//setup dialog
	setCaption(i18n("Create new puzzle"));
	setButtons(KDialog::Ok | KDialog::Cancel);
	enableButton(KDialog::Ok, false); //too much data is missing (see checkData())
	setButtonIcon(KDialog::Ok, KIcon("tools-wizard")); //FIXME: This should be a custom "actions/puzzle-new" icon.
	setButtonText(KDialog::Ok, i18n("Create puzzle"));
	setButtonToolTip(KDialog::Ok, i18n("Create a new puzzle with the given information, and save it in my collection"));
	//setup image selector
	m_imageSelector->setMode(KFile::File | KFile::LocalOnly | KFile::ExistingOnly);
	//create slicers
	KService::List offers = KServiceTypeTrader::self()->query("Libpala/SlicerPlugin");
	foreach (KService::Ptr offer, offers) //FIXME: What happens if offers.isEmpty()?
	{
		const QString key = offer->library();
		Pala::Slicer* slicer = offer->createInstance<Pala::Slicer>(0, QVariantList());
		if (!slicer)
			continue;
		m_slicers[key] = slicer;
		m_slicerConfigWidgets[key] = new Palapeli::SlicerConfigWidget(slicer);
		m_slicerSelector->addItem(offer->name(), QVariant(key)); //key is stored as user data
	}
	//build sublayouts
	QFormLayout* basicLayout = new QFormLayout;
	basicLayout->addRow(i18n("Image file:"), m_imageSelector);
	basicLayout->addRow(i18n("Puzzle type:"), m_slicerSelector);
	QFormLayout* metadataLayout = new QFormLayout;
	metadataLayout->addRow(i18n("Puzzle name:"), m_nameEdit);
	metadataLayout->addRow(i18nc("Like in: This is an optional comment.", "Optional comment:"), m_commentEdit);
	metadataLayout->addRow(i18n("Name of image author:"), m_authorEdit);
	QMapIterator<QString, Palapeli::SlicerConfigWidget*> i(m_slicerConfigWidgets);
	while (i.hasNext())
		m_slicerConfigLayout->addWidget(i.next().value());
	selectSlicerConfigWidget(m_slicerSelector->currentIndex());
	//build group boxes
	QGroupBox* basicBox = new QGroupBox(i18n("Basic slicing options"));
	basicBox->setLayout(basicLayout);
	QGroupBox* metadataBox = new QGroupBox(i18n("Puzzle information"));
	metadataBox->setLayout(metadataLayout);
	QGroupBox* slicerConfigBox = new QGroupBox(i18n("Advanced slicing options"));
	slicerConfigBox->setLayout(m_slicerConfigLayout);
	//build main layout
	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainLayout->addWidget(basicBox);
	mainLayout->addWidget(slicerConfigBox);
	mainLayout->addWidget(metadataBox);
	mainLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding));
	mainLayout->setMargin(0);
	mainWidget()->setLayout(mainLayout);
	//wire up stuff
	connect(this, SIGNAL(okClicked()), this, SLOT(createPuzzle()));
	connect(m_imageSelector, SIGNAL(urlSelected(const KUrl&)), this, SLOT(checkData()));
	connect(m_slicerSelector, SIGNAL(currentIndexChanged(int)), this, SLOT(selectSlicerConfigWidget(int)));
	connect(m_nameEdit, SIGNAL(textChanged(const QString&)), this, SLOT(checkData()));
	connect(m_authorEdit, SIGNAL(textChanged(const QString&)), this, SLOT(checkData()));
}

Palapeli::Puzzle* Palapeli::PuzzleCreatorDialog::result() const
{
	return m_result;
}

void Palapeli::PuzzleCreatorDialog::checkData()
{
	bool enableOk = !m_slicerConfigWidgets.isEmpty();
	enableOk &= !m_imageSelector->url().isEmpty();
	enableOk &= !m_nameEdit->text().isEmpty();
	enableOk &= !m_authorEdit->text().isEmpty();
	enableButton(KDialog::Ok, enableOk);
}

void Palapeli::PuzzleCreatorDialog::selectSlicerConfigWidget(int index)
{
	const QByteArray slicerKey = m_slicerSelector->itemData(index).toByteArray();
	Palapeli::SlicerConfigWidget* scWidget = m_slicerConfigWidgets[slicerKey];
	m_slicerConfigLayout->setCurrentWidget(scWidget);
}

void Palapeli::PuzzleCreatorDialog::createPuzzle()
{
	if (m_result)
		return; //We won't create the puzzle if there is one already.
	//assemble data for slicer run
	const QByteArray slicerKey = m_slicerSelector->itemData(m_slicerSelector->currentIndex()).toByteArray();
	Pala::Slicer* slicer = m_slicers[slicerKey];
	if (!slicer)
	{
		KMessageBox::sorry(this, i18n("Puzzle cannot be created: The slicer plugin could not be loaded."));
		return;
	}
	QMap<QByteArray, QVariant> slicerArgs = m_slicerConfigWidgets[slicerKey]->arguments();
	QImage image;
	if (!image.load(m_imageSelector->url().path())) //TODO: allow to load remote images
	{
		KMessageBox::sorry(this, i18n("Puzzle cannot be created: The file you selected is not an image."));
		return;
	}
	Pala::SlicerJob job(image, slicerArgs);
	if (!slicer->process(&job))
	{
		KMessageBox::sorry(this, i18n("Puzzle cannot be created: Slicing failed because of undetermined problems."));
		return;
	}
	//create puzzle structs
	Palapeli::PuzzleMetadata* metadata = new Palapeli::PuzzleMetadata;
	metadata->name = m_nameEdit->text();
	metadata->comment = m_commentEdit->text();
	metadata->author = m_authorEdit->text();
	metadata->pieceCount = job.pieces().count();
	metadata->image = image;
	metadata->thumbnail = image.scaled(Palapeli::Puzzle::ThumbnailBaseSize, Qt::KeepAspectRatio);
	metadata->modifyProtection = false; //only enabled for puzzles in the default collection (see ListCollection::canDeletePuzzle)
	Palapeli::PuzzleContents* contents = new Palapeli::PuzzleContents;
	contents->imageSize = image.size();
	QMapIterator<int, QImage> pieceIter(job.pieces());
	while (pieceIter.hasNext())
	{
		pieceIter.next();
		contents->pieces[pieceIter.key()] = QPixmap::fromImage(pieceIter.value());
	}
	contents->pieceOffsets = job.pieceOffsets();
	contents->relations = job.relations();
	Palapeli::PuzzleCreationContext* creationContext = new Palapeli::PuzzleCreationContext;
	creationContext->usedSlicer = slicerKey;
	creationContext->usedSlicerArgs = slicerArgs;
	creationContext->pieces = job.pieces();
	//create puzzle
	m_result = new Palapeli::Puzzle(metadata, contents, creationContext);
}

#include "puzzlecreator.moc"
