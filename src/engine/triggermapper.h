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

#ifndef PALAPELI_TRIGGERMAPPER_H
#define PALAPELI_TRIGGERMAPPER_H

#include "interactor.h"
#include "trigger.h"

#include <QMap>
class QKeyEvent;
class QMouseEvent;
class QWheelEvent;

namespace Palapeli
{
	struct EventContext
	{
		Palapeli::EventProcessingFlags flags;
		Qt::MouseButtons triggeringButtons;
	};

	class TriggerMapper : public QObject
	{
		Q_OBJECT
		public:
			static Palapeli::TriggerMapper* instance();
			static QMap<QByteArray, Palapeli::Interactor*> createInteractors(QGraphicsView* view);

			QMap<QByteArray, Palapeli::Trigger> associations() const;
			static QMap<QByteArray, Palapeli::Trigger> defaultAssociations();
		Q_SIGNALS:
			void associationsChanged();
		public Q_SLOTS:
			void readSettings();
			void setAssociations(const QMap<QByteArray, Palapeli::Trigger>& associations);
		protected:
			friend class InteractorManager;

			Palapeli::EventProcessingFlags testTrigger(const QByteArray& interactor, QWheelEvent* event) const;
			Palapeli::EventContext testTrigger(const QByteArray& interactor, QMouseEvent* event) const;
			Palapeli::EventContext testTrigger(const QByteArray& interactor, QKeyEvent* event, Qt::MouseButtons buttons) const;
		private:
			TriggerMapper();

			Palapeli::EventProcessingFlags testTrigger(const Palapeli::Trigger& trigger, QWheelEvent* event) const;
			Palapeli::EventProcessingFlags testTrigger(const Palapeli::Trigger& trigger, QMouseEvent* event) const;
			Palapeli::EventProcessingFlags testTrigger(const Palapeli::Trigger& trigger, QKeyEvent* event, Qt::MouseButtons buttons) const;

			QMap<QByteArray, Palapeli::Trigger> m_associations;
			//quasi-static data
			QMap<Qt::Key, Qt::KeyboardModifier> m_keyModifierMap;
	};
}

#endif // PALAPELI_TRIGGERMAPPER_H
