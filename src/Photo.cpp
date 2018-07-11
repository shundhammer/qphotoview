/*
 * QPhotoView core classes
 *
 * License: GPL V2. See file COPYING for details.
 *
 * Author:  Stefan Hundhammer <Stefan.Hundhammer@gmx.de>
 */

#include <QFileInfo>
#include <QDir>
#include <QDebug>

#include "Photo.h"
#include "PhotoDir.h"
#include "PrefetchCache.h"

long  Photo::_pixmapAccessCount	     = 0;
long  Photo::_thumbnailAccessCount   = 0;
QSize Photo::_thumbnailSize	     = QSize( 120, 80 );


Photo::Photo( const QString & fileName, PhotoDir *parentDir )
    : _photoDir( parentDir )
    , _lastPixmapAccess( 0 )
    , _lastThumbnailAccess( 0 )
{
    // qDebug() << __PRETTY_FUNCTION__ << fileName << parentDir;

    if ( _photoDir )
    {
	_fileName = fileName;
	// _path is fetched from _photoDir when needed
    }
    else
    {
	if ( ! fileName.isEmpty() )
	{
	    QFileInfo fileInfo( fileName );
	    _fileName = fileInfo.fileName();
	    _path     = fileInfo.absolutePath();
	}
    }
}


Photo::~Photo()
{

}


QPixmap Photo::fullSizePixmap()
{
    QPixmap pixmap( fullPath() );
    _size = pixmap.size();

    return pixmap;
}


QPixmap Photo::pixmap( const QSizeF & size )
{
    return pixmap( QSize( qRound( size.width()	),
			  qRound( size.height() ) ) );
}


QPixmap Photo::pixmap( const QSize & size )
{
    QPixmap scaledPixmap;

    if ( _pixmap.isNull() )
    {
	if ( _photoDir && _photoDir->prefetchCache() )
	{
	    _size   = _photoDir->prefetchCache()->pixelSize( _fileName );
	    _pixmap = _photoDir->prefetchCache()->pixmap( _fileName,
							  true ); // take
	}
    }

    qreal scaleFac = scaleFactor( _pixmap.size(), size );

    if ( scaleFac <= 1.0 ) // not larger than cached pixmap
    {
	scaledPixmap = scale( _pixmap, scaleFac );
    }
    else // larger than cached pixmap
    {
	scaledPixmap = fullSizePixmap();
	scaleFac     = scaleFactor( scaledPixmap.size(), size );
	scaledPixmap = scale( scaledPixmap, scaleFac );
    }

    _lastPixmapAccess = ++_pixmapAccessCount;
    // In theory, here should be a check for integer overflow.
    // In the real world, this won't ever be relevant, so we don't bother.

    return scaledPixmap;
}


void Photo::dropCache()
{
    _pixmap = QPixmap();
}


QPixmap Photo::thumbnail()
{
    _lastThumbnailAccess = ++_thumbnailAccessCount;
    // In theory, here should be a check for integer overflow.
    // In the real world, this won't ever be relevant, so we don't bother.

    QPixmap thumb;

    return thumb;
}


void Photo::clearCachedThumbnail()
{
    _thumbnail = QPixmap();
}


QSize Photo::size()
{
    if ( ! _size.isValid() )
    {
	if ( _photoDir && _photoDir->prefetchCache() )
	    _size = _photoDir->prefetchCache()->pixelSize( _fileName );
    }

    return _size;
}


PhotoMetaData Photo::metaData()
{
    return PhotoMetaData( this );
}


QString Photo::path() const
{
    if ( _photoDir )
	return _photoDir->path();
    else
	return _path;
}


QString Photo::fullPath() const
{
    QString result = path();

    if ( ! result.endsWith( QDir::separator() ) )
	result += QDir::separator();

    result += _fileName;

    return result;
}


void Photo::reparent( PhotoDir * newParentDir )
{
    if ( ! newParentDir && _photoDir )
	_path = _photoDir->path();

    if ( newParentDir )
	_path.clear();

    _photoDir = newParentDir;
}


QSize Photo::scale( const QSize & origSize, const QSize & boundingSize )
{
    return origSize * scaleFactor( origSize, boundingSize );
}


qreal Photo::scaleFactor( const QSize & origSize, const QSize & boundingSize )
{
    if ( origSize.width() == 0 || origSize.height() == 0 )
	return 0.0;

    qreal scaleFactorX = boundingSize.width()  / ( (qreal) origSize.width()  );
    qreal scaleFactorY = boundingSize.height() / ( (qreal) origSize.height() );

    return qMin( scaleFactorX, scaleFactorY );
}


QPixmap Photo::scale( const QPixmap & origPixmap, qreal scaleFactor )
{
    if ( qFuzzyCompare( scaleFactor, 1.0 ) )
	return origPixmap;

    if ( origPixmap.isNull() )
	return origPixmap;

    QPixmap pixmap( origPixmap );

#if 0
    // This turned out to be counterproductive: Qt optimizes far better than we
    // can. Benchmarks show that Qt::FastTransformation is only about 10%
    // faster than Qt::SmoothTransformation. Calling QPixmap::scaled() twice,
    // however, is more costly than that small benefit, so the idea of doing a
    // two-step scaling (one with Qt::FastTransformation to get near the
    // desired size and one with Qt::SmoothScale for optimum results)
    // backfires: It is actually more expensive than scaling with
    // Qt::SmoothScale immediately.

    if ( scaleFactor < 1.0 )
    {
	qreal tmpScaleFactor = scaleFactor * 1.1;

	if ( tmpScaleFactor < 0.9 )
	{
	    // Optimization: Fast but crude approximation to the real result:
	    // Use fast transformation in a first step to drastically reduce
	    // the number of (original) pixels to be considered in the smooth
	    // (but exact) transformation.

	    pixmap = pixmap.scaled( tmpScaleFactor * origPixmap.size(),
				    Qt::KeepAspectRatio,
				    Qt::FastTransformation );
	}
    }
    else if ( scaleFactor > 1.2 )
    {
	pixmap = pixmap.scaled( (scaleFactor - 0.1 ) * origPixmap.size(),
				Qt::KeepAspectRatio,
				Qt::FastTransformation );
    }
#endif

    pixmap = pixmap.scaled( scaleFactor * origPixmap.size(),
			    Qt::KeepAspectRatio,
			    Qt::SmoothTransformation );

    return pixmap;
}

