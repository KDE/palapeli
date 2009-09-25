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

	/**
	 * \class Slicer slicer.h libpala/slicer.h
	 * \since libpala 1.0 / Palapeli 1.0
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
			 * \brief Constructs a new Slicer object.
			 * In any subclass, the constructor signature has to be the same (due to the way the plugin loader works). The arguments should be passed to this constructor and ignored by the subclass implementation, as their format might change without notice in future versions.
			 */
			explicit Slicer(QObject* parent = 0, const QVariantList& args = QVariantList());
			///Deletes this slicer.
			virtual ~Slicer();

			//The following function belongs to the interface to the Palapeli application. It is not documented because the documentation is targeted at slicer developers.
			///\internal
			QMap<QByteArray, const Pala::SlicerProperty*> properties() const;

			/**
			 * \brief The slicing algorithm.
			 * Implement the slicing algorithm in this method. The slicing algorithm should always respect the current values of the slicer's properties, as defined through the addProperty() method.
			 * \see Pala::SlicerJob
			 */
			virtual void run(Pala::SlicerJob* job) = 0;
		protected:
			///Add the given property to the property list of this slicer. Use this method in the subclass constructors to fill the slicer with properties. Properties let the user control how the slicing is done.
			void addProperty(const QByteArray& key, Pala::SlicerProperty* property);
		private:
			class Private;
			Private* const p;
	};
}

#endif // LIBPALA_SLICER_H
