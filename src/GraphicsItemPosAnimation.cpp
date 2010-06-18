/*
 * QPhotoView animation
 *
 * License: GPL V2. See file COPYING for details.
 *
 * Author:  Stefan Hundhammer <Stefan.Hundhammer@gmx.de>
 */

#include <QGraphicsItem>
#include <QEasingCurve>
#include <QDebug>

#include "GraphicsItemPosAnimation.h"


GraphicsItemPosAnimation::GraphicsItemPosAnimation( QGraphicsItem * item,
                                                    QObject * parent )
    : QVariantAnimation( parent )
    , m_graphicsItem( item )
{
    Q_CHECK_PTR( item );

    setEasingCurve( QEasingCurve::OutElastic );
}


void GraphicsItemPosAnimation::updateCurrentValue( const QVariant & value )
{
    QPointF pos = value.toPointF();
    // qDebug() << "Setting pos " << pos;
    m_graphicsItem->setPos( pos );
}

