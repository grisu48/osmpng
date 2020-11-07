CXX=g++
CXX_FLAGS=-Wall -Wextra -Werror -pedantic -std=c++11


default:	all
all:	osmpng


osmpng: osmpng.cpp String.o
	$(CXX) $(CXX_FLAGS) `libpng-config --cflags` `curl-config --cflags` -o $@ $^ `libpng-config --ldflags` `curl-config --libs`

String.o: String.cpp String.hpp
	$(CXX) $(CXX_FLAGS) -c -o $@ $^

clean:
	rm -f *.o

install:	osmpng
	install osmpng /usr/local/bin/osmpng
