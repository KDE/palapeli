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

#include <QGraphicsView>

namespace Palapeli
{
	struct InteractorMouseEvent
	{
		InteractorMouseEvent(QGraphicsView* view, const QPoint& pos, const QPoint& lastPos);
		QPoint pos, lastPos;
		QPointF scenePos, lastScenePos;
	};
	struct InteractorWheelEvent
	{
		InteractorWheelEvent(QGraphicsView* view, const QPoint& pos, int delta);
		int delta;
		QPoint pos;
		QPointF scenePos;
	};
	enum InteractorType
	{
		MouseInteractor,
		WheelInteractor
	};

	///This class implements interaction traits for the Palapeli::View.
	class Interactor : public QObject
	{
		public:
			Palapeli::InteractorType type() const;
			///The interactor will handle events only while exactly these modifiers are pressed.
			Qt::KeyboardModifiers triggerModifiers() const;
			void setTriggerModifiers(Qt::KeyboardModifiers modifiers);
			///The interactor will handle mouse events only while this mouse button is pressed.
			///This setting is only senseful for mouse interactors.
			Qt::MouseButton triggerButton() const;
			void setTriggerButton(Qt::MouseButton button);
			///The interactor will only handle wheel events with one of these orientations.
			///This setting is only senseful for wheel interactors.
			Qt::Orientations triggerOrientations() const;
			void setTriggerOrientations(Qt::Orientations orientations);

			void setScene(QGraphicsScene* scene);

			//NOTE: For reasons which I do not know, implementing these interactors via QObject::eventFilter does not work.
			bool handleMouseEvent(QMouseEvent* event);
			bool handleWheelEvent(QWheelEvent* event);
		protected:
			///\warning You may not expect at this point that the \a view has a scene assigned to it.
			Interactor(Palapeli::InteractorType type, QGraphicsView* view);

			QGraphicsView* view() const;
			QGraphicsScene* scene() const;

			///This method is always called shortly before an event is delivered to the interactor. The parameter \a item is the item currently under the mouse, or 0 if there is no such item. Return false to abort event processing in this interactor, and forward the event to that item instead. The default implementation returns true, indicating that the interactor handles all events, regardless of the items on the scene.
			///NOTE: For mouse interactors, those items that do not accept the interactor's trigger button are ignored.
			virtual bool acceptItemUnderMouse(QGraphicsItem* item);
			///This method is always called shortly before an event is delivered to the interactor. The parameter \a pos is the position of the mouse cursor on the view. Return false to abort event processing in this interactor. The default implementation returns true, indicating that the interactor handles all events, regardless of the mouse position.
			virtual bool acceptMousePosition(const QPoint& pos);

			virtual void mouseMoveEvent(const Palapeli::InteractorMouseEvent& event);
			virtual void mousePressEvent(const Palapeli::InteractorMouseEvent& event);
			virtual void mouseReleaseEvent(const Palapeli::InteractorMouseEvent& event);
			virtual void wheelEvent(const Palapeli::InteractorWheelEvent& event);
			///This method is called whenever the view() changes its scene. Both parameters may be null pointers, indicating that the view had no scene up to now, or that the view will have no scene after this operation.
			virtual void sceneChangeEvent(QGraphicsScene* oldScene, QGraphicsScene* newScene);
		private:
			Palapeli::InteractorType m_type;
			QGraphicsView* m_view;
			QGraphicsScene* m_scene;
			Qt::KeyboardModifiers m_triggerModifiers;
			Qt::MouseButton m_triggerButton;
			Qt::Orientations m_triggerOrientations;
			QPoint m_lastPos;
	};
}

#endif // PALAPELI_INTERACTOR_H
