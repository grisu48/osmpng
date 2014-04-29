/* String extension class
 * Based on std::string, extending it with some cool features
 *
 * 2014 Felix Niederwanger
 * GPLv3
 */

#ifndef _FLEXS_STRING_LIBRARY_
#define _FLEXS_STRING_LIBRARY_

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <string.h>
#include <algorithm>
#include <string> 
#include <functional> 
#include <cctype>
#include <locale>


/* extended string */
class String : public std::string {
public:
	String();
	String(std::string str);
	String(const char*);
	String(int);
	String(long);
	String(float);
	String(double);
	
	bool contains(std::string);
	
	String toLowercase();
	String toUppercase();
	String trim();
	String ltrim();
	String rtrim();
	
	std::vector<String> split(char);
	bool endsWith(std::string seq);
	bool endsWith(char c);
	bool startsWith(std::string seq);
	bool startsWith(char c);
	bool equalsIgnoreCase(std::string str);
	
	bool isEmpty();
	
	String replace(std::string, std::string);

	String operator+(char*);
	String operator+(const char*);
	String operator+(std::string s);
	String operator+(int i);
	String operator+(float f);
	String operator+(long l);
	String operator+(double d);
	String operator+(unsigned int i);
	String operator+(unsigned long l);
	bool operator==(std::string);
	bool operator==(char *c);
	bool operator==(const char *c);
	
	String append(int);
	String append(float);
	String append(double);
	String append(long);
	String append(unsigned long);
	String append(unsigned int);
	String append(std::string s);
	String append(const char* );
	String append(char* );
};


#endif
