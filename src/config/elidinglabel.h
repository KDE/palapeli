/*
    SPDX-FileCopyrightText: 2010 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

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
