/*
 * QPhotoView border panel.
 *
 * License: GPL V2. See file COPYING for details.
 *
 * Author:  Stefan Hundhammer <Stefan.Hundhammer@gmx.de>
 */

#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QColor>
#include <QGraphicsSceneMouseEvent>
#include <QEasingCurve>
#include <QGraphicsBlurEffect>

#include "BorderPanel.h"
#include "SensitiveBorder.h"
#include "PhotoView.h"
#include "GraphicsItemPosAnimation.h"
#include "Logger.h"


static const int   AppearAnimationDuration    =	 850; // millisec
static const int   DisappearAnimationDuration =	 850; // millisec
static const int   EnterLeaveTimeout	      = 1000; // millisec

static const qreal PanelCornerRadius	      =	 8.0;
static const qreal DefaultBorderMargin	      = 10.0;
static const qreal DefaultMargin	      = 10.0;



BorderPanel::BorderPanel( PhotoView * parent, SensitiveBorder * border )
    : QObject()
    , QGraphicsItem()
    , _photoView( parent )
    , _sensitiveBorder( border )
    , _appearAnimation( 0 )
    , _disappearAnimation( 0 )
    , _alignment( Qt::AlignCenter )
    , _borderMargin( DefaultBorderMargin )
    , _margin( DefaultMargin )
    , _sticky( false )
    , _active( false )
{
    _photoView->scene()->addItem( this );
    hide();
    _leaveTimer.setSingleShot( true );

#if (QT_VERSION < QT_VERSION_CHECK( 5, 0, 0 ))
    setAcceptsHoverEvents( true );
#else
    setAcceptHoverEvents( true );
#endif

    const int grey  = 0x40;
    const int grey0 = 0x50;
    const int grey1 = 0x30;

    _grad = new QLinearGradient();
    _grad->setColorAt( 0.0, QColor( grey0, grey0, grey0, 255*0.9 ) );
    _grad->setColorAt( 1.0, QColor( grey1, grey1, grey1, 255*0.7 ) );

    _brush = QBrush( QColor( grey, grey, grey, 255*0.7 ) );
    _pen   = QPen( Qt::NoPen );

    if ( border )
    {
	connect( border, SIGNAL( borderEntered()     ),
		 this,	 SLOT  ( appearAnimated()    ) );

	connect( border, SIGNAL( borderLeft()	     ),
		 this,	 SLOT  ( disappearAnimated() ) );
    }

    connect( &_leaveTimer, SIGNAL( timeout() ),
	     this,	    SLOT  ( maybeDisappear() ) );
}


BorderPanel::~BorderPanel()
{
    if ( _appearAnimation )
	delete _appearAnimation;

    if ( _disappearAnimation )
	delete _disappearAnimation;

    if ( _grad )
	delete _grad;
}


GraphicsItemPosAnimation * BorderPanel::appearAnimation()
{
    if ( ! _appearAnimation )
    {
	_appearAnimation = new GraphicsItemPosAnimation( this );
	_appearAnimation->setEasingCurve( QEasingCurve::OutCubic );
    }

    return _appearAnimation;
}


GraphicsItemPosAnimation * BorderPanel::disappearAnimation()
{
    if ( ! _disappearAnimation )
    {
	_disappearAnimation = new GraphicsItemPosAnimation( this );
	_disappearAnimation->setEasingCurve( QEasingCurve::OutCubic );

	connect( _disappearAnimation, SIGNAL( finished() ),
		 this,		      SLOT  ( hide()	 ) );

	connect( _disappearAnimation, SIGNAL( finished()    ),
		 this,		      SIGNAL( disappeared() ) );
    }

    return _disappearAnimation;
}


QRectF BorderPanel::boundingRect() const
{
    return QRectF( QPointF( 0, 0 ), size() );
}


void BorderPanel::paint( QPainter * painter,
			 const QStyleOptionGraphicsItem * option,
			 QWidget * widget )
{
    Q_UNUSED( option );
    Q_UNUSED( widget );

    QRectF rect = boundingRect();

    if ( _grad )
    {
	_grad->setStart	   ( rect.topLeft() );
	_grad->setFinalStop( rect.bottomLeft() );
	painter->setBrush( *_grad );
    }
    else
    {
	painter->setBrush( _brush );
    }

    painter->setPen( _pen );
    painter->drawRoundedRect( rect, PanelCornerRadius, PanelCornerRadius );
}


void BorderPanel::setBrush( const QBrush & brush )
{
    _brush = brush;

    if ( _grad )
    {
	delete _grad;
	_grad = 0;
    }
}


void BorderPanel::setGradient( QLinearGradient * grad )
{
    if ( _grad )
	delete _grad;

    _grad = grad;
}


void BorderPanel::setSticky( bool sticky )
{
    _sticky = sticky;

    if ( _sticky && ! _active )
	appearNow();
}


void BorderPanel::appearNow()
{
    // logDebug() << endl;

    emit aboutToAppear();
    setPos( activePos() );
    show();
    _active = true;
}


void BorderPanel::appearAnimated()
{
    // logDebug() << endl;

    if ( ! scene() )
    {
	logError() << "BorderPanel " << hex << (void *) this
                   << " not added to a scene!" << endl;
    }

    if ( _borderFlags == NoBorder )
    {
	logError() << "No BorderFlags specified for BorderPanel "
                   << hex << (void *) this << endl;
    }

    emit aboutToAppear();
    QPointF startPos = pos();

    if ( _disappearAnimation &&
	 _disappearAnimation->state() == QAbstractAnimation::Running )
    {
	_disappearAnimation->stop();
    }
    else
    {
	startPos = inactivePos();
	setPos( startPos );
    }

    show();
    GraphicsItemPosAnimation * animation = appearAnimation();
    animation->setStartValue( startPos );
    animation->setEndValue  ( activePos() );
    animation->setDuration  ( AppearAnimationDuration );
    animation->start();

    _active = true;
}


void BorderPanel::disappearAnimated()
{
    // logDebug() << endl;

    if ( _appearAnimation &&
	 _appearAnimation->state() == QAbstractAnimation::Running )
    {
	_appearAnimation->stop();
    }

    GraphicsItemPosAnimation * animation = disappearAnimation();
    animation->setStartValue( pos() );
    animation->setEndValue  ( inactivePos() );
    animation->setDuration  ( DisappearAnimationDuration );
    animation->start();

    _active = false;
}


void BorderPanel::maybeDisappear()
{
    if ( ! _sticky )
	disappearAnimated();
}


QPointF BorderPanel::activePos()
{
    // logDebug() << endl;

    QSizeF viewportSize = _photoView->size();
    QSizeF panelSize	= size();

    // Start with fallback: Centered on viewport
    // This is important when the calling application forgets to set any border
    // flags at all.
    qreal  panelX = ( viewportSize.width()  - panelSize.width()	 ) / 2;
    qreal  panelY = ( viewportSize.height() - panelSize.height() ) / 2;

    if ( _borderFlags & LeftBorder )
	panelX = _borderMargin;

    if ( _borderFlags & RightBorder )
	panelX = viewportSize.width() - panelSize.width() - _borderMargin;

    if ( _borderFlags & TopBorder )
	panelY = _borderMargin;

    if ( _borderFlags & BottomBorder )
	panelY = viewportSize.height() - panelSize.height() - _borderMargin;

#if 0
    logDebug() << "viewport size: " << viewportSize << endl;
    logDebug() << "panel size: " << panelSize << endl;
    logDebug() << "panel X: " << panelX << " Y: " << panelY << endl;
#endif

    return secondaryPos( QPointF( panelX, panelY ) );
}


QPointF BorderPanel::inactivePos()
{
    // logDebug() << endl;

    QSizeF viewportSize = _photoView->size();
    QSizeF panelSize	= size();
    qreal  panelX = 0.0;
    qreal  panelY = 0.0;

    if ( _borderFlags & LeftBorder )
	panelX = -panelSize.width() - 1;

    if ( _borderFlags & RightBorder )
	panelX = viewportSize.width() + 1;

    if ( _borderFlags & TopBorder )
	panelY = -panelSize.height() - 1;

    if ( _borderFlags & BottomBorder )
	panelY = viewportSize.height() + 1;

#if 0
    logDebug() << "viewport size: " << viewportSize << endl;
    logDebug() << "panel size: " << panelSize << endl;
    logDebug() << "panel X: " << panelX << " Y: " << panelY << endl;
#endif

    return secondaryPos( QPointF( panelX, panelY ) );
}


QPointF BorderPanel::secondaryPos( const QPointF( primaryPos ) )
{
    // logDebug() << endl;

    qreal  panelX = primaryPos.x();
    qreal  panelY = primaryPos.y();
    QSizeF viewportSize = _photoView->size();
    QSizeF panelSize	= size();

    QRectF parentRect = _sensitiveBorder ?
	_sensitiveBorder->rect() :
	QRectF( QPointF( 0.0, 0.0 ), viewportSize );

    if ( ( _borderFlags & ( LeftBorder | RightBorder ) ) == 0 )
    {
	// Horizontal alignment

	if ( _alignment & Qt::AlignHCenter )
	{
	    panelX = ( parentRect.width() - panelSize.width() ) / 2;
	    panelX += parentRect.x();
	}

	if ( _alignment & Qt::AlignLeft )
	    panelX = parentRect.x();

	if ( _alignment & Qt::AlignRight )
	    panelX = parentRect.width() - panelSize.width();
    }

    if ( ( _borderFlags & ( TopBorder | BottomBorder ) ) == 0 )
    {
	// Vertical alignment

	if ( _alignment & Qt::AlignVCenter )
	{
	    panelY = ( parentRect.height() - panelSize.height() ) / 2;
	    panelY += parentRect.y();
	}

	if ( _alignment & Qt::AlignTop )
	    panelY = parentRect.y();

	if ( _alignment & Qt::AlignBottom )
	    panelY = parentRect.height() - panelSize.height();
    }

#if 0
    logDebug() << "parent rect: " << parentRect << endl;
    logDebug() << "panel X: " << panelX << " Y: " << panelY << endl;
    logDebug() << "\n" << endl;
#endif

    return QPointF( panelX, panelY );
}


QSizeF BorderPanel::size() const
{
    return _size;
}


void BorderPanel::hoverEnterEvent( QGraphicsSceneHoverEvent * event )
{
    Q_UNUSED( event) ;
    // logDebug() << endl;

    if ( _leaveTimer.isActive() )
	_leaveTimer.stop();
}


void BorderPanel::hoverLeaveEvent( QGraphicsSceneHoverEvent * event )
{
    Q_UNUSED( event) ;
    // logDebug() << endl;

    _leaveTimer.start( EnterLeaveTimeout );
}


void BorderPanel::mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event )
{
    Q_UNUSED( event );

    if ( _sticky )
    {
	_sticky = false;
	disappearAnimated();
    }
    else
    {
	_sticky = true;
    }
}
