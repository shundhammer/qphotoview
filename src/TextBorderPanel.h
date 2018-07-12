/*
 * QPhotoView border panel to display text.
 *
 * License: GPL V2. See file COPYING for details.
 *
 * Author:  Stefan Hundhammer <Stefan.Hundhammer@gmx.de>
 */

#ifndef TextBorderPanel_h
#define TextBorderPanel_h

#include <QFont>
#include "BorderPanel.h"


/**
 * Border panel that displays text.
 */
class TextBorderPanel: public BorderPanel
{
    Q_OBJECT

public:
    /**
     * Constructor. Create a TextBorderPanel as child of the specified parent.
     *
     * If 'border' is specified, its dimensions are used for alignments (rather
     * than the PhotoView viewport dimensions), and its borderEntered() and
     * borderLeft() signals are automatically connected to this BorderPanel's
     * appearAnimated() and disappearAnimated() slots, respectively.
     */
    TextBorderPanel( PhotoView * parent, SensitiveBorder * border = 0 );

    /**
     * Destructor.
     */
    virtual ~TextBorderPanel();

    /**
     * Set the text. This can contain multiple lines.
     */
    void setText( const QString & text );

    /**
     * Return the text.
     */
    QString text() const { return _text; }

    /**
     * Set the font. If not explicitly set, QApplication::font() is used.
     */
    void setFont( const QFont & font );

    /**
     * Return the font.
     */
    QFont font() const { return _font; }

    /**
     * Set the text color.
     */
    void setTextColor( const QColor & color ) { _textColor = color; }

    /**
     * Return the text color.
     */
    QColor textColor() const { return _textColor; }

    /**
     * Set the text alignment.
     */
    void setTextAlignment( Qt::Alignment align )  { _textAlignment = align; }

    /**
     * Return the text alignment.
     */
    Qt::Alignment textAlignment() const { return _textAlignment; }

    /**
     * Reimplemented from BorderPanel: Return the current size.
     */
    virtual QSizeF size() const Q_DECL_OVERRIDE;

    /**
     * Reimplemented from BorderPanel: Paint this item.
     */
    virtual void paint( QPainter * painter,
			const QStyleOptionGraphicsItem * option,
			QWidget * widget = 0 ) Q_DECL_OVERRIDE;

private:

    /**
     * Recalculate the cached text size.
     */
    void recalcTextSize();


    // Data members

    QString		_text;
    QFont		_font;
    QColor		_textColor;
    Qt::Alignment	_textAlignment;
    QSizeF		_textSize;
};


#endif // TextBorderPanel_h
