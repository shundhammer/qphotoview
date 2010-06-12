/*
 * QPhotoView main program.
 *
 * License: GPL V2. See file COPYING for details.
 *
 * Author:  Stefan Hundhammer <Stefan.Hundhammer@gmx.de>
 */

#include <QApplication>
#include <QDebug>
#include "PhotoView.h"

int main ( int argc, char *argv[] )
{
    QApplication app( argc, argv );

    if ( argc != 2 )
    {
        qCritical() << "\nUsage:" << argv[0] << "<image-file-name>\n";
        return 1;
    }
    
    PhotoView viewer( argv[1] );
    
    viewer.show();
    app.exec();
    
    return 0;
}
