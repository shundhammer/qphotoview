# qmake .pro file for qphotoview/src
#
# Go to the project toplevel dir and build all Makefiles:
#
#     qmake
#
# Then build with
#
#     make
#

TEMPLATE         = app

QT		+= widgets
QT		+= widgets
CONFIG		+= debug
DEPENDPATH	+= .
MOC_DIR		 = .moc
OBJECTS_DIR	 = .obj

unix {
    # LIB_EXIV2_PREFIX = /usr
    LIB_EXIV2_PREFIX = /usr/local

    LIBS    += -L $${LIB_EXIV2_PREFIX}/lib -lexiv2
    INCLUDE += $${LIB_EXIV2_PREFIX}/include
}

TARGET	         = qphotoview
TARGET.files	 = qphotoview
TARGET.path	 = /usr/bin
INSTALLS	+= TARGET
# INSTALLS	+= TARGET desktop icons


SOURCES =			\
    main.cpp			\
    Exception.cpp               \
    Logger.cpp                  \
    PhotoView.cpp		\
    PhotoDir.cpp		\
    Photo.cpp			\
    PhotoMetaData.cpp		\
    PrefetchCache.cpp		\
    Canvas.cpp			\
    Panner.cpp			\
    Fraction.cpp		\
    SensitiveBorder.cpp		\
    BorderPanel.cpp		\
    TextBorderPanel.cpp		\
    ExifBorderPanel.cpp		\
    GraphicsItemPosAnimation.cpp


HEADERS =			\
    Exception.h                 \
    Logger.h                    \
    PhotoView.h			\
    PhotoDir.h			\
    Photo.h			\
    PhotoMetaData.h		\
    PrefetchCache.h		\
    Canvas.h			\
    Panner.h			\
    Fraction.h			\
    SensitiveBorder.h		\
    BorderPanel.h		\
    TextBorderPanel.h		\
    ExifBorderPanel.h		\
    GraphicsItemPosAnimation.h


# desktop.files = *.desktop
# desktop.path  = /usr/share/applications
#
# icons.files   = icons/qphotoview.png
# icons.path    = /usr/share/icons/hicolor/32x32/apps
