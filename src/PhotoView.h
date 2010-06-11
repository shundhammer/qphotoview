/*
 * QPhotoView viewer widget.
 *
 * License: GPL V2. See file COPYING for details.
 *
 * Author:  Stefan Hundhammer <Stefan.Hundhammer@gmx.de>
 */

#ifndef PhotoView_h
#define PhotoView_h

#include <QGraphicsView>


class PhotoView: public QGraphicsView
{
    Q_OBJECT
    
public:
    PhotoView();
    virtual ~PhotoView();
};


#endif // PhotoView_h
