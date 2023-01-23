CXXFLAGS=-std=c++14 -O3

LIB=-ljsoncpp

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