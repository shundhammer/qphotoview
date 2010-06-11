#include "PrefetchCache.h"
#include <QDebug>
#include <QApplication>
#include <QDesktopWidget>


PrefetchCache::PrefetchCache( int sizeMB )
    : m_workerThread( this )
{
    m_maxSize  = sizeMB;
    m_maxSize *= 1024 * 1024;
    m_cacheMisses = 0;
    m_cacheHits   = 0;

    m_fullScreenSize = qApp->desktop()->screenGeometry().size();
}


PrefetchCache::~PrefetchCache()
{
    clear();
    
    if ( m_workerThread.isRunning() )
	m_workerThread.wait();

    qDebug() << "Prefetch cache hits:" << m_cacheHits
	     << "cache misses:" << m_cacheMisses;
}


void PrefetchCache::prefetch( const QString & path,
			      const QStringList & filenames )
{
    {
	QMutexLocker locker( &m_cacheMutex );

	foreach ( QString filename, filenames )
	{
	    m_jobQueue.append( path + "/" + filename );
	}
    }

    if ( ! m_workerThread.isRunning() )
	m_workerThread.start();
}


QPixmap PrefetchCache::pixmap( const QString & fullPixmapPath, const QSize & size )
{
    QImage image;
    bool cacheMiss = true;

    {
	QMutexLocker locker( &m_cacheMutex );

	if ( m_cache.contains( fullPixmapPath ) )
	{
	    image = m_cache.value( fullPixmapPath );

	    if ( image.size().height() == size.height() ||
		 image.size().width()  == size.width()    )
	    {
		++m_cacheHits;
		cacheMiss = false;
		// qDebug() << "Prefetch cache hit:" << fullPixmapPath;
	    }
	    else
	    {
		// qDebug() << "Cached image with" << image.size() << "available, but" << size << "requested";
	    }
	}
    }

    if ( cacheMiss )
    {
	++m_cacheMisses;
	// qDebug() << "Prefetch cache miss:" << fullPixmapPath;
	image.load( fullPixmapPath );
	image = image.scaled( size, Qt::KeepAspectRatio, Qt::SmoothTransformation );

#if 0
	QMutexLocker locker( &m_cacheMutex );
	m_cache.insert( fullPixmapPath, image );
	
	if ( m_jobQueue.contains( fullPixmapPath ) )
	    m_jobQueue.removeAll( fullPixmapPath );
#endif
    }


    return QPixmap::fromImage( image );
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
}


int PrefetchCache::finalImageCount()
{
    QMutexLocker locker( &m_cacheMutex );
    return m_cache.size() + m_jobQueue.size();
}


int PrefetchCache::imageCount()
{
    QMutexLocker locker( &m_cacheMutex );
    return m_cache.size();
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
		qDebug() << "Prefetch jobs done - terminating worker thread;"
			 << "images in cache:" << m_prefetchCache->m_cache.size();
		return;
	    }
	    
	    imageName = m_prefetchCache->m_jobQueue.takeFirst();
	}

	// qDebug() << "Prefetching" << imageName;
	QImage image;
	if ( ! image.load( imageName ) )
	    qDebug() << "Prefetching failed for" << imageName;
	else
	{
	    QImage scaledImage = image.scaled( m_prefetchCache->m_fullScreenSize,
					       Qt::KeepAspectRatio,
					       Qt::SmoothTransformation );
	    QMutexLocker locker( &m_prefetchCache->m_cacheMutex );
	    m_prefetchCache->m_cache.insert( imageName, scaledImage );
	}
    }
}
