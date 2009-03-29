/***************************************************************************
 *   Copyright 2008 Stefan Majewsky <majewsky@gmx.net>
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

#include "pattern-executor.h"
#include "pattern.h"

#include <QImage>
#include <QMutex>
#include <QMutexLocker>

namespace Palapeli
{

	class PatternExecutorPrivate
	{
		public:
			Pattern* m_pattern;
			QMutex m_mutex; //secures access to the m_image
			QImage m_image;
			bool m_deleteWhenFinished;

			PatternExecutorPrivate(Pattern* pattern) : m_pattern(pattern), m_deleteWhenFinished(false) {}
	};

}

Palapeli::PatternExecutor::PatternExecutor(Palapeli::Pattern* pattern)
	: p(new Palapeli::PatternExecutorPrivate(pattern))
{
	connect(this, SIGNAL(finished()), this, SLOT(slotFinished()));
}

Palapeli::PatternExecutor::~PatternExecutor()
{
        delete p;
}

void Palapeli::PatternExecutor::setImage(const QImage& image)
{
	QMutexLocker locker(&p->m_mutex);
	p->m_image = image;
}

void Palapeli::PatternExecutor::setDeleteWhenFinished(bool deleteWhenFinished)
{
	p->m_deleteWhenFinished = deleteWhenFinished;
}

void Palapeli::PatternExecutor::run()
{
	QMutexLocker locker(&p->m_mutex);
	p->m_pattern->slice(p->m_image);
}

void Palapeli::PatternExecutor::slotFinished()
{
	if (p->m_deleteWhenFinished)
	{
		this->deleteLater();
		p->m_pattern->deleteLater();
	}
}

#include "pattern-executor.moc"
