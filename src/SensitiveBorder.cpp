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
#include "Logger.h"

static const int EnterLeaveTimeout = 1000; // millisec

bool SensitiveBorder::_debugMode = false;


SensitiveBorder::SensitiveBorder( PhotoView * parent )
    : QObject()
    , QGraphicsRectItem()
    , _photoView( parent )
{
    _photoView->scene()->addItem( this );
    setCursor( Qt::PointingHandCursor );

#if (QT_VERSION < QT_VERSION_CHECK( 5, 0, 0 ))
    setAcceptsHoverEvents( true );
#else
    setAcceptHoverEvents( true );
#endif

    _enterTimer.setSingleShot( true );
    _leaveTimer.setSingleShot( true );

    connect( &_enterTimer, SIGNAL( timeout()	   ),
	     this,	    SIGNAL( borderEntered() ) );

    connect( &_leaveTimer, SIGNAL( timeout()	   ),
	     this,	    SIGNAL( borderLeft()    ) );
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

    if ( _debugMode )
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
    _debugMode = on;
}


void SensitiveBorder::hoverEnterEvent( QGraphicsSceneHoverEvent * event )
{
    Q_UNUSED( event) ;
    // logDebug() << objectName() << endl;

    if ( _leaveTimer.isActive() )
	_leaveTimer.stop();
    else
	_enterTimer.start( EnterLeaveTimeout );
}


void SensitiveBorder::hoverLeaveEvent( QGraphicsSceneHoverEvent * event )
{
    Q_UNUSED( event) ;
    // logDebug() << objectName() << endl;

    if ( _enterTimer.isActive() )
	_enterTimer.stop();
    else
	_leaveTimer.start( EnterLeaveTimeout );
}
