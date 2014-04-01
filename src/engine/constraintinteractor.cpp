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

#include "constraintinteractor.h"
#include "scene.h"

#include <KLocalizedString>

Palapeli::ConstraintInteractor::ConstraintInteractor(QGraphicsView* view)
	: Palapeli::Interactor(10, Palapeli::MouseInteractor, view) //priority: less than interaction with pieces, but more than interaction with viewport
{
	setMetadata(TableInteraction, i18n("Change size of puzzle table area by dragging its edges"), QIcon());
}

QList<Palapeli::ConstraintInteractor::Side> Palapeli::ConstraintInteractor::touchingSides(const QPointF& scenePos) const
{
	QList<Side> result;
	// The scene must be Palapeli::Scene type, to get handleWidth().
	Palapeli::Scene* scene = qobject_cast<Palapeli::Scene*>(this->scene());
	if (!scene)
		return result;		// No scene, so no sides touching.
	const QRectF sceneRect = scene->sceneRect();
	const qreal w = scene->handleWidth();
	const QSizeF handleSize = QSizeF(w, w);
	if ((scenePos.x() > sceneRect.left()) && (scenePos.x() < sceneRect.left() + handleSize.width()))
		result << LeftSide;
	else if ((scenePos.x() < sceneRect.right()) && (scenePos.x() > sceneRect.right() - handleSize.width()))
		result << RightSide;
	if ((scenePos.y() > sceneRect.top()) && (scenePos.y() < sceneRect.top() + handleSize.height()))
		result << TopSide;
	else if ((scenePos.y() < sceneRect.bottom()) && (scenePos.y() > sceneRect.bottom() - handleSize.height()))
		result << BottomSide;
	return result;
}

bool Palapeli::ConstraintInteractor::startInteraction(const Palapeli::MouseEvent& event)
{
	if (!scene())
		return false;
	//determine touching sides
	m_draggingSides = touchingSides(event.scenePos);
	if (m_draggingSides.isEmpty())
		return false;
	//record the position where we grabbed the handles (more precisely: its distance to the sides of the scene rect)
	m_baseSceneRectOffset = QPointF();
	const QRectF sceneRect = scene()->sceneRect();
	if (m_draggingSides.contains(LeftSide))
		m_baseSceneRectOffset.rx() = event.scenePos.x() - sceneRect.left();
	else if (m_draggingSides.contains(RightSide))
		m_baseSceneRectOffset.rx() = event.scenePos.x() - sceneRect.right();
	if (m_draggingSides.contains(TopSide))
		m_baseSceneRectOffset.ry() = event.scenePos.y() - sceneRect.top();
	else if (m_draggingSides.contains(BottomSide))
		m_baseSceneRectOffset.ry() = event.scenePos.y() - sceneRect.bottom();
	return true;
}

void Palapeli::ConstraintInteractor::continueInteraction(const Palapeli::MouseEvent& event)
{
	// In this method, we need the scene() to be Palapeli::Scene type.
	Palapeli::Scene* scene = qobject_cast<Palapeli::Scene*>(this->scene());
	if (!scene)
		return;
	QRectF sceneRect = scene->sceneRect();
	//change scene rect
	const QPointF newBounds = event.scenePos - m_baseSceneRectOffset;
	if (m_draggingSides.contains(LeftSide))
		sceneRect.setLeft(newBounds.x());
	else if (m_draggingSides.contains(RightSide))
		sceneRect.setRight(newBounds.x());
	if (m_draggingSides.contains(TopSide))
		sceneRect.setTop(newBounds.y());
	else if (m_draggingSides.contains(BottomSide))
		sceneRect.setBottom(newBounds.y());
	scene->setSceneRect(sceneRect | scene->extPiecesBoundingRect());
}

void Palapeli::ConstraintInteractor::stopInteraction(const Palapeli::MouseEvent& event)
{
	Q_UNUSED(event)
	m_draggingSides.clear();
}
