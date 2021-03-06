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
    _isEmpty = true;

    if ( photo )
    {
	_photoFullPath = photo->fullPath();
	_size = photo->size();
	readExifData( _photoFullPath );
    }
}


void PhotoMetaData::readExifData( const QString & fileName )
{
    try
    {
	Exiv2::Image::AutoPtr image =
	    Exiv2::ImageFactory::open( _photoFullPath.toStdString() );

	image->readMetadata();
	Exiv2::ExifData &exifData = image->exifData();

	if ( exifData.empty() )
	    return;

	_isEmpty = false;

	_exposureTime = exifFract( exifData, "Exif.Photo.ExposureTime" );
	_aperture     = exifFract( exifData, "Exif.Photo.FNumber"      );
	_iso	      = exifInt	 ( exifData, "Exif.Photo.ISOSpeedRatings" );
	_focalLength  = exifFract( exifData, "Exif.Photo.FocalLength" ).toDouble();
	_focalLength35mmEquiv = exifInt( exifData, "Exif.Photo.FocalLengthIn35mmFilm" );

	int origWidth  = exifInt( exifData, "Exif.Photo.PixelXDimension" );
	int origHeight = exifInt( exifData, "Exif.Photo.PixelYDimension" );
	_origSize = QSize( origWidth, origHeight );

	QString dateTimeStr = exifString( exifData, "Exif.Photo.DateTimeOriginal" );
	_dateTimeTaken = QDateTime::fromString( dateTimeStr, Qt::ISODate );
    }
    catch ( Exiv2::Error& exception )
    {
	qWarning() << "Caught Exiv2 exception:" << exception.what()
		   << "for" << fileName;
    }
}


Fraction PhotoMetaData::exifFract( Exiv2::ExifData &  exifData,
				   const char *	      exifKey )
{
    Exiv2::ExifData::const_iterator it =
	exifData.findKey( Exiv2::ExifKey( exifKey ) );

    Fraction val;

    if ( it != exifData.end() )
    {
	Exiv2::Rational rational = it->value().toRational();
	val = Fraction( rational.first, rational.second );
	val.simplify();
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
				   const char *	      exifKey )
{
    Exiv2::ExifData::const_iterator it =
	exifData.findKey( Exiv2::ExifKey( exifKey ) );

    QString val;

    if ( it != exifData.end() )
    {
	val = it->value().toString().c_str();
    }

    return val;
}


