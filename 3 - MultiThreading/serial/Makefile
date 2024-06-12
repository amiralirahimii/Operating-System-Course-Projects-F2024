all: ImageFilters.out

ImageFilters.out: build/main.o build/filter.o
	g++ -o ImageFilters.out build/main.o build/filter.o

build/main.o: main.cpp filter.hpp
	@mkdir -p build
	g++ -std=c++11 -c main.cpp -o build/main.o

build/filter.o: filter.cpp filter.hpp
	@mkdir -p build
	g++ -std=c++11 -c filter.cpp -o build/filter.o

clean:
	rm  -f *.out output.bmp
	rm -rf build
