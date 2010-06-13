/*
 * QPhotoView core classes
 *
 * License: GPL V2. See file COPYING for details.
 *
 * Author:  Stefan Hundhammer <Stefan.Hundhammer@gmx.de>
 */

#ifndef Photo_h
#define Photo_h

#include <QString>
#include <QPixmap>
// #include "PhotoMetaData"

class PhotoDir;


/**
 * Class representing one photo.
 */
class Photo
{
public:
    /**
     * Constructor. If 'parentDir' is non-null, 'fileName' is relative to
     * 'parentDir'.
     */
    Photo( const QString & fileName, PhotoDir *parentDir = 0 );

    /**
     * Destructor.
     */
    virtual ~Photo();

    /**
     * Return the full size pixmap of the photo.
     * If the photo is not loaded yet, it will be loaded first.
     */
    QPixmap pixmap();

    /**
     * Return the pixmap of the photo resized to the specified size.
     * If the photo is not loaded yet, it will be loaded first.
     * Notice that this may be more efficient than fetching the full size
     * pixmap with pixmap() and scaling it down since a pixmap of this size
     * might be available in a cache.
     */
    QPixmap pixmap( const QSize & size );

    /**
     * Return the size of the photo.
     * If the photo is not loaded yet, it will be loaded first.
     */
    QSize size();

#if 0
    /**
     * Return the meta data for this photo.
     * If they are not loaded yet, load them first.
     * Notice that this is independent of loading the pixmap.
     */ 
    PhotoMetaData metaData();
#endif

    /**
     * Return the file name (without path) of this photo.
     */
    QString fileName() const { return m_fileName; }

    /**
     * Return the path name (without file name) of this photo.
     */
    QString path() const;

    /**
     * Return the full path and file name of this photo.
     */
    QString fullPath() const;

    /**
     * Return the parent PhotoDir or 0 if there is none.
     */
    PhotoDir * photoDir() const { return m_photoDir; }

    /**
     * Reparent this photo to the specified PhotoDir.
     * If 'parentDir' is 0 (i.e., this photo gets orphaned), the path is taken
     * from there and stored in m_photoDir.
     */
    void reparent( PhotoDir * parentDir );

private:
    Q_DISABLE_COPY( Photo );

    PhotoDir *  m_photoDir;
    QString     m_fileName;
    QString     m_path;
};


#endif // Photo_h
