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
#include <QDateTime>

#include "Fraction.h"

class Photo;

namespace Exiv2
{
    class ExifData;
};


/**
 * Class representing meta data (EXIF/IPTC/XMP) for one photo.
 */
class PhotoMetaData
{
public:
    /**
     * Constructor. 'photo' is only used during the constructor; it is not a
     * problem if the pointer becomes invalid later during the lifetime of this
     * object.
     */
    PhotoMetaData( Photo * photo );

    // Gladly using the default C++ provided default bitwise copy constructor

    /**
     * Return 'true' if no meta data are available for this photo.
     * Always check this first before accessing any of the other fields.
     */
    bool isEmpty() const { return m_isEmpty; }

    /**
     * Return the exposure time (1/320 etc.).
     */
    Fraction exposureTime() const { return m_exposureTime; }

    /**
     * Return the aperture (the F-number).
     */
    Fraction aperture() const { return m_aperture; }

    /**
     * Return the ISO speed (the light sensitivity).
     */
    int iso() const { return m_iso; }

    /**
     * Return the true focal length. For most digital cameras (except full
     * frame cameras like the Nikon D700/D3 or the Canon 5D) this is not the
     * same as the 35 mm equivalent focal length.
     */
    int focalLength() const { return m_focalLength; }

    /**
     * Return the focal length in 35 mm (full frame) equivalent
     * or 0 if unknown.
     */
    int focalLength35mmEquiv() const { return m_focalLength35mmEquiv; }

    /**
     * Return the original image size as captured by the camera.
     */
    QSize origSize() const { return m_origSize; }

    /**
     * Return the current image size. If the photo was resized with image
     * processing software like Gimp or PhotoShop, this might be different from
     * origSize().
     */
    QSize size() const { return m_size; }

    /**
     * Return the date and time the photo was taken.
     */
    QDateTime dateTimeTaken() const { return m_dateTimeTaken; }

    /**
     * Return the full path name of the photo these meta data belong to.
     */
    QString photoFullPath() const { return m_photoFullPath; }

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


private:

    /**
     * Read the EXIF data from the specified file name.
     */
    void readExifData( const QString & fileName );

    /**
     * Get the EXIF value with key 'exifKey' return it as Fraction.
     */
    Fraction exifFract( Exiv2::ExifData &  exifData,
                        const char *       exifKey );

    /**
     * Get the EXIF value with key 'exifKey' return it as int.
     */
    int exifInt( Exiv2::ExifData &  exifData,
                 const char *       exifKey );

    /**
     * Get the EXIF value with key 'exifKey' return it as QString.
     */
    QString exifString( Exiv2::ExifData &  exifData,
                        const char *       exifKey );

    // Data members

    QString     m_photoFullPath;
    bool        m_isEmpty;
    Fraction    m_exposureTime;
    Fraction    m_aperture;
    int         m_iso;
    int         m_focalLength;
    int         m_focalLength35mmEquiv;
    QSize       m_origSize;
    QSize       m_size;
    QDateTime   m_dateTimeTaken;
};


#endif // PhotoMetaData_h
