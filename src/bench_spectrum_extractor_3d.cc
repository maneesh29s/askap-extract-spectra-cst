#include <fstream>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <vector>

#include <json/json.h>

#include "CasaImageAccess.h"
#include "FitsImageAccess.h"
#include "helper.hpp"
#include "spectralExtractors.hpp"

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

  std::string imageFilePath = "test_data_3d/" + inputImageType + "_dummy_image";
  std::string outputDirPathForsortedGroupedRead = "test_data_3d/" + inputImageType +
                                           "-to-" + outputImageType +
                                           "_sorted_grouped_read_output_sources";
  std::string outputDirPathForSlicedRead = "test_data_3d/" + inputImageType +
                                           "-to-" + outputImageType +
                                           "_multi_read_output_sources";

  int status;
  if (outputImageType != "bp") {
    status = mkdir(outputDirPathForsortedGroupedRead.c_str(), 0700);
    status = mkdir(outputDirPathForSlicedRead.c_str(), 0700);
  }

  Timer timer;

#ifdef SORTED_GROUPED_READ
  Parameters sortedGroupedRead(inputImageType, imageFilePath, jsonFilePath,
                        outputImageType, outputDirPathForsortedGroupedRead);

  timer.start_timer();
  spectrumExtractionWithSortedGroupedReads(sortedGroupedRead);
  timer.stop_timer();

  std::cout << "SourceSpectralExtractor:SORTED_GROUPED_READ:" << inputImageType << " to " << outputImageType
            << ":Time elapsed - " << timer.time_elapsed()
            << std::endl;
#endif

#ifdef SLICED_READ
  Parameters slicedRead(inputImageType, imageFilePath, jsonFilePath,
                        outputImageType, outputDirPathForSlicedRead);

  timer.start_timer();
  spectrumExtractionWithSlicedReads(slicedRead);
  timer.stop_timer();

  std::cout << "SourceSpectralExtractor:SLICED_READ:" << inputImageType << " to " << outputImageType
            << ":Time elapsed - " << timer.time_elapsed()
            << std::endl;
#endif

  return 0;
}
