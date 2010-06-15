/*
 * QPhotoView core classes
 *
 * License: GPL V2. See file COPYING for details.
 *
 * Author:  Stefan Hundhammer <Stefan.Hundhammer@gmx.de>
 */

#ifndef PhotoDir_h
#define PhotoDir_h

#include <QString>
#include <QList>
#include <QSize>

class Photo;
class PrefetchCache;


/**
 * A collection of photos that corresponds to one disk directory.
 * This class takes care of reading the disk directory, filtering out all image
 * files in that directory that can be displayed, creating a corresponding
 * instance of the Photo class for each one, managing those objects and
 * destroying them in its destructor.
 */
class PhotoDir
{
public:
    /**
     * Constructor. 'path' can be the file system path of the directory or one
     * of its files, in which case that file (if it is an image file) becomes
     * the current photo of this PhotoDir.
     * If 'jpgOnly' is false (the default), this will take all image files into
     * account that can be displayed, not just JPG files.
     */
    PhotoDir( const QString & path, bool jpgOnly = false );

    /**
     * Destructor. Destroys all Photo objects managed by this PhotoDir.
     */
    virtual ~PhotoDir();

    /**
     * Return the disk path (without a trailing '/') of this photo directory.
     * Notice that this does not need to be the same as the path passed in the
     * constructor: This is a normalized absolute path to the directory, even
     * if a relative path or an image file was passed in the constructor.
     */
    QString path() const { return m_path; }

    /**
     * Return the number of photos in this PhotoDir.
     */
    int size() const { return m_photos.size(); }

    /**
     * Check if this photo directory is empty.
     */
    bool isEmpty() const { return m_photos.isEmpty(); }

    /**
     * Return the photo with the specifed index or 0 if there is no photo with
     * that index.
     */
    Photo * photo( int index ) const;

    /**
     * Return the current Photo.
     */
    Photo * current() const;

    /**
     * Return the index of the current Photo (the first one is 0)
     * or -1 if there is none.
     */
    int currentIndex() const { return m_current; }

    /**
     * Return the first photo.
     */
    Photo * first() const;

    /**
     * Return the last photo.
     */
    Photo * last() const;

    /**
     * Set the current photo to the one with the specified index and return the
     * corresponding Photo object or 0 if there is none with that index.
     */
    Photo * setCurrent( int index );

    /**
     * Set the current photo to the specified one.
     */
    void setCurrent( Photo * photo );

    /**
     * Find the specified photo and return its index (the first one is 0)
     * or -1 if not found.
     */
    int find( Photo * photo );

    /**
     * Make the first photo the current one and return it
     * (or 0 if this PhotoDir is empty).
     */
    Photo * toFirst();

    /**
     * Make the last photo the current one and return it
     * (or 0 if this PhotoDir is empty).
     */
    Photo * toLast();

    /**
     * Make the next photo the current one and return it.
     */
    Photo * toNext();

    /**
     * Make the previous photo the current one and return it.
     */
    Photo * toPrevious();

    /**
     * Begin prefetching photos.
     */
    void prefetch();

    /**
     * Drop (expensive) cached values like pixmaps.
     */
    void dropCache();

    /**
     * Take the specified photo out of this collection. Ownership is
     * transferred to the caller, i.e. the caller has to take care of deleting
     * it.
     */
    void take( Photo * photo );

    /**
     * Return 'true' if only JPG files are considered in this directory
     * (see also constructor).
     */
    bool jpgOnly() const { return m_jpgOnly; }

    /**
     * Return the prefetch cache for this directory.
     */
    PrefetchCache * prefetchCache() const { return m_prefetchCache; }


protected:

    /**
     * Read the disk directory 'dirPath', create a Photo object for each image
     * file, and set m_current to the directory entry 'startPhotoName'.
     */
    void read( const QString & dirPath, const QString & startPhotoName );

    /**
     * Add a prefetch job for the photo with the specified index to 'jobs'.
     */
    void addJob( QStringList & jobs, int index );


private:

    QString             m_path;
    QList<Photo *>      m_photos;
    int                 m_current;
    bool                m_jpgOnly;
    PrefetchCache *     m_prefetchCache;
};

#endif // PhotoDir_h
