/***************************************************************************
 *   Copyright 2009 Stefan Majewsky <majewsky@gmx.net>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Library General Public
 *   License as published by the Free Software Foundation; either
 *   version 2 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details.
 *
 *   You should have received a copy of the GNU Library General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 ***************************************************************************/

#ifndef LIBPALA_SLICERPROPERTYSET_H
#define LIBPALA_SLICERPROPERTYSET_H

#if defined(MAKE_LIBPALA) || defined(USE_LOCAL_LIBPALA)
# include "libpalamacros.h"
#else
# include <libpala/libpalamacros.h>
#endif

class QByteArray;
#include <QtCore/QSize>

namespace Pala
{
	class Slicer;
	class SlicerJob;
	class SlicerProperty;

	/**
	 * \class SlicerPropertySet slicerpropertyset.h <Pala/SlicerPropertySet>
	 * \brief Representation of a set of configurable parameters of a slicing algorithm.
	 *
	 * In many slicers, you can find common patterns of configuration parameters. For example, many slicers divide the image into X times Y pieces, where X is some configurable "piece count in horizontal direction", and Y is some configurable "piece count in vertical direction".
	 *
	 * Such common property sets can be represented by a Pala::SlicerPropertySet subclass. For example, you can add the Pala::SimpleGridPropertySet to your Pala::Slicer subclass to get a set of properties that is useful for slicers that follow the "X times Y pieces" pattern. The recommended way to use property sets is to derive your Pala::Slicer subclass from them (using multiple inheritance; you can safely mix multiple Pala::SlicerPropertySet subclasses into one Pala::Slicer subclass).
	 *
	 * A property set is fully self-contained: It adds some properties to your slicer which remain totally invisible to your own implementation. You should always use the methods provided by the property set to read its properties, because the internal representation of the property set might change over time (esp. for those property sets that come with libpala).
	 *
	 * \note This is an abstract base class. Use the subclasses provided by this library. Defining own subclasses outside libpala is a good idea if you want to logically separate the configurable parameters of your slicer from the slicing algorithm itself.
	 */
	class LIBPALA_EXPORT SlicerPropertySet
	{
		public:
			Pala::Slicer* slicer() const;
		protected:
			SlicerPropertySet(Pala::Slicer* slicer);
			~SlicerPropertySet();
			///A synonym for Pala::Slicer::addProperty (because the latter is "protected"; this abstract base class can access it because it is a "protected friend" of Pala::Slicer).
			void addPropertyToSlicer(const QByteArray& key, Pala::SlicerProperty* property);
		private:
			class Private;
			Private* const p;
	};

	/**
	 * \class SimpleGridPropertySet slicerpropertyset.h <Pala/SlicerPropertySet>
	 *
	 * This property set can be used for slicers that create pieces which are aligned on a rectangular grid. The property set gives a piece count in horizontal direction, and one in vertical direction. The total piece count is the product of both one-dimensional piece counts.
	 *
	 * \note The user interface of this property set does not use the one-dimensional piece counts. It asks for a total piece count and an aspect (which can be anything from tall to square to wide), and calculates the one-dimensional piece counts from this input.
	 */
	class LIBPALA_EXPORT SimpleGridPropertySet : public Pala::SlicerPropertySet
	{
		public:
			SimpleGridPropertySet(Pala::Slicer* slicer);
			~SimpleGridPropertySet();

			QSize pieceCount(Pala::SlicerJob* job) const;
		private:
			class Private;
			Private* const p;
	};
}

#endif // LIBPALA_SLICERPROPERTYSET_H
