
#ifndef _PREFETCH_CACHE_H_
#define _PREFETCH_CACHE_H_

#include <QPixmap>
#include <QImage>
#include <QMutex>
#include <QThread>
#include <QMap>


class PrefetchCache;

class PrefetchCacheWorkerThread: public QThread
{
    Q_OBJECT

public:
    PrefetchCacheWorkerThread( PrefetchCache *prefetchCache );

protected:
    virtual void run();

private:
    PrefetchCache * m_prefetchCache;
};


class PrefetchCache
{
public:

    PrefetchCache( int sizeMB );
    virtual ~PrefetchCache();

    void prefetch( const QString & path, const QStringList & filenames );
    QPixmap pixmap( const QString & fullPixmapPath, const QSize & size );
    void clear();
    int finalImageCount();
    int imageCount();
    int cacheHits()   const { return m_cacheHits;   }
    int cacheMisses() const { return m_cacheMisses; }

    friend class PrefetchCacheWorkerThread;

private:

    QMap<QString, QImage> m_cache;
    QStringList m_jobQueue;
    QMutex	m_cacheMutex; // protects m_cache and m_jobQueue
    QSize	m_fullScreenSize;
    PrefetchCacheWorkerThread m_workerThread;
    long long	m_maxSize;
    int		m_cacheHits;
    int		m_cacheMisses;
};




#endif //_PREFETCH_CACHE_H_
