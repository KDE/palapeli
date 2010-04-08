/***************************************************************************
 *   Copyright 2010 Stefan Majewsky <majewsky@gmx.net>
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

#ifndef PALAPELI_INTERACTOR_H
#define PALAPELI_INTERACTOR_H

#include "interactorutils.h"

#include <QEvent>
#include <QGraphicsView>
#include <QIcon>
#include <QMetaType>

namespace Palapeli
{
	enum InteractorType
	{
		MouseInteractor,
		WheelInteractor
	};
	Q_DECLARE_FLAGS(InteractorTypes, InteractorType)

	class Interactor
	{
		//TODO: add a flag for interactors which misbehave extremely with "NoButton;NoModifier" or "NoButton;*" triggers
		Q_DISABLE_COPY(Interactor)
		public:
			virtual ~Interactor();

			QString description() const;
			QIcon icon() const;

			bool isMouseInteractor() const;
			bool isWheelInteractor() const;
			Palapeli::InteractorTypes interactorTypes() const;

			bool handleEvent(const Palapeli::MouseEvent& event, QEvent::Type type);
			bool handleEvent(const Palapeli::WheelEvent& event);
			///Call this method when the view for this interactor sets a different scene.
			void updateScene();
		protected:
			Interactor(Palapeli::InteractorTypes types, QGraphicsView* view);
			void setMetadata(const QString& description, const QIcon& icon);

			QGraphicsView* view() const;
			QGraphicsScene* scene() const;

			///This method is always called shortly before an event is delivered to the interactor. The parameter \a pos is the position of the mouse cursor on the view. Return false to abort event processing in this interactor. The default implementation returns true, indicating that the interactor handles all events, regardless of the mouse position.
			virtual bool acceptMousePosition(const QPoint& pos) { Q_UNUSED(pos) return true; }

			virtual void mouseMoveEvent(const Palapeli::MouseEvent& event) { Q_UNUSED(event) }
			virtual void mousePressEvent(const Palapeli::MouseEvent& event) { Q_UNUSED(event) }
			virtual void mouseReleaseEvent(const Palapeli::MouseEvent& event) { Q_UNUSED(event) }
			virtual void wheelEvent(const Palapeli::WheelEvent& event) { Q_UNUSED(event) }
			///This method is called whenever the view() changes its scene. Both parameters may be null pointers, indicating that the view had no scene up to now, or that the view will have no scene after this operation.
			virtual void sceneChangeEvent(QGraphicsScene* oldScene, QGraphicsScene* newScene) { Q_UNUSED(oldScene) Q_UNUSED(newScene) }
		private:
			Palapeli::InteractorTypes m_types;
			QGraphicsView* m_view;
			QGraphicsScene* m_scene;
			//metadata
			QString m_description;
			QIcon m_icon;
	};
}

Q_DECLARE_OPERATORS_FOR_FLAGS(Palapeli::InteractorTypes)
Q_DECLARE_METATYPE(Palapeli::Interactor*)

#endif // PALAPELI_INTERACTOR_H
