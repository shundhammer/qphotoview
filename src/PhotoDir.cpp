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


PhotoDir::PhotoDir( const QString & path, bool jpgOnly )
 : m_path( path )
 , m_current( -1 )
 , m_jpgOnly( jpgOnly )
{
    QString startPhotoName;
    QFileInfo fileInfo( path );

    if ( ! fileInfo.isDir() )
    {
        m_path         = fileInfo.absolutePath();
        startPhotoName = fileInfo.fileName();
    }

    m_prefetchCache = new PrefetchCache( m_path );
    read( m_path, startPhotoName );
}


PhotoDir::~PhotoDir()
{
    qDeleteAll( m_photos );
    delete m_prefetchCache;
}


void PhotoDir::read( const QString & dirPath, const QString & startPhotoName )
{
    QStringList nameFilters;
    nameFilters << "*.jpg" << "*.jpeg" << "*.JPG" << "*.JPEG";

    if ( ! m_jpgOnly )
    {
        nameFilters << "*.png"  << "*.PNG"
                    << "*.gif"  << "*.GIF"
                    << "*.bmp"  << "*.BMP"
                    << "*.tif"  << "*.TIF"
                    << "*.tiff" << "*.TIFF"
                    << "*.xpm"  << "*.XPM"
                    << "*.ppm"  << "*.PPM"
                    << "*.pgm"  << "*.PGM"
                    << "*.pbm"  << "*.PBM";
    }

    QDir dir ( dirPath );
    m_path = dir.absolutePath();
    QStringList imageFileNames = dir.entryList( nameFilters,
                                                QDir::Files,  // wanted type
                                                QDir::Name ); // sort by

    foreach ( QString imageFileName, imageFileNames )
    {
        Photo * photo = new Photo( imageFileName, this );
        m_photos.append( photo );

        if ( imageFileName == startPhotoName )
            m_current = m_photos.size()-1;
    }

    if ( ! m_photos.isEmpty() && m_current < 0 )
        m_current = 0;
}


Photo * PhotoDir::photo( int index ) const
{
    if ( index < 0 || index >= m_photos.size() )
        return 0;

    return m_photos.at( index );
}


Photo * PhotoDir::current() const
{
    if ( m_photos.isEmpty() )
        return 0;

    int current = qBound( 0, m_current, m_photos.size()-1 );
    return m_photos.at( current );
}


Photo * PhotoDir::first() const
{
    if ( m_photos.isEmpty() )
        return 0;

    return m_photos.first();
}


Photo * PhotoDir::last() const
{
    if ( m_photos.isEmpty() )
        return 0;

    return m_photos.last();
}


Photo * PhotoDir::setCurrent( int index )
{
    if ( m_photos.isEmpty() )
        return 0;

    m_current = qBound( 0, index, m_photos.size()-1 );
    return m_photos.at( m_current );
}


void PhotoDir::setCurrent( Photo * photo )
{
    int index = find( photo );

    if ( index >= 0 )
        m_current = index;
}


int PhotoDir::find( Photo * photo )
{
    return m_photos.indexOf( photo );
}


Photo * PhotoDir::toFirst()
{
    if ( m_photos.isEmpty() )
        return 0;

    m_current = 0;
    return m_photos.first();
}


Photo * PhotoDir::toLast()
{
    if ( m_photos.isEmpty() )
        return 0;

    m_current = m_photos.size()-1;
    return m_photos.last();
}


Photo * PhotoDir::toNext()
{
    if ( m_photos.isEmpty() )
        return 0;

    ++m_current;
    m_current = qBound( 0, m_current, m_photos.size()-1 );

    return m_photos.at( m_current );
}


Photo * PhotoDir::toPrevious()
{
    if ( m_photos.isEmpty() )
        return 0;

    --m_current;
    m_current = qBound( 0, m_current, m_photos.size()-1 );

    return m_photos.at( m_current );
}


void PhotoDir::prefetch()
{
    if ( m_photos.isEmpty() )
        return;

    QStringList jobs;
    int last = m_photos.size()-1;

    if ( m_current >= 0   )     addJob( jobs, m_current   );
    if ( m_current < last )     addJob( jobs, m_current+1 );
    if ( m_current > 0    )     addJob( jobs, m_current-1 );

    if ( m_current > 1      )   addJob( jobs, 0 );
    if ( last > m_current+1 )   addJob( jobs, last );

    for ( int i = m_current+2; i < last; ++i )
        addJob( jobs, i );

    for ( int i = m_current-2; i > 0; --i )
        addJob( jobs,  i );

    m_prefetchCache->prefetch( jobs );
}


void PhotoDir::addJob( QStringList & jobs, int index )
{
    jobs.append( m_photos.at( index )->fileName() );
}


void PhotoDir::dropCache()
{
    m_prefetchCache->clear();

    foreach ( Photo * photo, m_photos )
    {
        photo->dropCache();
    }
}


void PhotoDir::take( Photo * photo )
{
    int index = m_photos.indexOf( photo );

    if ( index == -1 ) // Not found
        return;

    if ( m_current >= index )
        --m_current;

    photo->reparent( 0 );
    m_photos.removeAt( index );
}
