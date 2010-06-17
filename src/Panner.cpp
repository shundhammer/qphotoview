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

static const int FrameThickness   = 4;
static const int PanRectThickness = 3;


Panner::Panner( const QSizeF & pannerMaxSize, PhotoView * photoView )
    : QGraphicsItem()
    , m_pannerMaxSize( pannerMaxSize )
    , m_photoView( photoView )
{
    m_pannerMaxSize -= QSizeF( 2*FrameThickness, 2*FrameThickness );
    m_size = m_pannerMaxSize;

    m_pixmapItem = new QGraphicsPixmapItem( this );
    m_pixmapItem->setPos( QPointF( FrameThickness, FrameThickness ) );

    m_panRect = new QGraphicsRectItem( QRectF( 0, 0, 20, 20 ), m_pixmapItem );
    QPen panRectPen( Qt::yellow, PanRectThickness );
    panRectPen.setJoinStyle( Qt::MiterJoin );
    m_panRect->setPen( panRectPen );
    // hide();
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
                       m_size.width()  - FrameThickness,
                       m_size.height() - FrameThickness );
}


QRectF Panner::boundingRect() const
{
    return QRectF( QPointF( 0, 0 ), m_size );
}


void Panner::setPixmap( const QPixmap & pixmap )
{
    m_pixmap = pixmap;

    m_size = pixmap.size();
    m_size.scale( m_pannerMaxSize, Qt::KeepAspectRatio );
    m_size += QSizeF( 2*FrameThickness, 2*FrameThickness );
}


void Panner::lazyScalePixmap()
{
    if ( m_pixmap.isNull() )
        return;

    QSizeF pannerPixmapSize = m_pixmap.size();
    pannerPixmapSize.scale( m_pannerMaxSize, Qt::KeepAspectRatio );

    // This is expensive
    QPixmap scaledPixmap = m_pixmap.scaled( qRound( pannerPixmapSize.width() ),
                                            qRound( pannerPixmapSize.height() ),
                                            Qt::KeepAspectRatio,
                                            Qt::SmoothTransformation );
    m_pixmapItem->setPixmap( scaledPixmap );
    m_pixmap = QPixmap();
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
            m_photoView &&
            m_photoView->canvas() &&
            m_photoView->canvas()->panning();

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
        QSizeF pixmapSize = m_pixmapItem->pixmap().size();
        m_panRect->setRect( QRectF( QPointF( 0.0, 0.0 ), pixmapSize ) );
    }
    else
    {
        qreal   panPixmapWidth = m_size.width() - 2*FrameThickness;
        qreal   scale   = panPixmapWidth / (qreal) origSize.width();
        QPointF panPos  = scale * visibleRect.topLeft();
        QSizeF  panSize = scale * visibleRect.size();

        m_panRect->setRect( QRectF( panPos, panSize ) );
    }
}

