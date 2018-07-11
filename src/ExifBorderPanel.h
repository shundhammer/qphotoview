/*
 * QPhotoView border panel to photo meta data.
 *
 * License: GPL V2. See file COPYING for details.
 *
 * Author:  Stefan Hundhammer <Stefan.Hundhammer@gmx.de>
 */

#ifndef ExifBorderPanel_h
#define ExifBorderPanel_h

#include "TextBorderPanel.h"

class Photo;

/**
 * Border panel that shows EXIF data (photo meta data) for the current photo.
 */
class ExifBorderPanel: public TextBorderPanel
{
    Q_OBJECT

public:
    /**
     * Constructor. Create an ExifBorderPanel as child of the specified parent.
     */
    ExifBorderPanel( PhotoView * parent, SensitiveBorder * border = 0 );

    /**
     * Return the meta data for the specified photo formatted in a multi-line
     * string.
     */
    QString formatMetaData( Photo * photo );

public slots:

    /**
     * Set the meta data for the current photo.
     */
    void setMetaData();

private:

    Photo * _lastPhoto;
};


#endif // ExifBorderPanel_h

