/*
    SPDX-FileCopyrightText: 2009 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "puzzletablewidget.h"
#include "loadingwidget.h"
#include "../engine/scene.h"
#include "../engine/view.h"
#include "../engine/zoomwidget.h"
#include "settings.h"
#include "palapeli_debug.h"

#include <QToolBar>
#include <QGridLayout>
#include <QProgressBar>
#include <QStackedWidget>
#include <KLocalizedString>

//BEGIN Palapeli::TextProgressBar

namespace Palapeli
{
	class TextProgressBar : public QProgressBar
	{
		public:
			TextProgressBar(QWidget* parent = nullptr) : QProgressBar(parent) {}

			QString text() const override { return m_text; }
			void setText(const QString& text) { m_text = text; update(); }
		private:
			QString m_text;
	};
}

//END Palapeli::TextProgressBar

Palapeli::PuzzleTableWidget::PuzzleTableWidget()
	: m_zoomAdjustable(true)
	, m_stack(new QStackedWidget)
	, m_loadingWidget(new Palapeli::LoadingWidget)
	, m_view(new Palapeli::View)
	, m_progressBar(new Palapeli::TextProgressBar(this))
	, m_zoomWidget(new Palapeli::ZoomWidget(this))
{
	//setup progress bar
	m_progressBar->setText(i18n("No puzzle loaded"));
	//setup zoom widget
	m_zoomWidget->setLevel((View::MaximumZoomLevel+View::MinimumZoomLevel)/2);
	connect(m_zoomWidget, &Palapeli::ZoomWidget::levelChanged, m_view, &Palapeli::View::zoomSliderInput);
	connect(m_zoomWidget, &Palapeli::ZoomWidget::zoomInRequest, m_view, &Palapeli::View::zoomIn);
	connect(m_zoomWidget, &Palapeli::ZoomWidget::zoomOutRequest, m_view, &Palapeli::View::zoomOut);
	connect(m_view, &Palapeli::View::zoomLevelChanged, m_zoomWidget, &Palapeli::ZoomWidget::setLevel);
	connect(m_view, &Palapeli::View::zoomAdjustable, this, &PuzzleTableWidget::setZoomAdjustable);
	connect(m_zoomWidget, &ZoomWidget::constrainedChanged, m_view->scene(), &Scene::setConstrained);
	connect(m_view->scene(), &Scene::constrainedChanged, m_zoomWidget, &ZoomWidget::setConstrained);
	//setup widget stack
	// /* IDW test. Disable LOADING WIDGET.
	m_stack->addWidget(m_loadingWidget);
	// */
	m_stack->addWidget(m_view);
	// /* IDW test. Disable LOADING WIDGET.
	m_stack->setCurrentWidget(m_loadingWidget);
	// */
	m_stack->setCurrentWidget(m_view);

	//setup layout
	// IDW TODO - Make the background look like a toolbar? Below succeeds,
	//            but nothing gets painted on it. Try QToolBar::addWidget().
	// QToolBar* pseudoStatusBar = new QToolBar(this);
	QWidget* pseudoStatusBar = new QWidget(this);
	QHBoxLayout* barLayout = new QHBoxLayout(pseudoStatusBar);
	barLayout->addWidget(m_progressBar, 3);		// Need not be long.
	barLayout->addWidget(m_zoomWidget, 2);		// Must hold 200 steps.
	barLayout->setContentsMargins(10, 0, 10, 0);	// Margins at ends.

	QVBoxLayout* mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);	// No margins.
	mainLayout->setSpacing(0);			// No wasted gray space.
	mainLayout->addWidget(m_stack, 30);		// Max puzzle height.
	mainLayout->addWidget(pseudoStatusBar, 1);	// Min bar height.
}

Palapeli::View* Palapeli::PuzzleTableWidget::view() const
{
	return m_view;
}

void Palapeli::PuzzleTableWidget::showStatusBar(bool visible)
{
	//apply setting
	m_progressBar->setVisible(visible);
	m_zoomWidget->setVisible(visible);
	//save setting
	Settings::setShowStatusBar(visible);
	Settings::self()->save();
}

void Palapeli::PuzzleTableWidget::reportProgress(int pieceCount, int partCount)
{
	qCDebug(PALAPELI_LOG) << "PuzzleTableWidget::reportProgress(" << pieceCount << partCount;
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
		// ; /* // IDW test. Disable LOADING WIDGET.
		m_stack->setCurrentWidget(m_loadingWidget);
		// */
}

void Palapeli::PuzzleTableWidget::setZoomAdjustable(bool adjustable)
{
	m_zoomAdjustable = adjustable;
	m_zoomWidget->setVisible(Settings::showStatusBar() && adjustable);
}


