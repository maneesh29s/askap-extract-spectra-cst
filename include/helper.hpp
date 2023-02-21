#ifndef include_helpers_hpp
#define include_helpers_hpp

#include <chrono>
#include <fstream>
#include <json/json.h>
#include <string>
#include <vector>

#include <adios2.h>
#include <casacore/casa/Arrays.h>
#include <casacore/coordinates/Coordinates/CoordinateSystem.h>
#include <casacore/coordinates/Coordinates/CoordinateUtil.h>

#include "CasaImageAccess.h"
#include "FitsImageAccess.h"

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

void writeDataBinary(const std::vector<size_t> &naxis,
                     const std::vector<float> &arr,
                     const std::string &filename) {
  std::ofstream writer;
  writer.open(filename);

  size_t naxes = naxis.size();
  writer.write((char *)&naxes, sizeof(size_t));

  for (size_t i = 0; i < naxes; i++) {
    writer.write((char *)&naxis[i], sizeof(size_t));
  }

  for (size_t i = 0; i < arr.size(); i++) {
    writer.write((char *)&arr[i], sizeof(float));
  }

  writer.close();
}

void writeData(const std::vector<size_t> &naxis,
               const std::vector<float> &inputArr, const std::string &filename,
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

void readDataBinary(const std::string &binaryDataFilePath,
                    const std::string &jsonFilePath) {
  Json::Reader jsonReader; // for reading the data
  Json::Value root;        // for modifying and storing new values

  std::ifstream dataFile;
  dataFile.open(binaryDataFilePath);
  std::ifstream jsonFile;
  jsonFile.open(jsonFilePath);

  if (!jsonReader.parse(jsonFile, root, false)) {
    std::cerr << jsonReader.getFormattedErrorMessages();
    exit(1);
  }

  size_t naxes;
  dataFile.read((char *)&naxes, sizeof(size_t));

  // for current application, naxes must be 4
  if (naxes != 4) {
    std::cerr << "This application requires a 4D array. Please recreate array "
                 "using array_creator.";
    exit(1);
  }

  std::vector<size_t> naxis(naxes);

  for (size_t i = 0; i < naxes; i++) {
    dataFile.read((char *)&naxis[i], sizeof(size_t));
  }

  // reading whole data
  casacore::IPosition arrSize(naxes, naxis[0], naxis[1], naxis[2], naxis[3]);
  casacore::Array<casacore::Float> arr(arrSize);

  float value;
  for (size_t i = 0; i < arr.size(); i++) {
    casacore::IPosition currentPos =
        casacore::toIPositionInArray(i, arr.shape());
    dataFile.read((char *)&value, sizeof(float));
    arr(currentPos) = value;
  }

  for (Json::Value::ArrayIndex i = 0; i != root.size(); i++) {
    int sourceID = root[i]["sourceID"].asInt();

    casacore::Vector<casacore::Int64> slicerBegin(naxes);
    casacore::Vector<casacore::Int64> slicerEnd(naxes);
    casacore::Vector<casacore::Int64> stride(naxes);
    casacore::Vector<casacore::Int64> length(naxes);

    for (Json::Value::ArrayIndex j = 0; j < naxes; j++) {
      slicerBegin(j) = root[i]["slicerBegin"][j].asInt64();
      slicerEnd(j) = root[i]["slicerEnd"][j].asInt64();
      stride(j) = root[i]["stride"][j].asInt64();
      length(j) = root[i]["length"][j].asInt64();
    }

    casacore::IPosition blc(slicerBegin);
    casacore::IPosition trc(slicerEnd);

    casacore::Slicer slicer =
        casacore::Slicer(blc, trc, casacore::Slicer::endIsLast);
    casacore::Array<casacore::Float> output = arr(slicer);
  }

  dataFile.close();
  jsonFile.close();
}

void extractSourcesWithSingleRead(Parameters &parameters) {
  adios2::ADIOS adios;
  adios2::IO io;
  adios2::Engine writer;

  boost::shared_ptr<askap::accessors::IImageAccess<casacore::Float>>
      inputAccessor, outputAccessor;

  Json::Reader jsonReader; // for reading the data
  Json::Value root;        // for modifying and storing new values

  std::string imageFilePath = parameters.imageFilePath;
  std::string jsonFilePath = parameters.jsonFilePath;
  std::string outputDirPath = parameters.outputDirPath;

  inputAccessor = generateAccessorFromImageType(parameters.inputImageType);

  if (parameters.outputImageType == "bp") {
    io = adios.DeclareIO("imstat_adios_reader");
    writer = io.Open(outputDirPath.substr(0, outputDirPath.size() - 1) + ".bp",
                     adios2::Mode::Write);
  } else {
    outputAccessor = generateAccessorFromImageType(parameters.outputImageType);
  }

  std::ifstream jsonFile;
  jsonFile.open(jsonFilePath);
  if (!jsonReader.parse(jsonFile, root, false)) {
    std::cerr << jsonReader.getFormattedErrorMessages();
    exit(1);
  }

  // reading whole data
  casacore::Array<casacore::Float> arr = inputAccessor->read(imageFilePath);
  const int naxes = arr.ndim();
  if (naxes != 4) {
    std::cerr << "This application requires a 4D array. Please recreate array "
                 "using array_creator.";
    exit(1);
  }

  for (Json::Value::ArrayIndex i = 0; i != root.size(); i++) {
    int sourceID = root[i]["sourceID"].asInt();

    casacore::Vector<casacore::Int64> slicerBegin(naxes);
    casacore::Vector<casacore::Int64> slicerEnd(naxes);
    casacore::Vector<casacore::Int64> stride(naxes);
    casacore::Vector<casacore::Int64> length(naxes);

    for (Json::Value::ArrayIndex j = 0; j < naxes; j++) {
      slicerBegin(j) = root[i]["slicerBegin"][j].asInt64();
      slicerEnd(j) = root[i]["slicerEnd"][j].asInt64();
      stride(j) = root[i]["stride"][j].asInt64();
      length(j) = root[i]["length"][j].asInt64();
    }

    casacore::IPosition blc(slicerBegin);
    casacore::IPosition trc(slicerEnd);

    casacore::Slicer slicer =
        casacore::Slicer(blc, trc, casacore::Slicer::endIsLast);
    casacore::Array<casacore::Float> output = arr(slicer);

    if (parameters.outputImageType == "bp") {
      writeToBp(sourceID, output, io, writer);
    } else {
      std::string outFileName =
          outputDirPath + "Image_" + std::to_string(sourceID);
      // create file
      outputAccessor->create(outFileName, output.shape(),
                             casacore::CoordinateUtil::defaultCoords4D());
      // write the array
      outputAccessor->write(outFileName, output);
    }
  }

  jsonFile.close();
  if (parameters.outputImageType == "bp") {
    writer.Close();
  }
}

void extractSourcesWithSlicedReads(Parameters &parameters) {
  // constant
  const int NAXES = 4;

  adios2::ADIOS adios;
  adios2::IO io;
  adios2::Engine writer;

  boost::shared_ptr<askap::accessors::IImageAccess<casacore::Float>>
      inputAccessor, outputAccessor;

  Json::Reader jsonReader; // for reading the data
  Json::Value root;        // for modifying and storing new values

  std::string imageFilePath = parameters.imageFilePath;
  std::string jsonFilePath = parameters.jsonFilePath;
  std::string outputDirPath = parameters.outputDirPath;

  inputAccessor = generateAccessorFromImageType(parameters.inputImageType);

  if (parameters.outputImageType == "bp") {
    io = adios.DeclareIO("imstat_adios_reader");
    writer = io.Open(outputDirPath.substr(0, outputDirPath.size() - 1) + ".bp",
                     adios2::Mode::Write);
  } else {
    outputAccessor = generateAccessorFromImageType(parameters.outputImageType);
  }

  std::ifstream jsonFile;
  jsonFile.open(jsonFilePath);
  if (!jsonReader.parse(jsonFile, root, false)) {
    std::cerr << jsonReader.getFormattedErrorMessages();
    exit(1);
  }

  for (Json::Value::ArrayIndex i = 0; i != root.size(); i++) {
    int sourceID = root[i]["sourceID"].asInt();

    casacore::Vector<casacore::Int64> slicerBegin(NAXES);
    casacore::Vector<casacore::Int64> slicerEnd(NAXES);
    casacore::Vector<casacore::Int64> stride(NAXES);
    casacore::Vector<casacore::Int64> length(NAXES);

    for (Json::Value::ArrayIndex j = 0; j < NAXES; j++) {
      slicerBegin(j) = root[i]["slicerBegin"][j].asInt64();
      slicerEnd(j) = root[i]["slicerEnd"][j].asInt64();
      stride(j) = root[i]["stride"][j].asInt64();
      length(j) = root[i]["length"][j].asInt64();
    }

    casacore::IPosition blc(slicerBegin);
    casacore::IPosition trc(slicerEnd);

    casacore::Array<casacore::Float> output =
        inputAccessor->read(imageFilePath, blc, trc);

    if (parameters.outputImageType == "bp") {
      writeToBp(sourceID, output, io, writer);
    } else {
      std::string outFileName =
          outputDirPath + "Image_" + std::to_string(sourceID);
      // create file
      outputAccessor->create(outFileName, output.shape(),
                             casacore::CoordinateUtil::defaultCoords4D());
      // write the array
      outputAccessor->write(outFileName, output);
    }
  }

  jsonFile.close();
  if (parameters.outputImageType == "bp") {
    writer.Close();
  }
}

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

#endif