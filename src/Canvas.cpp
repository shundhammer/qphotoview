/**
 * QPhotoView canvas graphics item for viewer widget.
 *
 * License: GPL V2. See file COPYING for details.
 *
 * Author:  Stefan Hundhammer <Stefan.Hundhammer@gmx.de>
 */


#include "Canvas.h"
#include "PhotoView.h"

Canvas::Canvas( PhotoView * parent )
    : QGraphicsPixmapItem( QPixmap() )
    , m_photoView( parent )
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
