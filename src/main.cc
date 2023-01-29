#include <iostream>
#include <vector>
#include <fstream>
#include <json/json.h>
#include <string>

#include <casacore/casa/Arrays.h>

#include "helper.hpp"

int main(int argc, char const *argv[])
{
    Timer timer;

    std::string casaFileName = "data/test_casa_data.FITS";

    if (argc != 2)
    {
        std::cerr << "Usage: ./build/main.out <path to processed.json file>";
        exit(1);
    }

    std::string jsonFileName = argv[1];
    
    timer.start_timer();
    readDataCasa(casaFileName, jsonFileName);
    timer.stop_timer();

    std::cout << "Time elapsed for whole CASA read at once " << timer.time_elapsed() << " us" << std::endl;

    timer.start_timer();
    readDataSlicedCasa(casaFileName, jsonFileName);
    timer.stop_timer();

    std::cout << "Time elapsed for slice by slice CASA read " << timer.time_elapsed() << " us" << std::endl;

    return 0;
}
