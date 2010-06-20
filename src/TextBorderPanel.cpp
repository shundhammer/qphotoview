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


TextBorderPanel::TextBorderPanel( PhotoView *       parent,
                                  SensitiveBorder * border )
    : BorderPanel( parent, border )
{
    m_textColor     = QColor( Qt::white );
    m_textAlignment = Qt::AlignLeft | Qt::AlignVCenter;
    // m_font is automatically initialized to QApplication::font()
}


TextBorderPanel::~TextBorderPanel()
{

}


void TextBorderPanel::setText( const QString & text )
{
    m_text = text;
    recalcTextSize();
    update();
}


void TextBorderPanel::setFont( const QFont & font )
{
    m_font = font;
    recalcTextSize();
    update();
}


void TextBorderPanel::recalcTextSize()
{
    QFontMetrics fontMetrics( m_font );

    // Any other alignment than left and top can return negative coordinates
    // which only lead to confusion. Let's keep it simple. For the bounding
    // rect, the alignment doesn't really matter anyway.
    QRectF textRect = fontMetrics.boundingRect( QRect( 0, 0, 1, 1 ),
                                                Qt::AlignLeft | Qt::AlignTop,
                                                m_text );
    // qDebug() << "Text rect: " << textRect;
    m_textSize = textRect.size();
}


QSizeF TextBorderPanel::size() const
{
    return m_textSize + QSizeF( 2*margin(), 2*margin() );
}


void TextBorderPanel::paint( QPainter * painter,
                             const QStyleOptionGraphicsItem * option,
                             QWidget * widget )
{
    BorderPanel::paint( painter, option, widget );

    QPointF textPos( margin(), margin() );
    painter->setPen( m_textColor );
    painter->drawText( QRectF( textPos, m_textSize ),
                       m_textAlignment,
                       m_text );
}

