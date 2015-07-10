osmpng
======

OSM Title downloader and merger tool


Compile
=======

osmpng depends on libpng++ and libcurl. You will need both libraries to complete the compile process.
To compile just run

make && sudo make install

Usage
=====

Type osmpng --help if you need help.

To download for instance the map of Innsbruck: osmpng -o ibk.png 11.3425-11.4614 47.2761-47.2484 14

Screenshot
==========

![alt tag](http://www.feldspaten.org/wp-content/uploads/2014/04/osmpng.png)

Running
=======

osmpng -version
