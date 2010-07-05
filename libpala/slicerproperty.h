/***************************************************************************
 *   Copyright 2009, 2010 Stefan Majewsky <majewsky@gmx.net>
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

#ifndef LIBPALA_SLICERPROPERTY_H
#define LIBPALA_SLICERPROPERTY_H

#if defined(MAKE_LIBPALA) || defined(USE_LOCAL_LIBPALA)
# include "libpalamacros.h"
#else
# include <libpala/libpalamacros.h>
#endif

#include <QtCore/QPair>
#include <QtCore/QVariant>
#include <QtCore/QVariantList>

namespace Pala
{
	/**
	 * \class SlicerProperty slicerproperty.h <Pala/SlicerProperty>
	 * \brief Representation of a single configurable parameter of a slicing algorithm.
	 *
	 * Slicer properties describe configurable parameters of slicing algorithms (i.e. Pala::Slicer instances) in a presentation-agnostic way. They do not store any user-generated values, they just describe the possible input forms.
	 *
	 * \note This is an abstract base class. Use the subclasses provided by this library. Defining own subclasses outside libpala is senseless, because Palapeli needs to know about them to use them correctly and to their full extent.
	 *
	 * \sa Pala::Slicer::addProperty, Pala::AbstractSlicerPropertySet
	 */
	class LIBPALA_EXPORT SlicerProperty
	{
		protected:
			explicit SlicerProperty(QVariant::Type type, const QString& caption);
		public:
			///Deletes this slicer property.
			virtual ~SlicerProperty();

			//The following functions belong to the interface to the Palapeli application. They are not documented because the documentation is targeted at slicer developers.
			///\internal
			QString caption() const;
			///\internal
			QVariantList choices() const;
			///\internal
			QVariant defaultValue() const;
			///\internal
			///\since libpala 1.2 (KDE SC 4.6)
			bool isAdvanced() const;
			///\internal
			///\since libpala 1.2 (KDE SC 4.6)
			bool isEnabled() const;
			///\internal
			QVariant::Type type() const;

			///Sets whether this property is advanced (false by default). If it is set, Palapeli is allowed to hide the property widget from the puzzle creation interface unless an "Advanced" button is pressed (or similar).
			///\since libpala 1.2 (KDE SC 4.6)
			void setAdvanced(bool advanced = true);
			///Limits the user input to the selection of one of the given values. The first value in the given list will be the default.
			///\warning This setting will override any other constraints to the user input, including the default value defined by setDefaultValue().
			void setChoices(const QVariantList& choices);
			///Sets the default value of this property.
			void setDefaultValue(const QVariant& value);
			///Sets whether this property is enabled (true by default). If you do not use multiple slicer modes (see Pala::Slicer::addSlicerMode), setting this to false is senseless. On the other hand, if you do use multiple slicer modes and have certain properties which are only useful in single modes, you probably want to set this to false, and enable the property in the relevant slicer modes using the Pala::SlicerMode::setPropertyEnabled method.
			///\since libpala 1.2 (KDE SC 4.6)
			void setEnabled(bool enabled);
		private:
			class Private;
			Private* const p;
	};

	/**
	 * \class BooleanProperty slicerproperty.h <Pala/SlicerProperty>
	 */
	class LIBPALA_EXPORT BooleanProperty : public Pala::SlicerProperty
	{
		public:
			explicit BooleanProperty(const QString& caption);
			virtual ~BooleanProperty();
		private:
			class Private;
			Private* const p;
	};

	/**
	 * \class IntegerProperty slicerproperty.h <Pala/SlicerProperty>
	 */
	class LIBPALA_EXPORT IntegerProperty : public Pala::SlicerProperty
	{
		public:
			///Decides how the property is represented in the user interface of Palapeli.
			enum Representation { SpinBox, Slider, DefaultRepresentation = SpinBox };

			explicit IntegerProperty(const QString& caption);
			virtual ~IntegerProperty();

			///\internal
			QPair<int, int> range() const;
			///\internal
			Representation representation() const;

			///Limits the user input to the selection of a number inside the given range (including the bounds).
			void setRange(int min, int max);
			///Decides how the property is represented in the user interface of Palapeli.
			void setRepresentation(Representation representation);
		private:
			class Private;
			Private* const p;
	};

	/**
	 * \class StringProperty slicerproperty.h <Pala/SlicerProperty>
	 */
	class LIBPALA_EXPORT StringProperty : public Pala::SlicerProperty
	{
		public:
			explicit StringProperty(const QString& caption);
			virtual ~StringProperty();
		private:
			class Private;
			Private* const p;
	};
}

#endif // LIBPALA_SLICERPROPERTY_H
