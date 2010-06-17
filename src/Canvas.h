/**
 * QPhotoView canvas graphics item for viewer widget.
 *
 * License: GPL V2. See file COPYING for details.
 *
 * Author:  Stefan Hundhammer <Stefan.Hundhammer@gmx.de>
 */

#ifndef Canvas_h
#define Canvas_h

#include <QGraphicsPixmapItem>

class PhotoView;

/**
 * Canvas for PhotoView: A graphics item that shows the photo.
 */
class Canvas: public QGraphicsPixmapItem
{
public:

    /**
     * Constructor.
     */
    Canvas( PhotoView * parent );

    /**
     * Destructor.
     */
    virtual ~Canvas();

    /**
     * Return the current size.
     */
    QSize size() const;

    /**
     * Clear the current pixmap.
     */
    void clear();

    /**
     * Center inside the viewport of the PhotoView parent if this canvas is
     * smaller than the viewport.
     */
    void center( const QSize & parentSize );

    /**
     * Return the parent photo view.
     */
    PhotoView * photoView() const { return m_photoView; }

private:
    PhotoView * m_photoView;
};

#endif // Canvas_h
