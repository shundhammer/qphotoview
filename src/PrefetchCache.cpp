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
#include "Logger.h"


PrefetchCache::PrefetchCache( const QString & path )
    : _path( path )
    , _workerThread( this )
{
    _fullScreenSize = qApp->desktop()->screenGeometry().size();
}


PrefetchCache::~PrefetchCache()
{
    logDebug() << "Unused images in prefetch cache: " << _cache.size()
               << " (" << 100*_cache.size() / _sizes.size() << "%)" << endl;
    clear();

    if ( _workerThread.isRunning() )
	_workerThread.wait();
}


void PrefetchCache::prefetch( const QStringList & fileNames )
{
    {
	QMutexLocker locker( &_cacheMutex );

	foreach ( QString fileName, fileNames )
	{
	    _jobQueue.append( fileName );
	}
    }

    if ( ! _workerThread.isRunning() )
	_workerThread.start();
}


QPixmap PrefetchCache::pixmap( const QString & imageFileName, bool take )
{
    QImage image;
    bool cacheMiss = true;

    {
	QMutexLocker locker( &_cacheMutex );

	if ( _cache.contains( imageFileName ) )
	{

	    image = take ?
		_cache.take ( imageFileName ) :
		_cache.value( imageFileName );

	    cacheMiss = false;
	    // logDebug() << "Prefetch cache hit: " << imageFileName << endl;
	}
    }

    if ( cacheMiss )
    {
	// logDebug() << "Prefetch cache miss: " << imageFileName << endl;
	image.load( fullPath( imageFileName ) );
	QSize size = image.size();
	qreal scaleFactor = Photo::scaleFactor( size, _fullScreenSize );

	if ( scaleFactor < 1.0 )
	{
	    image = image.scaled( _fullScreenSize,
				  Qt::KeepAspectRatio,
				  Qt::SmoothTransformation );
	}

	QMutexLocker locker( &_cacheMutex );
	if ( take )
	    _cache.insert( imageFileName, image );
	_sizes.insert( imageFileName, size  );

	if ( _jobQueue.contains( imageFileName ) )
	    _jobQueue.removeAll( imageFileName );
    }

    return QPixmap::fromImage( image );
}


QSize PrefetchCache::pixelSize( const QString & imageFileName )
{
    if ( _sizes.contains( imageFileName ) )
	return _sizes.value( imageFileName );
    else
	return pixmap( imageFileName ).size();
}


void PrefetchCache::clear()
{
    {
	QMutexLocker locker( &_cacheMutex );
	_jobQueue.clear();
    }

    if ( _workerThread.isRunning() )
	_workerThread.wait();

    QMutexLocker locker( &_cacheMutex ); // not strictly necessary
    _cache.clear();
    // not clearing _sizes - this is very cheap
}


QString PrefetchCache::fullPath( const QString & imageFileName )
{
    return _path + "/" + imageFileName;
}



PrefetchCacheWorkerThread::PrefetchCacheWorkerThread( PrefetchCache * prefetchCache )
    : _prefetchCache( prefetchCache )
{

}


void PrefetchCacheWorkerThread::run()
{
    while ( true )
    {
	QString imageName;

	{
	    QMutexLocker locker( &_prefetchCache->_cacheMutex );

	    if ( _prefetchCache->_jobQueue.isEmpty() )
	    {
#if 0
		logDebug() << "Prefetch jobs done - terminating worker thread;"
                           << "images in cache: "
                           << _prefetchCache->_cache.size()
                           << endl;
#endif
		return;
	    }

	    imageName = _prefetchCache->_jobQueue.takeFirst();
	}

	QString fullPath = _prefetchCache->fullPath( imageName );
	// logDebug() << "Prefetching " << fullPath << endl;
	QImage image;

	if ( ! image.load( fullPath ) )
	{
	    // logDebug() << "Prefetching failed for " << fullPath << endl;
	}
	else
	{
	    QSize size = image.size();
	    QSize targetSize = _prefetchCache->_fullScreenSize;

	    if ( Photo::scaleFactor( size, targetSize ) < 1.0 )
	    {
		image = image.scaled( targetSize,
				      Qt::KeepAspectRatio,
				      Qt::SmoothTransformation );
	    }

	    QMutexLocker locker( &_prefetchCache->_cacheMutex );
	    _prefetchCache->_cache.insert( imageName, image );
	    _prefetchCache->_sizes.insert( imageName, size  );
	}
    }
}
