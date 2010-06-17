/**
 * QPhotoView canvas graphics item for viewer widget.
 *
 * License: GPL V2. See file COPYING for details.
 *
 * Author:  Stefan Hundhammer <Stefan.Hundhammer@gmx.de>
 */

#include <QGraphicsSceneMouseEvent>
#include <QDebug>

#include "Canvas.h"
#include "PhotoView.h"
#include "Panner.h"


Canvas::Canvas( PhotoView * parent )
    : QGraphicsPixmapItem( QPixmap() )
    , m_photoView( parent )
    , m_panning( false )
{
    setCursor( Qt::ArrowCursor );
}


Canvas::~Canvas()
{
    
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


void Canvas::mousePressEvent( QGraphicsSceneMouseEvent * event )
{
    // qDebug() << __PRETTY_FUNCTION__;

    if ( event && event->button() == Qt::LeftButton )
    {
        m_panning = true;
        setCursor( Qt::ClosedHandCursor );
    }
}


void Canvas::mouseReleaseEvent( QGraphicsSceneMouseEvent * event )
{
    Q_UNUSED( event );
    
    // qDebug() << __PRETTY_FUNCTION__;

    if ( m_panning )
    {
        m_panning = false;
        setCursor( Qt::ArrowCursor );
    
        if ( m_photoView )
        {
            m_photoView->updatePanner();
        }
    }
}


void Canvas::mouseMoveEvent( QGraphicsSceneMouseEvent * event )
{
    // qDebug() << __PRETTY_FUNCTION__;

    if ( event && m_panning )
    {
        QPointF diff   = event->pos() - event->lastPos();
        QPointF newPos = pos() + diff;
        setPos( newPos );
        // qDebug() << "Mouse move diff:" << diff;

        if ( m_photoView )
        {
            QPointF pannerPos = m_photoView->panner()->pos();
            m_photoView->updatePanner();
            m_photoView->panner()->setPos( pannerPos );
        }
    }
}


void Canvas::mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event )
{
    qDebug() << __PRETTY_FUNCTION__;

    if ( event && m_photoView )
    {
        switch ( event->button() )
        {
            case Qt::LeftButton:
                m_photoView->zoomIn();
                // TO DO: Center on click position
                break;
                
            case Qt::RightButton:
                m_photoView->zoomOut();
                // TO DO: Center on click position
                break;

            default:
                break;
        }
    }

    setCursor( Qt::ArrowCursor );
}
