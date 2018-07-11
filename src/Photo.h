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
#include <QSize>

#include "PhotoMetaData.h"

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
     * Return the full size pixmap of this photo.
     */
    QPixmap fullSizePixmap();

    /**
     * Return the pixmap of this photo resized to the specified size.
     * This might use a cached pixmap that gets scaled down.
     */
    QPixmap pixmap( const QSize	 & size );
    QPixmap pixmap( const QSizeF & size );

    /**
     * Clear any cached pixmaps for this photo.
     */
    void dropCache();

    /**
     * Return the original pixel size of the photo.
     */
    QSize size();

    /**
     * Return a thumbnail for this photo.
     * See also thumbnailSize() and setThumbnailSize().
     */
    QPixmap thumbnail();

    /**
     * Clear the cached thumbnail for this photo.
     */
    void clearCachedThumbnail();

    /**
     * Return the meta data for this photo.
     * If they are not loaded yet, load them first.
     * Notice that this is independent of loading the pixmap.
     */
    PhotoMetaData metaData();

    /**
     * Return the file name (without path) of this photo.
     */
    QString fileName() const { return _fileName; }

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
    PhotoDir * photoDir() const { return _photoDir; }

    /**
     * Reparent this photo to the specified PhotoDir.
     * If 'parentDir' is 0 (i.e., this photo gets orphaned), the path is taken
     * from there and stored in _photoDir.
     */
    void reparent( PhotoDir * parentDir );

    /**
     * Return 'true' if the pixmap for this photo (not the thumbnail!) was ever
     * accessed.
     */
    bool pixmapAccessed() { return _lastPixmapAccess > 0; }

    /**
     * Return a timestamp when the pixmap for this photo was last
     * accessed. This value makes only sense when compared to the timestamp of
     * the pixmap of another photo. This is meant for cache optimization
     * purposes.
     */
    long lastCachedPixmapAccess() { return _lastPixmapAccess; }

    /**
     * Return a timestamp when the thumbnail for this photo was last
     * accessed. Similar to lastPixmapAccess(), this makes only sense when
     * compared to the timestamp of the thumbnail of another photo.
     */
    long lastThumbnailAccess() { return _lastThumbnailAccess; }

    /**
     * Return the thumbnail size.
     */
    static QSize thumbnailSize() { return _thumbnailSize; }

    /**
     * Set the thumbnail size. This affects only thumbnails created after this
     * call. Notice that thumbnails are cached; only the first access to a
     * photo's thumbnail actually creates the thumbnail. From then on, only the
     * cached value is used.
     */
    static void setThumbnailSize( const QSize & size ) { _thumbnailSize = size; }

    /**
     * Helper function: Scale down 'origSize' to fit into 'boundingSize' while
     * maintaining the aspect ratio of 'origSize'.
     */
    static QSize scale( const QSize & origSize, const QSize & boundingSize );

    /**
     * Helper function: Return the scale factor for scaling down 'origSize' to
     * fit into 'boundingSize' while maintaining the aspect ratio of
     * 'origSize'.
     */
    static qreal scaleFactor( const QSize & origSize,
			      const QSize & boundingSize );

    /**
     * Return a scaled pixmap.
     */
    static QPixmap scale( const QPixmap & origPixmap, qreal scaleFactor );

private:
    Q_DISABLE_COPY( Photo );

    PhotoDir *	_photoDir;
    QString	_fileName;
    QString	_path;

    QPixmap	_pixmap;
    QPixmap	_thumbnail;
    QSize	_size;

    long	_lastPixmapAccess;
    long	_lastThumbnailAccess;

    static long		_pixmapAccessCount;
    static long		_thumbnailAccessCount;
    static QSize	_thumbnailSize;
};


#endif // Photo_h
