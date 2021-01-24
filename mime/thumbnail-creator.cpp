/*
    SPDX-FileCopyrightText: 2009 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <KIO/ThumbCreator>
#include <QImage>
#include <QTemporaryDir>
#include <KTar>

namespace Palapeli
{
	class ThumbCreator : public ::ThumbCreator
	{
		public:
			bool create(const QString& path, int width, int height, QImage& image) override;
	};
}

extern "C"
{
	Q_DECL_EXPORT ThumbCreator* new_creator()
	{
		return new Palapeli::ThumbCreator;
	}
}

bool Palapeli::ThumbCreator::create(const QString& path, int width, int height, QImage& image)
{
	Q_UNUSED(width) Q_UNUSED(height) //NOTE: The ThumbCreator APIDOX says that these params should be ignored for images read from the disk.
	//read archive
	KTar tar(path, QStringLiteral("application/x-gzip"));
	if (!tar.open(QIODevice::ReadOnly))
		return false;
	QTemporaryDir cache;
	const QString cachePath = cache.path() + QLatin1Char('/');
	tar.directory()->copyTo(cachePath);
	tar.close();
	//read image
	image.load(cachePath + QLatin1String("image.jpg"));
	return true;
}
