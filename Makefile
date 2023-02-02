CXXFLAGS=-std=c++14 -Iinclude/

OPTIMISATION=-O3
# OPTIMISATION=-fsanitize=address -g

LIB=-ljsoncpp -lcasa_casa -lcasa_lattices -lcasa_images -lcasa_tables -lcasa_coordinates -lcasa_scimath -lCommon -lcasa_coordinates -lcasa_fits -lcfitsio

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
	$(CXX) $(CXXFLAGS) $(OPTIMISATION) $(LIB) -o $@ $< build/FitsImageAccess.o build/FITSImageRW.o $(OPT) 

# test
%.out: test/%.cc dir build/FITSImageRW.o build/FitsImageAccess.o 
	$(CXX) $(CXXFLAGS) $(LIB) $(OPTIMISATION) -o build/$@ $< build/FitsImageAccess.o build/FITSImageRW.o $(OPT) 

build/FITSImageRW.o: include/FITSImageRW.cc
	$(CXX) $(CXXFLAGS) -c $(OPTIMISATION) -o $@ $< $(OPT) 

build/FitsImageAccess.o: include/FitsImageAccess.cc build/FITSImageRW.o
	$(CXX) $(CXXFLAGS) -c $(OPTIMISATION) -o $@ $< build/FITSImageRW.o $(OPT) 

dir: 
	mkdir -p build

.PHONY: dir

clean:
	rm -rf build/*