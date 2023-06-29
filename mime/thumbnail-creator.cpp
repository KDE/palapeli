/*
    SPDX-FileCopyrightText: 2009 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "thumbnail-creator.h"
#include <KIO/ThumbnailCreator>
#include <KPluginFactory>
#include <QImage>
#include <QTemporaryDir>
#include <KTar>

K_PLUGIN_CLASS_WITH_JSON(PalapeliThumbCreator, "palathumbcreator.json")

PalapeliThumbCreator::PalapeliThumbCreator(QObject *parent, const QVariantList &args)
    : KIO::ThumbnailCreator(parent, args)
{
}

PalapeliThumbCreator::~PalapeliThumbCreator() = default;

KIO::ThumbnailResult PalapeliThumbCreator::create(const KIO::ThumbnailRequest &request)
{
	//read archive
    KTar tar(request.url().toLocalFile(), QStringLiteral("application/x-gzip"));
	if (!tar.open(QIODevice::ReadOnly))
        return KIO::ThumbnailResult::fail();
	QTemporaryDir cache;
	const QString cachePath = cache.path() + QLatin1Char('/');
	tar.directory()->copyTo(cachePath);
	tar.close();
	//read image
    QImage image;
    image.load(cachePath + QLatin1String("image.jpg"));
    if (!image.isNull()) {
        return KIO::ThumbnailResult::pass(image);
    }
    return KIO::ThumbnailResult::fail();
}

#include "moc_thumbnail-creator.cpp"
#include "thumbnail-creator.moc"
