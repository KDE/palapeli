/***************************************************************************
 *   Copyright (C) 2008 Stefan Majewsky <majewsky@gmx.net>
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

#ifndef PALAPELI_PATTERN_EXECUTOR_H
#define PALAPELI_PATTERN_EXECUTOR_H

#if defined(MAKE_LIBPALAPELIPART)
 #include "macros.h"
#else
 #include <Palapeli/Macros>
#endif

class QImage;
#include <QtCore/QThread>

namespace Palapeli
{

	class Pattern;
	class PatternExecutorPrivate;

	/**
	 * \class PatternExecutor pattern-executor.h Palapeli/PatternExecutor
	 *
	 * This is a wrapper which can be used to easily slice an image with a pattern in a separate thread. The following code snippet shows you how to use this class:
\code
Palapeli::Pattern* pattern;
// ...
Palapeli::PatternExecutor executor(pattern);
executor.setImage(baseImage);
executor.start();
\endcode
	 * Use the signal finished() to find out when the slicing is finished.
	 *
	 * \author Stefan Majewsky <majewsky@gmx.net>
	 */
	class PALAPELIPATTERN_EXPORT PatternExecutor : public QThread
	{
		Q_OBJECT
		public:
			/**
			 * \brief Constructs a new executor for the given \a pattern.
			 */
			PatternExecutor(Pattern* pattern);
			/**
			 * \brief Destructor.
			 */
			~PatternExecutor();
			/**
			 * \brief Enable/disable automatic deletion.
			 * If \a deleteWhenFinished is true (default is false), the executor and the pattern will be deleted automatically when the slicing is finished.
			 */
			void setDeleteWhenFinished(bool deleteWhenFinished);
			/**
			 * \brief Set image.
			 * This image will be sliced when the execution starts.
			 */
			void setImage(const QImage& image);
		protected:
			virtual void run();
		private Q_SLOTS:
			void slotFinished();
		private:
			PatternExecutorPrivate* const p;
	};

}

#endif // PALAPELI_PATTERN_EXECUTOR_H
