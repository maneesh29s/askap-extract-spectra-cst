CXXFLAGS=-std=c++14 -Iinclude/

LIB=-ljsoncpp -lcasa_casa -lcasa_lattices -lcasa_images -lcasa_tables -lcasa_coordinates -lcasa_scimath -lCommon -lcasa_coordinates -lcasa_fits -lcfitsio

DEBUGFLAGS=-fsanitize=address -g

CXX=g++

SRC = build/array_creator.out \
	build/main.out \

TEST = test_json.out \
	test_array_slicer.out

OBJECT_FILES = build/FITSImageRW.o \
			   build/FitsImageAccess.o 

all:  $(OBJECT_FILES) $(SRC) $(TEST)

# src
build/%.out: src/%.cc dir build/FITSImageRW.o build/FitsImageAccess.o 
	$(CXX) $(CXXFLAGS) -O3 $(LIB) -o $@ $< build/FitsImageAccess.o build/FITSImageRW.o $(OPT) 

# test
%.out: test/%.cc dir
	$(CXX) $(CXXFLAGS) $(LIB) $(DEBUGFLAGS) -o build/$@ $< $(OPT) 

build/FITSImageRW.o: include/FITSImageRW.cc
	$(CXX) $(CXXFLAGS) -c -O3 -o $@ $< $(OPT) 

build/FitsImageAccess.o: include/FitsImageAccess.cc build/FITSImageRW.o
	$(CXX) $(CXXFLAGS) -c -O3 -o $@ $< build/FITSImageRW.o $(OPT) 

dir: 
	mkdir -p build

.PHONY: dir

clean:
	rm -rf build/*