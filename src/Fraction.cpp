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


double Fraction::toDouble() const
{
    if ( m_denominator == 0 )
        return 0.0;

    else
        return m_numerator / (double) m_denominator;
}



int Fraction::greatestCommonDivisor( int a, int b )
{
    // Euklid's algorithm, stolen from Wikipedia
    // http://en.wikipedia.org/wiki/Euclidean_algorithm

    if ( a == 0 )
        return b;

    while ( b != 0 )
    {
        if ( a > b )
            a -= b;
        else
            b -= a;
    }

    return a;
}


void Fraction::simplify()
{
    int gcd = greatestCommonDivisor( m_numerator, m_denominator );

    if ( gcd == 0 )
        return;

    m_numerator   /= gcd;
    m_denominator /= gcd;
}


Fraction Fraction::simplified() const
{
    int gcd = greatestCommonDivisor( m_numerator, m_denominator );

    if ( gcd == 0 )
        return Fraction();

    return Fraction( m_numerator   / gcd,
                     m_denominator / gcd );
}


bool Fraction::operator>( double num )
{
    if ( m_denominator == 0 )
        return false;

    return (m_numerator / (double) m_denominator) > num;
}


bool Fraction::operator<( double num )
{
    if ( m_denominator == 0 )
        return false;

    return (m_numerator / (double) m_denominator) < num;
}
