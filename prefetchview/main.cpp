#include <QtGui>
#include <QDebug>
#include "SimpleView.h"

int main( int argc, char * argv[] )
{
    QApplication app( argc, argv );
    QDir dir( QString( argc > 1 ? argv[1] : "."  ) );
    QStringList photoList = dir.entryList( QStringList( "*.jpg" ), QDir::Files, QDir::Name );
    SimpleView view( dir.absolutePath(), photoList );
    view.show();

    app.exec();
}
