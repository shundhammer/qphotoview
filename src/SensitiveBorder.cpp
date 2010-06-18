/**
 * QPhotoView sensitive border graphics item
 *
 * License: GPL V2. See file COPYING for details.
 *
 * Author:  Stefan Hundhammer <Stefan.Hundhammer@gmx.de>
 */

#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QColor>
#include <QDebug>

#include "SensitiveBorder.h"
#include "PhotoView.h"

static const int EnterLeaveTimeout = 1000; // millisec

bool SensitiveBorder::m_debugMode = false;


SensitiveBorder::SensitiveBorder( PhotoView * parent )
    : QObject()
    , QGraphicsRectItem()
    , m_photoView( parent )
{
    setAcceptsHoverEvents( true );
    m_enterTimer.setSingleShot( true );
    m_leaveTimer.setSingleShot( true );

    connect( &m_enterTimer, SIGNAL( timeout()       ),
             this,          SIGNAL( borderEntered() ) );

    connect( &m_leaveTimer, SIGNAL( timeout()       ),
             this,          SIGNAL( borderLeft()    ) );
}


SensitiveBorder::~SensitiveBorder()
{

}


void SensitiveBorder::paint( QPainter * painter,
                             const QStyleOptionGraphicsItem * option,
                             QWidget * widget )
{
    Q_UNUSED( option );
    Q_UNUSED( widget );

    if ( m_debugMode )
    {
        QPen pen( Qt::black, 1 );
        pen.setJoinStyle( Qt::MiterJoin );

        const int grey = 32;
        QBrush brush( QColor( grey, grey, grey, 255*0.5 ) ); // grey transparent

        painter->setPen( pen );
        painter->setBrush( brush );

        painter->drawRect( rect() );
    }
}


void SensitiveBorder::setDebugMode( bool on )
{
    m_debugMode = on;
}


void SensitiveBorder::hoverEnterEvent( QGraphicsSceneHoverEvent * event )
{
    Q_UNUSED( event) ;
    // qDebug() << __PRETTY_FUNCTION__ << objectName();

    if ( m_leaveTimer.isActive() )
        m_leaveTimer.stop();
    else
        m_enterTimer.start( EnterLeaveTimeout );
}


void SensitiveBorder::hoverLeaveEvent( QGraphicsSceneHoverEvent * event )
{
    Q_UNUSED( event) ;
    // qDebug() << __PRETTY_FUNCTION__ << objectName();

    if ( m_enterTimer.isActive() )
        m_enterTimer.stop();
    else
        m_leaveTimer.start( EnterLeaveTimeout );
}
