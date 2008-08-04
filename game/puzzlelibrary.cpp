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

#include "puzzlelibrary.h"
#include "manager.h"
#include "../storage/gamestorage.h"
#include "../storage/gamestorageitem.h"

#include <QAbstractItemDelegate>
#include <QAbstractListModel>
#include <QApplication>
#include <QPainter>
#include <QMutex>
#include <QTimer>
#include <QtConcurrentRun>
#include <KConfigGroup>
#include <KDesktopFile>
#include <KIcon> //for temporary code
#include <KLocalizedString>
#include <KStandardDirs>

namespace Palapeli
{

	struct PuzzleLibraryEntry
	{
		PuzzleLibraryEntry(const QString& identifier, const QString& name, const QString& comment, const QString& author, const QImage& image);
		QString identifier;
		QString name;
		QString comment;
		QString author;
		QImage image;
	};

	class PuzzleLibraryModel : public QAbstractListModel
	{
		public:
			PuzzleLibraryModel(QObject* parent = 0);
			virtual ~PuzzleLibraryModel();
			virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
			virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
			void reload();

			enum
			{
				CommentRole = Qt::UserRole,
				AuthorRole
			};
		protected:
			void doReload();
		private:
			QList<PuzzleLibraryEntry> m_entries;
			QMutex* m_entriesMutex;
			QFuture<void> m_entriesReloading;
	};

	class PuzzleLibraryDelegate : public QAbstractItemDelegate
	{
		public:
			PuzzleLibraryDelegate(QObject* parent = 0);
			virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
			void paintLine(const QString& text, QRect& boundingRect, QPainter* painter, const QStyleOptionViewItem& option) const;
			virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;

			static const int IconSize = 64;
			static const int Margin = 5;
	};

	class PuzzleLibraryPrivate
	{
		public:
			PuzzleLibraryPrivate();

			PuzzleLibraryModel m_model;
			PuzzleLibraryDelegate m_delegate;
	};

}

//BEGIN Palapeli::PuzzleLibraryModel

Palapeli::PuzzleLibraryEntry::PuzzleLibraryEntry(const QString& entryIdentifier, const QString& entryName, const QString& entryComment, const QString& entryAuthor, const QImage& entryImage)
	: identifier(entryIdentifier)
	, name(entryName)
	, comment(entryComment)
	, author(entryAuthor)
	, image(entryImage)
{
}

Palapeli::PuzzleLibraryModel::PuzzleLibraryModel(QObject* parent)
	: QAbstractListModel(parent)
	, m_entriesMutex(new QMutex)
{
	reload();
}

Palapeli::PuzzleLibraryModel::~PuzzleLibraryModel()
{
	if (m_entriesReloading.isRunning())
		m_entriesReloading.waitForFinished(); //the running thread would otherwise cause a crash
	delete m_entriesMutex;
}

int Palapeli::PuzzleLibraryModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent)
	m_entriesMutex->lock();
	int count = m_entries.count();
	m_entriesMutex->unlock();
	return count;
}

QVariant Palapeli::PuzzleLibraryModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return QVariant();
	m_entriesMutex->lock();
	if (index.row() >= m_entries.count())
		return QVariant();
	const Palapeli::PuzzleLibraryEntry& entry = m_entries[index.row()];
	m_entriesMutex->unlock();
	switch (role)
	{
		case Qt::DisplayRole:
			return entry.name;
		case Qt::DecorationRole:
			return entry.image;
		case Palapeli::PuzzleLibraryModel::CommentRole:
			return entry.comment;
		case Palapeli::PuzzleLibraryModel::AuthorRole:
			return entry.author;
		default:
			return QVariant();
	}
}

#include <KDebug>

void Palapeli::PuzzleLibraryModel::reload()
{
	if (!m_entriesReloading.isRunning())
		m_entriesReloading = QtConcurrent::run(this, &Palapeli::PuzzleLibraryModel::doReload);
}

void Palapeli::PuzzleLibraryModel::doReload() //to be executed in a separate thread
{
	m_entriesMutex->lock();
	m_entries.clear();
	m_entriesMutex->unlock();
	//find entries
	KStandardDirs dirs;
	QStringList puzzleFiles = dirs.findAllResources("data", QLatin1String("palapeli/puzzlelibrary/*.desktop"), KStandardDirs::NoDuplicates);
	foreach (const QString& puzzleFile, puzzleFiles)
	{
		const QString identifier = puzzleFile.section('/', -1, -1).section('.', 1, 1);
		const KDesktopFile df(puzzleFile);
		//gather data - name and comment
		QString name = df.readName();
		if (name.isEmpty())
			name = identifier;
		const QString comment = df.readComment();
		const QString author = df.desktopGroup().readEntry("X-KDE-PluginInfo-Author", QString());
		//gather data - image (scale down to the icon size used in the list view)
		const QString iconName = df.readIcon().remove('/'); //slashes are removed to avoid directory changing
		const QString iconFile = dirs.locate("data", QLatin1String("palapeli/puzzlelibrary/") + iconName);
		const QImage baseImage(iconFile);
		const QImage scaledImage = baseImage.scaled(Palapeli::PuzzleLibraryDelegate::IconSize, Palapeli::PuzzleLibraryDelegate::IconSize, Qt::KeepAspectRatio);
		//save entry
		m_entriesMutex->lock();
		m_entries << Palapeli::PuzzleLibraryEntry(identifier, name, comment, author, scaledImage);
		m_entriesMutex->unlock();
	}
	//propagate changes
	reset();
}

//END Palapeli::PuzzleLibraryModel

//BEGIN Palapeli::PuzzleLibraryDelegate

Palapeli::PuzzleLibraryDelegate::PuzzleLibraryDelegate(QObject* parent)
	: QAbstractItemDelegate(parent)
{
}

void Palapeli::PuzzleLibraryDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	//highlight selected item
	QStyleOptionViewItemV4 opt = option;
	QStyle* style = opt.widget ? opt.widget->style() : QApplication::style();
	style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);
	//draw icon - resize to fit into QRect((Margin, Margin) IconSize x IconSize); and center in this area
	const QImage icon = index.data(Qt::DecorationRole).value<QImage>();
	if (!icon.isNull())
	{
		//find metrics
		const int xPadding = (IconSize - icon.width()) / 2;
		const int yPadding = (IconSize - icon.height()) / 2;
		int x = option.rect.x() + Margin + xPadding;
		if (option.direction == Qt::RightToLeft)
			x = option.rect.right() - (Margin + xPadding + IconSize);
		//draw icon
		painter->save();
		painter->translate(x, option.rect.y() + Margin + yPadding);
		painter->drawImage(QPointF(0, 0), icon);
		painter->restore();
	}
	//draw text besides the icon space
	painter->save();
	static const int iconAreaWidth = IconSize + 2 * Margin;
	QRect boundingRect(option.rect.x() + iconAreaWidth, option.rect.y(), option.rect.width() - iconAreaWidth, option.rect.height());
	//font for first line
	QFont font = option.font;
	font.setWeight(QFont::Bold);
	painter->setFont(font);
	//first line
	const QString name = index.data(Qt::DisplayRole).toString();
	paintLine(name, boundingRect, painter, option);
	//font for second line
	font.setWeight(QFont::Normal);
	painter->setFont(font);
	//second line
	const QString comment = index.data(Palapeli::PuzzleLibraryModel::CommentRole).toString();
	if (!comment.isEmpty())
		paintLine(comment, boundingRect, painter, option);
	//font for third line
	font.setStyle(QFont::StyleItalic);
	painter->setFont(font);
	//third line
	const QString author = index.model()->data(index, Palapeli::PuzzleLibraryModel::AuthorRole).toString();
	if (!author.isEmpty())
		paintLine(i18nc("@item:intext", "by %1", author), boundingRect, painter, option);
	painter->restore();
}

void Palapeli::PuzzleLibraryDelegate::paintLine(const QString& text, QRect& boundingRect, QPainter* painter, const QStyleOptionViewItem& option) const
{
	//set font
	const QFontMetrics fm(painter->font());
	//adjust painting position
	const QRect textRect = fm.boundingRect(text);
	int left = boundingRect.left() + Margin;
	if (option.direction == Qt::RightToLeft)
		left = boundingRect.right() - textRect.width() - Margin;
	boundingRect.setTop(boundingRect.top() + textRect.height());
	painter->drawText(left, boundingRect.y(), fm.elidedText(text, option.textElideMode, boundingRect.width() - Margin));
}

QSize Palapeli::PuzzleLibraryDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	Q_UNUSED(option)
	Q_UNUSED(index)
	return QSize(100, 2 * Margin + IconSize); //height is the important point, width is determined by widget bounds
}

//END Palapeli::PuzzleLibraryDelegate

//BEGIN Palapeli::PuzzleLibrary

Palapeli::PuzzleLibraryPrivate::PuzzleLibraryPrivate()
{
}

Palapeli::PuzzleLibrary::PuzzleLibrary(QWidget* parent)
	: QListView(parent)
	, p(new Palapeli::PuzzleLibraryPrivate)
{
	setModel(&p->m_model);
	setItemDelegate(&p->m_delegate);
}

Palapeli::PuzzleLibrary::~PuzzleLibrary()
{
	delete p;
}

void Palapeli::PuzzleLibrary::reload()
{
	p->m_model.reload();
	//TODO: set selection on first item
}

void Palapeli::PuzzleLibrary::startSelectedGame()
{
}

//END Palapeli::PuzzleLibrary

#include "puzzlelibrary.moc"
