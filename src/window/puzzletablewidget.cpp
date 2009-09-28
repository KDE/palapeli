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

#include "puzzletablewidget.h"
#include "../engine/view.h"
#include "../engine/scene.h"

#include <QProgressBar>
#include <QVBoxLayout>
#include <KActionCollection>
#include <KLocalizedString>
#include <KStandardAction>

//BEGIN Palapeli::TextProgressBar

namespace Palapeli
{
	class TextProgressBar : public QProgressBar
	{
		public:
			TextProgressBar(QWidget* parent = 0) : QProgressBar(parent) {}

			virtual QString text() const { return m_text; }
			void setText(const QString& text) { m_text = text; update(); }
		private:
			QString m_text;
	};
}

//END Palapeli::TextProgressBar

Palapeli::PuzzleTableWidget::PuzzleTableWidget()
	: Palapeli::TabWindow(QLatin1String("palapeli-puzzletable"))
	, m_view(new Palapeli::View)
	, m_progressBar(new Palapeli::TextProgressBar(this))
{
	//setup actions
	KStandardAction::zoomIn(m_view, SLOT(zoomIn()), actionCollection());
	KStandardAction::zoomOut(m_view, SLOT(zoomOut()), actionCollection());
	setupGUI();
	//setup widgets
	m_progressBar->setText(i18n("No puzzle loaded"));
	connect(m_view->scene(), SIGNAL(reportProgress(int, int)), this, SLOT(reportProgress(int, int)));
	//setup layout
	QWidget* container = new QWidget;
	QVBoxLayout* layout = new QVBoxLayout;
	layout->addWidget(m_view);
	layout->addWidget(m_progressBar);
	layout->setMargin(0);
	container->setLayout(layout);
	setCentralWidget(container);
}

void Palapeli::PuzzleTableWidget::reportProgress(int pieceCount, int partCount)
{
	if (m_progressBar->minimum() != 0)
		m_progressBar->setMinimum(0);
	if (m_progressBar->maximum() != pieceCount - 1)
		m_progressBar->setMaximum(pieceCount - 1);
	const int value = pieceCount - partCount;
	if (m_progressBar->value() != value)
	{
		m_progressBar->setValue(value);
		if (partCount == 1)
			m_progressBar->setText(i18n("You finished the puzzle."));
		else
		{
			int percentFinished = qreal(value) / qreal(pieceCount - 1) * 100;
			m_progressBar->setText(i18nc("Progress display", "%1% finished", percentFinished));
		}
	}
}

void Palapeli::PuzzleTableWidget::loadPuzzle(Palapeli::PuzzleReader* puzzle)
{
	m_view->scene()->loadPuzzle(puzzle);
}

#include "puzzletablewidget.moc"
