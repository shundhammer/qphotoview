/*
 * QPhotoView viewer widget.
 *
 * License: GPL V2. See file COPYING for details.
 *
 * Author:  Stefan Hundhammer <Stefan.Hundhammer@gmx.de>
 */

#include <QApplication>
#include <QGraphicsPixmapItem>
#include <QWindowsStyle>
#include <QResizeEvent>
#include <QKeyEvent>
#include <QTime>
#include <QDebug>

#include "PhotoView.h"
#include "PhotoDir.h"
#include "Photo.h"
#include "Fraction.h"


PhotoView::PhotoView( PhotoDir * photoDir )
    : QGraphicsView()
    , m_photoDir( photoDir )
{
    Q_CHECK_PTR( photoDir );
    setScene( new QGraphicsScene );
    m_canvas = scene()->addPixmap( QPixmap() );

    //
    // Visual tweaks
    //

    QPalette pal = palette();
    pal.setColor( QPalette::Base, Qt::black );
    setPalette( pal );
    setFrameStyle( QFrame::NoFrame );

#if 1
    setVerticalScrollBarPolicy  ( Qt::ScrollBarAlwaysOff );
    setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    setDragMode( QGraphicsView::ScrollHandDrag );
#endif

    // Some styles (e.g. Plastique) have an undesired two pixel wide focus rect
    // around QGraphicsView widgets. This is not what we want here, so let's
    // select a style that does not do this. This does not have an effect on
    // existing or future child widgets. And since scroll bars are turned off,
    // there is no other visual effect anyway.
    setStyle( new QWindowsStyle() );


    //
    // Load images
    //

    m_photoDir->prefetch();

    if ( ! m_photoDir->isEmpty() )
        loadImage();
}


PhotoView::~PhotoView()
{
    if ( style() != qApp->style() )
    {
        // Delete the style we explicitly created just for this widget
        delete style();
    }

    delete scene();
}


bool PhotoView::loadImage()
{
    bool success = reloadCurrent( size() );

    if ( success )
    {
        Photo * photo = m_photoDir->current();

        if ( success && photo )
        {
            QString title( "QPhotoView  " + photo->fileName() );

            if ( photo->size().isValid() )
            {
                title += QString( "  %1 x %2" )
                    .arg( photo->size().width() )
                    .arg( photo->size().height() );
            }

            setWindowTitle( title );
        }
    }
    else // ! success
    {
        clear();
        setWindowTitle( "QPhotoView -- ERROR" );
    }

    return success;
}


void PhotoView::clear()
{
    m_canvas->setPixmap( QPixmap() );
    setWindowTitle( "QPhotoView" );
}


void PhotoView::resizeEvent ( QResizeEvent * event )
{
    if ( event->size() != event->oldSize() )
    {
        reloadCurrent( event->size() );
    }
}


bool PhotoView::reloadCurrent( const QSize & size )
{
    Photo * photo = m_photoDir->current();

    if ( ! photo )
        return false;

    QPixmap pixmap = photo->pixmap( size );
    m_canvas->setPixmap( pixmap );

    if ( pixmap.isNull() )
        return false;

    //
    // Center m_canvas
    //

    qreal x = ( size.width()  - pixmap.width()  ) / 2.0;
    qreal y = ( size.height() - pixmap.height() ) / 2.0;
    m_canvas->setPos( x, y );

    setSceneRect( 0, 0, size.width(), size.height() );

    return true;
}


void PhotoView::keyPressEvent( QKeyEvent * event )
{
    if ( ! event )
	return;

    switch ( event->key() )
    {
	case Qt::Key_PageDown:
	case Qt::Key_Space:
            m_photoDir->toNext();
            loadImage();
	    break;

	case Qt::Key_PageUp:
	case Qt::Key_Backspace:

            m_photoDir->toPrevious();
            loadImage();
	    break;

	case Qt::Key_Home:
            m_photoDir->toFirst();
            loadImage();
	    break;

	case Qt::Key_End:
            m_photoDir->toLast();
            loadImage();
	    break;

	case Qt::Key_Q:
	case Qt::Key_Escape:
	    qApp->quit();
	    break;

	case Qt::Key_Return:
	    setWindowState( windowState() ^ Qt::WindowFullScreen );
	    break;

        case Qt::Key_B:
            {
                const int max=10;
                qDebug() << "*** Benchmark start";
                QTime time;
                time.start();

                for ( int i=0; i < max; ++i )
                {
                    m_photoDir->toNext();
                    loadImage();
                }
                qDebug() << "*** Benchmark end; time:"
                         << time.elapsed() / 1000.0 << "sec /"
                         << max << "images";
            }

	default:
	    QGraphicsView::keyPressEvent( event );
    }
}
