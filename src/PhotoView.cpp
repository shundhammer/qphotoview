/*
 * QPhotoView viewer widget.
 *
 * License: GPL V2. See file COPYING for details.
 *
 * Author:  Stefan Hundhammer <Stefan.Hundhammer@gmx.de>
 */

#include <QApplication>
#include <QGraphicsPixmapItem>
#include <QResizeEvent>
#include <QKeyEvent>
#include <QTime>
#include <QDebug>
#include <QDesktopWidget>
#include <QStyle>

#include "PhotoView.h"
#include "PhotoDir.h"
#include "Photo.h"
#include "Canvas.h"
#include "Panner.h"
#include "SensitiveBorder.h"
#include "BorderPanel.h"
#include "TextBorderPanel.h"
#include "ExifBorderPanel.h"


static const int DefaultIdleTimeout = 4000; // millisec


PhotoView::PhotoView( PhotoDir * photoDir )
    : QGraphicsView()
    , m_photoDir( photoDir )
    , m_lastPhoto( 0 )
    , m_zoomMode( ZoomFitImage )
    , m_zoomFactor( 1.0	 )
    , m_zoomIncrement( 1.2 )
    , m_idleTimeout( DefaultIdleTimeout )
{
    Q_CHECK_PTR( photoDir );
    setScene( new QGraphicsScene );

    m_canvas = new Canvas( this );
    createBorders();

    QSize pannerMaxSize( qApp->desktop()->screenGeometry().size() / 6 );
    m_panner = new Panner( pannerMaxSize, this );

    createPanels();

    //
    // Visual tweaks
    //

    QPalette pal = palette();
    pal.setColor( QPalette::Base, Qt::black );
    setPalette( pal );
    setFrameStyle( QFrame::NoFrame );

    setVerticalScrollBarPolicy	( Qt::ScrollBarAlwaysOff );
    setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

#if 0
    // Some styles (e.g. Plastique) have an undesired two pixel wide focus rect
    // around QGraphicsView widgets. This is not what we want here, so let's
    // select a style that does not do this. This does not have an effect on
    // existing or future child widgets. And since scroll bars are turned off,
    // there is no other visual effect anyway.

    // FIXME
    // FIXME
    // FIXME
    setStyle( new QWindowsStyle() );
    // FIXME
    // FIXME
    // FIXME
#endif

    // Enable mouse tracking so a mouse cursor that was set invisible by an
    // item (e.g., Canvas) can be made visible again upon mouse movement.
    setMouseTracking( true );

    connect( &m_idleTimer, SIGNAL( timeout()    ),
             this,         SLOT  ( hideCursor() ) );

    m_idleTimer.setSingleShot( true );
    m_idleTimer.start( m_idleTimeout );
    m_cursor = viewport()->cursor();

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
            QString resolution;

	    if ( photo->size().isValid() )
	    {
		resolution = QString( "  %1 x %2" )
		    .arg( photo->size().width() )
		    .arg( photo->size().height() );
	    }

	    setWindowTitle( title + "  " + resolution );

            QString panelText = photo->fullPath();
            panelText += "\n" + resolution;

            m_titlePanel->setText( panelText );
            m_titlePanel->setTextAlignment( Qt::AlignRight | Qt::AlignVCenter );

            if ( m_exifPanel->isActive() )
                m_exifPanel->setMetaData();
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
    m_canvas->clear();
    setWindowTitle( "QPhotoView" );
}


void PhotoView::resizeEvent ( QResizeEvent * event )
{
    if ( event->size() != event->oldSize() )
    {
        layoutBorders( event->size() );
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
    QSizeF origSize = photo->size();

    switch ( m_zoomMode )
    {
	case NoZoom:
	    pixmap = photo->fullSizePixmap();
	    m_zoomFactor = 1.0;
	    break;


	case ZoomFitImage:
	    pixmap = photo->pixmap( size );

	    if ( origSize.width() != 0 )
		m_zoomFactor = pixmap.width() / origSize.width();
	    break;


	case ZoomFitWidth:

	    if ( origSize.width() != 0 )
	    {
		m_zoomFactor = size.width() / origSize.width();
		pixmap = photo->pixmap( m_zoomFactor * origSize );
	    }
	    break;


	case ZoomFitHeight:

	    if ( origSize.height() != 0 )
	    {
		m_zoomFactor = size.height() / origSize.height();
		pixmap = photo->pixmap( m_zoomFactor * origSize );
	    }
	    break;


	case ZoomFitBest:

	    if ( origSize.width() != 0 && origSize.height() != 0 )
	    {
		qreal zoomFactorX = size.width()  / origSize.width();
		qreal zoomFactorY = size.height() / origSize.height();
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
        if ( photo != m_lastPhoto )
        {
            m_panner->setPixmap( pixmap );
            m_lastPhoto = photo;
        }

        updatePanner( size );
        m_canvas->fixPosAnimated( false ); // not animated
    }

    setSceneRect( 0, 0, size.width(), size.height() );

    return success;
}


void PhotoView::updatePanner( const QSizeF & vpSize )
{
    QSizeF viewportSize = vpSize;

    if ( ! viewportSize.isValid() )
        viewportSize = size();

    if ( viewportSize.width()  < m_panner->size().width()  * 2  ||
         viewportSize.height() < m_panner->size().height() * 2  )
    {
        // If the panner would take up more than half the available space
        // in any direction, don't show it.

        m_panner->hide();
    }
    else
    {
        Photo * photo = m_photoDir->current();

        if ( ! photo )
        {
            m_panner->hide();
        }
        else
        {
            QSizeF  origSize   = photo->size();
            QPointF canvasPos  = m_canvas->pos();
            QSizeF  canvasSize = m_canvas->size();

            qreal   pannerX    = 0.0;
            qreal   pannerY    = -m_panner->size().height();

            if ( canvasSize.width() < viewportSize.width() )
                pannerX = canvasPos.x();

            if ( canvasSize.height() < viewportSize.height() )
                pannerY += canvasPos.y() + canvasSize.height();
            else
                pannerY += viewportSize.height();

            m_panner->setPos( pannerX, pannerY );

            QPointF visiblePos( -canvasPos.x(), -canvasPos.y() );

            QSizeF visibleSize( qMin( viewportSize.width(),
                                      canvasSize.width()  ),
                                qMin( viewportSize.height(),
                                      canvasSize.height() ) );

            QRectF visibleRect( visiblePos  / m_zoomFactor,
                                visibleSize / m_zoomFactor );

            m_panner->updatePanRect( visibleRect, origSize );
        }
    }
}


void PhotoView::createBorders()
{
    m_topLeftCorner     = createBorder( "TopLeftCorner"     );
    m_topBorder         = createBorder( "TopBorder"         );
    m_topRightCorner    = createBorder( "TopRightCorner"    );
    m_rightBorder       = createBorder( "RightBorder"       );
    m_bottomRightCorner = createBorder( "BottomRightCorner" );
    m_bottomBorder      = createBorder( "BottomBorder"      );
    m_bottomLeftCorner  = createBorder( "BottomLeftCorner"  );
    m_leftBorder        = createBorder( "LeftBorder"        );
}


SensitiveBorder * PhotoView::createBorder( const QString & objName )
{
    SensitiveBorder * border = new SensitiveBorder( this );
    border->setObjectName( objName );

#if 0
    connect( border, SIGNAL( borderEntered() ),
             this,   SLOT  ( showBorder()    ) );

    connect( border, SIGNAL( borderLeft()    ),
             this,   SLOT  ( hideBorder()    ) );
#endif

    return border;
}


void PhotoView::layoutBorders( const QSizeF & size )
{
    qreal top       = 0.0;
    qreal left      = 0.0;
    qreal width     = size.width();
    qreal height    = size.height();
    qreal thickness = 50.0;

    if ( width < 4 * thickness )
        thickness = width / 4;

    if ( height < 4 * thickness )
        thickness = height / 4;

    m_topLeftCorner->setRect( left, top, thickness, thickness );

    m_topBorder->setRect( thickness, top,
                          width - 2 * thickness, thickness );

    m_topRightCorner->setRect( width - thickness, top,
                               thickness, thickness );

    m_rightBorder->setRect( width - thickness, thickness,
                            thickness, height - 2 * thickness );

    m_bottomRightCorner->setRect( width - thickness, height - thickness,
                                  thickness, thickness );

    m_bottomBorder->setRect( thickness, height - thickness,
                             width - 2 * thickness, thickness );

    m_bottomLeftCorner->setRect( left, height - thickness,
                                 thickness, thickness );

    m_leftBorder->setRect( left, thickness,
                           thickness, height - 2 * thickness );
}


void PhotoView::createPanels()
{
    m_titlePanel = new TextBorderPanel( this, m_topRightCorner );
    // m_titlePanel->setSize( 500, 50 );
    m_titlePanel->setBorderFlags( BorderPanel::RightBorder |
                                  BorderPanel::TopBorder );

    m_exifPanel = new ExifBorderPanel( this, m_rightBorder );
    // m_exifPanel->setSize( 150, 300 );
    m_exifPanel->setBorderFlags( BorderPanel::RightBorder );
    m_exifPanel->setAlignment( Qt::AlignVCenter );

    m_navigationPanel = new BorderPanel( this, m_bottomBorder );
    m_navigationPanel->setSize( 400, 100 );
    m_navigationPanel->setBorderFlags( BorderPanel::BottomBorder );
    m_navigationPanel->setAlignment( Qt::AlignRight );

    m_toolPanel = new BorderPanel( this, m_leftBorder );
    m_toolPanel->setSize( 100, 400 );
    m_toolPanel->setBorderFlags( BorderPanel::LeftBorder );
    m_toolPanel->setAlignment( Qt::AlignTop );
}


void PhotoView::showBorder()
{
    if ( sender() )
    {
        qDebug() << "Show border" << sender()->objectName();
    }
}


void PhotoView::hideBorder()
{
    if ( sender() )
    {
        qDebug() << "Hide border" << sender()->objectName();
    }
}


void PhotoView::setIdleTimeout( int millisec )
{
    m_idleTimeout = millisec;

    if ( m_idleTimeout > 0 )
        m_idleTimer.start( m_idleTimeout );
    else
        m_idleTimer.stop();
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


void PhotoView::hideCursor()
{
    // qDebug() << __PRETTY_FUNCTION__;
    m_cursor = viewport()->cursor();
    viewport()->setCursor( Qt::BlankCursor );
    m_canvas->hideCursor();
}


void PhotoView::showCursor()
{
    viewport()->setCursor( m_cursor );
    m_canvas->showCursor();
}


void PhotoView::mouseMoveEvent ( QMouseEvent * event )
{
    // qDebug() << __PRETTY_FUNCTION__;
    m_idleTimer.start( m_idleTimeout );
    showCursor();

    QGraphicsView::mouseMoveEvent( event );
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
