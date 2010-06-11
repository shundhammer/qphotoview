#ifndef _SIMPLE_VIEW_H_
#define _SIMPLE_VIEW_H_

#include <QLabel>
#include <QStringList>

class PrefetchCache;


class SimpleView: public QLabel
{
    Q_OBJECT
public:

    SimpleView( const QString & photoPath, const QStringList & photoList );
    virtual ~SimpleView();

    void showPhoto( int index, bool force = false );

protected:

    virtual void keyPressEvent( QKeyEvent * event );
    virtual void resizeEvent( QResizeEvent * event );

private:
    QString	m_photoPath;
    QStringList m_photoList;
    int		m_current;
    PrefetchCache * m_prefetchCache;
};

#endif // _SIMPLE_VIEW_H_
