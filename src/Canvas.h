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
class QGraphicsSceneMouseEvent;

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
     * Return 'true' if panning is in progress, i.e. if the user currently
     * drags (scrolls) the image.
     */
    bool panning() const { return m_panning; }

    /**
     * Return the parent photo view.
     */
    PhotoView * photoView() const { return m_photoView; }

protected:

    //
    // Event handlers, all reimplemented from QGraphicsItem
    //

    /**
     * Pan (scroll) the image with left mouse button pressed
     */
    virtual void mouseMoveEvent   ( QGraphicsSceneMouseEvent * event );

    /**
     * Prepare panning
     */
    virtual void mousePressEvent  ( QGraphicsSceneMouseEvent * event );

    /**
     * Clan up after panning
     */
    virtual void mouseReleaseEvent( QGraphicsSceneMouseEvent * event );

    /**
     * Zoom in (double click left) or out (double click right)
     **/
    virtual void mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event );

private:

    PhotoView * m_photoView;
    bool        m_panning;
};

#endif // Canvas_h
