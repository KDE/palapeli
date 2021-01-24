/*
    SPDX-FileCopyrightText: 2009-2011 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PALAPELI_IMPORTHELPER_H
#define PALAPELI_IMPORTHELPER_H

#include <QObject>

namespace Palapeli
{
	class ImportHelper : public QObject
	{
		Q_OBJECT
		public:
            explicit ImportHelper(const QString &path);
		public Q_SLOTS:
			void doWork();
		private:
            QString m_path;
	};
}

#endif // PALAPELI_IMPORTHELPER_H
