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
    , _graphicsItem( item )
{
    Q_CHECK_PTR( item );

    setEasingCurve( QEasingCurve::OutElastic );
}


void GraphicsItemPosAnimation::updateCurrentValue( const QVariant & value )
{
    QPointF pos = value.toPointF();
    _graphicsItem->setPos( pos );
}

