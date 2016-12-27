# Toplevel qmake .pro file. Create a Makefile from this with
#
#     qmake
#
# Then build the program with
#
#     make

# !!! make sure to build 3rdparty/exiv2* first !!!

# TO DO: Integrate 3rdparty/exiv2 build

TEMPLATE = subdirs
CONFIG  += ordered
SUBDIRS  = src
