/*
 * QPhotoView core classes
 *
 * License: GPL V2. See file COPYING for details.
 *
 * Author:  Stefan Hundhammer <Stefan.Hundhammer@gmx.de>
 */

#ifndef Fractional_h
#define Fractional_h

#include <QString>

class Fraction
{
public:
    /**
     * Constructor: Create a fractional number numerator/denominator.
     */
    Fraction( int numerator = 0, int denominator = 1 )
	: _numerator( numerator )
	, _denominator( denominator )
	{}

    /**
     * Return the numerator of the fraction.
     */
    int numerator() const { return _numerator; }

    /**
     * Return the denominator of the fraction.
     */
    int denominator() const { return _denominator; }

    /**
     * Format the fraction as string "numerator/denominator".
     */
    QString toString() const;

    /**
     * Convert the fraction to double.
     */
    double toDouble() const;

    /**
     * Return 'true' if this is an integer (a whole number).
     */
    bool isInt() const;

    /**
     * Simplify this fraction in place: Reduce it with the greatest common
     * divisor.
     */
    void simplify();

    /**
     * Return this fraction simplified, but don't change the original.
     */
    Fraction simplified() const;


    /**
     * Return the greatest common divisor of two numbers.
     */
    static int greatestCommonDivisor( int a, int b );

    /**
     * Return 'true' if this fraction is greater than 'num'.
     */
    bool operator>( double num );

    /**
     * Return 'true' if this fraction is smaller than 'num'.
     */
    bool operator<( double num );

protected:

    int _numerator;
    int _denominator;
};


#endif // Fractional_h
