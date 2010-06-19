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
#include <QSizeF>

class QGraphicsPixmapItem;
class QResizeEvent;
class QKeyEvent;
class PhotoDir;
class Photo;
class Canvas;
class Panner;
class SensitiveBorder;
class BorderPanel;
class TextBorderPanel;


/**
 * Photo view widget: Display an image and optional meta data (EXIF data).
 */
class PhotoView: public QGraphicsView
{
    Q_OBJECT

public:

    enum ZoomMode
    {
	NoZoom = 0,	// 1:1 (1 original pixel is one screen pixel)
	ZoomFitImage,	// Fit complete image into window to avoid all scrolling
	ZoomFitWidth,	// Fit width  into window, scroll only up and down
	ZoomFitHeight,	// Fit height into window, scroll only left and right
	ZoomFitBest,	// Fit width or height, whichever is better
	UseZoomFactor	// Use arbitrary zoom factor
    };


    /**
     * Constructor.
     */
    PhotoView( PhotoDir * photoDir );

    /**
     * Destructor.
     */
    virtual ~PhotoView();


public slots:

    /**
     * Load the current photo of the PhotoDir.
     * Returns 'true' on success, 'false' on failure.
     */
    bool loadImage();

    /**
     * Clear the currently displayed image.
     */
    void clear();

    /**
     * Set the zoom mode.
     */
    void setZoomMode( ZoomMode mode );

    /**
     * Set the zoom factor. This automatically sets the zoom mode:
     * 'NoZoom' for 1.0, 'UseZoomFactor' for everything else.
     */
    void setZoomFactor( qreal factor );

    /**
     * Zoom in using the default zoom increment.
     */
    void zoomIn();

    /**
     * Zoom out using the default zoom increment.
     */
    void zoomOut();

    /**
     * Set the default zoom increment.
     */
    void setZoomIncrement( qreal increment ) { m_zoomIncrement = increment; }

    /**
     * Update the panner based on the specified viewport size.
     * If not specified, the current size of the PhotoView widget is used.
     */
    void updatePanner( const QSizeF & viewportSize = QSizeF() );

public:

    /**
     * Return the current zoom mode.
     */
    ZoomMode zoomMode() const { return m_zoomMode; }

    /**
     * Return the current zoom factor.
     */
    qreal zoomFactor() const { return m_zoomFactor; }

    /**
     * Return the default zoom increment.
     */
    qreal zoomIncrement() const { return m_zoomIncrement; }

    /**
     * Return the current photo directory.
     **/
    PhotoDir * photoDir() const { return m_photoDir; }

    /**
     * Return the internal canvas graphics item that displays the image.
     */
    Canvas * canvas() const { return m_canvas; }

    /**
     * Return the internal panner graphics item that displays the scroll status.
     */
    Panner * panner() const { return m_panner; }


protected slots:

    /**
     * Show the border corresponding to the sender() of a signal connected to
     * this slot.
     */
    void showBorder();

    /**
     * Hide the border corresponding to the sender() of a signal connected to
     * this slot.
     */
    void hideBorder();

protected:

    /**
     * Reload the current photo in the specified size.
     * Return 'true' on success, 'false' on error.
     */
    bool reloadCurrent( const QSize & size );

    /**
     * Reimplemented from QGraphicsView/QWidget
     */
    virtual void resizeEvent ( QResizeEvent * event );

    /**
     * Reimplemented from QGraphicsView
     */
    virtual void keyPressEvent( QKeyEvent * event );

    /**
     * Create sensitive borders.
     */
    void createBorders();

    /**
     * Create one border with object name 'objName'.
     */
    SensitiveBorder * createBorder( const QString & objName );

    /**
     * Layout sensitive borders with PhotoView size 'size'.
     */
    void layoutBorders( const QSizeF & size );

    /**
     * Create tool panels.
     */
    void createPanels();


private:

    PhotoDir *	m_photoDir;
    Canvas   *	m_canvas;
    Panner   *	m_panner;
    Photo    *	m_lastPhoto;
    ZoomMode	m_zoomMode;
    qreal	m_zoomFactor;
    qreal	m_zoomIncrement;

    SensitiveBorder *   m_topLeftCorner;
    SensitiveBorder *   m_topBorder;
    SensitiveBorder *   m_topRightCorner;
    SensitiveBorder *   m_rightBorder;
    SensitiveBorder *   m_bottomRightCorner;
    SensitiveBorder *   m_bottomBorder;
    SensitiveBorder *   m_bottomLeftCorner;
    SensitiveBorder *   m_leftBorder;

    TextBorderPanel *   m_titlePanel;           // top right
    BorderPanel *       m_exifPanel;            // right
    BorderPanel *       m_navigationPanel;      // bottom
    BorderPanel *       m_toolPanel;            // left
};


#endif // PhotoView_h
