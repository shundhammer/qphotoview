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

long  Photo::m_pixmapAccessCount      = 0;
long  Photo::m_thumbnailAccessCount   = 0;
QSize Photo::m_thumbnailSize          = QSize( 120, 80 );


Photo::Photo( const QString & fileName, PhotoDir *parentDir )
    : m_photoDir( parentDir )
    , m_lastPixmapAccess( 0 )
    , m_lastThumbnailAccess( 0 )
{
    // qDebug() << __PRETTY_FUNCTION__ << fileName << parentDir;

    if ( m_photoDir )
    {
        m_fileName = fileName;
        // m_path is fetched from m_photoDir when needed
    }
    else
    {
        if ( ! fileName.isEmpty() )
        {
            QFileInfo fileInfo( fileName );
            m_fileName = fileInfo.fileName();
            m_path     = fileInfo.absolutePath();
        }
    }
}


Photo::~Photo()
{

}


QPixmap Photo::fullSizePixmap()
{
    QPixmap pixmap( fullPath() );
    m_size = pixmap.size();

    return pixmap;
}


QPixmap Photo::pixmap( const QSize & size )
{
    QPixmap scaledPixmap;

    if ( m_pixmap.isNull() )
    {
        if ( m_photoDir && m_photoDir->prefetchCache() )
        {
            m_size   = m_photoDir->prefetchCache()->pixelSize( m_fileName );
            m_pixmap = m_photoDir->prefetchCache()->pixmap( m_fileName,
                                                            true ); // take
        }
    }

    qreal scaleFac = scaleFactor( m_pixmap.size(), size );

    if ( scaleFac <= 1.0 ) // not larger than cached pixmap
    {
        scaledPixmap = scale( m_pixmap, scaleFac );
    }
    else // larger than cached pixmap
    {
        scaledPixmap = fullSizePixmap();
        scaleFac     = scaleFactor( scaledPixmap.size(), size );
        scaledPixmap = scale( scaledPixmap, scaleFac );
    }

    m_lastPixmapAccess = ++m_pixmapAccessCount;
    // In theory, here should be a check for integer overflow.
    // In the real world, this won't ever be relevant, so we don't bother.

    return scaledPixmap;
}


void Photo::dropCache()
{
    m_pixmap = QPixmap();
}


QPixmap Photo::thumbnail()
{
    m_lastThumbnailAccess = ++m_thumbnailAccessCount;
    // In theory, here should be a check for integer overflow.
    // In the real world, this won't ever be relevant, so we don't bother.

    QPixmap thumb;

    return thumb;
}


void Photo::clearCachedThumbnail()
{
    m_thumbnail = QPixmap();
}


QSize Photo::size()
{
    if ( ! m_size.isValid() )
    {
        if ( m_photoDir && m_photoDir->prefetchCache() )
            m_size = m_photoDir->prefetchCache()->pixelSize( m_fileName );
    }

    return m_size;
}


#if 0
PhotoMetaData Photo::metaData()
{

}
#endif


QString Photo::path() const
{
    if ( m_photoDir )
        return m_photoDir->path();
    else
        return m_path;
}


QString Photo::fullPath() const
{
    QString result = path();

    if ( ! result.endsWith( QDir::separator() ) )
        result += QDir::separator();

    result += m_fileName;

    return result;
}


void Photo::reparent( PhotoDir * newParentDir )
{
    if ( ! newParentDir && m_photoDir )
        m_path = m_photoDir->path();

    if ( newParentDir )
        m_path.clear();

    m_photoDir = newParentDir;
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

