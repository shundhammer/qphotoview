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
#include <QAction>
#include <QTimer>
#include <QCursor>

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
class ExifBorderPanel;


/**
 * Photo view widget: Display an image and optional meta data (EXIF data).
 */
class PhotoView: public QGraphicsView
{
    Q_OBJECT

public:

    /**
     * Helper class to bundle all the interactive actions provided by this the
     * photo view widget so they can easier be put into menus etc.
     */
    class Actions
    {
    public:
        Actions( PhotoView * photoView );

        QAction * zoomIn;
        QAction * zoomOut;
        QAction * noZoom;
        QAction * zoomFitImage; // fit image into window
        QAction * zoomFitWidth;
        QAction * zoomFitHeight;
        QAction * zoomFitBest;  // width or height, whichever fits best
        QAction * forceReload;
        QAction * toggleFullscreen;
        QAction * quit;

    private:
        /**
         * Create a QAction with a text, an optional shortcut and optional
         * data.
         *
         * Ownership of the action is transferred to the caller, but since it's
         * created as a QObject child of this object's PhotoView, Qt takes care
         * of deleting it when the PhotoView is deleted.
         */
        QAction * createAction( const QString & text,
                                QKeySequence    shortcut = QKeySequence(),
                                QVariant        data     = QVariant() );

        /**
         * Add a shortcut to an action.
         */
        void addShortcut( QAction * action, QKeySequence newShortcut ) const;

        PhotoView * _photoView;
    };

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

    const Actions & actions() const { return _actions; }


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
     * Set the zoom mode that is set from the data() of a QAction if this is
     * where the signal was sent from.
     */
    void setZoomMode();

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
    void setZoomIncrement( qreal increment ) { _zoomIncrement = increment; }

    /**
     * Update the panner based on the specified viewport size.
     * If not specified, the current size of the PhotoView widget is used.
     */
    void updatePanner( const QSizeF & viewportSize = QSizeF() );

    /**
     * Hide the cursor. Called when the idle timer times out.
     */
    void hideCursor();

    /**
     * Show the cursor.
     */
    void showCursor();

    /**
     * Switch back and forth between fullscreen and windowed mode.
     */
    void toggleFullscreen();

    /**
     * Force a reload of the current image.
     */
    void forceReload();


public:

    /**
     * Return the current zoom mode.
     */
    ZoomMode zoomMode() const { return _zoomMode; }

    /**
     * Return the current zoom factor.
     */
    qreal zoomFactor() const { return _zoomFactor; }

    /**
     * Return the default zoom increment.
     */
    qreal zoomIncrement() const { return _zoomIncrement; }

    /**
     * Set the idle timeout in milliseconds: The time of inactivity (no mouse
     * movement) after which the mouse cursor is hidden. 0 disables this
     * feature. Any mouse movement makes the cursor visible again.
     */
    void setIdleTimeout( int millisec );

    /**
     * Return the idle timeout in milliseconds.
     */
    int idleTimeout() const { return _idleTimeout; }

    /**
     * Return the current photo directory.
     **/
    PhotoDir * photoDir() const { return _photoDir; }

    /**
     * Return the internal canvas graphics item that displays the image.
     */
    Canvas * canvas() const { return _canvas; }

    /**
     * Return the internal panner graphics item that displays the scroll status.
     */
    Panner * panner() const { return _panner; }


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
     * Reimplemented from QGraphicsView:
     * Handle key presses for this PhotoView.
     */
    virtual void keyPressEvent( QKeyEvent * event );

    /**
     * Reimplemented from QGraphicsView:
     * Reset idle timer.
     */
    virtual void mouseMoveEvent ( QMouseEvent * event );

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

    PhotoDir *	_photoDir;
    Canvas   *	_canvas;
    Panner   *	_panner;
    Photo    *	_lastPhoto;
    ZoomMode	_zoomMode;
    qreal	_zoomFactor;
    qreal	_zoomIncrement;
    QTimer	_idleTimer;
    int		_idleTimeout;
    QCursor	_cursor;
    Actions     _actions;

    SensitiveBorder *	_topLeftCorner;
    SensitiveBorder *	_topBorder;
    SensitiveBorder *	_topRightCorner;
    SensitiveBorder *	_rightBorder;
    SensitiveBorder *	_bottomRightCorner;
    SensitiveBorder *	_bottomBorder;
    SensitiveBorder *	_bottomLeftCorner;
    SensitiveBorder *	_leftBorder;

    TextBorderPanel *	_titlePanel;	       // top right
    ExifBorderPanel *	_exifPanel;	       // right
    BorderPanel *	_navigationPanel;      // bottom
    BorderPanel *	_toolPanel;	       // left
};


#endif // PhotoView_h
