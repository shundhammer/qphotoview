/*
 * QPhotoView animation
 *
 * License: GPL V2. See file COPYING for details.
 *
 * Author:  Stefan Hundhammer <Stefan.Hundhammer@gmx.de>
 */

#ifndef GraphicsItemPosAnimation_h
#define GraphicsItemPosAnimation_h

#include <QVariantAnimation>

class QGraphicsItem;


/**
 * Animation class that animates the position of a QGraphicsItem.
 */
class GraphicsItemPosAnimation: public QVariantAnimation
{
    Q_OBJECT

public:

    /**
     * Constructor. Create a position animation for 'item'.
     */
    GraphicsItemPosAnimation( QGraphicsItem * item, QObject * parent = 0 );

    /**
     * Return the graphics item.
     */
    QGraphicsItem * graphicsItem() const { return _graphicsItem; }

protected:

    /**
     * Implemented from QVariantAnimation: Set the current value (here: the
     * position) during the animation.
     */
    virtual void updateCurrentValue( const QVariant & value );

private:

    QGraphicsItem * _graphicsItem;
};


#endif // GraphicsItemPosAnimation_h
