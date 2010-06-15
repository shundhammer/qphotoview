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
#include "PhotoDir.h"


int main ( int argc, char *argv[] )
{
    QApplication app( argc, argv );

    if ( argc > 2 )
    {
        qCritical() << "\nUsage:" << argv[0] << "<image-file-or-dir>\n";
        return 1;
    }

    QString path = ".";

    if ( argc > 1 )
        path = argv[1];

    PhotoDir dir( path );
    PhotoView viewer( &dir );

    viewer.show();
    app.exec();

    return 0;
}
