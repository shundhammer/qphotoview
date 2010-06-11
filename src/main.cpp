/*
 * QPhotoView main program.
 *
 * License: GPL V2. See file COPYING for details.
 *
 * Author:  Stefan Hundhammer <Stefan.Hundhammer@gmx.de>
 */

#include <QApplication>
#include "PhotoView.h"

int main ( int argc, char *argv[] )
{
    QApplication app( argc, argv );
    PhotoView viewer;
    
    viewer.show();
    app.exec();
    
    return 0;
}
