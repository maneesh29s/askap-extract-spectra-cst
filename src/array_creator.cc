#include <fstream>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <vector>

#include "CasaImageAccess.h"
#include "FitsImageAccess.h"
#include "helper.hpp"

int main(int argc, char const *argv[]) {
  float offset = -5.0f;
  float range = 10.0f;

  if (argc < 3) {
    std::cerr << "\nUsage: ./build/array_creator suffix naxis1 naxis2 ... \n";
    exit(1);
  }

  std::string suffix(argv[1]);
  std::cout << "suffix: " << suffix << std::endl;

  std::string outDirName = "test_data_" + suffix;
  int status = mkdir(outDirName.c_str(), 0700);

  size_t naxes = argc - 2;
  std::cout << "naxes: " << naxes << std::endl;

  std::vector<size_t> naxis(naxes);
  for (size_t i = 0; i < naxes; i++) {
    naxis[i] = static_cast<size_t>(atoi(argv[i + 2]));
    std::cout << "naxis" << i << ": " << naxis[i] << " ";
  }
  std::cout << std::endl;

  std::vector<float> arr = generateRandomData(naxis, range, offset);
  std::cout << "Array generation done" << std::endl;

  std::string image;
  std::string imageFilePath;

  image = "fits";
  imageFilePath = outDirName + "/" + image + "_dummy_image";
  askap::accessors::FitsImageAccess fitsAcceessor;
  writeData(naxis, arr, imageFilePath, fitsAcceessor);
  std::cout << "Array writen to " << imageFilePath << ".fits" << std::endl;

  // image = "casa";
  // imageFilePath = outDirName + "/" + image + "_dummy_image";
  // askap::accessors::CasaImageAccess<casacore::Float> casaAcceessor;
  // writeData(naxis, arr, imageFilePath, casaAcceessor);
  // std::cout << "Array writen to " << imageFilePath << std::endl;

  return 0;
}
