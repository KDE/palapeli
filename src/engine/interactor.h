/*
    SPDX-FileCopyrightText: 2010 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PALAPELI_INTERACTOR_H
#define PALAPELI_INTERACTOR_H

class QGraphicsScene;
class QGraphicsView;
#include <QIcon>

namespace Palapeli
{
	struct MouseEvent
	{
		public:
			MouseEvent(QGraphicsView* view, const QPoint& pos);
			QPoint pos;
			QPointF scenePos;
		protected:
			friend class Interactor;
			MouseEvent();
	};

	struct WheelEvent
	{
		public:
			WheelEvent(QGraphicsView* view, const QPoint& pos, int delta);
			QPoint pos;
			QPointF scenePos;
			int delta;
	};

	enum InteractorType
	{
		MouseInteractor = 1,
		WheelInteractor = 2
	};

	enum EventProcessingFlag {
		EventMatches = 1 << 0,
		EventMatchesExactly = (1 << 1) + EventMatches,
		EventStartsInteraction = 1 << 2,
		EventConcludesInteraction = 1 << 3
	};
	Q_DECLARE_FLAGS(EventProcessingFlags, EventProcessingFlag)
	Q_DECLARE_OPERATORS_FOR_FLAGS(EventProcessingFlags)

	class Interactor
	{
		//TODO: add a flag for interactors which misbehave extremely with "NoButton;NoModifier" or "NoButton;*" triggers
		Q_DISABLE_COPY(Interactor)
		public:
			virtual ~Interactor();

			enum Category { NoCategory, PieceInteraction, TableInteraction, ViewportInteraction };
			Category category() const;
			QString description() const;
			QIcon icon() const;
			Palapeli::InteractorType interactorType() const;
			int priority() const;

			bool isActive() const;
			void setInactive();
			void sendEvent(const Palapeli::MouseEvent& event, Palapeli::EventProcessingFlags flags);
			void sendEvent(const Palapeli::WheelEvent& event);

			///Call this method when the view for this interactor sets a different scene.
			void updateScene();
		protected:
			Interactor(int priority, Palapeli::InteractorType type, QGraphicsView* view);
			void setMetadata(Category category, const QString& description, const QIcon& icon);

			QGraphicsView* view() const;
			QGraphicsScene* scene() const;

			///This corresponds to a mousePressEvent. Return false unless you want to accept this interaction. The default implementation does nothing and accepts all interactions.
			virtual bool startInteraction(const Palapeli::MouseEvent& event) { Q_UNUSED(event) return true; }
			///This corresponds to a mouseMoveEvent.
			virtual void continueInteraction(const Palapeli::MouseEvent& event) { Q_UNUSED(event) }
			///This corresponds to a mouseRelease.
			virtual void stopInteraction(const Palapeli::MouseEvent& event) { Q_UNUSED(event) }
			///This corresponds to a wheelEvent
			virtual void doInteraction(const Palapeli::WheelEvent& event) { Q_UNUSED(event) }
			///This method is called whenever the view() changes its scene. Both parameters may be null pointers, indicating that the view had no scene up to now, or that the view will have no scene after this operation.
			virtual void sceneChangeEvent(QGraphicsScene* oldScene, QGraphicsScene* newScene) { Q_UNUSED(oldScene) Q_UNUSED(newScene) }
		private:
			Palapeli::InteractorType m_type;
			//context
			QGraphicsView* m_view;
			QGraphicsScene* m_scene;
			//state
			bool m_active;
			Palapeli::MouseEvent m_lastMouseEvent;
			//metadata
			Category m_category;
			QString m_description;
			QIcon m_icon;
			int m_priority;
	};
}

#endif // PALAPELI_INTERACTOR_H
