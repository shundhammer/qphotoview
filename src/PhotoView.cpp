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
#include <QDesktopWidget>

#include "PhotoView.h"
#include "PhotoDir.h"
#include "Photo.h"
#include "Panner.h"
#include "Fraction.h"


PhotoView::PhotoView( PhotoDir * photoDir )
    : QGraphicsView()
    , m_photoDir( photoDir )
    , m_lastPhoto( 0 )
    , m_zoomMode( ZoomFitImage )
    , m_zoomFactor( 0.0	 )
    , m_zoomIncrement( 1.2 )
{
    Q_CHECK_PTR( photoDir );
    setScene( new QGraphicsScene );
    m_canvas = scene()->addPixmap( QPixmap() );
    m_canvas->setCursor( Qt::ArrowCursor );

    QSize pannerMaxSize( qApp->desktop()->screenGeometry().size() / 6 );
    m_panner = new Panner( pannerMaxSize );
    scene()->addItem( m_panner );

    //
    // Visual tweaks
    //

    QPalette pal = palette();
    pal.setColor( QPalette::Base, Qt::black );
    setPalette( pal );
    setFrameStyle( QFrame::NoFrame );

#if 1
    setVerticalScrollBarPolicy	( Qt::ScrollBarAlwaysOff );
    setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
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
    m_zoomMode = ZoomFitImage;
    bool success = reloadCurrent( size() );

    if ( success )
    {
	Photo * photo = m_photoDir->current();

	if ( success && photo )
	{
	    QString title( "QPhotoView	" + photo->fileName() );

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
    bool success = true;
    Photo * photo = m_photoDir->current();

    if ( ! photo )
	return false;

    QPixmap pixmap;
    QSize origSize = photo->size();

    switch ( m_zoomMode )
    {
	case NoZoom:
	    pixmap = photo->fullSizePixmap();
	    m_zoomFactor = 1.0;
	    break;


	case ZoomFitImage:
	    pixmap = photo->pixmap( size );

	    if ( origSize.width() != 0 )
		m_zoomFactor = pixmap.width() / (qreal) origSize.width();
	    break;


	case ZoomFitWidth:

	    if ( origSize.width() != 0 )
	    {
		m_zoomFactor = size.width() / (qreal) origSize.width();
		pixmap = photo->pixmap( m_zoomFactor * origSize );
	    }
	    break;


	case ZoomFitHeight:

	    if ( origSize.height() != 0 )
	    {
		m_zoomFactor = size.height() / (qreal) origSize.height();
		pixmap = photo->pixmap( m_zoomFactor * origSize );
	    }
	    break;


	case ZoomFitBest:

	    if ( origSize.width() != 0 && origSize.height() != 0 )
	    {
		qreal zoomFactorX = size.width()  / (qreal) origSize.width();
		qreal zoomFactorY = size.height() / (qreal) origSize.height();
		m_zoomFactor = qMax( zoomFactorX, zoomFactorY );

		pixmap = photo->pixmap( m_zoomFactor * origSize );
	    }
	    break;

	case UseZoomFactor:
	    pixmap = photo->pixmap( m_zoomFactor * origSize );
	    break;

	    // Deliberately omitting 'default' branch so the compiler will warn
	    // about unhandled enum values
    };

    m_canvas->setPixmap( pixmap );
    success = ! pixmap.isNull();

    if ( success )
    {
	//
	// Center m_canvas if it doesn't fill all available space
	//

	qreal x = m_canvas->pos().x();
	qreal y = m_canvas->pos().y();

	if ( pixmap.width() < size.width() )
	    x = ( size.width()	- pixmap.width()  ) / 2.0;
	else if ( x > 0.0 )
	    x = 0.0;

	if ( pixmap.height() < size.height() )
	    y = ( size.height() - pixmap.height() ) / 2.0;
	else if ( y > 0.0 )
	    y = 0.0;

	m_canvas->setPos( x, y );


	//
	// Update panner
	//

        if ( size.width()  < m_panner->size().width()  * 2  ||
             size.height() < m_panner->size().height() * 2  )
        {
            // If the panner would take up more than half the available space
            // in any direction, don't show it.

            m_panner->hide();
        }
        else
        {
            if ( photo != m_lastPhoto )
            {
                m_panner->setPixmap( pixmap );
                m_lastPhoto = photo;
            }

            m_panner->setPos( qMax( x, 0.0 ),
                              qMin( (qreal) size.height(), y + pixmap.height() )
                              - m_panner->size().height() );

            QPoint visiblePos( qMax( -x, 0.0 ),
                               qMax( -y, 0.0 ) );
            QSize visibleSize( qMin( size.width(),  pixmap.width()  ),
                               qMin( size.height(), pixmap.height() ) );
            QRect visibleRect( visiblePos  / m_zoomFactor,
                               visibleSize / m_zoomFactor );

            m_panner->updatePanRect( visibleRect, origSize );
        }
    }

    setSceneRect( 0, 0, size.width(), size.height() );

    return success;
}


void PhotoView::setZoomMode( ZoomMode mode )
{
    m_zoomMode = mode;
    reloadCurrent( size() );
}


void PhotoView::setZoomFactor( qreal factor )
{
    m_zoomFactor = factor;

    if ( qFuzzyCompare( m_zoomFactor, 1.0 ) )
	setZoomMode( NoZoom );
    else
	setZoomMode( UseZoomFactor );
}


void PhotoView::zoomIn()
{
    if ( ! qFuzzyCompare( m_zoomIncrement, 0.0 ) )
	setZoomFactor( m_zoomFactor * m_zoomIncrement );
}


void PhotoView::zoomOut()
{
    if ( ! qFuzzyCompare( m_zoomIncrement, 0.0 ) )
	setZoomFactor( m_zoomFactor / m_zoomIncrement );
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

	case Qt::Key_F5: // Force reload
	    {
		Photo * photo = m_photoDir->current();

		if ( photo )
		{
		    photo->dropCache();
		    loadImage();
		}
	    }
	    break;

	case Qt::Key_Plus:
	    zoomIn();
	    break;

	case Qt::Key_Minus:
	    zoomOut();
	    break;

	case Qt::Key_1:
            setZoomMode( NoZoom );
            break;
            
	case Qt::Key_2:	       setZoomFactor( 1/2.0 );	break;
	case Qt::Key_3:	       setZoomFactor( 1/3.0 );	break;
	case Qt::Key_4:	       setZoomFactor( 1/4.0 );	break;
	case Qt::Key_5:	       setZoomFactor( 1/5.0 );	break;
	case Qt::Key_6:	       setZoomFactor( 1/6.0 );	break;
	case Qt::Key_7:	       setZoomFactor( 1/7.0 );	break;
	case Qt::Key_8:	       setZoomFactor( 1/8.0 );	break;
	case Qt::Key_9:	       setZoomFactor( 1/9.0 );  break;
	case Qt::Key_0:	       setZoomFactor( 1/10.0 ); break;

	case Qt::Key_F:
	case Qt::Key_M:
	    setZoomMode( ZoomFitImage );
	    break;

	case Qt::Key_B:
	    setZoomMode( ZoomFitBest );
	    break;

	case Qt::Key_W:
	    setZoomMode( ZoomFitWidth );
	    break;

	case Qt::Key_H:
	    setZoomMode( ZoomFitHeight );
	    break;

	case Qt::Key_Q:
	case Qt::Key_Escape:
	    qApp->quit();
	    break;

	case Qt::Key_Return:
	    setWindowState( windowState() ^ Qt::WindowFullScreen );
	    break;

	case Qt::Key_Y:
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
