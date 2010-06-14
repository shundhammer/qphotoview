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

    if ( ! m_photoDir->isEmpty() )
        loadImage();

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
    Photo * photo = m_photoDir->current();

    if ( ! photo )
        return false;

    m_origPixmap = photo->pixmap();
    qDebug() << photo->fileName() << "Size:" << m_origPixmap.size();

    if ( ! m_origPixmap.isNull() )
    {
        QPixmap pixmap = m_origPixmap.scaled( size(),
                                              Qt::KeepAspectRatio,
                                              Qt::SmoothTransformation );
        m_canvas->setPixmap( pixmap );
        setWindowTitle( "QPhotoView  " + photo->fileName() );
        
        //
        // Center m_canvas
        //
        
        qreal x = ( size().width()  - pixmap.width()  ) / 2.0;
        qreal y = ( size().height() - pixmap.height() ) / 2.0;
        m_canvas->setPos( x, y );
        
        setSceneRect( 0, 0, size().width(), size().height() );
    }
    else
    {
        qWarning() << "Error: Can't display image" << photo->fullPath();
        clear();
        setWindowTitle( "QPhotoView -- ERROR" );
    }

    return ! m_origPixmap.isNull();
}


void PhotoView::clear()
{
    m_origPixmap = QPixmap();
    m_canvas->setPixmap( m_origPixmap );
    setWindowTitle( "QPhotoView" );
}


void PhotoView::resizeEvent ( QResizeEvent * event )
{
    if ( event->size() != event->oldSize() && ! m_origPixmap.isNull() )
    {
        //
        // Resize pixmap
        //
        
        QPixmap pixmap = m_origPixmap.scaled( event->size(),
                                              Qt::KeepAspectRatio,
                                              Qt::SmoothTransformation );
        m_canvas->setPixmap( pixmap );

        //
        // Center m_canvas
        //
        
        qreal x = ( event->size().width()  - pixmap.width()  ) / 2.0;
        qreal y = ( event->size().height() - pixmap.height() ) / 2.0;
        m_canvas->setPos( x, y );
        
        setSceneRect( 0, 0, event->size().width(), event->size().height() );
    }
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
	    
	default:
	    QGraphicsView::keyPressEvent( event );
    }
}
