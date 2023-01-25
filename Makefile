CXXFLAGS=-std=c++14 -O3

LIB=-ljsoncpp -Iinclude/ -lcasa_casa -lcasa_meas -lcasa_measures

DEBUGFLAGS=-fsanitize=address -g

CXX=g++

SRC = array_creator.out \
	  json_test.out \
      main.out 
	  
all: $(SRC)

%.out: src/%.cc dir
	$(CXX) $(CXXFLAGS) $(LIB) -o build/$@ $< $(OPT)

dir: 
	mkdir -p build

.PHONY: dir