/**
 * QPhotoView panner graphics item for viewer widget.
 *
 * License: GPL V2. See file COPYING for details.
 *
 * Author:  Stefan Hundhammer <Stefan.Hundhammer@gmx.de>
 */


#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QPixmap>
#include <QSize>
#include <QRect>
#include <QPen>
#include <QPainter>
#include <QDebug>

#include "Panner.h"
#include "PhotoView.h"
#include "Canvas.h"

static const int FrameThickness	  = 4;
static const int PanRectThickness = 3;


Panner::Panner( const QSizeF & pannerMaxSize, PhotoView * photoView )
    : QGraphicsItem()
    , _pannerMaxSize( pannerMaxSize )
    , _photoView( photoView )
{
    _photoView->scene()->addItem( this );
    _pannerMaxSize -= QSizeF( 2*FrameThickness, 2*FrameThickness );
    _size = _pannerMaxSize;

    _pixmapItem = new QGraphicsPixmapItem( this );
    _pixmapItem->setPos( QPointF( FrameThickness, FrameThickness ) );

    _panRect = new QGraphicsRectItem( QRectF( 0, 0, 20, 20 ), _pixmapItem );
    QPen panRectPen( Qt::yellow, PanRectThickness );
    panRectPen.setJoinStyle( Qt::MiterJoin );
    _panRect->setPen( panRectPen );

    // The panner doesn't really accept hover events, but it's irritating for
    // the user if hover events go to a SensitiveBorder underneath the
    // panner and a BorderPanel opens when the mouse pointer hovers over the
    // panner.

#if (QT_VERSION < QT_VERSION_CHECK( 5, 0, 0 ))
    setAcceptsHoverEvents( true );
#else
    setAcceptHoverEvents( true );
#endif

    setCursor( Qt::CrossCursor );
}


Panner::~Panner()
{
    // Child QGraphicsItems are automatically deleted
}


void Panner::paint( QPainter * painter,
		    const QStyleOptionGraphicsItem * option,
		    QWidget * widget )
{
    Q_UNUSED( option );
    Q_UNUSED( widget );

    QPen pen( Qt::white, FrameThickness );
    pen.setJoinStyle( Qt::MiterJoin );
    painter->setPen( pen );

    painter->drawRect( FrameThickness/2.0,
		       FrameThickness/2.0,
		       _size.width()  - FrameThickness,
		       _size.height() - FrameThickness );
}


QRectF Panner::boundingRect() const
{
    return QRectF( QPointF( 0, 0 ), _size );
}


void Panner::setPixmap( const QPixmap & pixmap )
{
    _pixmap = pixmap;

    _size = pixmap.size();
    _size.scale( _pannerMaxSize, Qt::KeepAspectRatio );
    _size += QSizeF( 2*FrameThickness, 2*FrameThickness );
}


void Panner::lazyScalePixmap()
{
    if ( _pixmap.isNull() )
	return;

    QSizeF pannerPixmapSize = _pixmap.size();
    pannerPixmapSize.scale( _pannerMaxSize, Qt::KeepAspectRatio );

    // This is expensive
    QPixmap scaledPixmap = _pixmap.scaled( qRound( pannerPixmapSize.width() ),
					   qRound( pannerPixmapSize.height() ),
					   Qt::KeepAspectRatio,
					   Qt::SmoothTransformation );
    _pixmapItem->setPixmap( scaledPixmap );
    _pixmap = QPixmap();
}


void Panner::updatePanRect( const QRectF & visibleRect,
			    const QSizeF & origSize )
{
    if ( ! visibleRect.isValid() )
    {
	qDebug() << __PRETTY_FUNCTION__ << ": Invalid visible rect";
	return;
    }

    qreal visibleXPart = visibleRect.width()  / origSize.width();
    qreal visibleYPart = visibleRect.height() / origSize.height();

    bool completelyVisible = visibleXPart > 0.99 && visibleYPart > 0.99;

    if ( completelyVisible )
    {
#if 0
	bool panning =
	    _photoView &&
	    _photoView->canvas() &&
	    _photoView->canvas()->panning();

	if ( ! panning )
#endif
	{
	    // qDebug() << "Complete image visible";
	    hide();
	    return;
	}
    }

    lazyScalePixmap();
    show();

    if ( completelyVisible )
    {
	QSizeF pixmapSize = _pixmapItem->pixmap().size();
	_panRect->setRect( QRectF( QPointF( 0.0, 0.0 ), pixmapSize ) );
    }
    else
    {
	qreal	panPixmapWidth = _size.width() - 2*FrameThickness;
	qreal	scale	= panPixmapWidth / (qreal) origSize.width();
	QPointF panPos	= scale * visibleRect.topLeft();
	QSizeF	panSize = scale * visibleRect.size();

	_panRect->setRect( QRectF( panPos, panSize ) );
    }
}

