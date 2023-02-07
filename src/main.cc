#include <fstream>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <vector>

#include <json/json.h>

#include "CasaImageAccess.h"
#include "FitsImageAccess.h"
#include "helper.hpp"

int main(int argc, char const *argv[]) {
  if (argc != 4) {
    std::cerr << "Usage: ./build/main.out <casa>/<fits> <casa>/<fits> <path to "
                 "processed.json file>";
    exit(1);
  }
  std::string inputImageType = argv[1];
  std::string outputImageType = argv[2];
  std::string jsonFilePath = argv[3];

  assert(inputImageType == "casa" | inputImageType == "fits");
  assert(outputImageType == "casa" | outputImageType == "fits" |
         outputImageType == "bp");

  std::cout << "Running " << inputImageType << " to " << outputImageType
            << std::endl;

  std::string imageFilePath = "test_data/" + inputImageType + "_dummy_image";
  std::string outputDirPathForSingleRead = "test_data/" + inputImageType +
                                           "-to-" + outputImageType +
                                           "_single_read_output_sources";
  std::string outputDirPathForSlicedRead = "test_data/" + inputImageType +
                                           "-to-" + outputImageType +
                                           "_multi_read_output_sources";

  int status;
  status = mkdir(outputDirPathForSingleRead.c_str(), 0700);
  if (outputImageType != "bp")
    status = mkdir(outputDirPathForSlicedRead.c_str(), 0700);

  Timer timer;

  Parameters singleRead(inputImageType, imageFilePath, jsonFilePath,
                        outputImageType, outputDirPathForSingleRead);

  timer.start_timer();
  extractSourcesWithSingleRead(singleRead);
  timer.stop_timer();

  std::cout << "Time elapsed : " << inputImageType << " to " << outputImageType
            << " : for whole image read at once " << timer.time_elapsed()
            << std::endl;

  Parameters slicedRead(inputImageType, imageFilePath, jsonFilePath,
                        outputImageType, outputDirPathForSlicedRead);

  timer.start_timer();
  extractSourcesWithSlicedReads(slicedRead);
  timer.stop_timer();

  std::cout << "Time elapsed : " << inputImageType << " to " << outputImageType
            << " : for slice by slice image read " << timer.time_elapsed()
            << std::endl;

  return 0;
}
