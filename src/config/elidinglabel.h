/***************************************************************************
 *   Copyright 2010 Stefan Majewsky <majewsky@gmx.net>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Library General Public
 *   License as published by the Free Software Foundation; either
 *   version 2 of the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details.
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
***************************************************************************/

#ifndef PALAPELI_ELIDINGLABEL_H
#define PALAPELI_ELIDINGLABEL_H

#include <QLabel>

namespace Palapeli
{
	class ElidingLabel : public QLabel
	{
		public:
			explicit ElidingLabel(QWidget* parent = nullptr);

			QSize minimumSizeHint() const override;
			QSize sizeHint() const override;

			QString fullText() const;
			void setFullText(const QString& text);
		protected:
			void resizeEvent(QResizeEvent* event) override;
		private:
			QString m_fullText;
	};
}

#endif // PALAPELI_ELIDINGLABEL_H
