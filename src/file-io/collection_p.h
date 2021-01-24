/*
    SPDX-FileCopyrightText: 2009-2011 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PALAPELI_COLLECTION_P_H
#define PALAPELI_COLLECTION_P_H

#include "collection.h"

class Palapeli::Collection::Item : public QObject, public QStandardItem
{
	Q_OBJECT
	public:
		Item(Palapeli::Puzzle* puzzle);

		Palapeli::Puzzle* puzzle() const { return m_puzzle; }
	public Q_SLOTS:
		void populate();
	private:
		Palapeli::Puzzle* m_puzzle;
};

#endif // PALAPELI_COLLECTION_P_H
