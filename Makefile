CXX=g++
#CXX_FLAGS=-Wall -pedantic -std=c++11
# Use this CXX_FLAGS if gcc < 4.7
CXX_FLAGS=-Wall -Wextra -Werror -pedantic -std=c++11


default:	all
all:	osmpng


osmpng: src/osmpng.cpp src/String.hpp String
	$(CXX) $(CXX_FLAGS) `libpng-config --cflags` `curl-config --cflags` -o bin/osmpng src/osmpng.cpp bin/String.o `libpng-config --ldflags` `curl-config --libs`

String:	src/String.hpp src/String.cpp
	mkdir -p bin
	$(CXX) $(CXX_FLAGS) -c -o bin/String.o src/String.cpp


clean:
	rm -f bin/*.o
	rm -f bin/*.f

install:	osmpng
	install bin/osmpng /usr/local/bin/osmpng
