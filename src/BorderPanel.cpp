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
#include <QDebug>

#include "BorderPanel.h"
#include "SensitiveBorder.h"
#include "PhotoView.h"
#include "GraphicsItemPosAnimation.h"


static const int   AppearAnimationDuration    =  850; // millisec
static const int   DisappearAnimationDuration =  850; // millisec
static const int   EnterLeaveTimeout          = 1000; // millisec

static const qreal PanelCornerRadius          =  8.0;
static const qreal DefaultBorderMargin        = 10.0;
static const qreal DefaultMargin              = 10.0;



BorderPanel::BorderPanel( PhotoView * parent, SensitiveBorder * border )
    : QObject()
    , QGraphicsItem()
    , m_photoView( parent )
    , m_sensitiveBorder( border )
    , m_appearAnimation( 0 )
    , m_disappearAnimation( 0 )
    , m_alignment( Qt::AlignCenter )
    , m_borderMargin( DefaultBorderMargin )
    , m_margin( DefaultMargin )
    , m_sticky( false )
    , m_active( false )
{
    hide();
    setAcceptsHoverEvents( true );
    m_leaveTimer.setSingleShot( true );

    // const int grey = 0xE0;
    const int grey = 0x40;
    m_brush = QBrush( QColor( grey, grey, grey, 255*0.7 ) );
    m_pen   = QPen( Qt::NoPen );
    // m_pen.setColor( Qt::red );

    if ( border )
    {
        connect( border, SIGNAL( borderEntered()     ),
                 this,   SLOT  ( appearAnimated()    ) );

        connect( border, SIGNAL( borderLeft()        ),
                 this,   SLOT  ( disappearAnimated() ) );
    }

    connect( &m_leaveTimer, SIGNAL( timeout() ),
             this,          SLOT  ( maybeDisappear() ) );
}


BorderPanel::~BorderPanel()
{
    if ( m_appearAnimation )
        delete m_appearAnimation;

    if ( m_disappearAnimation )
        delete m_disappearAnimation;
}


GraphicsItemPosAnimation * BorderPanel::appearAnimation()
{
    if ( ! m_appearAnimation )
    {
        m_appearAnimation = new GraphicsItemPosAnimation( this );
        m_appearAnimation->setEasingCurve( QEasingCurve::OutCubic );
    }

    return m_appearAnimation;
}


GraphicsItemPosAnimation * BorderPanel::disappearAnimation()
{
    if ( ! m_disappearAnimation )
    {
        m_disappearAnimation = new GraphicsItemPosAnimation( this );
        m_disappearAnimation->setEasingCurve( QEasingCurve::OutCubic );

        connect( m_disappearAnimation, SIGNAL( finished() ),
                 this,                 SLOT  ( hide()     ) );

        connect( m_disappearAnimation, SIGNAL( finished()    ),
                 this,                 SIGNAL( disappeared() ) );
    }

    return m_disappearAnimation;
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

    painter->setBrush( m_brush );
    painter->setPen( m_pen );
    painter->drawRoundedRect( QRectF( QPointF( 0, 0 ), size() ),
                              PanelCornerRadius, PanelCornerRadius );
}


void BorderPanel::setSticky( bool sticky )
{
    m_sticky = sticky;

    if ( m_sticky && ! m_active )
        appearNow();
}


void BorderPanel::appearNow()
{
    // qDebug() << __PRETTY_FUNCTION__;

    emit aboutToAppear();
    setPos( activePos() );
    show();
    m_active = true;
}


void BorderPanel::appearAnimated()
{
    // qDebug() << __PRETTY_FUNCTION__;

    if ( ! scene() )
    {
        qWarning() << "ERROR: BorderPanel" << hex << (void *) this
                   << "not added to a scene!";
    }

    if ( m_borderFlags == NoBorder )
    {
        qWarning() << "ERROR: No BorderFlags specified for BorderPanel"
                   << hex << (void *) this;
    }

    emit aboutToAppear();
    QPointF startPos = pos();

    if ( m_disappearAnimation &&
         m_disappearAnimation->state() == QAbstractAnimation::Running )
    {
        m_disappearAnimation->stop();
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

    m_active = true;
}


void BorderPanel::disappearAnimated()
{
    // qDebug() << __PRETTY_FUNCTION__;

    if ( m_appearAnimation &&
         m_appearAnimation->state() == QAbstractAnimation::Running )
    {
        m_appearAnimation->stop();
    }

    GraphicsItemPosAnimation * animation = disappearAnimation();
    animation->setStartValue( pos() );
    animation->setEndValue  ( inactivePos() );
    animation->setDuration  ( DisappearAnimationDuration );
    animation->start();

    m_active = false;
}


void BorderPanel::maybeDisappear()
{
    if ( ! m_sticky )
        disappearAnimated();
}


QPointF BorderPanel::activePos()
{
    // qDebug() << __PRETTY_FUNCTION__;

    QSizeF viewportSize = m_photoView->size();
    QSizeF panelSize    = size();

    // Start with fallback: Centered on viewport
    // This is important when the calling application forgets to set any border
    // flags at all.
    qreal  panelX = ( viewportSize.width()  - panelSize.width()  ) / 2;
    qreal  panelY = ( viewportSize.height() - panelSize.height() ) / 2;

    if ( m_borderFlags & LeftBorder )
        panelX = m_borderMargin;

    if ( m_borderFlags & RightBorder )
        panelX = viewportSize.width() - panelSize.width() - m_borderMargin;

    if ( m_borderFlags & TopBorder )
        panelY = m_borderMargin;

    if ( m_borderFlags & BottomBorder )
        panelY = viewportSize.height() - panelSize.height() - m_borderMargin;

#if 0
    qDebug() << "viewport size: " << viewportSize;
    qDebug() << "panel size:" << panelSize;
    qDebug() << "panel X:" << panelX << "Y:" << panelY;
#endif

    return secondaryPos( QPointF( panelX, panelY ) );
}


QPointF BorderPanel::inactivePos()
{
    // qDebug() << __PRETTY_FUNCTION__;

    QSizeF viewportSize = m_photoView->size();
    QSizeF panelSize    = size();
    qreal  panelX = 0.0;
    qreal  panelY = 0.0;

    if ( m_borderFlags & LeftBorder )
        panelX = -panelSize.width() - 1;

    if ( m_borderFlags & RightBorder )
        panelX = viewportSize.width() + 1;

    if ( m_borderFlags & TopBorder )
        panelY = -panelSize.height() - 1;

    if ( m_borderFlags & BottomBorder )
        panelY = viewportSize.height() + 1;

#if 0
    qDebug() << "viewport size: " << viewportSize;
    qDebug() << "panel size:" << panelSize;
    qDebug() << "panel X:" << panelX << "Y:" << panelY;
#endif

    return secondaryPos( QPointF( panelX, panelY ) );
}


QPointF BorderPanel::secondaryPos( const QPointF( primaryPos ) )
{
    // qDebug() << __PRETTY_FUNCTION__;

    qreal  panelX = primaryPos.x();
    qreal  panelY = primaryPos.y();
    QSizeF viewportSize = m_photoView->size();
    QSizeF panelSize    = size();

    QRectF parentRect = m_sensitiveBorder ?
        m_sensitiveBorder->rect() :
        QRectF( QPointF( 0.0, 0.0 ), viewportSize );

    if ( ( m_borderFlags & ( LeftBorder | RightBorder ) ) == 0 )
    {
        // Horizontal alignment

        if ( m_alignment & Qt::AlignHCenter )
        {
            panelX = ( parentRect.width() - panelSize.width() ) / 2;
            panelX += parentRect.x();
        }

        if ( m_alignment & Qt::AlignLeft )
            panelX = parentRect.x();

        if ( m_alignment & Qt::AlignRight )
            panelX = parentRect.width() - panelSize.width();
    }

    if ( ( m_borderFlags & ( TopBorder | BottomBorder ) ) == 0 )
    {
        // Vertical alignment

        if ( m_alignment & Qt::AlignVCenter )
        {
            panelY = ( parentRect.height() - panelSize.height() ) / 2;
            panelY += parentRect.y();
        }

        if ( m_alignment & Qt::AlignTop )
            panelY = parentRect.y();

        if ( m_alignment & Qt::AlignBottom )
            panelY = parentRect.height() - panelSize.height();
    }

#if 0
    qDebug() << "parent rect: " << parentRect;
    qDebug() << "panel X:" << panelX << "Y:" << panelY;
    qDebug() << "\n";
#endif

    return QPointF( panelX, panelY );
}


QSizeF BorderPanel::size() const
{
    return m_size;
}


void BorderPanel::hoverEnterEvent( QGraphicsSceneHoverEvent * event )
{
    Q_UNUSED( event) ;
    // qDebug() << __PRETTY_FUNCTION__ << objectName();

    if ( m_leaveTimer.isActive() )
        m_leaveTimer.stop();
}


void BorderPanel::hoverLeaveEvent( QGraphicsSceneHoverEvent * event )
{
    Q_UNUSED( event) ;
    // qDebug() << __PRETTY_FUNCTION__ << objectName();

    m_leaveTimer.start( EnterLeaveTimeout );
}


#if 0
void BorderPanel::mousePressEvent  ( QGraphicsSceneMouseEvent * event )
{

}


void BorderPanel::mouseReleaseEvent( QGraphicsSceneMouseEvent * event )
{

}
#endif


void BorderPanel::mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event )
{
    Q_UNUSED( event );

    if ( m_sticky )
    {
        m_sticky = false;
        disappearAnimated();
    }
    else
    {
        m_sticky = true;
    }
}
