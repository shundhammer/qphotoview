/*
 * QPhotoView core classes
 *
 * License: GPL V2. See file COPYING for details.
 *
 * Author:  Stefan Hundhammer <Stefan.Hundhammer@gmx.de>
 */

#include <QDir>
#include <QFileInfo>
#include <QStringList>
#include <QDebug>

#include "PhotoDir.h"
#include "Photo.h"
#include "PrefetchCache.h"
#include "Logger.h"


PhotoDir::PhotoDir( const QString & path, bool jpgOnly )
    : _path( path )
    , _current( -1 )
    , _jpgOnly( jpgOnly )
{
    while ( _path.endsWith( "/" ) && _path.size() > 1 )
        _path.chop( 1 );

    QString startPhotoName;
    QFileInfo fileInfo( _path );

    if ( fileInfo.isDir() )
    {
	_path = fileInfo.absolutePath() + "/" + _path;
    }
    else
    {
	_path	       = fileInfo.absolutePath();
	startPhotoName = fileInfo.fileName();
    }

    logInfo() << "New photo dir " << _path << endl;
    _prefetchCache = new PrefetchCache( _path );
    read( _path, startPhotoName );
}


PhotoDir::~PhotoDir()
{
    qDeleteAll( _photos );
    delete _prefetchCache;
}


void PhotoDir::read( const QString & dirPath, const QString & startPhotoName )
{
    QStringList nameFilters;
    nameFilters << "*.jpg" << "*.jpeg" << "*.JPG" << "*.JPEG";

    if ( ! _jpgOnly )
    {
	nameFilters << "*.png"	<< "*.PNG"
		    << "*.gif"	<< "*.GIF"
		    << "*.bmp"	<< "*.BMP"
		    << "*.tif"	<< "*.TIF"
		    << "*.tiff" << "*.TIFF"
		    << "*.xpm"	<< "*.XPM"
		    << "*.ppm"	<< "*.PPM"
		    << "*.pgm"	<< "*.PGM"
		    << "*.pbm"	<< "*.PBM";
    }

    QDir dir ( dirPath );
    _path = dir.absolutePath();
    QStringList imageFileNames = dir.entryList( nameFilters,
						QDir::Files,  // wanted type
						QDir::Name ); // sort by

    foreach ( QString imageFileName, imageFileNames )
    {
	Photo * photo = new Photo( imageFileName, this );
	_photos.append( photo );

	if ( imageFileName == startPhotoName )
	    _current = _photos.size()-1;
    }

    if ( ! _photos.isEmpty() && _current < 0 )
	_current = 0;
}


Photo * PhotoDir::photo( int index ) const
{
    if ( index < 0 || index >= _photos.size() )
	return 0;

    return _photos.at( index );
}


Photo * PhotoDir::current() const
{
    if ( _photos.isEmpty() )
	return 0;

    int current = qBound( 0, _current, _photos.size()-1 );
    return _photos.at( current );
}


Photo * PhotoDir::first() const
{
    if ( _photos.isEmpty() )
	return 0;

    return _photos.first();
}


Photo * PhotoDir::last() const
{
    if ( _photos.isEmpty() )
	return 0;

    return _photos.last();
}


Photo * PhotoDir::setCurrent( int index )
{
    if ( _photos.isEmpty() )
	return 0;

    _current = qBound( 0, index, _photos.size()-1 );
    return _photos.at( _current );
}


void PhotoDir::setCurrent( Photo * photo )
{
    int index = find( photo );

    if ( index >= 0 )
	_current = index;
}


int PhotoDir::find( Photo * photo )
{
    return _photos.indexOf( photo );
}


Photo * PhotoDir::toFirst()
{
    if ( _photos.isEmpty() )
	return 0;

    _current = 0;
    return _photos.first();
}


Photo * PhotoDir::toLast()
{
    if ( _photos.isEmpty() )
	return 0;

    _current = _photos.size()-1;
    return _photos.last();
}


Photo * PhotoDir::toNext()
{
    if ( _photos.isEmpty() )
	return 0;

    ++_current;
    _current = qBound( 0, _current, _photos.size()-1 );

    return _photos.at( _current );
}


Photo * PhotoDir::toPrevious()
{
    if ( _photos.isEmpty() )
	return 0;

    --_current;
    _current = qBound( 0, _current, _photos.size()-1 );

    return _photos.at( _current );
}


void PhotoDir::prefetch()
{
    if ( _photos.isEmpty() )
	return;

    QStringList jobs;
    int last = _photos.size()-1;

    if ( _current >= 0	 )     addJob( jobs, _current	);
    if ( _current < last )     addJob( jobs, _current+1 );
    if ( _current > 0	 )     addJob( jobs, _current-1 );

    if ( _current > 1	   )   addJob( jobs, 0 );
    if ( last > _current+1 )   addJob( jobs, last );

    for ( int i = _current+2; i < last; ++i )
	addJob( jobs, i );

    for ( int i = _current-2; i > 0; --i )
	addJob( jobs,  i );

    _prefetchCache->prefetch( jobs );
}


void PhotoDir::addJob( QStringList & jobs, int index )
{
    jobs.append( _photos.at( index )->fileName() );
}


void PhotoDir::dropCache()
{
    _prefetchCache->clear();

    foreach ( Photo * photo, _photos )
    {
	photo->dropCache();
    }
}


void PhotoDir::take( Photo * photo )
{
    int index = _photos.indexOf( photo );

    if ( index == -1 ) // Not found
	return;

    if ( _current >= index )
	--_current;

    photo->reparent( 0 );
    _photos.removeAt( index );
}
