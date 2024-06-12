all: main.out building.out resource.out billsOffice.out

main.out: build/main.o build/logger.o build/strutils.o
	g++ -o main.out build/main.o build/logger.o build/strutils.o

build/main.o: main.cpp logger.hpp consts.hpp strutils.hpp
	@mkdir -p build
	g++ -std=c++17 -c main.cpp -o build/main.o

building.out: build/building.o build/logger.o build/strutils.o
	g++ -o building.out build/building.o build/logger.o build/strutils.o

build/building.o: building.cpp logger.hpp consts.hpp strutils.hpp
	@mkdir -p build
	g++ -std=c++17 -c building.cpp -o build/building.o

resource.out: build/resource.o build/logger.o build/strutils.o
	g++ -o resource.out build/resource.o build/logger.o build/strutils.o

build/resource.o: resource.cpp logger.hpp consts.hpp strutils.hpp
	@mkdir -p build
	g++ -std=c++17 -c resource.cpp -o build/resource.o

billsOffice.out: build/billsOffice.o build/logger.o build/strutils.o
	g++ -o billsOffice.out build/billsOffice.o build/logger.o build/strutils.o

build/billsOffice.o: billsOffice.cpp logger.hpp consts.hpp strutils.hpp
	@mkdir -p build
	g++ -std=c++17 -c billsOffice.cpp -o build/billsOffice.o

build/logger.o: logger.cpp logger.hpp colorPrint.hpp
	@mkdir -p build
	g++ -std=c++17 -c logger.cpp -o build/logger.o

build/strutils.o: strutils.cpp strutils.hpp
	@mkdir -p build
	g++ -std=c++17 -c strutils.cpp -o build/strutils.o

clean:
	rm *.out
	rm -rf build
	rm -rf namedpipes
