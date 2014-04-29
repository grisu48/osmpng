/* osmpng.cpp
 * 2014, Felix Niederwanger
 * 
 * Tool for downloading PNG titles from OSM and merge them together to a 
 * single PNG map file.
 *
 * Licensed under the conditions of GPLv3
 */

#include <dirent.h>
#include <iostream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <sstream>
#include <signal.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/timeb.h>

#include <curl/curl.h>
#include <png++/png.hpp>

#include "String.hpp"


using namespace std;

// VERSION
#define VERSION "0.2a APR 2014"

// Use float or double precision
#define REAL float

// Makro for printing stuff only when not quiet
#define COUT if(!quiet) cout



/* ==== GLOBAL PROGRAM VARIABLES ============================================ */

// Rectangle to be downloaded
static REAL bounds[4];
// True if we should be quiet
static bool quiet = false;
// Cache directory
static String cacheDir = "/tmp/.osmpng_cache";
// Destination file
static String destFile = "output.png";
// If cached files should be deleted
static bool deleteCached = true;

/* ==== INTERNAL PROGRAM VARIABLES ========================================== */

// Cached files
static vector<string> files;

// Get milliseconds since epoch
static unsigned long get_millis() {
	timeb tb;
	ftime(&tb);
	return tb.millitm + (tb.time & 0xfffff) * 1000L;
}

// Get slippy map tile coordinates
/* See http://wiki.openstreetmap.org/wiki/Slippy_map_tilenames for details */
REAL getTileY(REAL latitude, REAL n) {
	REAL rad_lat = latitude * M_PI / 180.0;
	REAL sec = 1.0/cos(rad_lat);
	return n * (1.0 - (log(tan(rad_lat) + sec) / M_PI)) / 2.0;
}
REAL getTileX(REAL longitude, REAL n) {
	return n * ((longitude + 180.0) / 360.0);
}

inline REAL toReal(std::string str) { return atof(str.c_str()); }
//inline REAL toReal(const char* str) { return atof(str); }
inline int toInt(std::string str) { return atoi(str.c_str()); }
//inline int toInt(const char* str) { return atoi(str); }

REAL round(REAL f, int precision) {
	REAL prec = pow(10, precision);
	int i = (int)(f * prec);
	return (REAL)(i) / prec;
}
inline REAL round(REAL f) { return round(f,2); }


static void clear_cached_files() {
	for(vector<string>::iterator it = files.begin(); it != files.end(); it++) {
		std::string file = *it;
		remove(file.c_str());
	}
}

static void printHeader() {
	if(quiet) return;
	else {
		cout << "OSM title downloader version " << VERSION << endl;
		cout << "  2014, Felix Niederwanger" << endl;
		cout << endl;
	}
}

static inline bool file_exists(string name) {
	return file_exists(name.c_str());
}
static inline bool dir_exists(string name) {
	DIR* dir = opendir(name.c_str());
	if(dir) {
		closedir(dir);
		return true;
	} else
		return false;
}

// Get file size in bytes
static size_t get_file_size(std::string filename)
{
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}


// Callback for receiving http data
static size_t write_http(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    return fwrite(ptr, size, nmemb, stream);
}

// Download a slippy map tile to a given destination file
size_t download(int tileX, int tileY, int zoom, string file) {
	stringstream ss;
	static int i = 0;
	switch(i++) {
	case 0:
		ss << "http://a.tile.openstreetmap.org/";
		break;
	case 1:
		ss << "http://b.tile.openstreetmap.org/";
		break;
	default:
		ss << "http://c.tile.openstreetmap.org/";
		i = 0;
		break;
	}
	ss << zoom << '/' << tileX << '/' << tileY << ".png";
	String url = ss.str();
	
	CURL *curl;
	CURLcode code;
	curl = curl_easy_init();
	if (curl) {
		FILE *fp = fopen(file.c_str(), "wb");
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_http);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		// Allow max. 10 redirections
		// curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 10L);
		
		code = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
        fclose(fp);
        
        if(code != CURLE_OK) {
    	    ss.str(std::string());
	        ss << "CURL returns error " << code << "!";
	        throw ss.str();
        }
        
        return get_file_size(file);
		
	} else
		throw "Error setting up curl";
}

// Create standartized cache filename
std::string get_filename(int x, int y, int zoom) {
	stringstream ss;
	
	ss << cacheDir;
	if(!cacheDir.endsWith('/')) ss << '/';
	ss << zoom << '-' << x << '.' << y << ".png";
	return ss.str();
}


// Header message, when an error occurred
static void error_help_msg() {
	cerr << "A terrible error happend. Please consider in reporting a bug to "
		<< "me. Write a email to felix@feldspaten.org including version of "
		<< " the program and a detailed description, WHAT you did." << endl
		<< "Visit the project homepage at GitHub: "
		<< "https://github.com/grisu48/segeo" << endl;
}

// Singal callback function
static void signal_function(int sig_nr) {
	switch(sig_nr) {
	case SIGSEGV:
		error_help_msg();
		cerr << "Segmentation fault (SIGSEGV)" << endl;
		exit(101);
	case SIGINT:
		cerr << "Caught cancel signal. Cleaning up ... ";
		cerr.flush();
		if(deleteCached) clear_cached_files();
		cerr << "done" << endl;
		exit(42);
	}
}

// Merge routine to merge different PNG files.
// Writes the result to the given destination filename
static void merge(int* bounds, int zoom, std::string destination) {
	size_t width, height;
	size_t total_width, total_height;
	std::string file = get_filename(bounds[0],bounds[2], zoom);
	{
		png::image<png::rgb_pixel> source(file.c_str());
		width = source.get_width();
		height = source.get_height();
		
		total_width = width * (bounds[1]-bounds[0]+1);
		total_height = height * (bounds[3]-bounds[2]+1);
	}
	
	// cout << "Creating picture (" << total_width << "x" << total_height << ") ... " << endl;
	
	png::image<png::rgb_pixel> image(total_width, total_height);
	size_t offset[2];
	offset[0] = 0;
	offset[1] = 0;
	int index[2];
	for(int x = bounds[0]; x<=bounds[1]; x++) {
		index[0] = x - bounds[0];
		offset[0]++;
		offset[1]=0;
		for(int y = bounds[2]; y<=bounds[3]; y++) {
			size_t base[2];
			index[1] = y - bounds[2];

			file = get_filename(x,y, zoom);
			png::image<png::rgb_pixel> source(file.c_str());
			size_t c_width, c_height;
			c_width = source.get_width();
			c_height = source.get_height();
			if (c_width != width) throw "Width of tile mismatch";
			if (c_height != height) throw "Width of tile mismatch";
			
			// Copy image pixels
			base[0] = width * ( index[0] );
			base[1] = height * ( index[1] );

			// Copy pixel data
			for(size_t p_x = 0; p_x < c_width; p_x++) {
				for(size_t p_y = 0; p_y < c_height; p_y++) {
					image.set_pixel(p_x + base[0],p_y + base[1],source.get_pixel(p_x,p_y));
				}
			}


			offset[1]++;
		}
	}
	
	// cout << "Writing destination file ... " << endl;
	image.write(destination.c_str());
}

// Print help message
static void printHelp(char* progname) {
	cout << "OSM tile downloader - Version " << VERSION << endl;
	cout << "2014, Felix Niederwanger" << endl << endl;

	cout << "Usage: " << progname << " [OPTIONS] [LONGITUDE LATITUDE ZOOM]" << endl;
	cout << endl << "OPTIONS" << endl <<
			"\t-help                    Print this help message" << endl <<
			"\t-version                 Print program version" << endl <<
			"\t--cache=CACHE" << endl <<
			"\t-c CACHE                 Define cache directory" << endl;
	cout << "\t-o OUTPUT                Define output file" << endl <<
			"\t--keep-cache" << endl <<
			"\t-k                       Do not delete cached files after download" << endl;
	cout << endl;
	cout << "If the destination is given, LONGITUDE LATITUDE and ZOOM must be defined" << endl;
}


// Get as size (given in bytes) in a human readable approximation
static string sizeHumanReadable(size_t size) {
	stringstream ss;
	if (size > 1024) {
		float kb = size / 1024.0;
		if (kb > 1024.0)
			 ss << round(kb / 1024.0) << " MiB";
		else
			ss << round(kb,2) << " kiB";
	} else
		ss << size << " B";
	
	return ss.str();
}

// Print a size (in bytes) to a human readable approximation
static void printSizeHumanReadable(size_t size) {
	cout << sizeHumanReadable(size);
}


// Get as speed (given in bytes) in a human readable approximation
static string speedHumandReadable(double speed) {
	stringstream ss;
	if (speed > 1024.0) {
		float kb = speed / 1024.0;
		if (kb > 1024.0)
			 ss << round(kb / 1024.0) << " MiB/s";
		else
			ss << round(kb,2) << " kiB/s";
	} else
		ss << speed << " B/s";
	
	return ss.str();
}

static void _mkdir(const char *dir) {
	char tmp[1024];
	char *p = NULL;
	size_t len;

	snprintf(tmp, sizeof(tmp),"%s",dir);
	len = strlen(tmp);
	if(tmp[len - 1] == '/')
	tmp[len - 1] = 0;
	for(p = tmp + 1; *p; p++)
		if(*p == '/') { 
			*p = 0;
			mkdir(tmp, S_IRWXU);
			*p = '/';
		}
	mkdir(tmp, S_IRWXU);
}

int main(int argc, char** argv) {
	// Register signal handler
	signal(SIGINT, signal_function);
	signal(SIGSEGV, signal_function);

	// slon, slat and szoom are the LONGITUE LATITUDE ZOOM paramters
	String slon, slat, szoom;
	bool stdinInput = true;		// If we must read LONGITUDE LATITUDE ZOOM from
								// stdin
	
	// Parse program arguments
	if (argc > 1) {
		int i = 0;
		while(++i < argc) {
			String arg = argv[i];
			if(arg.size() == 0) continue;
			
			bool isLast = i == argc-1;
			if(arg.equalsIgnoreCase("-help") || arg.equalsIgnoreCase("help")) {
				printHelp(argv[0]);
				exit(0);
			} else if(arg == "-version" || arg == "version") {
				cout << VERSION << endl;
				exit(0);
			} else if(arg.startsWith("--cache=") && arg.size() > 8) {
				cacheDir = arg.substr(8);
			} else if(arg == "-c") {
				if(isLast) continue;
				cacheDir = argv[++i];
			} else if(arg == "-o") {
				if(isLast) continue;
				destFile = argv[++i];
			} else if(arg == "--keep-cache" || arg == "-k") {
				// Keep cache
				deleteCached = false;
			} else if(arg == "-q") {
				quiet = true;
			} else {
				// LONGITUDE LATITUDE ZOOM argument?
				if (arg.at(0) == '-') {
					cerr << "Illegal argument: " << argv[i] << endl;
					return EXIT_FAILURE;
				} else {
					// Check if enough parameters left
					if (i+1 >= argc) {
						cerr << "If providing a LONGITUDE you must also provide a LATITUDE" << endl;
						return EXIT_FAILURE;
					}
					slon = argv[i];
					slat = argv[i+1];
					if(i+2 >= argc) {
						szoom = "12";
						i++;
					} else {
						szoom = argv[i+2];
						i+=2;
					}
					stdinInput = false;
				}
			}
		}

		// Print options if not quiet
		if(!quiet) {
			printHeader();
			if(!deleteCached) cout << "Keeping cached files" << endl;
			if(!stdinInput) {
				cout << "Longitude: " << slon << endl 
					<< "Latitude : " << slat << endl
					<< "Zoom:      " << szoom << endl;
			}
		}
			
	}

	// cacheDir must end with "/"
	if(cacheDir.isEmpty()) cacheDir = "./";
	if(!cacheDir.endsWith('/')) cacheDir += '/';
	
	// Read from stdin, if not yet given as program parameter
	if (stdinInput) {
		try {
			COUT << "Type in coordinates for the map to download" << endl;
			COUT << "  Longitude and Latitude can also be a range of coordinates" << endl;
			COUT << endl;
			cout << "Longitude : "; getline(cin, slon); if (cin.eof()) throw "";
			cout << "Latitude  : "; getline(cin, slat); if (cin.eof()) throw "";
			cout << "Zoom      : "; getline(cin, szoom);if (cin.eof()) throw "";
		} catch (...) {
			cerr << "Cancelled" << endl;
			return EXIT_FAILURE;
		}
	}
	
	// Parse LONGITUDE LATITUDE ZOOM
	int zoom = toInt(szoom);
	REAL n = pow(2.0, zoom);
	if(slon.contains("-")) {
		std::vector<String> vec = slon.split('-');
		bounds[0] = toReal(vec[0].trim());
		bounds[1] = toReal(vec[1].trim());
	} else {
		bounds[0] = toReal(slon);
		bounds[1] = bounds[0];
	}
	if(slat.contains("-")) {
		std::vector<String> vec = slat.split('-');
		bounds[2] = toReal(vec[0].trim());
		bounds[3] = toReal(vec[1].trim());
	} else {
		bounds[2] = toReal(slat);
		bounds[3] = bounds[2];
	}
	
	
	// Check bounds and organise bounds
	try {
		for(int i=0;i<2;i++) {
			if(bounds[i] < -180.0) throw "Longitude < -180 degree";
			if(bounds[i] > 180.0) throw "Longitude > 180 degree";
		}
		
		for(int i=2;i<4;i++) {
			if(bounds[i] < -90.0) throw "Latitude < -90 degree";
			if(bounds[i] > 90.0) throw "Latitude > 90 degree";
		}
		
	} catch (const char* msg) {
		cerr << "ERROR: Bounds invalid (" << msg << ")" << endl;
		return EXIT_FAILURE;
	}
	
	// Create destination folder, if not existing
	if(!dir_exists(cacheDir))
		mkdir(cacheDir.c_str(), S_IRWXU | S_IRWXG);


	// Calculate tile coordinates
	bounds[0] = getTileX(bounds[0], n);
	bounds[1] = getTileX(bounds[1], n);
	bounds[2] = getTileY(bounds[2], n);
	bounds[3] = getTileY(bounds[3], n);
	if(bounds[0] > bounds[1]) {
		REAL tmp = bounds[0];
		bounds[0] = bounds[1];
		bounds[1] = tmp;
	}
	if(bounds[2] > bounds[3]) {
		REAL tmp = bounds[2];
		bounds[2] = bounds[3];
		bounds[3] = tmp;
	}
	
	// Create cache dir, if not yet done
	_mkdir(cacheDir.c_str());
	
	// Begin download
	COUT << "Downloading tiles (" << bounds[0] << " - " << bounds[1] << ") - ("
		<< bounds[2] << " - " << bounds[3] << ") ... " << endl;
	COUT.flush();
	
	int ibounds[4];
	for (int i=0;i<4;i++) 
		ibounds[i] = (int)bounds[i];
	int total = (bounds[1] - bounds[0] + 1) * (bounds[3] - bounds[2] + 1);
	int progress = 0;
	size_t total_size = 0;
	
	unsigned long total_millis = -get_millis();
	for(int x=ibounds[0];x<=ibounds[1];x++) {
		for (int y=ibounds[2];y<=ibounds[3];y++) {
			COUT << " ["<< round(100.0 * (REAL)progress / (REAL)total) << "%]" 
				<< "\tDownloading tile [" << x << "-" << y << "] ... ";
			COUT.flush();
			
			std::string file = get_filename(x,y,zoom);
			files.push_back(file);
			unsigned long millis = -get_millis();
			size_t size = download(x,y,zoom, file);
			millis += get_millis();
			total_size += size;
			progress++;
			
			if (!quiet) {
				double speed = round(size*1000.0/(double)millis);
				printSizeHumanReadable(size);
				cout << " @ " << speedHumandReadable(speed);
				cout << "                    \r";
				cout.flush();
			}
		}
	}
	total_millis += get_millis();
	if (!quiet) {
		double speed = round(total_size*1000.0/(double)total_millis);
		
		cout << "Downloaded totally ";
		printSizeHumanReadable(total_size);
		cout << " within " << total_millis << " ms @ " 
			<< speedHumandReadable(speed) 
			<< "                                        " << endl;
	}
	
	COUT << "Merging tiles ... ";
	COUT.flush();
	merge(ibounds, zoom, destFile);
	COUT << "done" << "                                        \r";
	
	if(deleteCached) {
		COUT << "Clearing cache ... ";
		COUT.flush();
		clear_cached_files();

		COUT << "done" << endl;
	}
	
	COUT << endl;
	return EXIT_SUCCESS;
}
