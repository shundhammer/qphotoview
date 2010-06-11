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
#include "PhotoMetaData"


/**
 * Class representing one photo.
 */
class Photo
{
public:
    /**
     * Constructor.
     */
    Photo( const QString & filename );

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

    /**
     * Return the meta data for this photo.
     * If they are not loaded yet, load them first.
     * Notice that this is independent of loading the pixmap.
     */ 
    PhotoMetaData metaData();

    /**
     * Return the filename of this photo.
     */
    QString filename() const;

#if 0
    PhotoDir * photoDir() const;
#endif

private:
    Q_DISABLE_COPY( Photo );
};


#endif // Photo_h
