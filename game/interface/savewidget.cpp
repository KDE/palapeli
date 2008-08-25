#include "savewidget.h"
#include "interfacemanager.h"
#include "../manager.h"

#include <KLineEdit>
#include <KLocalizedString>
#include <KStandardGuiItem>

Palapeli::SaveWidget* Palapeli::SaveWidget::create(QGraphicsItem* parent)
{
	return new Palapeli::SaveWidget(new KLineEdit, parent);
}

Palapeli::SaveWidget::SaveWidget(KLineEdit* edit, QGraphicsItem* parent)
	: Palapeli::OnScreenDialog(edit, QList<KGuiItem>() << KStandardGuiItem::save() << KStandardGuiItem::cancel(), i18n("Enter a name"), parent)
	, m_edit(edit)
{
	connect(ppMgr(), SIGNAL(gameNameChanged(const QString&)), this, SLOT(setPredefinedName(const QString&)));
	connect(this, SIGNAL(buttonPressed(int)), this, SLOT(handleButton(int)));
}

void Palapeli::SaveWidget::setPredefinedName(const QString& name)
{
	m_edit->setText(name);
}

void Palapeli::SaveWidget::handleButton(int id)
{
	if (id == 0) // save (the other option, cancel, does nothing)
	{
		if (!m_edit->text().isEmpty())
			ppMgr()->saveGame(m_edit->text());
	}
	ppIMgr()->hide(Palapeli::InterfaceManager::SaveWidget);
}
