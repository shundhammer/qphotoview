/**
 * QPhotoView canvas graphics item for viewer widget.
 *
 * License: GPL V2. See file COPYING for details.
 *
 * Author:  Stefan Hundhammer <Stefan.Hundhammer@gmx.de>
 */

#include <QGraphicsSceneMouseEvent>
#include <QMenu>

#include "Canvas.h"
#include "PhotoView.h"
#include "Panner.h"
#include "GraphicsItemPosAnimation.h"
#include "Logger.h"


static const int AnimationDuration =  850; // millisec


Canvas::Canvas( PhotoView * parent )
    : QGraphicsPixmapItem( QPixmap() )
    , _photoView( parent )
    , _panning( false )
    , _animation( 0 )
{
    Q_CHECK_PTR( _photoView );

    _photoView->scene()->addItem( this );
    setCursor( Qt::OpenHandCursor );
    _cursor = cursor();
}


Canvas::~Canvas()
{
    if ( _animation )
	delete _animation;
}


QSize Canvas::size() const
{
    return pixmap().size();
}


void Canvas::clear()
{
    setPixmap( QPixmap() );
}


void Canvas::center( const QSize & parentSize )
{
    QSize pixmapSize = pixmap().size();
    qreal x = pos().x();
    qreal y = pos().y();

    if ( pixmapSize.width() < parentSize.width() )
	x = ( parentSize.width() - pixmapSize.width()  ) / 2.0;
    else if ( x > 0.0 )
	x = 0.0;

    if ( pixmapSize.height() < parentSize.height() )
	y = ( parentSize.height() - pixmapSize.height() ) / 2.0;
    else if ( y > 0.0 )
	y = 0.0;

    setPos( x, y );
}


void Canvas::hideCursor()
{
    setCursor( Qt::BlankCursor );
}


void Canvas::showCursor()
{
    if ( _panning )
	setCursor( Qt::ClosedHandCursor );
    else
	setCursor( _cursor );
}


void Canvas::mousePressEvent( QGraphicsSceneMouseEvent * event )
{
    // logDebug() << endl;

    if ( event && event->button() == Qt::LeftButton )
    {
	_panning = true;
	setCursor( Qt::ClosedHandCursor );

	if ( _animation && _animation->state() == QAbstractAnimation::Running )
	    _animation->stop();

	_photoView->updatePanner();
    }
}


void Canvas::mouseReleaseEvent( QGraphicsSceneMouseEvent * event )
{
    Q_UNUSED( event );
    // logDebug() << endl;

    if ( _panning )
    {
	_panning = false;
	setCursor( _cursor );

	_photoView->updatePanner();
	fixPosAnimated();
    }
}


void Canvas::mouseMoveEvent( QGraphicsSceneMouseEvent * event )
{
    // logDebug() << endl;

    if ( event && _panning )
    {
	QPointF diff   = event->pos() - event->lastPos();
	QPointF newPos = pos() + diff;
	setPos( newPos );
	// logDebug() << "Mouse move diff: " << diff;

	QPointF pannerPos = _photoView->panner()->pos();
	_photoView->updatePanner();
	_photoView->panner()->setPos( pannerPos );
    }
    else
    {
	setCursor( _cursor );
    }
}


void Canvas::mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event )
{
    // logDebug() << endl;

    if ( event )
    {
	switch ( event->button() )
	{
	    case Qt::LeftButton:
		_photoView->zoomIn();
		// TO DO: Center on click position
		break;

	    case Qt::RightButton:
                // Double click right doesn't since the first click will
                // already open the context menu

	    case Qt::MidButton:
		_photoView->zoomOut();
		// TO DO: Center on click position
		break;

	    default:
		break;
	}
    }

    setCursor( _cursor );
}


void Canvas::wheelEvent( QGraphicsSceneWheelEvent * event )
{
    if ( event->delta() > 0 ) // scroll up
    {
	_photoView->navigate( PhotoView::NavigatePrevious );
    }
    else if ( event->delta() < 0 ) // scroll down
    {
	_photoView->navigate( PhotoView::NavigateNext );
    }
}


void Canvas::contextMenuEvent( QGraphicsSceneContextMenuEvent * event )
{
    QMenu menu;

    menu.addAction( _photoView->actions().zoomIn           );
    menu.addAction( _photoView->actions().zoomOut          );
    menu.addAction( _photoView->actions().noZoom           );
    menu.addAction( _photoView->actions().zoomFitImage     );
    menu.addAction( _photoView->actions().zoomFitWidth     );
    menu.addAction( _photoView->actions().zoomFitHeight    );
    menu.addAction( _photoView->actions().zoomFitBest      );
    menu.addSeparator();
    menu.addAction( _photoView->actions().loadNext         );
    menu.addAction( _photoView->actions().loadPrevious     );
    menu.addAction( _photoView->actions().loadFirst        );
    menu.addAction( _photoView->actions().loadLast         );
    menu.addAction( _photoView->actions().forceReload      );
    menu.addSeparator();
    menu.addAction( _photoView->actions().toggleFullscreen );
    menu.addSeparator();
    menu.addAction( _photoView->actions().quit );

    menu.exec( event->screenPos() );
}


void Canvas::fixPosAnimated( bool animate )
{
    QSize   viewportSize = _photoView->size();
    QSize   canvasSize	 = size();
    QPointF canvasPos	 = pos();
    QPointF wantedPos	 = canvasPos;

    if ( canvasSize.width() < viewportSize.width() )
    {
	// Center horizontally

	wantedPos.setX( ( viewportSize.width() - canvasSize.width() ) / 2.0 );
    }
    else
    {
	// Check if we panned too far left or right

	if ( canvasPos.x() > 0.0 ) // Black border left?
	    wantedPos.setX( 0.0 );

	if ( canvasPos.x() + canvasSize.width() < viewportSize.width() )
	    wantedPos.setX( viewportSize.width() - canvasSize.width() );
    }

    if ( canvasSize.height() < viewportSize.height() )
    {
	// Center vertically

	wantedPos.setY( ( viewportSize.height() -
			  canvasSize.height()	 ) / 2.0 );
    }
    else
    {
	// Check if we panned to far up or down

	if ( canvasPos.y() > 0.0 ) // Black border at the top?
	    wantedPos.setY( 0.0 );

	if ( canvasPos.y() + canvasSize.height() < viewportSize.height() )
	    wantedPos.setY( viewportSize.height() - canvasSize.height() );
    }

    QPointF diff = wantedPos - canvasPos;
    qreal manhattanLength = diff.manhattanLength();

#if 0
    logDebug() << "Canvas pos:\t"   << canvasPos    << endl;
    logDebug() << "Canvas size:\t"  << canvasSize   << endl;
    logDebug() << "VP size:\t"      << viewportSize << endl;
    logDebug() << "Wanted pos:\t"   << wantedPos    << endl;

    logDebug() << "Pos diff:\t"         << diff            << endl;
    logDebug() << "Manhattan length:\t" << manhattanLength << endl;
    logDebug() << "\n" << endl;
#endif

    if ( manhattanLength > 0.0 )
    {
	if ( manhattanLength < 5.0 || ! animate )
	{
	    setPos( wantedPos );
	    _photoView->updatePanner();
	}
	else
	{
	    // Animate moving to new position

	    if ( ! _animation )
	    {
		_animation = new GraphicsItemPosAnimation( this );
#if 0
		QObject::connect( _animation, SIGNAL( finished() ),
				  _photoView, SLOT  ( updatePanner() ) );
#endif
		QObject::connect( _animation, SIGNAL( valueChanged(QVariant)),
				  _photoView, SLOT  ( updatePanner() ) );
	    }

	    _animation->setStartValue( canvasPos );
	    _animation->setEndValue  ( wantedPos );
	    _animation->setDuration  ( AnimationDuration );
	    _animation->start();
	}
    }
}

