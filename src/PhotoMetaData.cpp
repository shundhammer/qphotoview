/*
 * QPhotoView core classes
 *
 * License: GPL V2. See file COPYING for details.
 *
 * Author:  Stefan Hundhammer <Stefan.Hundhammer@gmx.de>
 */

#include <exiv2/image.hpp>
#include <exiv2/exif.hpp>
#include <QDebug>

#include "PhotoMetaData.h"
#include "Photo.h"


PhotoMetaData::PhotoMetaData( Photo * photo )
{
    m_isEmpty = true;

    if ( photo )
    {
        m_photoFullPath = photo->fullPath();
        m_size = photo->size();
        readExifData( m_photoFullPath );
    }
}


void PhotoMetaData::readExifData( const QString & fileName )
{
    try
    {
        Exiv2::Image::AutoPtr image =
            Exiv2::ImageFactory::open( m_photoFullPath.toStdString() );

        image->readMetadata();
        Exiv2::ExifData &exifData = image->exifData();

        if ( exifData.empty() )
            return;

        m_isEmpty = false;

        m_exposureTime = exifFract( exifData, "Exif.Photo.ExposureTime" );
        m_aperture     = exifFract( exifData, "Exif.Photo.FNumber"      );
        m_iso          = exifInt  ( exifData, "Exif.Photo.ISOSpeedRatings" );
        m_focalLength  = exifFract( exifData, "Exif.Photo.FocalLength" ).toDouble();
        m_focalLength35mmEquiv = exifInt( exifData, "Exif.Photo.FocalLengthIn35mmFilm" );

        int origWidth  = exifInt( exifData, "Exif.Photo.PixelXDimension" );
        int origHeight = exifInt( exifData, "Exif.Photo.PixelYDimension" );
        m_origSize = QSize( origWidth, origHeight );

        QString dateTimeStr = exifString( exifData, "Exif.Image.DateTimeOriginal" );
        m_dateTimeTaken = QDateTime::fromString( dateTimeStr, Qt::ISODate );
    }
    catch ( Exiv2::Error& exception )
    {
        qWarning() << "Caught Exiv2 exception:" << exception.what()
                   << "for" << fileName;
    }
}


Fraction PhotoMetaData::exifFract( Exiv2::ExifData &  exifData,
                                   const char *       exifKey )
{
    Exiv2::ExifData::const_iterator it =
        exifData.findKey( Exiv2::ExifKey( exifKey ) );

    Fraction val;

    if ( it != exifData.end() )
    {
        Exiv2::Rational rational = it->value().toRational();
        val = Fraction( rational.first, rational.second );
    }

    return val;
}


int PhotoMetaData::exifInt( Exiv2::ExifData &  exifData,
                            const char *       exifKey )
{
    Exiv2::ExifData::const_iterator it =
        exifData.findKey( Exiv2::ExifKey( exifKey ) );

    int val = 0;

    if ( it != exifData.end() )
    {
        val = (int) it->value().toLong();
    }

    return val;
}


QString PhotoMetaData::exifString( Exiv2::ExifData &  exifData,
                                   const char *       exifKey )
{
    Exiv2::ExifData::const_iterator it =
        exifData.findKey( Exiv2::ExifKey( exifKey ) );

    QString val;

    if ( it != exifData.end() )
    {
        val = (int) it->value().toString().c_str();
    }

    return val;
}


