#include "savewidget.h"
#include "interfacemanager.h"
#include "../manager.h"

#include <KActionCollection>
#include <KLineEdit>
#include <KLocalizedString>
#include <KStandardGuiItem>
#include <KStandardShortcut>

//BEGIN Palapeli::SaveWidget

Palapeli::SaveWidget* Palapeli::SaveWidget::create(Palapeli::AutoscalingItem* parent)
{
	return new Palapeli::SaveWidget(new KLineEdit, parent);
}

Palapeli::SaveWidget::SaveWidget(KLineEdit* edit, Palapeli::AutoscalingItem* parent)
	: Palapeli::OnScreenDialog(edit, QList<KGuiItem>() << KStandardGuiItem::save() << KStandardGuiItem::cancel(), i18n("Enter a name"), parent)
	, m_edit(edit)
{
	connect(ppMgr(), SIGNAL(gameNameChanged(const QString&)), this, SLOT(setGameName(const QString&)));
	connect(this, SIGNAL(buttonPressed(int)), this, SLOT(handleButton(int)));
}

void Palapeli::SaveWidget::setGameName(const QString& name)
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

//END Palapeli::SaveWidget

//BEGIN Palapeli::SaveWidgetAction

Palapeli::SaveWidgetAction::SaveWidgetAction(QObject* parent)
	: KAction(KIcon("document-save"), i18n("&Save"), parent)
{
	setEnabled(false);
	setObjectName("palapeli_save");
	setShortcut(KStandardShortcut::shortcut(KStandardShortcut::Save));
	setToolTip(i18n("Save the current game"));

	connect(this, SIGNAL(triggered(bool)), this, SLOT(trigger()));
	connect(ppMgr(), SIGNAL(gameNameChanged(const QString&)), this, SLOT(setGameName(const QString&)));

	KActionCollection* collection = qobject_cast<KActionCollection*>(parent);
	if (collection)
		collection->addAction(objectName(), this);
}

void Palapeli::SaveWidgetAction::setGameName(const QString& name)
{
	Q_UNUSED(name)
	setEnabled(true); //a game is running now
}

void Palapeli::SaveWidgetAction::trigger()
{
	ppIMgr()->show(Palapeli::InterfaceManager::SaveWidget);
}

//END Palapeli::SaveWidget

#include "savewidget.moc"
