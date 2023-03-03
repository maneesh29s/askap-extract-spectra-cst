CXXFLAGS=-std=c++14 -Iinclude/ 

OPTIMISATION=-O3
# OPTIMISATION=-fsanitize=address -g

LIB=-ljsoncpp -lcasa_casa -lcasa_lattices -lcasa_images -lcasa_tables -lcasa_scimath \
	-lCommon -lcasa_coordinates -lcasa_fits `adios2-config --cxx-flags --cxx-libs` \
	`pkg-config --libs cfitsio` `pkg-config --cflags --libs ompi`

CXX=mpicxx

SRC = build/array_creator.out \
	build/bench_cubelet_extractor_2d.out \
	build/json_sorter_2d.out \
	build/json_sorter_3d.out \
	build/json_slicer_truncator_3d.out \
	build/bench_spectrum_extractor_3d.out

SCRATCH = test_array_slicer.out

OBJECT_FILES = build/FITSImageRW.o \
			build/FitsImageAccess.o

all: $(OBJECT_FILES) $(SRC) $(SCRATCH)

# src
build/%.out: src/%.cc dir build/FITSImageRW.o build/FitsImageAccess.o
	$(CXX) $(CXXFLAGS) $(OPTIMISATION) $(LIB) -o $@ $< build/FitsImageAccess.o build/FITSImageRW.o $(OPT)

# scratch
%.out: scratch/%.cc dir build/FITSImageRW.o build/FitsImageAccess.o
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