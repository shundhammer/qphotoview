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

    QDir dir ( m_path );
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


PhotoDir::~PhotoDir()
{
    qDeleteAll( m_photos );
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
    if ( m_photos.isEmpty() || m_current >= m_photos.size()-1 )
        return 0;

    return m_photos.at( ++m_current );
}


Photo * PhotoDir::toPrevious()
{
    if ( m_photos.isEmpty() || m_current < 1 )
        return 0;

    return m_photos.at( --m_current );
}


void PhotoDir::prefetch()
{
    // TO DO
    // TO DO
    // TO DO
}


void PhotoDir::obsoletePrefetchedPhotos()
{
    // TO DO
    // TO DO
    // TO DO
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
