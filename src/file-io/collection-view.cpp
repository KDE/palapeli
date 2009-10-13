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

#include <QMouseEvent>
#include <QToolButton>
#include <KIcon>

Palapeli::CollectionView::CollectionView()
	: m_delegate(new Palapeli::CollectionDelegate(this))
	, m_playButton(new QToolButton(viewport()))
{
	m_playButton->setVisible(false);
	m_playButton->setIconSize(QSize(48, 48));
	m_playButton->setIcon(KIcon("media-playback-start"));
	connect(m_playButton, SIGNAL(clicked()), this, SLOT(playButtonClicked()));
	setMouseTracking(true);
	viewport()->installEventFilter(this);
}

bool Palapeli::CollectionView::eventFilter(QObject* object, QEvent* event)
{
	//own event processing
	if (object == viewport())
	{
		if (event->type() == QEvent::MouseMove)
		{
			QPoint mousePos = static_cast<QMouseEvent*>(event)->pos();
			setHoveredIndex(indexAt(mousePos));
		}
		else if (event->type() == QEvent::Leave)
		{
			QListView::leaveEvent(event);
			setHoveredIndex(QModelIndex());
		}
	}
	//pass to base class (we do not filter any events, we just react on them)
	return QListView::eventFilter(object, event);
}

void Palapeli::CollectionView::setHoveredIndex(const QModelIndex& index)
{
	const QString identifier = index.data(Palapeli::Collection::IdentifierRole).toString();
	if (identifier == m_hoveredPuzzleIdentifier) //nothing changes
		return;
	m_hoveredPuzzleIdentifier = identifier;
	if (index.isValid())
	{
		m_playButton->setVisible(true);
		QRect thumbnailRect = m_delegate->thumbnailRect(visualRect(index));
		QSize buttonSize = m_playButton->sizeHint();
		QPoint buttonPos(
			thumbnailRect.left() + (thumbnailRect.width() - buttonSize.width()) / 2,
			thumbnailRect.top() + (thumbnailRect.height() - buttonSize.height()) / 2
		);
		m_playButton->setGeometry(QRect(buttonPos, buttonSize));
	}
	else
		m_playButton->setVisible(false);
}

void Palapeli::CollectionView::playButtonClicked()
{
	if (m_hoveredPuzzleIdentifier.isEmpty())
		return;
	//find the model index of this puzzle
	const QModelIndexList indexes = model()->match(
		model()->index(0, 0),                 //search below this index
		Palapeli::Collection::IdentifierRole, //search for identifiers...
		m_hoveredPuzzleIdentifier,            //...matching this one
		1,                                    //stop when the first one has been found
		Qt::MatchRecursive                    //descend to child items
	);
	if (!indexes.isEmpty())
		emit playRequest(indexes[0]);
}

#include "collection-view.moc"
