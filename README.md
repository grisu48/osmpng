# osmpng

OSM Title downloader and merger tool


## Build

osmpng depends on `libpng++` and `libcurl`. You will need both libraries to complete the compile process.

    make
    sudo make install

## Usage

Type `osmpng --help` for a helpful help message :-)

    Usage: osmpng [OPTIONS] [LONGITUDE LATITUDE ZOOM]
    
    OPTIONS
    	-help                    Print this help message
    	-version                 Print program version
    	--cache=CACHE
    	-c CACHE                 Define cache directory
    	-o OUTPUT                Define output file
    	--keep-cache
    	-k                       Do not delete cached files after download

### Demo 

To download for instance the map of Innsbruck

    osmpng -o ibk.png 11.3425-11.4614 47.2761-47.2484 14

![Screenshot of Innsbruck](osmpng.png)

