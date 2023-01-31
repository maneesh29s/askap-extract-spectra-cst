#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sys/stat.h>

#include <json/json.h>

#include "CasaImageAccess.h"
#include "FitsImageAccess.h"
#include "helper.hpp"

int main(int argc, char const *argv[])
{
    Timer timer;

    // std::string image="fits";
    // askap::accessors::FitsImageAccess accessor;

    std::string image="casa";
    askap::accessors::CasaImageAccess<casacore::Float> accessor;

    std::string imageFilePath = "test_data/" + image + "_dummy_image";
    std::string outputDirPathForSingleRead = "test_data/" + image + "_single_read_output_sources";
    std::string outputDirPathForSlicedRead = "test_data/" + image + "_multi_read_output_sources";

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
    extractSourcesWithSingleRead(imageFilePath, jsonFilePath, outputDirPathForSingleRead, accessor);
    timer.stop_timer();

    std::cout << "Time elapsed for whole " << image << " image read at once " << timer.time_elapsed() << std::endl;

    timer.start_timer();
    extractSourcesWithSlicedReads(imageFilePath, jsonFilePath, outputDirPathForSlicedRead, accessor);
    timer.stop_timer();

    std::cout << "Time elapsed for slice by slice " << image << " image read " << timer.time_elapsed() << std::endl;

    return 0;
}
