#include <iostream>
#include <vector>
#include <fstream>
#include <json/json.h>
#include <string>

#include <sys/stat.h>

#include <casacore/casa/Arrays.h>

#include "helper.hpp"

int main(int argc, char const *argv[])
{
    Timer timer;

    std::string casaFilePath = "test_data/dummy_casa_image";
    std::string outputDirPathForSingleRead = "test_data/single_read_output_sources";
    std::string outputDirPathForSlicedRead = "test_data/multi_read_output_sources";

    int status ;
    status = mkdir(outputDirPathForSingleRead.c_str(), 0700);
    status = mkdir(outputDirPathForSlicedRead.c_str(), 0700);
    
    if (argc != 2)
    {
        std::cerr << "Usage: ./build/main.out <path to processed.json file>";
        exit(1);
    }

    std::string jsonFilePath = argv[1];
    
    timer.start_timer();
    readDataCasa(casaFilePath, jsonFilePath, outputDirPathForSingleRead);
    timer.stop_timer();

    std::cout << "Time elapsed for whole CASA read at once " << timer.time_elapsed() << " us" << std::endl;

    timer.start_timer();
    readDataSlicedCasa(casaFilePath, jsonFilePath, outputDirPathForSlicedRead);
    timer.stop_timer();

    std::cout << "Time elapsed for slice by slice CASA read " << timer.time_elapsed() << " us" << std::endl;

    return 0;
}
