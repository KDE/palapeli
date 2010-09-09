/***************************************************************************
 *   Copyright 2009, 2010 Stefan Majewsky <majewsky@gmx.net>
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

#include "firsttimehelper.h"
#include "window/mainwindow.h"

#include <QDir>
#include <QFileInfo>
#include <QGridLayout>
#include <QLabel>
#include <QProcess>
#include <QProgressBar>
#include <QTimer>
#include <KCmdLineArgs>
#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KGlobal>
#include <KIcon>
#include <KStandardDirs>

//NOTE: A local copy of Palapeli::ListCollection::readUrl, with an extension to resolve local URLs.
static KUrl resolveUrl(const KUrl& url, bool local)
{
	if (url.protocol() == "palapeli")
	{
		//resolve special URLs with KStandardDirs
		QString path = url.path(KUrl::RemoveTrailingSlash);
		if (path.startsWith('/'))
			path.remove(0, 1);
		return KUrl(local ? KStandardDirs::locateLocal("appdata", path) : KStandardDirs::locate("appdata", path));
	}
	else
		return url;
}

Palapeli::FirstTimeHelper::FirstTimeHelper()
	: m_bar(0)
{
	//open config, read puzzles
	KConfig config("palapeli-collectionrc", KConfig::CascadeConfig);
	KConfigGroup mainGroup(&config, "Palapeli Collection");
	const QStringList puzzleIds = mainGroup.groupList();
	foreach (const QString& puzzleId, puzzleIds)
	{
		KConfigGroup puzzleGroup(&mainGroup, puzzleId);
		const KUrl barePuzzleUrl = puzzleGroup.readEntry("Location", KUrl());
		//generate name of corresponding .desktop file
		QString desktopFileName = barePuzzleUrl.fileName();
		desktopFileName.replace(QRegExp("\\.puzzle$"), QLatin1String(".desktop"));
		KUrl bareDesktopUrl(barePuzzleUrl);
		bareDesktopUrl.setFileName(desktopFileName);
		//if desktop file does not exist, we cannot do anything
		const KUrl puzzleUrl = resolveUrl(barePuzzleUrl, false);
		const KUrl desktopUrl = resolveUrl(bareDesktopUrl, false);
		if (desktopUrl.isEmpty())
			continue;
		//if puzzle file does not exist or is older than desktop file (e.g. because of translation update), schedule update
		if (puzzleUrl.isEmpty() || (QFileInfo(puzzleUrl.path()).lastModified() < QFileInfo(desktopUrl.path()).lastModified()))
		{
			const KUrl localPuzzleUrl = resolveUrl(barePuzzleUrl, true);
			Task task = { desktopUrl, localPuzzleUrl };
			m_tasks << task;
		}
	}
}

bool Palapeli::FirstTimeHelper::isNecessary() const
{
	return !m_tasks.isEmpty();
}

void Palapeli::FirstTimeHelper::execute()
{
	//create a progress dialog
	QWidget* widget = new QWidget;
	widget->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
	connect(this, SIGNAL(done()), widget, SLOT(deleteLater()));
	QGridLayout* layout = new QGridLayout;
	widget->setLayout(layout);
	QLabel* icon = new QLabel;
	QLabel* label = new QLabel(i18nc("content of first-run wizard (which behaves more like a splashscreen in fact, i.e. no user interaction)", "<title>Palapeli first-run and update wizard</title><para>Creating default puzzles...</para><para>This only needs to be done once.</para>"), widget);
// 	label2->setWordWrap(true);
	m_bar = new QProgressBar(widget);
	layout->addWidget(icon, 0, 0, 2, 1);
	layout->addWidget(label, 0, 1);
	layout->addWidget(m_bar, 1, 1);
	const int iconSize = layout->sizeHint().height() - layout->contentsMargins().top() - layout->contentsMargins().bottom();
	icon->setPixmap(KIcon("palapeli").pixmap(iconSize));
	icon->setFixedSize(QSize(iconSize, iconSize));
	m_bar->setMinimum(0);
	m_bar->setMaximum(m_tasks.count() + 1); //do not let the bar reach 100% while it is visible
	m_bar->setValue(0);
	widget->show();
	QTimer::singleShot(0, this, SLOT(next()));
}

void Palapeli::FirstTimeHelper::next()
{
	if (m_tasks.isEmpty())
	{
		//done
		emit done();
		(new Palapeli::MainWindow(KCmdLineArgs::parsedArgs()))->show();
		deleteLater();
		return;
	}
	m_bar->setValue(m_bar->value() + 1);
	//create next puzzle
	Task task = m_tasks.takeFirst();
	QProcess* process = new QProcess;
	QFileInfo desktopFile(task.desktopUrl.path());
	process->setWorkingDirectory(desktopFile.dir().absolutePath());
	QStringList args;
	args << desktopFile.fileName() << task.puzzleUrl.path();
	process->start("libpala-puzzlebuilder", args);
	connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(next()));
}

#include "firsttimehelper.moc"
