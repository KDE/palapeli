/*
    SPDX-FileCopyrightText: 2009, 2010 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef LIBPALA_SLICERPROPERTY_H
#define LIBPALA_SLICERPROPERTY_H

#include "libpala_export.h"

#include <QPair>
#include <QVariant>
#include <QVariantList>

#include <memory>

namespace Pala
{
	class SlicerPropertyPrivate;

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
			explicit SlicerProperty(Pala::SlicerPropertyPrivate& dd);
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
			///\since libpala 1.2 (KDE SC 4.6)
			QByteArray key() const;
			///\internal
			///\since libpala 1.2 (KDE SC 4.6)
			void setKey(const QByteArray& key);
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
		protected:
			std::unique_ptr<SlicerPropertyPrivate> const d_ptr;
		private:
			Q_DECLARE_PRIVATE(SlicerProperty)
			Q_DISABLE_COPY(SlicerProperty)
	};

	class BooleanPropertyPrivate;
	/**
	 * \class BooleanProperty slicerproperty.h <Pala/SlicerProperty>
	 */
	class LIBPALA_EXPORT BooleanProperty : public Pala::SlicerProperty
	{
		public:
			explicit BooleanProperty(const QString& caption);
			~BooleanProperty() override;
		private:
			Q_DECLARE_PRIVATE(BooleanProperty)
	};

	class IntegerPropertyPrivate;
	/**
	 * \class IntegerProperty slicerproperty.h <Pala/SlicerProperty>
	 */
	class LIBPALA_EXPORT IntegerProperty : public Pala::SlicerProperty
	{
		public:
			///Decides how the property is represented in the user interface of Palapeli.
			enum Representation { SpinBox, Slider, DefaultRepresentation = SpinBox };

			explicit IntegerProperty(const QString& caption);
			~IntegerProperty() override;

			///\internal
			QPair<int, int> range() const;
			///\internal
			Representation representation() const;

			///Limits the user input to the selection of a number inside the given range (including the bounds).
			void setRange(int min, int max);
			///Decides how the property is represented in the user interface of Palapeli.
			void setRepresentation(Representation representation);
		private:
			Q_DECLARE_PRIVATE(IntegerProperty)
	};

	class StringPropertyPrivate;
	/**
	 * \class StringProperty slicerproperty.h <Pala/SlicerProperty>
	 */
	class LIBPALA_EXPORT StringProperty : public Pala::SlicerProperty
	{
		public:
			explicit StringProperty(const QString& caption);
			~StringProperty() override;
		private:
			Q_DECLARE_PRIVATE(StringProperty)
	};
}

#endif // LIBPALA_SLICERPROPERTY_H
