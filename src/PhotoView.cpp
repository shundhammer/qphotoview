/*
 * QPhotoView viewer widget.
 *
 * License: GPL V2. See file COPYING for details.
 *
 * Author:  Stefan Hundhammer <Stefan.Hundhammer@gmx.de>
 */

#include <QDebug>
#include "PhotoView.h"
#include "Fraction.h"


PhotoView::PhotoView()
{
    setScene( new QGraphicsScene );
    QString text;
    Fraction exposure( 1, 320 );
    Fraction aperture( 50, 10 );
    text += exposure.toString();

    if ( exposure.isInt() )
        text += " sec";
    
    text += "\n";
    text += "f/" + aperture.toString();
    
    scene()->addText( text );
    // scene()->addText( "Hello, World!" );
}


PhotoView::~PhotoView()
{
    delete scene();
}
