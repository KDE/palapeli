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

#ifndef PALAPELI_PATTERN_CONFIGURATION_H
#define PALAPELI_PATTERN_CONFIGURATION_H

#if defined(MAKE_LIBPALAPELIPART)
 #include "../macros.h"
#else
 #include <Palapeli/Macros>
#endif

#include <QtCore/QObject>
#include <QtCore/QVariant>
class QWidget;
class KConfigGroup;

namespace Palapeli
{

	class Pattern;
	class PatternConfigurationPrivate;

	/**
	 * \class PatternConfiguration pattern-configuration.h Palapeli/PatternConfiguration
	 *
	 * This class is used in a Palapeli pattern plugin to handle the configuration of the pattern. Furthermore, it will have to create a Palapeli::Pattern instance which will then do the actual slicing.
	 *
	 * To simplify the creation of arbitrary configurations, this class exposes to subclasses the ability to define properties with a range of different data types. These properties will then be used to construct configuration dialogs automatically. The following example shows how to define a property (this should be done in the constructor of your subclass):
\code
addProperty("myprop", Palapeli::PatternConfiguration::Integer, i18n("My own property:"));
QVariantList range; range << 1 << 10;
addPropertyParameters("myprop", range); //set minimum and maximum value
setProperty("myprop", 3); //set default value
\endcode
	 *
	 * Instances of Palapeli::PatternConfiguration instances are created in the Palapeli::PatternPlugin::createInstances() method for the Palapeli::PatternPlugin subclass of the plugin.
	 *
	 * \author Stefan Majewsky <majewsky@gmx.net>
	 */
	class PALAPELI_EXPORT PatternConfiguration : public QObject
	{
		Q_OBJECT
		public:
			/**
			 * \enum DataType
			 * \brief Defines acceptable data types for properties.
			 * Note that this does not affect the internal handling of values, these are always passed as QVariant (but it is ensured that they have a specific meta type). The data type defined through this enumeration mainly specifies which configuration widget is used to let the user manipulate this property.
			 */
			enum DataType
			{
				Variant = 0, ///< no conversions, editing through KLineEdit
				String,      ///< conversion to QVariant::String, editing through KLineEdit
				Integer,     ///< conversion to QVariant::Int, editing through KIntSpinBox
				Boolean      ///< conversion to QVariant::Bool, editing through QCheckBox
			};
			/**
			 * \brief Constructs a new PatternConfiguration object.
			 * This constructor creates the three properties \a "PatternName", \a "DisplayName" and \a "IconName" which are filled with the given values. You should not modify these properties after this initialisation.
			 *
			 * For the parameters \a patternName and \a displayName, you should modify the plugin name and the display name in Palapeli::PatternPlugin.
			 *
			 * In your subclass implementation, this is the recommended point to add properties.
			 */
			PatternConfiguration(const QString& patternName, const QString& displayName, const QString& iconName);
			/**
			 * \brief Destructor.
			 */
			virtual ~PatternConfiguration();
			/**
			 * \brief Creates a Pattern instance.
			 * You will have to implement this function in your subclass to create your own Palapeli::Pattern instance. This is also the point to pass any property values to your pattern.
			 */
			virtual Pattern* createPattern() const = 0;
			/**
			 * \brief Reads a property.
			 * \param key the key of the property
			 * \return the value of this property (or an empty QVariant if this property does not exist)
			 */
			QVariant property(const QByteArray& key) const;
			/**
			 * \brief Loads parameters from a configuration.
			 * This function reads all defined properties from the passed \a config.
			 * \param config the KConfigGroup to read from
			 * \sa readCustomArguments
			 */
			void readArguments(KConfigGroup* config);
			/**
			 * \brief Saves parameters to a configuration.
			 * This function writes the values of all defined properties to the passed \a config.
			 * \param config the KConfigGroup to write to
			 * \sa writeCustomArguments
			 */
			void writeArguments(KConfigGroup* config) const;
			/**
			 * Creates a layout with configuration widgets inside the given \a widget.
			 */
			void populateWidget(QWidget* widget);
		public Q_SLOTS:
			/**
			 * \brief Sets a property to the given value.
			 * \param key the key of the property
			 * \param value the new value for this property
			 */
			void setProperty(const QByteArray& key, const QVariant& value);
		Q_SIGNALS:
			/**
			 * \brief This signal is emitted whenever a defined property's value changed.
			 * \param key the key of the property
			 * \param value the new value for this property
			 */
			void propertyChanged(const QByteArray& key, const QVariant& value);
		protected:
			/**
			 * If you have own configuration values which are not handled as configurable property and are therefore not visible to the user (e.g. a random seed), use this method to load them from the given \a config.
			 * \param config the KConfigGroup to read from
			 * \sa readArguments
			 */
			virtual void readCustomArguments(KConfigGroup* config);
			/**
			 * If you have own configuration values which are not handled as configurable property and are therefore not visible to the user (e.g. a random seed), use this method to save them to the given \a config.
			 * \param config the KConfigGroup to write to
			 * \sa writeArguments
			 */
			virtual void writeCustomArguments(KConfigGroup* config) const;
			/**
			 * \brief Define a new property.
			 * \param key the key of the new property (used to access its value etc.)
			 * \param type the type of the property (see documentation of DataType enumeration)
			 * \param caption caption for use in configuration dialogs
			 */
			void addProperty(const QByteArray& key, DataType type, const QString& caption);
			/**
			 * \brief Add parameters to this property.
			 * It depends on the DataType of the property what these parameters do. For String properties, the parameters are understood as valid values; the input is then restricted to these options (by using a KComboBox instead of a KLineEdit in configuration dialogs). For Int properties, the first parameter is the minimum value (default is 0), and the second parameter is the maximum value (default is 100); any further parameters are discarded. For all other data types, all parameters are discarded.
			 * \param key the key of the property
			 * \param parameters the parameters for this property
			 */
			void addPropertyParameters(const QByteArray& key, const QVariantList& parameters);
		private:
			PatternConfigurationPrivate* const p;
	};

}

#endif // PALAPELI_PATTERN_CONFIGURATION_H
