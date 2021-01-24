/*
    SPDX-FileCopyrightText: 2009 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "collection-view.h"
#include "collection.h"
#include "collection-delegate.h"
#include "puzzle.h"

#include <QApplication>
#include <QGridLayout>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QAction>
#include <KLocalizedString>

Palapeli::CollectionView::CollectionView(QWidget* parent)
	: QWidget(parent)
	, m_view(new QListView(this))
	, m_delegate(new Palapeli::CollectionDelegate(m_view))
	, m_proxyModel(new QSortFilterProxyModel(this))
{
	//setup view
	connect(m_view, &QListView::activated, this, &CollectionView::handleActivated);

	// Set up for multi-column display of the Puzzle Collection, which
	// allows the user to see more of the collection at one time.
	m_view->setWrapping(true);
	m_view->setResizeMode(QListView::Adjust);
	m_view->setUniformItemSizes(true);
	m_view->setFlow(QListView::LeftToRight);

	// Avoid a resize loop (with the scrollbar appearing and disappearing)
	// when the number of items and display-columns hits a bad combination.
	m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

	m_view->setMouseTracking(true);
	m_view->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_view->setVerticalScrollMode(QListView::ScrollPerPixel); // Smooth.
	//setup proxy model
	m_view->setModel(m_proxyModel);
	connect(m_view->selectionModel(), &QItemSelectionModel::selectionChanged, this, &CollectionView::handleSelectionChanged);
	m_proxyModel->setDynamicSortFilter(true);
	m_proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
	m_proxyModel->setSortLocaleAware(true);
	m_proxyModel->setSortRole(Qt::DisplayRole);
	m_proxyModel->sort(0, Qt::AscendingOrder);
	//TODO: save sorting role between sessions
	//setup filter search line
	QLineEdit* searchLine = new QLineEdit(this);
	searchLine->setClearButtonEnabled(true);
	connect(searchLine, &QLineEdit::textChanged, this, &Palapeli::CollectionView::slotTextChanged);
	//setup sort button
	QPushButton* sortButton = new QPushButton(i18nc("@action:button that pops up sorting strategy selection menu", "Sort list..."), this);
	QMenu* sortMenu = new QMenu(sortButton);
	sortButton->setMenu(sortMenu);
	m_sortByTitle = sortMenu->addAction(i18nc("@action:inmenu selects sorting strategy for collection list", "By title"));
	m_sortByPieceCount = sortMenu->addAction(i18nc("@action:inmenu selects sorting strategy for collection list", "By piece count"));
	m_sortByTitle->setCheckable(true);
	m_sortByPieceCount->setCheckable(true);
	m_sortByTitle->setChecked(true);
	m_sortByPieceCount->setChecked(false);
	connect(sortMenu, &QMenu::triggered, this, &CollectionView::sortMenuTriggered);
	//construct layout
	QGridLayout* layout = new QGridLayout(this);
	layout->addWidget(sortButton, 0, 0);
	layout->addWidget(searchLine, 0, 1);
	layout->addWidget(m_view, 1, 0, 1, 2);
	// Removed this because setMargin is obsolete and (0) cuts off the right
	// hand and bottom edges of the search and ListView widgets --- on Apple
	// OSX at least. The default margin is 11 pixels all round and looks OK.
	// layout->setContentsMargins(0, 0, 0, 0);
}

void Palapeli::CollectionView::slotTextChanged(const QString &str)
{
    m_proxyModel->setFilterFixedString(str);
}

void Palapeli::CollectionView::setModel(QAbstractItemModel* model)
{
	m_proxyModel->setSourceModel(model);
}

QModelIndexList Palapeli::CollectionView::selectedIndexes() const
{
	return m_view->selectionModel()->selectedIndexes();
}

//NOTE The QAbstractItemView::activated signal honors the mouseclick selection behavior defined by the user (e.g. one-click is default on Linux, while two-click is default on Windows).
void Palapeli::CollectionView::handleActivated(const QModelIndex& index)
{
	//do not emit a play request when the Control modifier is pressed (without this rule, users with one-click activation could not ever select multiple puzzles at once)
	if (QApplication::keyboardModifiers() & Qt::ControlModifier)
		return;
	//change selection to indicate that the given puzzle has been chosen
	m_view->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
	Q_EMIT playRequest(Palapeli::Collection::instance()->puzzleFromIndex(index));
}

void Palapeli::CollectionView::handleSelectionChanged()
{
	const QModelIndexList indexes = m_view->selectionModel()->selectedIndexes();
	const bool someSelection = !indexes.isEmpty();
	Q_EMIT canExportChanged(someSelection);
	for (const QModelIndex& index : indexes)
		if (!index.data(Palapeli::Collection::IsDeleteableRole).toBool())
		{
			Q_EMIT canDeleteChanged(false);
			return;
		}
	Q_EMIT canDeleteChanged(someSelection);
}

void Palapeli::CollectionView::sortMenuTriggered(QAction* action)
{
	//find out what was requested
	int sortRole = Qt::DisplayRole; //corresponds to action == m_sortByTitle
	if (action == m_sortByPieceCount)
		sortRole = Palapeli::Collection::PieceCountRole;
	//update sorting and menu
	m_proxyModel->setSortRole(sortRole);
	m_sortByTitle->setChecked(sortRole == Qt::DisplayRole);
	m_sortByPieceCount->setChecked(sortRole == Palapeli::Collection::PieceCountRole);
}


