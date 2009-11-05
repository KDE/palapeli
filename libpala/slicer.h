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

#ifndef LIBPALA_SLICER_H
#define LIBPALA_SLICER_H

#if defined(MAKE_LIBPALA) || defined(USE_LOCAL_LIBPALA)
# include "libpalamacros.h"
#else
# include <libpala/libpalamacros.h>
#endif

#include <QtCore/QObject>
#include <QtCore/QVariant>

namespace Pala
{
	class SlicerJob;
	class SlicerProperty;
	class SlicerPropertySet;

	/**
	 * \class Slicer slicer.h <Pala/Slicer>
	 * \brief Representation of a slicing algorithm.
	 *
	 * This class represents a slicing algorithm. It has to be subclassed by slicing plugin developers. Subclasses need to implement the constructor (where the slicer's property list has to be created) and the run() method (where the actual slicing is performed).
	 *
	 * Additionally, the class must be flagged as entry point into the plugin, with the following code:
\code
class MySlicer : public Pala::Slicer { ... };

#include <KPluginFactory>
#include <KPluginLoader>

K_PLUGIN_FACTORY(MySlicerFactory, registerPlugin<MySlicer>();)
K_EXPORT_PLUGIN(MySlicerFactory("myslicer"))
\endcode
	 * Replace \a myslicer with the file name of the plugin library (e.g. \a myslicer is the name for a library  \a libmyslicer.so on unixoid systems, or \a myslicer.dll on Windows systems).
	 */
	class LIBPALA_EXPORT Slicer : public QObject
	{
		Q_OBJECT
		public:
			/**
			 * \brief Behavioral flags of a slicer.
			 * These flags can be used to programmatically configure the behavior of libpala for a single slicer. You should only set the slicer's flags once in the constructor, and not modify it later. (The latter might cause unexpected behavior.)
			 * \see setFlags
			 */
			enum SlicerFlag
			{
				NoFlags = 0x0,
				AllowFullTransparency = 0x1 ///< By default, libpala will increase the minimum alpha value of input images to avoid invisible pieces. Set this flag if you rely on the alpha channel in your slicing algorithm.
			};
			Q_DECLARE_FLAGS(SlicerFlags, SlicerFlag)

			/**
			 * \brief Constructs a new Slicer object.
			 * In any subclass, the constructor signature has to be the same (due to the way the plugin loader works). The arguments should be passed to this constructor and ignored by the subclass implementation, as their format might change without notice in future versions.
			 */
			explicit Slicer(QObject* parent = 0, const QVariantList& args = QVariantList());
			///Deletes this slicer.
			virtual ~Slicer();

			//The following function belongs to the interface to the Palapeli application. It is not documented because the documentation is targeted at slicer developers.
			///\internal
			QMap<QByteArray, const Pala::SlicerProperty*> properties() const;
			///\internal
			SlicerFlags flags() const;
			///\internal
			bool process(Pala::SlicerJob* job); //a wrapper function for Pala::Slicer::run

			//This class is the only interface that slicers can use to communicate with Palapeli, and it is only instantiated very few times (one instance per slicer plugin), so it should be reasonable to reserve some space in the virtual table for future additions.
			RESERVE_VIRTUAL_5
		protected:
			///Add the given property to the property list of this slicer. Use this method in the subclass constructors to fill the slicer with properties. Properties let the user control how the slicing is done.
			void addProperty(const QByteArray& key, Pala::SlicerProperty* property);
			friend class SlicerPropertySet;
			///\see Pala::Slicer::SlicerFlags
			void setFlags(SlicerFlags flags);

			/**
			 * \brief The slicing algorithm.
			 * Implement the slicing algorithm in this method. The slicing algorithm should always respect the current values of the slicer's properties, as defined through the addProperty() method.
			 * \returns whether the operation has been completed successfully
			 * \see Pala::SlicerJob
			 */
			virtual bool run(Pala::SlicerJob* job) = 0;
		private:
			class Private;
			Private* const p;
	};
}

Q_DECLARE_OPERATORS_FOR_FLAGS(Pala::Slicer::SlicerFlags)

#endif // LIBPALA_SLICER_H
