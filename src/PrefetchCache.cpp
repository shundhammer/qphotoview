/*
 * QPhotoView core classes
 *
 * License: GPL V2. See file COPYING for details.
 *
 * Author:  Stefan Hundhammer <Stefan.Hundhammer@gmx.de>
 */

#include <QDebug>
#include <QApplication>
#include <QDesktopWidget>

#include "PrefetchCache.h"
#include "Photo.h"


PrefetchCache::PrefetchCache( const QString & path )
    : m_path( path )
    , m_workerThread( this )
{
    m_fullScreenSize = qApp->desktop()->screenGeometry().size();
}


PrefetchCache::~PrefetchCache()
{
    qDebug() << "Unused images in prefetch cache:" << m_cache.size()
             << "(" << 100*m_cache.size() / m_sizes.size() << "%)";
    clear();

    if ( m_workerThread.isRunning() )
	m_workerThread.wait();
}


void PrefetchCache::prefetch( const QStringList & fileNames )
{
    {
	QMutexLocker locker( &m_cacheMutex );

	foreach ( QString fileName, fileNames )
	{
	    m_jobQueue.append( fileName );
	}
    }

    if ( ! m_workerThread.isRunning() )
	m_workerThread.start();
}


QPixmap PrefetchCache::pixmap( const QString & imageFileName, bool take )
{
    QImage image;
    bool cacheMiss = true;

    {
	QMutexLocker locker( &m_cacheMutex );

	if ( m_cache.contains( imageFileName ) )
	{
            
	    image = take ?
                m_cache.take ( imageFileName ) :
                m_cache.value( imageFileName );
            
            cacheMiss = false;
            // qDebug() << "Prefetch cache hit:" << imageFileName;
	}
    }

    if ( cacheMiss )
    {
	// qDebug() << "Prefetch cache miss:" << imageFileName;
	image.load( fullPath( imageFileName ) );
        QSize size = image.size();
        qreal scaleFactor = Photo::scaleFactor( size, m_fullScreenSize );

        if ( scaleFactor < 1.0 )
        {
            image = image.scaled( m_fullScreenSize,
                                  Qt::KeepAspectRatio,
                                  Qt::SmoothTransformation );
        }

	QMutexLocker locker( &m_cacheMutex );
        if ( take )
            m_cache.insert( imageFileName, image );
        m_sizes.insert( imageFileName, size  );

	if ( m_jobQueue.contains( imageFileName ) )
	    m_jobQueue.removeAll( imageFileName );
    }

    return QPixmap::fromImage( image );
}


QSize PrefetchCache::pixelSize( const QString & imageFileName )
{
    if ( m_sizes.contains( imageFileName ) )
        return m_sizes.value( imageFileName );
    else
        return pixmap( imageFileName ).size();
}


void PrefetchCache::clear()
{
    {
	QMutexLocker locker( &m_cacheMutex );
	m_jobQueue.clear();
    }

    if ( m_workerThread.isRunning() )
	m_workerThread.wait();

    QMutexLocker locker( &m_cacheMutex ); // not strictly necessary
    m_cache.clear();
    // not clearing m_sizes - this is very cheap
}


QString PrefetchCache::fullPath( const QString & imageFileName )
{
    return m_path + "/" + imageFileName;
}



PrefetchCacheWorkerThread::PrefetchCacheWorkerThread( PrefetchCache * prefetchCache )
    : m_prefetchCache( prefetchCache )
{

}


void PrefetchCacheWorkerThread::run()
{
    while ( true )
    {
	QString imageName;

	{
	    QMutexLocker locker( &m_prefetchCache->m_cacheMutex );

	    if ( m_prefetchCache->m_jobQueue.isEmpty() )
	    {
#if 0
		qDebug() << "Prefetch jobs done - terminating worker thread;"
			 << "images in cache:"
                         << m_prefetchCache->m_cache.size();
#endif
		return;
	    }

	    imageName = m_prefetchCache->m_jobQueue.takeFirst();
	}

        QString fullPath = m_prefetchCache->fullPath( imageName );
	// qDebug() << "Prefetching" << fullPath;
	QImage image;

	if ( ! image.load( fullPath ) )
        {
	    qDebug() << "Prefetching failed for" << fullPath;
        }
	else
	{
            QSize size = image.size();
            QSize targetSize = m_prefetchCache->m_fullScreenSize;

            if ( Photo::scaleFactor( size, targetSize ) < 1.0 )
            {
                image = image.scaled( targetSize,
                                      Qt::KeepAspectRatio,
                                      Qt::SmoothTransformation );
            }

	    QMutexLocker locker( &m_prefetchCache->m_cacheMutex );
	    m_prefetchCache->m_cache.insert( imageName, image );
	    m_prefetchCache->m_sizes.insert( imageName, size  );
	}
    }
}
