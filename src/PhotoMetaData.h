/*
 * QPhotoView core classes
 *
 * License: GPL V2. See file COPYING for details.
 *
 * Author:  Stefan Hundhammer <Stefan.Hundhammer@gmx.de>
 */

#ifndef PhotoMetaData_h
#define PhotoMetaData_h

#include <QString>
#include <QSize>
#include "Rational.h"
#include "Fraction.h"


/**
 * Class representing meta data (EXIF/IPTC/XMP) for one photo.
 */
class PhotoMetaData
{
    /**
     * Constructor.
     */
    PhotoMetaData( Photo * photo );

    /**
     * Copy constructor.
     */
    PhotoMetaData( const PhotoMetaData & );

    /**
     * Return 'true' if no meta data are available for this photo.
     * Always check this first before accessing any of the other fields.
     */
    bool isEmpty() const;

    /**
     * Return the exposure time (1/320 etc.).
     */
    Fraction exposureTime() const;

    /**
     * Return the aperture (the F-number).
     */
    Fraction aperture() const;

    /**
     * Return the ISO speed (the light sensitivity).
     */
    int iso() const;

    /**
     * Return the true focal length. For most digital cameras (except full
     * frame cameras like the Nikon D700/D3 or the Canon 5D) this is not the
     * same as the 35 mm equivalent focal length.
     */
    int focalLength() const;

    /**
     * Return the focal length in 35 mm (full frame) equivalent
     * or 0 if unknown.
     */
    int focalLength35mmEquiv() const;

    /**
     * Return the original image size as captured by the camera.
     */
    QSize origSize() const;

    /**
     * Return the current image size. If the photo was resized with image
     * processing software like Gimp or PhotoShop, this might be different from
     * origSize().
     */
    QSize size() const;

    /**
     * Return the date and time the photo was taken.
     */
    QDateTime dateTimeTaken() const;

#if 0
    /**
     * Return the camera the photo was taken with.
     */
    Camera camera() const;

    /**
     * Return the lens the photo was taken with.
     */
    Lens lens() const;
#endif
};


#endif // PhotoMetaData_h
