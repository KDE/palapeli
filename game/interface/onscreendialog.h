#ifndef PALAPELI_ONSCREENDIALOG_H
#define PALAPELI_ONSCREENDIALOG_H

#include "onscreenwidget.h"

class QSignalMapper;
class KGuiItem;
class KIcon;
class KPushButton;

namespace Palapeli
{

	class OnScreenDialog : public OnScreenWidget
	{
		Q_OBJECT
		public:
			OnScreenDialog(QWidget* widget, QList<KGuiItem> buttons, const QString& title = QString(), QGraphicsItem* parent = 0); //takes ownership of widget
			~OnScreenDialog();

			void setButtonGuiItem(int id, const KGuiItem& item);
			void setButtonIcon(int id, const KIcon& icon);
			void setButtonText(int id, const QString& text);
		Q_SIGNALS:
			void buttonPressed(int id);
		private:
			QSignalMapper* m_mapper;
			QList<KPushButton*> m_buttons;
	};

}

#endif // PALAPELI_ONSCREENDIALOG_H
