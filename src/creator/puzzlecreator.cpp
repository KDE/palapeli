/***************************************************************************
 *   Copyright 2009, 2010 Stefan Majewsky <majewsky@gmx.net>
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
#include <QLabel>
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
	, m_nameEdit(new KLineEdit)
	, m_commentEdit(new KLineEdit)
	, m_authorEdit(new KLineEdit)
	, m_slicerSelector(new KComboBox)
	, m_slicerConfigLayout(new QStackedLayout)
{
	//setup dialog
	setCaption(i18nc("@title:window", "Create new puzzle"));
	showButton(KDialog::Help, false);
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
	QFormLayout* sourceLayout = new QFormLayout;
	sourceLayout->addRow(i18nc("@label:chooser", "Image file:"), m_imageSelector);
	sourceLayout->addRow(new QLabel(i18nc("@info", "Please describe below the image which you have chosen.")));
	sourceLayout->addRow(i18nc("@label:textbox", "Image name:"), m_nameEdit);
	sourceLayout->addRow(i18nc("@label:textbox (like in: This comment is optional.)", "Optional comment:"), m_commentEdit);
	sourceLayout->addRow(i18nc("@label:textbox", "Name of image author:"), m_authorEdit);
	QFormLayout* slicerLayout = new QFormLayout;
	slicerLayout->addRow(i18n("Puzzle type:"), m_slicerSelector);
	QMapIterator<QString, Palapeli::SlicerConfigWidget*> i(m_slicerConfigWidgets);
	while (i.hasNext())
		m_slicerConfigLayout->addWidget(i.next().value());
	selectSlicerConfigWidget(m_slicerSelector->currentIndex());
	//build page widget items
	m_sourcePage = addPage(new QWidget, i18nc("@item:inlistbox (page name in an assistant dialog)", "Choose image"));
	m_sourcePage->setHeader(i18nc("@title:tab (page header in an assistant dialog)", "Specify the source image to be sliced into pieces"));
	m_sourcePage->widget()->setLayout(sourceLayout);
	m_slicerPage = addPage(new QWidget, i18nc("@item:inlistbox (page name in an assistant dialog)", "Choose slicer"));
	m_slicerPage->setHeader(i18nc("@title:tab (page header in an assistant dialog)", "Choose a slicing method"));
	m_slicerPage->widget()->setLayout(slicerLayout);
	m_slicerConfigPage = addPage(new QWidget, i18nc("@item:inlistbox (page name in an assistant dialog)", "Configure slicer"));
	m_slicerConfigPage->setHeader(i18nc("@title:tab (page header in an assistant dialog)", "Tweak the parameters of the chosen slicing method"));
	m_slicerConfigPage->widget()->setLayout(m_slicerConfigLayout);
	//wire up stuff
	connect(this, SIGNAL(accepted()), this, SLOT(createPuzzle()));
	connect(m_imageSelector, SIGNAL(urlSelected(const KUrl&)), this, SLOT(checkData()));
	connect(m_slicerSelector, SIGNAL(currentIndexChanged(int)), this, SLOT(selectSlicerConfigWidget(int)));
	connect(m_nameEdit, SIGNAL(textChanged(const QString&)), this, SLOT(checkData()));
	connect(m_authorEdit, SIGNAL(textChanged(const QString&)), this, SLOT(checkData()));
	checkData(); //to invalidate first page
}

Palapeli::Puzzle* Palapeli::PuzzleCreatorDialog::result() const
{
	return m_result;
}

void Palapeli::PuzzleCreatorDialog::checkData()
{
	bool sourceValid = !m_imageSelector->url().isEmpty();
	sourceValid &= !m_nameEdit->text().isEmpty();
	sourceValid &= !m_authorEdit->text().isEmpty();
	setValid(m_sourcePage, sourceValid);
	setValid(m_slicerPage, !m_slicers.isEmpty());
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
	if (!image.load(m_imageSelector->url().toLocalFile())) //TODO: allow to load remote images
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
