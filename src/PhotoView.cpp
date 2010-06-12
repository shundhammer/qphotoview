/*
 * QPhotoView viewer widget.
 *
 * License: GPL V2. See file COPYING for details.
 *
 * Author:  Stefan Hundhammer <Stefan.Hundhammer@gmx.de>
 */

#include <QApplication>
#include <QGraphicsPixmapItem>
#include <QWindowsStyle>
#include <QResizeEvent>
#include <QDebug>

#include "PhotoView.h"
#include "Fraction.h"


PhotoView::PhotoView( const QString & imageFileName )
    : QGraphicsView()
    , m_imageFileName( imageFileName )
{
    setScene( new QGraphicsScene );
    m_canvas = scene()->addPixmap( QPixmap() );

    if ( ! imageFileName.isEmpty() )
        loadImage( imageFileName );

    //
    // Visual tweaks
    //

    QPalette pal = palette();
    pal.setColor( QPalette::Base, Qt::black );
    setPalette( pal );

    setVerticalScrollBarPolicy  ( Qt::ScrollBarAlwaysOff );
    setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

    // Some styles (e.g. Plastique) have an undesired two pixel wide focus rect
    // around QGraphicsView widgets. This is not what we want here, so let's
    // select a style that does not do this. This does not have an effect on
    // existing or future child widgets. And since scroll bars are turned off,
    // there is no other visual effect anyway.
    setStyle( new QWindowsStyle() );
}


PhotoView::~PhotoView()
{
    if ( style() != qApp->style() )
    {
        // Delete the style we explicitly created just for this widget
        delete style();
    }
    
    delete scene();
}


bool PhotoView::loadImage( const QString & imageFileName )
{
    m_imageFileName = imageFileName;
    bool success = m_origPixmap.load( m_imageFileName );

    if ( success )
    {
        QPixmap pixmap = m_origPixmap.scaled( size(), Qt::KeepAspectRatio,
                                              Qt::SmoothTransformation );
        m_canvas->setPixmap( pixmap );
        setWindowTitle( "QPhotoView  " + imageFileName );
    }
    else
    {
        qWarning() << "Error: Can't display image" << imageFileName;
        clear();
        setWindowTitle( "QPhotoView -- ERROR" );
    }

    return success;
}


void PhotoView::clear()
{
    m_imageFileName.clear();
    m_origPixmap = QPixmap();
    m_canvas->setPixmap( m_origPixmap );
    setWindowTitle( "QPhotoView" );
}


void PhotoView::resizeEvent ( QResizeEvent * event )
{
    if ( event->size() != event->oldSize() && ! m_origPixmap.isNull() )
    {
        QPixmap pixmap = m_origPixmap.scaled( size(), Qt::KeepAspectRatio,
                                            Qt::SmoothTransformation );
        m_canvas->setPixmap( pixmap );
    }

    QGraphicsView::resizeEvent( event );
}
