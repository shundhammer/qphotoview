/*
 * QPhotoView core classes
 *
 * License: GPL V2. See file COPYING for details.
 *
 * Author:  Stefan Hundhammer <Stefan.Hundhammer@gmx.de>
 */

#include "Fraction.h"


bool Fraction::isInt() const
{
    return ( m_denominator == 0 || m_numerator % m_denominator == 0 );
}


QString Fraction::toString() const
{
    if ( m_denominator == 0 )
        return "0";
    
    QString result;

    if ( isInt() )
    {
        result.setNum( m_numerator / m_denominator );
    }
    else
    {
        result = QString( "%1/%2" ).arg( m_numerator ).arg( m_denominator );
    }

    return result;
}

