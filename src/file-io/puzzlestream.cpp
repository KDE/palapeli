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

#include "puzzlestream.h"

#include <KConfig>
#include <KConfigGroup>
#include <KIO/NetAccess>
#include <KJob>

//BEGIN PuzzleStreamJob

namespace Palapeli
{
	class PuzzleStreamJob : public KJob
	{
		public:
			PuzzleStreamJob(const KUrl& url) { this->url = url; }

			KUrl url;
			Palapeli::PuzzleStreamMetadata metadata;

			virtual void start();
			void doWork();
		protected:
			virtual void timerEvent(QTimerEvent* event); //implements workload
		private:
			int m_timerID;
	};
}

void Palapeli::PuzzleStreamJob::start()
{
	m_timerID = startTimer(0);
}

void Palapeli::PuzzleStreamJob::timerEvent(QTimerEvent* event)
{
	Q_UNUSED(event)
	killTimer(m_timerID); //single-shot timer for workload
	doWork();
	emitResult();
}

void Palapeli::PuzzleStreamJob::doWork()
{
	//download stream file
	QString streamFile;
	if (!url.isLocalFile())
	{
		if (!KIO::NetAccess::download(url, streamFile, 0))
			return;
	}
	else
		streamFile = url.path();
	//read manifest
	KConfig config(streamFile, KConfig::SimpleConfig);
	KConfigGroup streamGroup(&config, "Palapeli Stream");
	KConfigGroup puzzleGroup(&streamGroup, "Latest");
	metadata.streamIdentifier = QString::fromLatin1("streamresource-") + streamGroup.readEntry("Name", QString());
	metadata.streamName = streamGroup.readEntry("Name", QString());
	metadata.name = puzzleGroup.readEntry("Name", QString());
	metadata.author = puzzleGroup.readEntry("Author", QString());
	metadata.comment = puzzleGroup.readEntry("Comment", QString());
	metadata.pieceCount = puzzleGroup.readEntry("PieceCount", 0);
	//cleanup
	KIO::NetAccess::removeTempFile(streamFile);
}

//END PuzzleStreamJob

Palapeli::PuzzleStream::PuzzleStream(const KUrl& url)
	: m_url(url)
	, m_metadata(0)
	, m_job(0)
{
	refresh();
}

Palapeli::PuzzleLocation Palapeli::PuzzleStream::location() const
{
	return Palapeli::PuzzleLocation::fromUrl(m_url);
}

Palapeli::PuzzleStreamMetadata* Palapeli::PuzzleStream::metadata() const
{
	return m_metadata;
}

void Palapeli::PuzzleStream::refresh()
{
	if (m_job)
		return;
	//m_job is used to ensure that only one refresh job runs at once
	m_job = new Palapeli::PuzzleStreamJob(m_url);
	connect(m_job, SIGNAL(result(KJob*)), this, SLOT(handleRefreshFinished()));
}

void Palapeli::PuzzleStream::handleRefreshFinished()
{
	delete m_metadata;
	m_metadata = new Palapeli::PuzzleStreamMetadata(m_job->metadata);
	m_job = 0;
	emit refreshed();
}

#include "puzzlestream.moc"
