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
#include "../libpala/slicer.h"
#include "../libpala/slicerjob.h"

#include <QBoxLayout>
#include <QFormLayout>
#include <QStackedLayout>
#include <KComboBox>
#include <KLineEdit>
#include <KLocalizedString>
#include <KServiceTypeTrader>
#include <KUrlRequester>

Palapeli::PuzzleCreatorDialog::PuzzleCreatorDialog()
	: KDialog(new KDialog)
	, m_result(0)
	, m_imageSelector(new KUrlRequester)
	, m_slicerSelector(new KComboBox)
	, m_nameEdit(new KLineEdit)
	, m_commentEdit(new KLineEdit)
	, m_authorEdit(new KLineEdit)
{
	//setup dialog
	setCaption(i18n("Create new puzzle"));
	setButtons(KDialog::Ok | KDialog::Cancel);
	setButtonIcon(KDialog::Ok, KIcon("tools-wizard")); //FIXME: This should be a custom "actions/puzzle-new" icon.
	setButtonText(KDialog::Ok, i18n("Create puzzle"));
	setButtonToolTip(KDialog::Ok, i18n("Create a new puzzle with the given information, and save it in my collection"));
	//setup image selector
	m_imageSelector->setMode(KFile::File | KFile::ExistingOnly);
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
	QStackedLayout* slicerConfigLayout = new QStackedLayout;
	QMapIterator<QString, Palapeli::SlicerConfigWidget*> i(m_slicerConfigWidgets);
	while (i.hasNext())
		slicerConfigLayout->addWidget(i.next().value());
	//build group boxes
	QGroupBox* basicBox = new QGroupBox(i18n("Basic slicing options"));
	basicBox->setLayout(basicLayout);
	QGroupBox* metadataBox = new QGroupBox(i18n("Puzzle information"));
	metadataBox->setLayout(metadataLayout);
	QGroupBox* slicerConfigBox = new QGroupBox(i18n("Advanced slicing options"));
	slicerConfigBox->setLayout(slicerConfigLayout);
	//build main layout
	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainLayout->addWidget(basicBox);
	mainLayout->addWidget(slicerConfigBox);
	mainLayout->addWidget(metadataBox);
	mainLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding));
	mainWidget()->setLayout(mainLayout);
}

Palapeli::Puzzle* Palapeli::PuzzleCreatorDialog::result() const
{
	return m_result;
}
