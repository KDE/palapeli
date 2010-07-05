/***************************************************************************
 *   Copyright 2010 Stefan Majewsky <majewsky@gmx.net>
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

#ifndef LIBPALA_SLICERMODE_H
#define LIBPALA_SLICERMODE_H

#if defined(MAKE_LIBPALA) || defined(USE_LOCAL_LIBPALA)
# include "libpalamacros.h"
#else
# include <libpala/libpalamacros.h>
#endif

#include <QtCore/QByteArray>
#include <QtCore/QPair>

namespace Pala
{
	class SlicerProperty;

	/**
	 * \class SlicerMode slicermode.h <Pala/SlicerMode>
	 * \brief Representation of an operational mode of a slicing algorithm.
	 * \since libpala 1.2 (KDE SC 4.6)
	 *
	 * Complex slicer plugins may include several slicing algorithms at once. This class represents such operational modes. Basically, the usage of slicer modes is identical to creating a Pala::StringProperty with a finite set of choices, but slicer modes have some advantages:
	 * \li You can choose to enable or disable properties depending on the selected slicer mode.
	 * \li You can create an own SlicerMode subclass and aggregate in it algorithms that are relevant to this mode.
	 * You are free not to use slicer modes: Just disregard this class and all functions in other classes that deal with slicer modes.
	 *
	 * \sa Pala::Slicer::addMode, Pala::SlicerJob::mode
	 */
	class SlicerMode
	{
		public:
			SlicerMode();
			virtual ~SlicerMode();

			//The following functions belong to the interface to the Palapeli application. They are not documented because the documentation is targeted at slicer developers.
			///\internal
			void filterProperties(QList<QPair<QByteArray, const Pala::SlicerProperty*> >& properties); //This function removes all properties from the given map which are disabled in this mode, taking both Pala::SlicerProperty::isEnabled and the exceptions defined by this mode into account.

			///Defines whether the property which has been added to Pala::Slicer with the given key, is enabled when this mode is selected. If this mode does not define the state for some property, the state defined by the property itself (through the Palapeli::SlicerProperty::setEnabled() method) is used. You will therefore probably use this function only to define exceptions from this default state.
			void setPropertyEnabled(const QByteArray& property, bool enabled);

			//Some space in the vtable reserved for future additions
			RESERVE_VIRTUAL_5
		private:
			class Private;
			Private* const p;
	};
}

#endif // LIBPALA_SLICERMODE_H
