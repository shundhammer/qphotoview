/**
 * QPhotoView panner graphics item for viewer widget.
 *
 * License: GPL V2. See file COPYING for details.
 *
 * Author:  Stefan Hundhammer <Stefan.Hundhammer@gmx.de>
 */

#ifndef Panner_h
#define Panner_h

#include <QGraphicsItem>
#include <QPixmap>
#include <QSize>
#include <QRect>

class QGraphicsPixmapItem;
class QGraphicsRectItem;
class PhotoView;

/**
 * Panner for PhotoView: A graphics item that shows a small preview of the
 * complete image and a frame on top of it that shows which part of the image
 * is currently being displayed.
 *
 * This is a substitute for scroll bars. Most digital cameras use this.
 */
class Panner: public QGraphicsItem
{
public:

    /**
     * Constructor. Create a panner that is at most 'pannerMaxSize' big.
     */
    Panner( const QSizeF & pannerMaxSize, PhotoView * parent );

    /**
     * Destructor.
     */
    virtual ~Panner();

    /**
     * Set the pixmap to display and adjust sizes.
     */
    void setPixmap( const QPixmap & pixmap );

    /**
     * Update the pan rect, i.e. the rectangle that shows which portion of the
     * image is being displayed.
     *
     * This also handles showing and hiding the panner: If the complete image
     * is shown, the panner is hidden. If only a portion of the image is shown,
     * the panner is shown.
     *
     * 'visibleRect' is the rectangle that is currently being displayed.
     * 'origSize' is the original full size of the image.
     */
    void updatePanRect( const QRectF & visibleRect, const QSizeF & origSize );

    /**
     * Reimplemented from QGraphicsItem: Paint the panner shape.
     */
    virtual void paint( QPainter * painter,
			const QStyleOptionGraphicsItem * option,
			QWidget * widget = 0 ) Q_DECL_OVERRIDE;

    /**
     * Reimplemented from QGraphicsItem: Return the bounding rect.
     */
    virtual QRectF boundingRect() const Q_DECL_OVERRIDE;

    /**
     * Return the pixel size of the panner.
     */
    QSizeF size() const { return _size; }

    /**
     * Return the parent PhotoView.
     */
    PhotoView * photoView() const { return _photoView; }

protected:

    /**
     * Use lazy initialization to actually use the pixmap set with setPixmap().
     */
    void lazyScalePixmap();

private:

    QGraphicsPixmapItem *	_pixmapItem;
    QGraphicsRectItem *		_panRect;
    QSizeF			_pannerMaxSize;
    QPixmap			_pixmap;
    qreal			_scale;
    QSizeF			_size;
    PhotoView *			_photoView;
};


#endif // Panner_h
