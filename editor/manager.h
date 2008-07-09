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

#ifndef PALADESIGN_MANAGER_H
#define PALADESIGN_MANAGER_H

#include <QList>
#include <QObject>
#include <QString>
class KUrl;

namespace Paladesign
{

	class LogicalRelation;
	class MainWindow;
	class ObjectView;
	class Points;
	class PropertyModel;
	class Relation;
	class Shapes;
	class View;

	class Manager : public QObject
	{
		Q_OBJECT
		public:
			Manager();
			~Manager();

			bool isPatternChanged() const;
			Points* points() const;
			Shapes* shapes() const;
			PropertyModel* propertyModel() const;
			ObjectView* objectView() const;
			View* view() const;
			MainWindow* window() const;

			Relation* relation(int index) const;
			int relationCount() const;
			bool addRelation(Paladesign::LogicalRelation* relation);
			bool removeRelation(int index);

			void newPattern();
			void loadPattern(const KUrl& url);
			void savePattern(const KUrl& url);
			QString fetchFile(const KUrl& url);
		protected Q_SLOTS:
			void patternChanged();
		private:
			Points* m_points;
			Shapes* m_shapes;
			QList<Relation*> m_relations;

			PropertyModel* m_propModel;
			ObjectView* m_objView;
			View* m_view;
			MainWindow* m_window;

			QList<QString> m_tempFiles;
			bool m_patternChanged;
	};

}

#endif // PALADESIGN_MANAGER_H
