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

#ifndef LIBPALA_SLICERJOB_H
#define LIBPALA_SLICERJOB_H

#if defined(MAKE_LIBPALA) || defined(USE_LOCAL_LIBPALA)
# include "libpalamacros.h"
#else
# include <libpala/libpalamacros.h>
#endif

#include <QtCore/QMap>
#include <QtCore/QPoint>
#include <QtCore/QVariant>
#include <QtGui/QImage>

//TODO: Add support for hidden debug output.

namespace Pala
{
	/**
	 * \class SlicerJob slicerjob.h <Pala/SlicerJob>
	 * \brief Representation of a single run of a slicing algorithm.
	 *
	 * This class holds everything that concerns a single slicing job: It starts out with the image and the arguments (i.e., the values that the user has chosen for the properties defined by the slicing algorithm). The Pala::Slicer::run method should then use these informations to slice the image into pieces and define neighborship relations between them.
	 *
	 * \sa Pala::Slicer::run
	 */
	class LIBPALA_EXPORT SlicerJob
	{
		public:
			///Creates a new slicer job.
			explicit SlicerJob(const QImage& image, const QMap<QByteArray, QVariant>& args);
			///Deletes this slicer job.
			virtual ~SlicerJob();

			///Returns an argument of this job, i.e. the value that the user has chosen for the slicing algorithm's property with the given \a key.
			QVariant argument(const QByteArray& key) const;
			///Returns the image that should be sliced.
			QImage image() const;

			//The following functions belong to the interface to the Palapeli application. They are not documented because the documentation is targeted at slicer developers.
			///\internal
			QMap<int, QImage> pieces() const;
			///\internal
			QMap<int, QPoint> pieceOffsets() const;
			///\internal
			QList<QPair<int, int> > relations() const;

			/**
			 * \brief Add a generated piece to the result set of this slicing job.
			 * The \a pieceID can later be used to refer to this piece in the addRelation() method. The given \a offset describes the position of the top-left edge of the given piece \a image in the complete image.
			 *
			 * \note It is good practice to make the piece image as small as possible, and use the offset to move it to the right position in the image's coordinates. Big images might result in bad performance, even if most of the images is fully transparent.
			 * \warning You should not use negative piece IDs. These might be used internally.
			 */
			void addPiece(int pieceID, const QImage& image, const QPoint& offset = QPoint());
			/**
			 * \brief Generate a piece, and add it to the result set of this slicing job.
			 * The \a pieceID can later be used to refer to this piece in the addRelation() method. The given \a offset describes the position of the top-left edge of the given mask in the complete image.
			 *
			 * \note It is good practice to make the mask as small as possible, and use the offset to move it to the right position in the image's coordinates. Big masks result in big piece images, which might in turn result in bad performance, even if most of the images is fully transparent.
			 * \warning You should not use negative piece IDs. These might be used internally.
			 */
			void addPieceFromMask(int pieceID, const QImage& mask, const QPoint& offset = QPoint());
			/**
			 * \brief Define a neighborship relation between two pieces.
			 * Neighborship relations are crucial for the gameplay: Palapeli needs to know somehow which pieces are neighbors, to let these pieces snap together when they're near each other.
			 */
			void addRelation(int pieceID1, int pieceID2);
		private:
			class Private;
			Private* const p;
	};
}

#endif // LIBPALA_SLICERJOB_H
