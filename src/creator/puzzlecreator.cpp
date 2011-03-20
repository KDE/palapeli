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
#include "slicerselector.h"
#include "file-io/puzzle.h"
#include "file-io/puzzlestructs.h"
#include "../libpala/slicer.h"
#include "../libpala/slicerjob.h"
#include "../libpala/slicermode.h"

#include <QBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QStackedWidget>
#include <KComboBox>
#include <KLineEdit>
#include <KLocalizedString>
#include <KMessageBox>
#include <KUrlRequester>

Palapeli::PuzzleCreatorDialog::PuzzleCreatorDialog()
	: m_result(0)
	, m_imageSelector(new KUrlRequester)
	, m_nameEdit(new KLineEdit)
	, m_commentEdit(new KLineEdit)
	, m_authorEdit(new KLineEdit)
	, m_slicerSelector(new Palapeli::SlicerSelector)
	, m_slicerConfigMasterWidget(new QStackedWidget)
{
	//setup dialog
	setCaption(i18nc("@title:window", "Create new puzzle"));
	showButton(KDialog::Help, false);
	//setup image selector
	m_imageSelector->setMode(KFile::File | KFile::LocalOnly | KFile::ExistingOnly);
	//build sublayouts
	QFormLayout* sourceLayout = new QFormLayout;
	sourceLayout->addRow(i18nc("@label:chooser", "Image file:"), m_imageSelector);
	sourceLayout->addRow(new QLabel(i18nc("@info", "Please describe below the image which you have chosen.")));
	sourceLayout->addRow(i18nc("@label:textbox", "Image name:"), m_nameEdit);
	sourceLayout->addRow(i18nc("@label:textbox (like in: This comment is optional.)", "Optional comment:"), m_commentEdit);
	sourceLayout->addRow(i18nc("@label:textbox", "Name of image author:"), m_authorEdit);
	foreach (const Pala::Slicer* slicer, m_slicerSelector->slicers())
	{
		m_slicerConfigWidgets[slicer] = new Palapeli::SlicerConfigWidget(slicer);
		m_slicerConfigMasterWidget->addWidget(m_slicerConfigWidgets[slicer]);
	}
	//build page widget items
	m_sourcePage = addPage(new QWidget, i18nc("@item:inlistbox (page name in an assistant dialog)", "Choose image"));
	m_sourcePage->setHeader(i18nc("@title:tab (page header in an assistant dialog)", "Specify the source image to be sliced into pieces"));
	m_sourcePage->widget()->setLayout(sourceLayout);
	m_slicerPage = addPage(m_slicerSelector, i18nc("@item:inlistbox (page name in an assistant dialog)", "Choose slicer"));
	m_slicerPage->setHeader(i18nc("@title:tab (page header in an assistant dialog)", "Choose a slicing method"));
	m_slicerConfigPage = addPage(m_slicerConfigMasterWidget, i18nc("@item:inlistbox (page name in an assistant dialog)", "Configure slicer"));
	m_slicerConfigPage->setHeader(i18nc("@title:tab (page header in an assistant dialog)", "Tweak the parameters of the chosen slicing method"));
	//wire up stuff
	connect(this, SIGNAL(accepted()), this, SLOT(createPuzzle()));
	connect(m_imageSelector, SIGNAL(urlSelected(const KUrl&)), this, SLOT(checkData()));
	connect(m_nameEdit, SIGNAL(textChanged(const QString&)), this, SLOT(checkData()));
	connect(m_authorEdit, SIGNAL(textChanged(const QString&)), this, SLOT(checkData()));
	checkData(); //to invalidate first page
	connect(m_slicerSelector, SIGNAL(currentSelectionChanged(Palapeli::SlicerSelection)), this, SLOT(updateSlicerSelection(Palapeli::SlicerSelection)));
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
}

void Palapeli::PuzzleCreatorDialog::updateSlicerSelection(const Palapeli::SlicerSelection& selection)
{
	setValid(m_slicerPage, (bool) selection.slicer);
	if (!selection.slicer) return;
	//update slicer configuration widget
	Palapeli::SlicerConfigWidget* scWidget = m_slicerConfigWidgets.value(selection.slicer);
	scWidget->setMode(selection.mode);
	m_slicerConfigMasterWidget->setCurrentWidget(scWidget);
}

void Palapeli::PuzzleCreatorDialog::createPuzzle()
{
	if (m_result)
		return; //We won't create the puzzle if there is one already.
	//assemble data for slicer run
	const Palapeli::SlicerSelection selection = m_slicerSelector->currentSelection();
	const Pala::Slicer* slicer = selection.slicer;
	if (!slicer)
	{
		KMessageBox::sorry(this, i18n("Puzzle cannot be created: The slicer plugin could not be loaded."));
		return;
	}
	QMap<QByteArray, QVariant> slicerArgs = m_slicerConfigWidgets[selection.slicer]->arguments();
	QImage image;
	if (!image.load(m_imageSelector->url().toLocalFile())) //TODO: allow to load remote images
	{
		KMessageBox::sorry(this, i18n("Puzzle cannot be created: The file you selected is not an image."));
		return;
	}
	Pala::SlicerJob job(image, slicerArgs);
	job.setMode(selection.mode);
	if (!const_cast<Pala::Slicer*>(slicer)->process(&job)) //BIC: make Pala::Slicer::process() and run() const
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
	contents->pieces = job.pieces();
	contents->pieceOffsets = job.pieceOffsets();
	contents->relations = job.relations();
	Palapeli::PuzzleCreationContext* creationContext = new Palapeli::PuzzleCreationContext;
	creationContext->slicer = selection.slicerPluginName;
	creationContext->slicerMode = selection.mode ? selection.mode->key() : QByteArray();
	creationContext->slicerArgs = slicerArgs;
	//create puzzle
	m_result = new Palapeli::Puzzle(metadata, contents, creationContext);
}

#include "puzzlecreator.moc"
