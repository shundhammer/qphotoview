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
    int percent = 0;

    if ( _sizes.size() > 0 )
        percent = 100 * _cache.size() / _sizes.size();

    logDebug() << "Unused images in prefetch cache: " << _cache.size()
               << " (" <<  percent << "%)" << endl;
    clear();

    if ( _workerThread.isRunning() )
	_workerThread.wait();
}


void PrefetchCache::prefetch( const QStringList & fileNames )
{
    {
	QMutexLocker locker( &_cacheMutex );
        logDebug() << "Prefetching " << fileNames.size() << " images" << endl;
        _stopWatch.start();

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
	    // logVerbose() << "Prefetch cache hit: " << imageFileName << endl;
	}
    }

    if ( cacheMiss )
    {
	logDebug() << "Prefetch cache miss: " << imageFileName << endl;
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


QString PrefetchCache::formatTime( qint64 millisec )
{
    QString formattedTime;
    int hours;
    int min;
    int sec;

    hours	= millisec / 3600000L;	// 60*60*1000
    millisec	%= 3600000L;

    min		= millisec / 60000L;	// 60*1000
    millisec	%= 60000L;

    sec		= millisec / 1000L;
    millisec	%= 1000L;

    if ( hours < 1 && min < 1 && sec < 60 )
    {
	formattedTime.sprintf ( "%2d.%03lld ", sec, millisec );
	formattedTime += QObject::tr( "sec" );
    }
    else
    {
	formattedTime.sprintf ( "%02d:%02d:%02d", hours, min, sec );
    }

    return formattedTime;
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
                qint64 elapsed = _prefetchCache->stopWatch().elapsed();
                qint64 timePerImage = 0;

                if ( _prefetchCache->size() > 0 )
                    timePerImage = elapsed / _prefetchCache->size();

		logInfo() << "Prefetching done after " << PrefetchCache::formatTime( elapsed ) << endl;
                logInfo() << "Cached images: " << _prefetchCache->size() << endl;
                logInfo() << "Time per image: " << PrefetchCache::formatTime( timePerImage ) << endl;
		return;
	    }

	    imageName = _prefetchCache->_jobQueue.takeFirst();
	}

	QString fullPath = _prefetchCache->fullPath( imageName );
	// logDebug() << "Prefetching " << fullPath << endl;
	QImage image;

	if ( ! image.load( fullPath ) )
	{
	    logWarning() << "Prefetching failed for " << fullPath << endl;
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
