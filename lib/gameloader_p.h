/***************************************************************************
 *   Copyright (C) 2008 Stefan Majewsky <majewsky@gmx.net>
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

#ifndef PALAPELI_GAMELOADER_PH
#define PALAPELI_GAMELOADER_PH

#include "gameloader.h"
#include "core/engine.h"
#include "library/puzzleinfo.h"

#include <QImage>
#include <QObject>
#include <QPointF>
#include <QRectF>

namespace Palapeli
{

	class GameLoaderPrivate : public QObject
	{
		Q_OBJECT
		public:
			GameLoaderPrivate(Palapeli::Engine* engine, const Palapeli::PuzzleInfo* info, bool takeLibraryOwnership, Palapeli::GameLoader* parent);
		public Q_SLOTS:
			void addPiece(const QImage& baseImage, const QImage& mask, const QRectF& positionInImage, const QPointF& sceneBasePosition);
			void addPiece(const QImage& image, const QRectF& positionInImage, const QPointF& sceneBasePosition);
			void addRelation(int piece1Id, int piece2Id);
		public:
			bool m_isValid, m_libraryOwnership;
			Palapeli::PuzzleInfo m_info;
			Palapeli::Engine* m_engine;
	};

}

#endif // PALAPELI_GAMELOADER_PH
