/*
    SPDX-FileCopyrightText: 2010 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef LIBPALA_SLICERMODE_H
#define LIBPALA_SLICERMODE_H

#include "libpala_export.h"

#include <QByteArray>

#include <memory>

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
	class LIBPALA_EXPORT SlicerMode
	{
		public:
			///Create a new SlicerMode instance.
			///\param key an identifier which is unique among the modes of one slicer
			///\param name a user-visible name that describes this mode
			SlicerMode(const QByteArray& key, const QString& name);
			virtual ~SlicerMode();

			//The following functions belong to the interface to the Palapeli application. They are not documented because the documentation is targeted at slicer developers.
			///\internal
			void filterProperties(QList<const Pala::SlicerProperty*>& properties) const; //This function removes all properties from the given map which are disabled in this mode, taking both Pala::SlicerProperty::isEnabled and the exceptions defined by this mode into account.
			///\internal
			QByteArray key() const;
			///\internal
			QString name() const;

			///Defines whether the property which has been added to Pala::Slicer with the given key, is enabled when this mode is selected. If this mode does not define the state for some property, the state defined by the property itself (through the Palapeli::SlicerProperty::setEnabled() method) is used. You will therefore probably use this function only to define exceptions from this default state.
			void setPropertyEnabled(const QByteArray& property, bool enabled);

			//Some space in the vtable reserved for future additions
			//RESERVE_VIRTUAL_5
		private:
			std::unique_ptr<class SlicerModePrivate> const d_ptr;
			Q_DECLARE_PRIVATE(SlicerMode)
			Q_DISABLE_COPY(SlicerMode)
			// TODO: consider turning this into a value-type class with shared data
	};
}

#endif // LIBPALA_SLICERMODE_H
