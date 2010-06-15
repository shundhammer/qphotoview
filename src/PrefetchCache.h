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
     */
    QPixmap pixmap( const QString & imageFileName );

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

    /**
     * Statistics: Get the final (the expected) image count, i.e. the number of
     * cached images plus the number of jobs in the job queue.
     */
    int finalImageCount();

    /**
     * Statistics: Get the count of current images in the cache.
     */
    int imageCount();

    /**
     * Return the number of cache hits when accessing pixmap().
     */
    int cacheHits()   const { return m_cacheHits;   }

    /**
     * Return the number of cache misses when accessing pixmap(), i.e. the
     * number of times an image had to be loaded directly from file rather than
     * from the cache.
     */
    int cacheMisses() const { return m_cacheMisses; }

    friend class PrefetchCacheWorkerThread;

private:

    QMap<QString, QImage> m_cache;
    QMap<QString, QSize>  m_sizes;
    QString     m_path;
    QStringList m_jobQueue;
    QMutex	m_cacheMutex; // protects m_cache, m_sizes, m_jobQueue
    QSize	m_fullScreenSize;
    PrefetchCacheWorkerThread m_workerThread;
    int		m_cacheHits;
    int		m_cacheMisses;
};


#endif // PrefetchCache_h
