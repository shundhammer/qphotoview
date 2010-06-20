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
#include "GraphicsItemPosAnimation.h"


static const int AnimationDuration =  850; // millisec


Canvas::Canvas( PhotoView * parent )
    : QGraphicsPixmapItem( QPixmap() )
    , m_photoView( parent )
    , m_panning( false )
    , m_animation( 0 )
{
    Q_CHECK_PTR( m_photoView );

    m_photoView->scene()->addItem( this );
    setCursor( Qt::OpenHandCursor );
    m_cursor = cursor();
}


Canvas::~Canvas()
{
    if ( m_animation )
        delete m_animation;
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
    if ( m_panning )
        setCursor( Qt::ClosedHandCursor );
    else
        setCursor( m_cursor );
}


void Canvas::mousePressEvent( QGraphicsSceneMouseEvent * event )
{
    // qDebug() << __PRETTY_FUNCTION__;

    if ( event && event->button() == Qt::LeftButton )
    {
        m_panning = true;
        setCursor( Qt::ClosedHandCursor );

        if ( m_animation && m_animation->state() == QAbstractAnimation::Running )
            m_animation->stop();

        m_photoView->updatePanner();
    }
}


void Canvas::mouseReleaseEvent( QGraphicsSceneMouseEvent * event )
{
    Q_UNUSED( event );

    // qDebug() << __PRETTY_FUNCTION__;

    if ( m_panning )
    {
        m_panning = false;
        setCursor( m_cursor );

        m_photoView->updatePanner();
        fixPosAnimated();
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

        QPointF pannerPos = m_photoView->panner()->pos();
        m_photoView->updatePanner();
        m_photoView->panner()->setPos( pannerPos );
    }
    else
    {
        setCursor( m_cursor );
    }
}


void Canvas::mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event )
{
    // qDebug() << __PRETTY_FUNCTION__;

    if ( event )
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

    setCursor( m_cursor );
}


void Canvas::fixPosAnimated( bool animate )
{
    QSize   viewportSize = m_photoView->size();
    QSize   canvasSize   = size();
    QPointF canvasPos    = pos();
    QPointF wantedPos    = canvasPos;

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
                          canvasSize.height()    ) / 2.0 );
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
    qDebug() << "Canvas pos:\t"  << canvasPos;
    qDebug() << "Canvas size:\t" << canvasSize;
    qDebug() << "VP size:\t" << viewportSize;
    qDebug() << "Wanted pos:\t" << wantedPos;

    qDebug() << "Pos diff:\t" << diff;
    qDebug() << "Manhattan length:\t" << manhattanLength;
    qDebug() << "\n";
#endif

    if ( manhattanLength > 0.0 )
    {
        if ( manhattanLength < 5.0 || ! animate )
        {
            setPos( wantedPos );
            m_photoView->updatePanner();
        }
        else
        {
            // Animate moving to new position

            if ( ! m_animation )
            {
                m_animation = new GraphicsItemPosAnimation( this );
#if 0
                QObject::connect( m_animation, SIGNAL( finished() ),
                                  m_photoView, SLOT  ( updatePanner() ) );
#endif
                QObject::connect( m_animation, SIGNAL( valueChanged(QVariant)),
                                  m_photoView, SLOT  ( updatePanner() ) );
            }

            m_animation->setStartValue( canvasPos );
            m_animation->setEndValue  ( wantedPos );
            m_animation->setDuration  ( AnimationDuration );
            m_animation->start();
        }
    }
}

