/***************************************************************************
 *   Copyright 2009 Stefan Majewsky <majewsky@gmx.net>
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

#include <KIO/ThumbCreator>
#include <kdemacros.h>
#include <QImage>
#include <KTar>
#include <KTempDir>

namespace Palapeli
{
	class ThumbCreator : public ::ThumbCreator
	{
		public:
			virtual bool create(const QString& path, int width, int height, QImage& image);
	};
}

extern "C"
{
	KDE_EXPORT ThumbCreator* new_creator()
	{
		return new Palapeli::ThumbCreator;
	}
}

bool Palapeli::ThumbCreator::create(const QString& path, int width, int height, QImage& image)
{
	Q_UNUSED(width) Q_UNUSED(height) //NOTE: The ThumbCreator APIDOX says that these params should be ignored for images read from the disk.
	//read archive
	KTar tar(path, "application/x-gzip");
	if (!tar.open(QIODevice::ReadOnly))
		return false;
	KTempDir cache;
	const QString cachePath = cache.name(); //note: includes trailing slash
	tar.directory()->copyTo(cachePath);
	tar.close();
	//read image
	image.load(cachePath + "image.jpg");
	return true;
}
