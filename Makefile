CXXFLAGS=-std=c++14

LIB=-ljsoncpp -Iinclude/ -lcasa_casa -lcasa_meas -lcasa_measures

DEBUGFLAGS=-fsanitize=address -g

CXX=g++

SRC = array_creator.out \
	main.out 

TEST = test_json.out \
	test_array_slicer.out


all: $(SRC) $(TEST)

%.out: src/%.cc dir
	$(CXX) -O3 $(CXXFLAGS) $(LIB) $(OPT) -o build/$@ $<

%.out: test/%.cc dir
	$(CXX) $(CXXFLAGS) $(LIB) $(DEBUGFLAGS) $(OPT) -o build/$@ $< 

dir: 
	mkdir -p build

.PHONY: dir