/***************************************************************************
 *   Copyright 2014 Ian Wadham <iandw.au@gmail.com>
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

#ifndef PALAPELI_PIECEHOLDER_H
#define PALAPELI_PIECEHOLDER_H

#include "../engine/view.h"

namespace Palapeli
{
	class Piece;

	/**
	 * Objects of this class are small windows that hold pieces temporarily
	 * while a large Palapeli jigsaw puzzle is being solved. There may be
	 * any number of such windows, including none in small puzzles. The
	 * pieces in a holder will usually have something in common, as decided
	 * by the user. For example, they might represent sky, skyline, water or
	 * other parts of the picture. In any large puzzle, there is a default
	 * holder called "Hand", which represents a player collecting pieces in
	 * his or her hand, then moving to the solution area to place them.
	 *
	 * The class has methods to assist in collecting and organizing pieces.
	 */

	class PieceHolder : public View
	{
		Q_OBJECT
		public:
			PieceHolder(const QSizeF& pieceArea, const QString& title);
			void receivePieces(QList<Piece*> pieces);
			void releasePieces(QList<Piece*> pieces);
			void repackPieces(QRectF& rect); // Belongs in scene()?
			void setSelected(bool onOff);
		protected:
			virtual void focusInEvent(QFocusEvent* e);
		Q_SIGNALS:
			void selected(PieceHolder* h);
	};
}

#endif // PALAPELI_PIECEHOLDER_H
