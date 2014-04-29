/* String extension class
 * Based on std::string, extending it with some cool features
 *
 * 2014 Felix Niederwanger
 * GPLv3
 */

#include "String.hpp"

String::String()                : std::string("") {}
String::String(std::string str) : std::string(str) {}
String::String(const char* str) : std::string(str) {}
String::String(int i)           : std::string(std::to_string(i)) {}
String::String(long l)			: std::string(std::to_string(l)) {}
String::String(float f)         : std::string(std::to_string(f)) {}
String::String(double d)        : std::string(std::to_string(d)) {}


bool String::contains(std::string value) { 
	return strstr(this->c_str(), value.c_str()) != NULL;
}

String String::append(int i)    { return std::string::append(std::to_string(i)); }
String String::append(float f)  { return std::string::append(std::to_string(f)); }
String String::append(double d) { return std::string::append(std::to_string(d)); }
String String::append(long l)    { return std::string::append(std::to_string(l)); }
String String::append(unsigned long l)    { return std::string::append(std::to_string(l)); }
String String::append(unsigned int i)    { return std::string::append(std::to_string(i)); }
String String::append(std::string s)  { return String( std::string::append(s.c_str()) ); }
String String::append(const char* c)  { return String( std::string::append(c) ); }
String String::append(char* c)  { return String( std::string::append(c) ); }


String String::toLowercase() {
	String result(this->c_str());
	std::transform(result.begin(), result.end(), result.begin(), ::tolower);
	return result;
}

String String::toUppercase() {
	String result(this->c_str());
	std::transform(result.begin(), result.end(), result.begin(), ::toupper);
	return result;
}

String String::trim() {
	std::string s = std::string(std::string::c_str());
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
	s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
	return String(s.c_str());
}


String String::ltrim() {
	std::string s = std::string(std::string::c_str());
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
	return String(s.c_str());
}


String String::rtrim() {
	std::string s = std::string(std::string::c_str());
	s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
	return String(s.c_str());
}

std::vector<String> String::split(char delim) {
    std::stringstream ss(this->c_str());
    std::string item;
    std::vector<String> elems;
    while (std::getline(ss, item, delim))
        elems.push_back(item);
    return elems;
}


bool String::endsWith(std::string seq) {
	size_t size2 = seq.size();
	size_t size1 = std::string::size();
	if(size1 < size2) return false;
	
	std::string equiv = std::string::substr( size1-size2 );
	return seq == equiv;
}

bool String::endsWith(char c) {
	size_t size = std::string::size();
	if(size == 0) return false;
	return std::string::at(size-1) == c;
}

bool String::startsWith(std::string seq) {
	size_t size2 = seq.size();
	size_t size1 = std::string::size();
	if(size1 < size2) return false;
	
	std::string equiv = std::string::substr(0,size2);
	return seq == equiv;
}

bool String::startsWith(char c) {
	size_t size = std::string::size();
	if(size == 0) return false;
	return std::string::at(0) == c;
}



bool String::equalsIgnoreCase(std::string s) {
	if(s.size() != std::string::size()) return false;
	
	std::string s_lower(s.c_str());
	std::transform(s_lower.begin(), s_lower.end(), s_lower.begin(), ::tolower);
	return this->toLowercase() == s_lower;
}

bool String::isEmpty() { return std::string::size() == 0; }


String String::replace(std::string searchFor, std::string replaceWith) {
	size_t size = searchFor.size();
	while(true) {
		size_t pos = std::string::find(searchFor);
		if (pos == std::string::npos) break;
		else {
			std::string::replace(pos, size, replaceWith);
		}
	}
	
	return String(std::string::c_str());
} 

bool String::operator==(std::string str) {
	size_t size = std::string::size();
	if(str.size() != size) return false;
	return strncmp(str.c_str(), std::string::c_str(), size) == 0;
}
bool String::operator==(char *c) {
	size_t size = strlen(c);
	if(std::string::size() != size) return false;
	return strncmp(c, std::string::c_str(), size) == 0;
}
bool String::operator==(const char *c) {
	size_t size = strlen(c);
	if(std::string::size() != size) return false;
	return strncmp(c, std::string::c_str(), size) == 0;
}

String String::operator+(char* c) { return String( std::string::append(c) ); }
String String::operator+(const char* c) { return String( std::string::append(c) ); }
String String::operator+(std::string s) { return String( std::string::append(s.c_str()) ); }
String String::operator+(int i) { return String (std::string::append(std::to_string(i))); }
String String::operator+(float f)  { return String (std::string::append(std::to_string(f))); }
String String::operator+(long l) { return String (std::string::append(std::to_string(l))); }
String String::operator+(double d)  { return String (std::string::append(std::to_string(d))); }
String String::operator+(unsigned int i)  { return String (std::string::append(std::to_string(i))); }
String String::operator+(unsigned long l) { return String (std::string::append(std::to_string(l))); }
