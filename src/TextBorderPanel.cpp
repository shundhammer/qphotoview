/*
 * QPhotoView border panel to display text.
 *
 * License: GPL V2. See file COPYING for details.
 *
 * Author:  Stefan Hundhammer <Stefan.Hundhammer@gmx.de>
 */

#include <QFontMetrics>
#include <QPainter>
#include <QDebug>

#include "TextBorderPanel.h"


TextBorderPanel::TextBorderPanel( PhotoView *	    parent,
				  SensitiveBorder * border )
    : BorderPanel( parent, border )
{
    _textColor	   = QColor( Qt::white );
    _textAlignment = Qt::AlignLeft | Qt::AlignVCenter;
    // _font is automatically initialized to QApplication::font()
}


TextBorderPanel::~TextBorderPanel()
{

}


void TextBorderPanel::setText( const QString & text )
{
    _text = text;
    recalcTextSize();
    update();
}


void TextBorderPanel::setFont( const QFont & font )
{
    _font = font;
    recalcTextSize();
    update();
}


void TextBorderPanel::recalcTextSize()
{
    QFontMetrics fontMetrics( _font );

    // Any other alignment than left and top can return negative coordinates
    // which only lead to confusion. Let's keep it simple. For the bounding
    // rect, the alignment doesn't really matter anyway.
    QRectF textRect = fontMetrics.boundingRect( QRect( 0, 0, 1, 1 ),
						Qt::AlignLeft | Qt::AlignTop,
						_text );
    // qDebug() << "Text rect: " << textRect;
    _textSize = textRect.size();
}


QSizeF TextBorderPanel::size() const
{
    return _textSize + QSizeF( 2*margin(), 2*margin() );
}


void TextBorderPanel::paint( QPainter * painter,
			     const QStyleOptionGraphicsItem * option,
			     QWidget * widget )
{
    BorderPanel::paint( painter, option, widget );

    QPointF textPos( margin(), margin() );
    painter->setPen( _textColor );
    painter->drawText( QRectF( textPos, _textSize ),
		       _textAlignment,
		       _text );
}

