/*
    SPDX-FileCopyrightText: 2009, 2010 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "puzzlecreator.h"
#include "slicerconfwidget.h"
#include "file-io/components.h"
#include "file-io/puzzle.h"
#include "file-io/puzzlestructs.h"

#include <Pala/Slicer>
#include <Pala/SlicerJob>
#include <Pala/SlicerMode>

#include <QFormLayout>
#include <QLabel>
#include <QStackedWidget>
#include <QUuid>
#include <QImageReader>

#include <KLineEdit>
#include <KLocalizedString>
#include <KMessageBox>
#include <KUrlRequester>

Palapeli::PuzzleCreatorDialog::PuzzleCreatorDialog()
	: m_result(nullptr)
	, m_imageSelector(new KUrlRequester)
	, m_nameEdit(new KLineEdit)
	, m_commentEdit(new KLineEdit)
	, m_authorEdit(new KLineEdit)
	, m_slicerSelector(new Palapeli::SlicerSelector)
	, m_slicerConfigMasterWidget(new QStackedWidget)
{
	//setup dialog
	setWindowTitle(i18nc("@title:window", "Create new puzzle"));
	buttonBox()->button(QDialogButtonBox::Help)->setVisible(false);
	//setup image selector
	m_imageSelector->setMode(KFile::File | KFile::LocalOnly | KFile::ExistingOnly);
	const auto supportedImageTypes = QImageReader::supportedMimeTypes();
	QStringList mimeTypeFilters;
	mimeTypeFilters.reserve(supportedImageTypes.size());
	for (const auto& imageType : supportedImageTypes) {
	    mimeTypeFilters.append(QString::fromUtf8(imageType));
	}
	m_imageSelector->setMimeTypeFilters(mimeTypeFilters);
	//build sublayouts
	QFormLayout* sourceLayout = new QFormLayout;
	sourceLayout->addRow(i18nc("@label:chooser", "Image file:"), m_imageSelector);
	sourceLayout->addRow(new QLabel(i18nc("@info", "Please describe below the image which you have chosen.")));
	sourceLayout->addRow(i18nc("@label:textbox", "Image name:"), m_nameEdit);
	sourceLayout->addRow(i18nc("@label:textbox (like in: This comment is optional.)", "Optional comment:"), m_commentEdit);
	sourceLayout->addRow(i18nc("@label:textbox", "Name of image author:"), m_authorEdit);
        const auto slicers = m_slicerSelector->slicers();
	for (const Pala::Slicer* slicer : slicers)
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
	connect(this, &PuzzleCreatorDialog::accepted, this, &PuzzleCreatorDialog::createPuzzle);
	connect(m_imageSelector, &KUrlRequester::urlSelected, this, &PuzzleCreatorDialog::checkData);
	connect(m_nameEdit, &KLineEdit::textChanged, this, &PuzzleCreatorDialog::checkData);
	connect(m_authorEdit, &KLineEdit::textChanged, this, &PuzzleCreatorDialog::checkData);
	checkData(); //to invalidate first page
	setValid(m_slicerPage, false);
	connect(m_slicerSelector, &Palapeli::SlicerSelector::currentSelectionChanged, this, &PuzzleCreatorDialog::updateSlicerSelection);
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
	//assemble data for creation context
	const Palapeli::SlicerSelection selection = m_slicerSelector->currentSelection();
	const Pala::Slicer* slicer = selection.slicer;
	if (!slicer)
	{
		KMessageBox::sorry(this, i18n("Puzzle cannot be created: The slicer plugin could not be loaded."));
		return;
	}
	QMap<QByteArray, QVariant> slicerArgs = m_slicerConfigWidgets[selection.slicer]->arguments();
	QImage image;
	if (!image.load(m_imageSelector->url().toLocalFile()))
	{
		KMessageBox::sorry(this, i18n("Puzzle cannot be created: The file you selected is not an image."));
		return;
	}
	image = image.convertToFormat(image.hasAlphaChannel() ? QImage::Format_ARGB32 : QImage::Format_RGB32);
	Pala::SlicerJob job(image, slicerArgs);
	job.setMode(selection.mode);
	if (!const_cast<Pala::Slicer*>(slicer)->process(&job)) //BIC: make Pala::Slicer::process() and run() const
	{
		KMessageBox::sorry(this, i18n("Puzzle cannot be created: Slicing failed because of undetermined problems."));
		return;
	}
	//create puzzle creation context
	Palapeli::PuzzleCreationContext creationContext;
	creationContext.name = m_nameEdit->text();
	creationContext.comment = m_commentEdit->text();
	creationContext.author = m_authorEdit->text();
	creationContext.pieceCount = job.pieces().count();
	creationContext.image = image;
	creationContext.thumbnail = image.scaled(Palapeli::PuzzleMetadata::ThumbnailBaseSize, Qt::KeepAspectRatio);
	creationContext.modifyProtection = false; //only enabled for default puzzles
	creationContext.slicer = selection.slicerPluginName;
	creationContext.slicerMode = selection.mode ? selection.mode->key() : QByteArray();
	creationContext.slicerArgs = slicerArgs;
	//create puzzle
	m_result = new Palapeli::Puzzle(new Palapeli::CreationContextComponent(creationContext), QString(), QUuid::createUuid().toString());
}


