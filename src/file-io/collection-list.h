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

#ifndef PALAPELI_COLLECTION_LIST_H
#define PALAPELI_COLLECTION_LIST_H

#include "collection.h"

#include <QStringList>
class KConfig;
class KJob;
#include <KUrl>

namespace Palapeli
{
	class ListCollection : public Palapeli::Collection
	{
		Q_OBJECT
		public:
			ListCollection(const KUrl& url);
			virtual ~ListCollection();

			virtual bool canImportPuzzles() const;
			virtual QModelIndex importPuzzle(const Palapeli::Puzzle* const puzzle);
			virtual bool canDeletePuzzle(const QModelIndex& index) const;
			virtual bool deletePuzzle(const QModelIndex& index);
			QModelIndex storeGeneratedPuzzle(Palapeli::Puzzle* puzzle);
		protected:
			ListCollection(); //interface to subclasses
			void setConfig(KConfig* config);
		private Q_SLOTS:
			void collectionDataCopyFinished(KJob* job);
		private:
			KUrl readUrl(const KUrl& url) const;
			QModelIndex addPuzzleInternal(Palapeli::Puzzle* puzzle, const QString& identifier);
			QModelIndex importPuzzleInternal(Palapeli::Puzzle* puzzle);

			QStringList m_features;
			KConfig* m_config;
	};

	class LocalCollection : public Palapeli::ListCollection
	{
		public:
			LocalCollection();
	};
}

#endif // PALAPELI_COLLECTION_LIST_H
