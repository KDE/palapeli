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

#ifndef PALAPELI_PATTERN_PLUGIN_H
#define PALAPELI_PATTERN_PLUGIN_H

#if defined(MAKE_LIBPALAPELIBASE)
 #include "../macros.h"
#else
 #include <Palapeli/Macros>
#endif

#include <QtCore/QObject>
#include <QtCore/QVariant>

namespace Palapeli
{

	class PatternConfiguration;
	class PatternPluginPrivate;

	/**
	 * \class PatternPlugin pattern-plugin.h Palapeli/PatternPlugin
	 *
	 * This class is used in a pattern plugin as an entry point. It will create all valid Palapeli::PatternConfiguration instances. To mark this entry point, include the following lines in the source code of your pattern (replace \a MyPatternPlugin by the name of your Palapeli::PatternPlugin subclass, and \a mypattern by the name of your library):
\code
#include <KPluginFactory>
#include <KPluginLoader>

K_PLUGIN_FACTORY(MyPatternFactory, registerPlugin<MyPatternPlugin>();)
K_EXPORT_PLUGIN(MyPatternFactory("mypattern"))
\endcode
	 *
	 * \warning Each plugin may only define one PatternPlugin subclass. See the documentation of the createInstances() method to see how multiple patterns may be included in one pattern plugin.
	 *
	 * \author Stefan Majewsky <majewsky@gmx.net>
	 */
	class PALAPELIBASE_EXPORT PatternPlugin : public QObject
	{
		Q_OBJECT
		public:
			/**
			 * \brief Constructs a new PatternPlugin object.
			 * The parameter signature has to be the same one in all subclasses, because these are passed by the plugin loader. Usually, you will not have to bother with these parameters.
			 */
			explicit PatternPlugin(QObject* parent = 0, const QVariantList& args = QVariantList());
			/**
			 * \brief Destructor.
			 */
			virtual ~PatternPlugin();
			/**
			 * \brief Returns Palapeli::PatternConfiguration instances for this plugin.
			 * To actually use your plugin, Palapeli needs access to the patterns defined by your plugin. (This will usually be only one pattern per plugin.) Therefore, this method creates Palapeli::PatternConfiguration instances for Palapeli. If you choose to have multiple patterns in one plugin, you can create one Palapeli::PatternConfiguration instance for each plugin.
			 *
			 * \note The Palapeli::PatternConfiguration instances in the list will be deleted automatically, you do not have to bother with this.
			 */
			virtual QList<PatternConfiguration*> createInstances() const = 0;
		protected:
			/**
			 * \brief The plugin's internal name.
			 * This method returns the plugin name specified in the plugin's desktop file. This string identifies the plugin internally, and can be used as a basis for the pattern name needed in the Palapeli::PatternConfiguration constructor.
			 */
			QString pluginName() const;
			/**
			 * \brief The plugin's display name.
			 * This method returns the plugin's display name as specified in the plugin's desktop file. This localizable string identifies the plugin in interfaces, and can be used as a basis for the display name needed in the Palapeli::PatternConfiguration constructor.
			 */
			QString displayName() const;
			/**
			 * \brief The plugin's icon name.
			 * This method returns the name of the icon recommended to depict this plugin, as specified in the plugin's desktop file.
			 */
			QString iconName() const;
		private:
			PatternPluginPrivate* const p;
	};

}

#endif // PALAPELI_PATTERN_PLUGIN_H
