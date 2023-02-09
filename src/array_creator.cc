#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sys/stat.h>

#include "CasaImageAccess.h"
#include "FitsImageAccess.h"
#include "helper.hpp"

int main(int argc, char const *argv[])
{
    float offset = -5.0f;
    float range = 10.0f;

    int status = mkdir("test_data", 0700);

    if (argc < 2)
    {
        std::cerr << "Must provide dimensions as input. \n Usage: ./build/array_creator naxis1 naxis2 ...";
        exit(1);
    }

    size_t naxes = argc-1;
    std::cout << "naxes: " << naxes << std::endl;

    std::vector<size_t> naxis(naxes);
    for (size_t i = 0; i < naxes; i++)
    {
        naxis[i] = static_cast<size_t>(atoi(argv[i+1]));
        std::cout << "naxis" << i << ": " << naxis[i] << " ";
    }
    std::cout << std::endl;

    std::vector<float> arr = generateRandomData(naxis, range, offset);
    std::cout << "Array generation done" << std::endl;
    
    std::string image;
    std::string imageFilePath;

    image="fits";
    imageFilePath = "test_data/" + image + "_dummy_image";
    askap::accessors::FitsImageAccess fitsAcceessor;
    writeData(naxis, arr, imageFilePath, fitsAcceessor);
    std::cout << "Array writen to " << imageFilePath << ".fits" << std::endl;

    image="casa";
    imageFilePath = "test_data/" + image + "_dummy_image";
    askap::accessors::CasaImageAccess<casacore::Float> casaAcceessor;
    writeData(naxis, arr, imageFilePath, casaAcceessor);
    std::cout << "Array writen to " << imageFilePath << std::endl;

    return 0;
}
