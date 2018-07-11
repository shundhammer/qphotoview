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
#include "Logger.h"


static const int DefaultIdleTimeout = 4000; // millisec


PhotoView::PhotoView( PhotoDir * photoDir )
    : QGraphicsView()
    , _photoDir( photoDir )
    , _lastPhoto( 0 )
    , _zoomMode( ZoomFitImage )
    , _zoomFactor( 1.0	 )
    , _zoomIncrement( 1.2 )
    , _idleTimeout( DefaultIdleTimeout )
    , _actions( this )
{
    Q_CHECK_PTR( photoDir );
    setScene( new QGraphicsScene );

    _canvas = new Canvas( this );
    createBorders();

    QSize pannerMaxSize( qApp->desktop()->screenGeometry().size() / 6 );
    _panner = new Panner( pannerMaxSize, this );

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

    connect( &_idleTimer, SIGNAL( timeout()    ),
	     this,	   SLOT	 ( hideCursor() ) );

    _idleTimer.setSingleShot( true );
    _idleTimer.start( _idleTimeout );
    _cursor = viewport()->cursor();

    //
    // Load images
    //

    _photoDir->prefetch();

    if ( ! _photoDir->isEmpty() )
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
    _zoomMode = ZoomFitImage;
    bool success = reloadCurrent( size() );

    if ( success )
    {
	Photo * photo = _photoDir->current();

	if ( success && photo )
	{
            logInfo() << "Loading " << photo->fileName() << endl;
	    QString title( "QPhotoView	" + photo->fileName() );
	    QString resolution;

	    if ( photo->size().isValid() )
	    {
		resolution = QString( "	 %1 x %2" )
		    .arg( photo->size().width() )
		    .arg( photo->size().height() );
	    }

	    setWindowTitle( title + "  " + resolution );

	    QString panelText = photo->fullPath();
	    panelText += "\n" + resolution;

	    _titlePanel->setText( panelText );
	    _titlePanel->setTextAlignment( Qt::AlignRight | Qt::AlignVCenter );

	    if ( _exifPanel->isActive() )
		_exifPanel->setMetaData();
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
    _canvas->clear();
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
    Photo * photo = _photoDir->current();

    if ( ! photo )
	return false;

    QPixmap pixmap;
    QSizeF origSize = photo->size();

    switch ( _zoomMode )
    {
	case NoZoom:
	    pixmap = photo->fullSizePixmap();
	    _zoomFactor = 1.0;
	    break;


	case ZoomFitImage:
	    pixmap = photo->pixmap( size );

	    if ( origSize.width() != 0 )
		_zoomFactor = pixmap.width() / origSize.width();
	    break;


	case ZoomFitWidth:

	    if ( origSize.width() != 0 )
	    {
		_zoomFactor = size.width() / origSize.width();
		pixmap = photo->pixmap( _zoomFactor * origSize );
	    }
	    break;


	case ZoomFitHeight:

	    if ( origSize.height() != 0 )
	    {
		_zoomFactor = size.height() / origSize.height();
		pixmap = photo->pixmap( _zoomFactor * origSize );
	    }
	    break;


	case ZoomFitBest:

	    if ( origSize.width() != 0 && origSize.height() != 0 )
	    {
		qreal zoomFactorX = size.width()  / origSize.width();
		qreal zoomFactorY = size.height() / origSize.height();
		_zoomFactor = qMax( zoomFactorX, zoomFactorY );

		pixmap = photo->pixmap( _zoomFactor * origSize );
	    }
	    break;

	case UseZoomFactor:
	    pixmap = photo->pixmap( _zoomFactor * origSize );
	    break;

	    // Deliberately omitting 'default' branch so the compiler will warn
	    // about unhandled enum values
    };

    _canvas->setPixmap( pixmap );
    success = ! pixmap.isNull();

    if ( success )
    {
	if ( photo != _lastPhoto )
	{
	    _panner->setPixmap( pixmap );
	    _lastPhoto = photo;
	}

	updatePanner( size );
	_canvas->fixPosAnimated( false ); // not animated
    }

    setSceneRect( 0, 0, size.width(), size.height() );

    return success;
}


void PhotoView::updatePanner( const QSizeF & vpSize )
{
    QSizeF viewportSize = vpSize;

    if ( ! viewportSize.isValid() )
	viewportSize = size();

    if ( viewportSize.width()  < _panner->size().width()  * 2  ||
	 viewportSize.height() < _panner->size().height() * 2  )
    {
	// If the panner would take up more than half the available space
	// in any direction, don't show it.

	_panner->hide();
    }
    else
    {
	Photo * photo = _photoDir->current();

	if ( ! photo )
	{
	    _panner->hide();
	}
	else
	{
	    QSizeF  origSize   = photo->size();
	    QPointF canvasPos  = _canvas->pos();
	    QSizeF  canvasSize = _canvas->size();

	    qreal   pannerX    = 0.0;
	    qreal   pannerY    = -_panner->size().height();

	    if ( canvasSize.width() < viewportSize.width() )
		pannerX = canvasPos.x();

	    if ( canvasSize.height() < viewportSize.height() )
		pannerY += canvasPos.y() + canvasSize.height();
	    else
		pannerY += viewportSize.height();

	    _panner->setPos( pannerX, pannerY );

	    QPointF visiblePos( -canvasPos.x(), -canvasPos.y() );

	    QSizeF visibleSize( qMin( viewportSize.width(),
				      canvasSize.width()  ),
				qMin( viewportSize.height(),
				      canvasSize.height() ) );

	    QRectF visibleRect( visiblePos  / _zoomFactor,
				visibleSize / _zoomFactor );

	    _panner->updatePanRect( visibleRect, origSize );
	}
    }
}


void PhotoView::createBorders()
{
    _topLeftCorner     = createBorder( "TopLeftCorner"	   );
    _topBorder	       = createBorder( "TopBorder"	   );
    _topRightCorner    = createBorder( "TopRightCorner"	   );
    _rightBorder       = createBorder( "RightBorder"	   );
    _bottomRightCorner = createBorder( "BottomRightCorner" );
    _bottomBorder      = createBorder( "BottomBorder"	   );
    _bottomLeftCorner  = createBorder( "BottomLeftCorner"  );
    _leftBorder	       = createBorder( "LeftBorder"	   );
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
    qreal top	    = 0.0;
    qreal left	    = 0.0;
    qreal width	    = size.width();
    qreal height    = size.height();
    qreal thickness = 50.0;

    if ( width < 4 * thickness )
	thickness = width / 4;

    if ( height < 4 * thickness )
	thickness = height / 4;

    _topLeftCorner->setRect( left, top, thickness, thickness );

    _topBorder->setRect( thickness, top,
			 width - 2 * thickness, thickness );

    _topRightCorner->setRect( width - thickness, top,
			      thickness, thickness );

    _rightBorder->setRect( width - thickness, thickness,
			   thickness, height - 2 * thickness );

    _bottomRightCorner->setRect( width - thickness, height - thickness,
				 thickness, thickness );

    _bottomBorder->setRect( thickness, height - thickness,
			    width - 2 * thickness, thickness );

    _bottomLeftCorner->setRect( left, height - thickness,
				thickness, thickness );

    _leftBorder->setRect( left, thickness,
			  thickness, height - 2 * thickness );
}


void PhotoView::createPanels()
{
    _titlePanel = new TextBorderPanel( this, _topRightCorner );
    // _titlePanel->setSize( 500, 50 );
    _titlePanel->setBorderFlags( BorderPanel::RightBorder |
				 BorderPanel::TopBorder );

    _exifPanel = new ExifBorderPanel( this, _rightBorder );
    // _exifPanel->setSize( 150, 300 );
    _exifPanel->setBorderFlags( BorderPanel::RightBorder );
    _exifPanel->setAlignment( Qt::AlignVCenter );

    _navigationPanel = new BorderPanel( this, _bottomBorder );
    _navigationPanel->setSize( 400, 100 );
    _navigationPanel->setBorderFlags( BorderPanel::BottomBorder );
    _navigationPanel->setAlignment( Qt::AlignRight );

    _toolPanel = new BorderPanel( this, _leftBorder );
    _toolPanel->setSize( 100, 400 );
    _toolPanel->setBorderFlags( BorderPanel::LeftBorder );
    _toolPanel->setAlignment( Qt::AlignTop );
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
    _idleTimeout = millisec;

    if ( _idleTimeout > 0 )
	_idleTimer.start( _idleTimeout );
    else
	_idleTimer.stop();
}


void PhotoView::setZoomMode( ZoomMode mode )
{
    _zoomMode = mode;
    reloadCurrent( size() );
}


void PhotoView::setZoomFactor( qreal factor )
{
    _zoomFactor = factor;

    if ( qFuzzyCompare( _zoomFactor, 1.0 ) )
	setZoomMode( NoZoom );
    else
	setZoomMode( UseZoomFactor );
}


void PhotoView::zoomIn()
{
    if ( ! qFuzzyCompare( _zoomIncrement, 0.0 ) )
	setZoomFactor( _zoomFactor * _zoomIncrement );
}


void PhotoView::zoomOut()
{
    if ( ! qFuzzyCompare( _zoomIncrement, 0.0 ) )
	setZoomFactor( _zoomFactor / _zoomIncrement );
}


void PhotoView::hideCursor()
{
    // qDebug() << __PRETTY_FUNCTION__;
    _cursor = viewport()->cursor();
    viewport()->setCursor( Qt::BlankCursor );
    _canvas->hideCursor();
}


void PhotoView::showCursor()
{
    viewport()->setCursor( _cursor );
    _canvas->showCursor();
}


void PhotoView::toggleFullscreen()
{
    setWindowState( windowState() ^ Qt::WindowFullScreen );
}


void PhotoView::forceReload()
{
    Photo * photo = _photoDir->current();

    if ( photo )
    {
        photo->dropCache();
        loadImage();
    }
}


void PhotoView::mouseMoveEvent ( QMouseEvent * event )
{
    // qDebug() << __PRETTY_FUNCTION__;
    _idleTimer.start( _idleTimeout );
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
	    _photoDir->toNext();
	    loadImage();
	    break;

	case Qt::Key_PageUp:
	case Qt::Key_Backspace:

	    _photoDir->toPrevious();
	    loadImage();
	    break;

	case Qt::Key_Home:
	    _photoDir->toFirst();
	    loadImage();
	    break;

	case Qt::Key_End:
	    _photoDir->toLast();
	    loadImage();
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
	case Qt::Key_9:	       setZoomFactor( 1/9.0 );	break;
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

	case Qt::Key_Y:
	    {
		const int max=10;
		qDebug() << "*** Benchmark start";
		QTime time;
		time.start();

		for ( int i=0; i < max; ++i )
		{
		    _photoDir->toNext();
		    loadImage();
		}
		qDebug() << "*** Benchmark end; time:"
			 << time.elapsed() / 1000.0 << "sec /"
			 << max << "images";
	    }
	    break;

	default:
	    QGraphicsView::keyPressEvent( event );
    }
}


#define CONNECT_ACTION(ACTION, RECEIVER, RCVR_SLOT) \
    connect( (ACTION), SIGNAL( triggered() ), (RECEIVER), SLOT( RCVR_SLOT ) )


PhotoView::Actions::Actions( PhotoView * photoView ):
    _photoView( photoView )
{
    forceReload = createAction( tr( "Force &Reload" ), Qt::Key_F5 );
    CONNECT_ACTION( forceReload, photoView, forceReload() );

    toggleFullscreen = createAction( tr( "Toggle &Fullscreen" ), Qt::Key_Return );
    CONNECT_ACTION( toggleFullscreen, photoView, toggleFullscreen() );

    quit = createAction( tr ( "Quit" ) );
    quit->setShortcuts( QList<QKeySequence>() << Qt::Key_Q << Qt::Key_Escape );
    CONNECT_ACTION( quit, qApp, quit() );
}


QAction * PhotoView::Actions::createAction( const QString & text,
                                            QKeySequence    shortcut )
{
    QAction * action = new QAction( text, _photoView );
    _photoView->addAction( action );

    if ( ! shortcut.isEmpty() )
        action->setShortcut( shortcut );

    return action;
}
