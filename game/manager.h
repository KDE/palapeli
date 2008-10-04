/***************************************************************************
 *   Copyright (C) 2008 Felix Lemke <lemke.felix@ages-skripte.org>
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

#ifndef PALAPELI_MANAGER_H
#define PALAPELI_MANAGER_H

class QImage;
#include <QObject>
class QPointF;
class QRectF;
class KUrl;

namespace Palapeli
{

	class Library;
	class MainWindow;
	class ManagerPrivate;
	class Minimap;
	class Part;
	class Piece;
	class PieceRelation;
	class Preview;
	struct PuzzleInfo;
	class SavegameModel;
	class View;

	class Manager : public QObject
	{
		Q_OBJECT
		public:
			static Manager* self();
			void init();

			void removePart(Part* part);
			//core objects (i.e. everything which is immediately relevant to gameplay)
			int partCount() const;
			Part* partAt(int index) const;
			int pieceCount() const;
			Piece* pieceAt(int index) const;
			int relationCount() const;
			PieceRelation relationAt(int index) const;
			View* view() const;
			//other objects (mostly user interface)
			Library* library() const;
			Minimap* minimap() const;
			Preview* preview() const;
			SavegameModel* savegameModel() const;
			MainWindow* window() const;
		public Q_SLOTS:
			void pieceMoveFinished();
			void searchConnections();
			void updateGraphics();

			void loadGame(Palapeli::PuzzleInfo* info);
			void deleteGame(const QString& name) { Q_UNUSED(name) } //deprecated
		protected Q_SLOTS:
			void estimatePieceCount(int pieceCount);
			void addPiece(const QImage& image, const QRectF& positionInImage, const QPointF& sceneBasePosition);
			void addPiece(const QImage& baseImage, const QImage& mask, const QRectF& positionInImage, const QPointF& sceneBasePosition);
			void addPiece(Palapeli::Piece* piece, const QPointF& sceneBasePosition);
			void endAddPiece();
			void addRelation(int piece1Id, int piece2Id);
		private Q_SLOTS:
			void finishGameLoading();
		Q_SIGNALS:
			void gameNameChanged(const QString& name);
			void savegameCreated(const QString& name);
			void savegameDeleted(const QString& name);
			void interactionModeChanged(bool gameInteractionAvailable);
		private:
			Manager();
			~Manager();
			Q_DISABLE_COPY(Manager)

			ManagerPrivate* const p;
	};

}

//abbreviation for Palapeli::Manager::self()
inline Palapeli::Manager* ppMgr() { return Palapeli::Manager::self(); }

#endif //PALAPELI_MANAGER_H
