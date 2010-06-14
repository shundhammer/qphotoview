/*
 * QPhotoView core classes
 *
 * License: GPL V2. See file COPYING for details.
 *
 * Author:  Stefan Hundhammer <Stefan.Hundhammer@gmx.de>
 */

#include <QFileInfo>
#include <QDir>

#include "Photo.h"
#include "PhotoDir.h"


Photo::Photo( const QString & fileName, PhotoDir *parentDir )
    : m_photoDir( parentDir )
{
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


QPixmap Photo::pixmap()
{
    QPixmap px( fullPath() );
    
    return px;
}


QPixmap Photo::pixmap( const QSize & size )
{
    QPixmap px;
    
    return px;
}


QSize Photo::size()
{
    QSize sz;

    return sz;
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
