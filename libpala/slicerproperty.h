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

#ifndef LIBPALA_SLICERPROPERTY_H
#define LIBPALA_SLICERPROPERTY_H

#if defined(MAKE_LIBPALA) || defined(USE_LOCAL_LIBPALA)
# include "libpalamacros.h"
#else
# include <libpala/libpalamacros.h>
#endif

#include <QtCore/QVariant>

namespace Palapeli
{
	/**
	 * \class SlicerProperty slicerproperty.h libpala/slicerproperty.h
	 * \since libpala 1.0 / Palapeli 1.0
	 * \brief Representation of a slicing algorithm's configurable parameters.
	 *
	 * Slicer properties describe configurable parameters of slicing algorithms (i.e. Palapeli::Slicer instances) in a presentation-agnostic way. Note that it does not store any user-generated values, it just describes the possible input forms.
	 *
	 * \sa Palapeli::Slicer::addProperty
	 *
	 * \warning Some setter functions are only logical for certain property types. If such a function is called on a property with a "wrong" type, the method will not only fail, but terminate the whole application, to let you discover such errors in the early run.
	 */
	class LIBPALA_EXPORT SlicerProperty
	{
		public:
			///This enumeration defines the possible property types. Note that (by far) not all possible QVariant types are included, as this would increase the implementation complexity and complicate the UI representation.
			enum Type
			{
				Boolean = 1, ///< These properties are represented by a check box.
				Integer = 2, ///< These properties are represented by an int spin box (or a combo box if setChoices() is used).
				String = 3   ///< These properties are represented by a line edit (or a combo box if setChoices() is used).
			};
			explicit SlicerProperty(Type type, const QString& caption);
			///Deletes this slicer property.
			~SlicerProperty();

			//The following functions belong to the interface to the Palapeli application. They are not documented because the documentation is targeted at slicer developers.
			///\internal
			QString caption() const;
			///\internal
			QStringList choices() const;
			///\internal
			QVariant defaultValue() const;
			///\internal
			int rangeMinimum() const;
			///\internal
			int rangeMaximum() const;
			///\internal
			Type type() const;

			///Only for string properties: Limits the user input to the selection of one of the given strings. (The first string in the given list will be the default.)
			void setChoices(const QStringList& strings);
			///Only for integer properties: Limits the user input to the selection of one of the given numbers. (The first number in the given list will be the default.) This will override setRange() in any case.
			void setChoices(const QList<int>& numbers);
			///Sets the default value of this property. This is overriden by setChoices(), because the first one of the choices given to that function will become the default in any case.
			void setDefaultValue(const QVariant& value);
			///Only for integer properties: Limits the user input to the selection of a number inside the given range (including the bounds). This will be overriden by setChoices() in any case.
			void setRange(int min, int max);
		private:
			class Private;
			Private* const p;
	};
}

#endif // LIBPALA_SLICERPROPERTY_H
