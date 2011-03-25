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

#include "collection-view.h"
#include "collection.h"
#include "collection-delegate.h"
#include "puzzle.h"

#include <QApplication>
#include <QGridLayout>
#include <QMenu>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <KAction>
#include <KFilterProxySearchLine>
#include <KLocale>

Palapeli::CollectionView::CollectionView(QWidget* parent)
	: QWidget(parent)
	, m_view(new QListView(this))
	, m_delegate(new Palapeli::CollectionDelegate(m_view))
	, m_proxyModel(new QSortFilterProxyModel(this))
{
	//setup view
	connect(m_view, SIGNAL(activated(const QModelIndex&)), this, SLOT(handleActivated(const QModelIndex&)));
	m_view->setAlternatingRowColors(true);
	m_view->setMouseTracking(true);
	m_view->setSelectionMode(QAbstractItemView::ExtendedSelection);
	//setup proxy model
	m_view->setModel(m_proxyModel);
	m_proxyModel->setDynamicSortFilter(true);
	m_proxyModel->sort(Qt::DisplayRole, Qt::AscendingOrder);
	//TODO: save sorting role between sessions
	//setup filter search line
	KFilterProxySearchLine* searchLine = new KFilterProxySearchLine(this);
	searchLine->setProxy(m_proxyModel);
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
	connect(sortMenu, SIGNAL(triggered(QAction*)), SLOT(sortMenuTriggered(QAction*)));
	//construct layout
	QGridLayout* layout = new QGridLayout(this);
	layout->addWidget(sortButton, 0, 0);
	layout->addWidget(searchLine, 0, 1);
	layout->addWidget(m_view, 1, 0, 1, 2);
}

void Palapeli::CollectionView::setModel(QAbstractItemModel* model)
{
	m_proxyModel->setSourceModel(model);
}

QItemSelectionModel* Palapeli::CollectionView::selectionModel() const
{
	return m_view->selectionModel();
}

//NOTE The QAbstractItemView::activated signal honors the mouseclick selection behavior defined by the user (e.g. one-click is default on Linux, while two-click is default on Windows).
void Palapeli::CollectionView::handleActivated(const QModelIndex& index)
{
	//do not emit a play request when the Control modifier is pressed (without this rule, users with one-click activation could not ever select multiple puzzles at once)
	if (QApplication::keyboardModifiers() & Qt::ControlModifier)
		return;
	//change selection to indicate that the given puzzle has been chosen
	m_view->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
	emit playRequest(Palapeli::Collection::instance()->puzzleFromIndex(index));
}

void Palapeli::CollectionView::sortMenuTriggered(QAction* action)
{
	//find out what was requested
	int sortRole = Qt::DisplayRole; //corresponds to action == m_sortByTitle
	if (action == m_sortByPieceCount)
		sortRole = Palapeli::Collection::PieceCountRole;
	//update sorting and menu
	m_proxyModel->sort(sortRole, Qt::AscendingOrder);
	m_sortByTitle->setChecked(sortRole == Qt::DisplayRole);
	m_sortByPieceCount->setChecked(sortRole == Palapeli::Collection::PieceCountRole);
}

#include "collection-view.moc"
