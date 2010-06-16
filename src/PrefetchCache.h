/*
 * QPhotoView core classes
 *
 * License: GPL V2. See file COPYING for details.
 *
 * Author:  Stefan Hundhammer <Stefan.Hundhammer@gmx.de>
 */

#ifndef PrefetchCache_h
#define PrefetchCache_h

#include <QPixmap>
#include <QImage>
#include <QMutex>
#include <QThread>
#include <QMap>
#include <QSize>


class PrefetchCache;

/**
 * Helper class: Worker thread. This is the secondary thread where images are
 * read and scaled down.
 */
class PrefetchCacheWorkerThread: public QThread
{
    Q_OBJECT

public:
    /**
     * Constructor.
     */
    PrefetchCacheWorkerThread( PrefetchCache *prefetchCache );

protected:
    /**
     * Reimplemented from QThread:
     * This is the worker function.
     */
    virtual void run();

private:
    PrefetchCache * m_prefetchCache;
};


/**
 * Prefetch cache: Load images in advance and scale them down to fullscreen
 * size.
 *
 * Contrary to popular belief, it's not reading JPG files that is so very
 * expensive, but scaling them down to a reasonable size. Scaling takes about
 * 4-5 times as long as loading.
 */
class PrefetchCache
{
public:

    /**
     * Constructor: Create a prefetch cache for 'path'.
     */
    PrefetchCache( const QString & path );

    /**
     * Destructor.
     */
    virtual ~PrefetchCache();

    /**
     * Prefetch all file names in 'fileNames' from the directory specified in
     * the constructor.
     */
    void prefetch( const QStringList & fileNames );

    /**
     * Get the pixmap for the specified file in full screen size, either from
     * the cache or directly from the disk file.
     * If 'take' is true, the pixmap is taken out of the cache, i.e., the
     * corresponding cached object is deleted.
     */
    QPixmap pixmap( const QString & imageFileName, bool take = false );

    /**
     * Return the original pixel size of the specified image.
     */
    QSize pixelSize( const QString & imageFileName );

    /**
     * Clear all cached images and the job queue.
     */
    void clear();

    /**
     * Return the full path for the specified image.
     */
    QString fullPath( const QString & imageFileName );


    friend class PrefetchCacheWorkerThread;

private:

    QMap<QString, QImage> m_cache;
    QMap<QString, QSize>  m_sizes;
    QString     m_path;
    QStringList m_jobQueue;
    QMutex	m_cacheMutex; // protects m_cache, m_sizes, m_jobQueue
    QSize	m_fullScreenSize;
    PrefetchCacheWorkerThread m_workerThread;
};


#endif // PrefetchCache_h
