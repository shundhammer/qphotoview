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

class QGraphicsPixmapItem;
class QResizeEvent;


/**
 * Photo view widget: Display an image and optional meta data (EXIF data).
 */
class PhotoView: public QGraphicsView
{
    Q_OBJECT

public:
    /**
     * Constructor. Takes an optional initial image file name. If this is
     * empty, an empty widget will be displayed.
     */
    PhotoView( const QString & imageFileName = QString() );

    /**
     * Destructor.
     */
    virtual ~PhotoView();

    /**
     * Load the specified image. Returns 'true' on success, 'false' on failure.
     */
    bool loadImage( const QString & imageFileName );

    /**
     * Clear the currently displayed image.
     */
    void clear();

    /**
     * Return the current image file name.
     **/
    QString imageFileName() const { return m_imageFileName; }

protected:

    /**
     * Reimplemented from QGraphicsView/QWidget
     */
    virtual void resizeEvent ( QResizeEvent * event );
    
private:

    QString                     m_imageFileName;
    QGraphicsPixmapItem *       m_canvas;
    QPixmap                     m_origPixmap;
};


#endif // PhotoView_h
