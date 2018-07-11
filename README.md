# QPhotoView

A Qt-based photo viewer that concentrates on what photographers need.

(c) 2015-2018 Stefan Hundhammer <Stefan.Hundhammer@gmx.de>

Target Platforms: Linux, BSD, Unix-like systems

License: GPL V2

Updated:  2018-07-11


## What is it?

QPhotoView is an image viewer for photographers. It first and foremost meant as
a tool to be used by photographers -- for viewing photos, for comparing
different shots of the same scene, for sorting out which shots to keep and
which ones to throw away, for opening them in some dedicated image processing
software such as Gimp or PhotoShop, and of course for presenting them to
people.

It is explicitly not meant as a one-size-fits all do-everything viewer, much
less as an image manipulation tool. There are other tools that try that, and
each of them fails misearably at it in one way or the other (see below: Why yet
another image viewer?).


## Screenshot

[<img src="https://github.com/shundhammer/qphotoview/blob/master/screenshots/QPhotoView-main-win.png" width="949">](https://raw.githubusercontent.com/shundhammer/qphotoview/master/screenshots/QPhotoView-main-win.png)

_Main window screenshot_


## Design documentation

doc/design/qphotoview-design.odp

(OpenOffice presentation)


## How to build

### Build libexiv2

    cd 3rdparty/exiv2*
    cmake .
    make
    sudo make install
    cd ../..

### Build QPhotoView itself

    qmake
    make
    sudo make install

You neeed a working C++ compiler environment and Qt 5.x. On most Linux
distributions, you will have to install the -devel (openSUSE) or -dev
(Debian/Ubuntu) packages for Qt which include the header files and development
tools like "qmake" and "moc".

Please make sure you can compile and run simple "Hello, world" type Qt programs
before contacting us because of build problems.



## Why yet another image viewer?

_This might be a bit outdated, but still mostly true_

Short version: Because the other ones (at least the ones that are available on
Linux / X11) all suck. They all suck in different ways, but they definitely all
suck.

The only one that sucked a lot less (kuickshow) than all the others went out of
active development a long time ago, and it will go out of maintenance in the
near future.


KDE's Gwenview is what comes closest, but it sucks in very subtle ways. While
recent versions (as of mid-2010) at least display images quick enough to enable
comparing them, it still leaves much to be desired. Its metadata display is,
just like ALL other viewers, just horrible. I really don't give a shit about
the JPEG compression algorithm used or other useless crap the EXIF data
contain. But I do need the exposure time, the aperture, ISO speed, focal
length, and maybe what lens was used. Is there any way to display this in
Gwenview (and not all that other stuff)? I don't think so. If it's there, it is
very well hidden. Also, deleting the current image is an operation that is (at
least in the default configuration) unreasonably difficult. Hell, when I
just imported images from my camera and I see one that is out of focus, I want
to get rid of it quickly -- hit [Delete] or **D** (or whatever) and good-bye.


KDE's digiKam is an attempt of a one-tool-does-everything program. It insists
on managing all images in its (MySQL or whatever) data base which is not what I
want: I have FILES. I want to be able to rsync them to my USB hard drive, to my
home NAS, to some other machine. I do not want to depend on somebody else's
idea of what an image data base should look like. I also don't want its
gazillion of image processing plug-ins for red eye correction and whatever.
Maybe they are what some people want, but I want to stick with my favourite
image processing software (Gimp for want of something better on the Linux / X11
platform). It is overloaded with features I don't want, but it lacks most of
the features I do want -- like rapidly displaying all files of a given
directory in sequence, deleting individual ones, or displaying the meta data
that are relevant (!) for photography.


GNOME's F-Spot is a Mono-driven program. Too bad it wants to taint my machine
with something as evil as Mono. Maybe F-Spot is great, but I will never know it
since it wants me to install that Mono resource hog which is not going to
happen. Anyway, it's GNOME which means Gtk look and feel which reminds me of
the early 1990s. Plus, I don't like the GNOME people's ideas of usability.


'xv' was great at its days. But those days (the early 1990s) are long
past. Today, it's just a crude leftover of days gone by. Plus, what were they
thinking with the way it keeps distorting image proportions? Hell, the only
thing NOBODY ever wants in an image viewer is changing the width/height aspect
ratio. This was insane even back in the 1990s, but even more insane is the fact
that this was kept like this during all that time.



## Contributing

See file [Contributing.md](https://github.com/shundhammer/qdirstat/blob/master/doc/Contributing.md)
and [GitHub-Workflow.md](https://github.com/shundhammer/qdirstat/blob/master/doc/GitHub-Workflow.md)


Author
======

Stefan Hundhammer <Stefan.Hundhammer@gmx.de>
