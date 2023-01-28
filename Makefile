CXXFLAGS=-std=c++14

LIB=-ljsoncpp -Iinclude/ -lcasa_casa -lcasa_lattices -lcasa_images -lcasa_tables -lcasa_coordinates -lcasa_scimath -lCommon -lcasa_coordinates

DEBUGFLAGS=-fsanitize=address -g

CXX=g++

SRC = array_creator.out \
	main.out 

TEST = test_json.out \
	test_array_slicer.out


all: $(SRC) $(TEST)

%.out: src/%.cc dir
	$(CXX) $(CXXFLAGS) -O3 $(LIB) $(OPT) -o build/$@ $<

%.out: test/%.cc dir
	$(CXX) $(CXXFLAGS) $(LIB) $(DEBUGFLAGS) $(OPT) -o build/$@ $< 

dir: 
	mkdir -p build

.PHONY: dir

clean:
	rm -rf build/*