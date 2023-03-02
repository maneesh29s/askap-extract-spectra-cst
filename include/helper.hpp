#ifndef helpers_hpp
#define helpers_hpp

#include <chrono>
#include <string>
#include <vector>

#include <adios2.h>
#include <casacore/casa/Arrays.h>
#include <casacore/coordinates/Coordinates/CoordinateSystem.h>
#include <casacore/coordinates/Coordinates/CoordinateUtil.h>

#include "CasaImageAccess.h"
#include "FitsImageAccess.h"

namespace chrono = std::chrono;

class Timer {
private:
  chrono::time_point<chrono::high_resolution_clock> start;
  chrono::time_point<chrono::high_resolution_clock> end;

public:
  void start_timer() { this->start = chrono::high_resolution_clock::now(); }
  void stop_timer() { this->end = chrono::high_resolution_clock::now(); }

  std::string time_elapsed() {
    auto time_taken =
        chrono::duration_cast<chrono::milliseconds>(this->end - this->start)
            .count();

    return std::to_string(time_taken) + " ms";
  }
};

class Parameters {
public:
  std::string inputImageType;
  std::string imageFilePath;
  std::string jsonFilePath;
  std::string outputImageType;
  std::string outputDirPath;

  Parameters() {}

  Parameters(std::string inputImageType, std::string &imageFilePath,
             std::string &jsonFilePath, std::string outputImageType,
             std::string &odp)
      : inputImageType(inputImageType), imageFilePath(imageFilePath),
        jsonFilePath(jsonFilePath), outputImageType(outputImageType),
        outputDirPath(odp) {
    if (outputDirPath.back() != '/') {
      outputDirPath.append("/");
    }
  }
};


class SpectralImageSource
{
public:
    std::string sourceID;
    std::vector<int64_t> slicerBegin;
    std::vector<int64_t> slicerEnd;
    std::vector<int64_t> stride;
    std::vector<int64_t> length;
    std::string stokes;

    SpectralImageSource(){};

    SpectralImageSource(std::string sid, std::vector<int64_t> sb, std::vector<int64_t> se, std::vector<int64_t> st, std::vector<int64_t> len, std::string sto) : sourceID(sid), slicerBegin(sb), slicerEnd(se), stride(st), length(len), stokes(sto){};

    bool operator<(const SpectralImageSource &str) const
    {
        for (size_t i = 0; i < slicerBegin.size(); i++)
        {
            if (slicerBegin[i] < str.slicerBegin[i])
            {
                return true;
            }
            if (slicerBegin[i] == str.slicerBegin[i])
            {
                continue;
            }
            return false;
        }
        return false;
    }
};


void writeToBp(int sourceID, casacore::Array<casacore::Float> output,
               adios2::IO io, adios2::Engine writer) {
  const adios2::Dims shape = {static_cast<std::size_t>(output.shape()(0)),
                              static_cast<std::size_t>(output.shape()(1)),
                              static_cast<std::size_t>(output.shape()(2)),
                              static_cast<std::size_t>(output.shape()(3))};
  const adios2::Dims start = {
      static_cast<std::size_t>(0), static_cast<std::size_t>(0),
      static_cast<std::size_t>(0), static_cast<std::size_t>(0)};

  const adios2::Dims count = {static_cast<std::size_t>(output.shape()(0)),
                              static_cast<std::size_t>(output.shape()(1)),
                              static_cast<std::size_t>(1),
                              static_cast<std::size_t>(1)};

  adios2::Variable<float> varGlobalArray =
      io.DefineVariable<float>("Image_" + std::to_string(sourceID), shape,
                               start, count, adios2::ConstantDims);

  std::vector<float> temp(output.shape()(0) * output.shape()(1));
  // writer.BeginStep();
  for (size_t r = 0; r < output.shape()(1); r++) {
    for (size_t c = 0; c < output.shape()(0); c++) {
      casacore::IPosition currentPos(4, c, r, 0, 0);
      temp[r * output.shape()(0) + c] = output(currentPos);
    }
  }
  writer.Put(varGlobalArray, temp.data(), adios2::Mode::Sync);
}

boost::shared_ptr<askap::accessors::IImageAccess<casacore::Float>>
generateAccessorFromImageType(const std::string &imageType) {
  boost::shared_ptr<IImageAccess<>> result;
  if (imageType == "casa") {
    boost::shared_ptr<CasaImageAccess<casacore::Float>> iaCASA(
        new CasaImageAccess<casacore::Float>());
    result = iaCASA;
  } else if (imageType == "fits") {
    boost::shared_ptr<FitsImageAccess> iaFITS(new FitsImageAccess());
    const bool fast = true;
    iaFITS->useFastAlloc(fast);
    result = iaFITS;
  } else {
    std::cerr << "include/helper.hpp:generateAccessorFromImageType(): "
                 "Unsupported image type : " +
                     imageType + " has been requested";
    exit(1);
  }
  return result;
}

std::vector<float> generateSequentialData(const std::vector<size_t> &naxis,
                                          float start) {
  size_t totpix = 1;
  for (size_t i = 0; i < naxis.size(); i++)
    totpix *= naxis[i];

  std::vector<float> arr(totpix);

  for (size_t i = 0; i < arr.size(); i++) {
    // logic
    arr[i] = start + (float)i;
  }
  return arr;
}

std::vector<float> generateRandomData(const std::vector<size_t> &naxis,
                                      float range, float offset) {
  size_t totpix = 1;
  for (size_t i = 0; i < naxis.size(); i++)
    totpix *= naxis[i];

  std::vector<float> arr(totpix);

  time_t seed = time(0);
  srand(seed);

  for (size_t i = 0; i < arr.size(); i++) {
    arr[i] = offset + range * (rand() / (float)RAND_MAX);
  }

  return arr;
}

void createTestImage(
    const std::vector<size_t> &naxis, const std::vector<float> &inputArr,
    const std::string &filename,
    askap::accessors::IImageAccess<casacore::Float> &accessor) {
  size_t naxes = naxis.size();

  if (naxes != 4) {
    std::cerr << "This application requires a 4D array.";
    exit(1);
  }

  casacore::IPosition arrSize(naxes);
  for (size_t i = 0; i < naxes; i++) {
    arrSize(i) = naxis[i];
  }

  casacore::Array<casacore::Float> arr(arrSize);

  for (size_t i = 0; i < inputArr.size(); i++) {
    casacore::IPosition currentPos =
        casacore::toIPositionInArray(i, arr.shape());
    arr(currentPos) = inputArr[i];
  }

  // create casa file
  accessor.create(filename, arr.shape(),
                  casacore::CoordinateUtil::defaultCoords4D());

  // write the array
  accessor.write(filename, arr);
}

#endif