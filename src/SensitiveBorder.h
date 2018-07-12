/**
 * QPhotoView sensitive border graphics item
 *
 * License: GPL V2. See file COPYING for details.
 *
 * Author:  Stefan Hundhammer <Stefan.Hundhammer@gmx.de>
 */

#ifndef SensitiveBorder_h
#define SensitiveBorder_h

#include <QGraphicsRectItem>
#include <QObject>
#include <QTimer>

class PhotoView;
class QGraphicsSceneHoverEvent;
class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;

/**
 * Sensitive border for PhotoView: A (normally invisible) graphics item that
 * detects the mouse pointer hovering over it and sends signals so tool windows
 * or menus can be displayed.
 *
 * The general idea behind this is that the screen should be as uncluttered as
 * possible while photos are viewed. Tool bars etc. should only be displayed as
 * needed, for example when the user moves the mouse to the left, right, top or
 * bottom border of the window. This class helps to detect this.
 */
class SensitiveBorder: public QObject, public QGraphicsRectItem
{
    Q_OBJECT

public:

    /**
     * Constructor.
     */
    SensitiveBorder( PhotoView * parent );

    /**
     * Destructor.
     */
    virtual ~SensitiveBorder();

    /**
     * Return the PhotoView parent.
     */
    PhotoView * photoView() const { return _photoView; }

    /**
     * Set debug mode on or off so the border can be made visible.
     */
    static void setDebugMode( bool on = true );

    /**
     * Reimplemented from QGraphicsRectItem: Paint this item. Unless debug mode
     * is set, nothing is painted, so this object will remain invisible.
     */
    virtual void paint( QPainter * painter,
			const QStyleOptionGraphicsItem * option,
			QWidget * widget = 0 ) Q_DECL_OVERRIDE;
signals:

    /**
     * Emitted when the mouse pointer entered this border.
     */
    void borderEntered();

    /**
     * Emitted when the mouse pointer left this border.
     */
    void borderLeft();

protected:

    // Event handlers, all reimiplemented from QGraphicsItem

    void hoverEnterEvent( QGraphicsSceneHoverEvent * event );
    void hoverLeaveEvent( QGraphicsSceneHoverEvent * event );

private:

    PhotoView * _photoView;
    QTimer	_enterTimer;
    QTimer	_leaveTimer;
    static bool _debugMode;
};


#endif // SensitiveBorder_h
