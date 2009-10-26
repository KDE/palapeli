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
#include "loadingwidget.h"
#include "../engine/scene.h"
#include "../engine/view.h"
#include "../engine/zoomwidget.h"

#include <QGridLayout>
#include <QProgressBar>
#include <QStackedWidget>
#include <KAction>
#include <KActionCollection>
#include <KLocalizedString>

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
	, m_stack(new QStackedWidget)
	, m_loadingWidget(new Palapeli::LoadingWidget)
	, m_view(new Palapeli::View)
	, m_progressBar(new Palapeli::TextProgressBar(this))
	, m_zoomWidget(new Palapeli::ZoomWidget(this))
{
	//setup actions
	KAction* restartPuzzleAct = new KAction(KIcon("document-reload"), i18n("&Restart puzzle..."), 0);
	restartPuzzleAct->setToolTip(i18n("Delete the saved progress"));
	actionCollection()->addAction("game_restart", restartPuzzleAct);
	connect(restartPuzzleAct, SIGNAL(triggered()), m_view->scene(), SLOT(restartPuzzle()));
	setupGUI();
	//setup progress bar
	m_progressBar->setText(i18n("No puzzle loaded"));
	connect(m_view->scene(), SIGNAL(reportProgress(int, int)), this, SLOT(reportProgress(int, int)));
	//setup zoom widget
	m_zoomWidget->setEnabled(false); //will be enabled when a puzzle is loaded
	connect(m_zoomWidget, SIGNAL(levelChanged(int)), m_view, SLOT(zoomTo(int)));
	connect(m_zoomWidget, SIGNAL(zoomInRequest()), m_view, SLOT(zoomIn()));
	connect(m_zoomWidget, SIGNAL(zoomOutRequest()), m_view, SLOT(zoomOut()));
	connect(m_view, SIGNAL(zoomLevelChanged(int)), m_zoomWidget, SLOT(setLevel(int)));
	connect(m_zoomWidget, SIGNAL(constrainedChanged(bool)), m_view->scene(), SLOT(setConstrained(bool)));
	connect(m_view->scene(), SIGNAL(constrainedChanged(bool)), m_zoomWidget, SLOT(setConstrained(bool)));
	//setup widget stack
	m_stack->addWidget(m_loadingWidget);
	m_stack->addWidget(m_view);
	m_stack->setCurrentWidget(m_loadingWidget);
	//setup layout
	QWidget* container = new QWidget;
	QGridLayout* layout = new QGridLayout;
	layout->addWidget(m_stack, 0, 0, 1, 2);
	layout->addWidget(m_progressBar, 1, 0);
	layout->addWidget(m_zoomWidget, 1, 1);
	layout->setColumnStretch(0, 10);
	layout->setMargin(0);
	container->setLayout(layout);
	setCentralWidget(container);
}

Palapeli::View* Palapeli::PuzzleTableWidget::view() const
{
	return m_view;
}

void Palapeli::PuzzleTableWidget::reportProgress(int pieceCount, int partCount)
{
	m_zoomWidget->setEnabled(pieceCount > 0); //zoom does not work reliably when no puzzle is loaded
	if (m_progressBar->minimum() != 0)
		m_progressBar->setMinimum(0);
	if (m_progressBar->maximum() != pieceCount - 1)
		m_progressBar->setMaximum(pieceCount - 1);
	if (m_progressBar->minimum() == m_progressBar->maximum())
		m_progressBar->setMaximum(m_progressBar->minimum());
	else
	{
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
	if (pieceCount > 0)
		m_stack->setCurrentWidget(m_view);
	else
		m_stack->setCurrentWidget(m_loadingWidget);
}

#include "puzzletablewidget.moc"
