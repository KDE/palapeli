/*
    SPDX-FileCopyrightText: 2014 Ian Wadham <iandw.au@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PALAPELI_PIECEHOLDER_H
#define PALAPELI_PIECEHOLDER_H

#include "../engine/view.h"

class QCloseEvent;

namespace Palapeli
{
	class Scene;
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
			PieceHolder(QWidget* parent, const QSizeF& pieceArea,
					const QString& title);
			void initializeZooming();
			void setSelected(bool onOff);
			QString name() { return windowTitle(); }
		protected:
			void focusInEvent(QFocusEvent* e) override;
			void closeEvent(QCloseEvent* event) override;
		Q_SIGNALS:
			void selected(PieceHolder* h);
			void closing(PieceHolder* h);
		private:
			Scene* m_scene;
	};
}

#endif // PALAPELI_PIECEHOLDER_H
