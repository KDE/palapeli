#include <ThumbCreator>

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
};

bool Palapeli::ThumbCreator::create(const QString& path, int width, int height, QImage& image)
{
	Q_UNUSED(width) Q_UNUSED(height) //NOTE: The ThumbCreator APIDOX says that these params should be ignored for images read from the disk.
	//read archive
	KTar tar(path, "application/x-bzip");
	if (!tar.open(QIODevice::ReadOnly))
		return false;
	KTempDir cache;
	const QString cachePath = cache.name(); //note: includes trailing slash
	tar.directory()->copyTo(cachePath);
	tar.close();
	//read image
	image.load(cachePath + "thumbnail.jpg");
	return true;
}
