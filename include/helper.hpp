#ifndef include_helpers_hpp
#define include_helpers_hpp

#include <chrono>
#include <vector>
#include <fstream>
#include <string>
#include <json/json.h>

#include <casacore/casa/Arrays.h>
#include <casacore/coordinates/Coordinates/CoordinateUtil.h>
#include <casacore/coordinates/Coordinates/CoordinateSystem.h>

#include "CasaImageAccess.h"
#include "FitsImageAccess.h"

class Parameters
{
public:
  std::string inputImageType;
  std::string imageFilePath;
  std::string jsonFilePath;
  std::string outputImageType;
  std::string outputDirPath;

  Parameters() {}

  Parameters(std::string inputImageType, std::string &imageFilePath, std::string &jsonFilePath, std::string outputImageType, std::string &odp) : inputImageType(inputImageType), imageFilePath(imageFilePath), jsonFilePath(jsonFilePath), outputImageType(outputImageType), outputDirPath(odp)
  {
    if (outputDirPath.back() != '/')
    {
      outputDirPath.append("/");
    }
  }

};

boost::shared_ptr<askap::accessors::IImageAccess<casacore::Float>> generateAccessorFromImageType(const std::string &imageType)
{
  boost::shared_ptr<IImageAccess<>> result;
  if (imageType == "casa")
  {
    boost::shared_ptr<CasaImageAccess<casacore::Float>> iaCASA(new CasaImageAccess<casacore::Float>());
    // optional parameter setting may come here
    result = iaCASA;
  }
  else if (imageType == "fits")
  {
    boost::shared_ptr<FitsImageAccess> iaFITS(new FitsImageAccess());
    const bool fast = true;
    iaFITS->useFastAlloc(fast);
    result = iaFITS;
  }
  else
  {
    std::cerr << "include/helper.hpp:generateAccessorFromImageType(): Unsupported image type : " + imageType + " has been requested";
    exit(1);
  }
  return result;
}

std::vector<float> generateSequentialData(const std::vector<size_t> &naxis, float start)
{
  size_t totpix = 1;
  for (size_t i = 0; i < naxis.size(); i++)
    totpix *= naxis[i];

  std::vector<float> arr(totpix);

  for (size_t i = 0; i < arr.size(); i++)
  {
    // logic
    arr[i] = start + (float)i;
  }
  return arr;
}

std::vector<float> generateRandomData(const std::vector<size_t> &naxis, float range, float offset)
{
  size_t totpix = 1;
  for (size_t i = 0; i < naxis.size(); i++)
    totpix *= naxis[i];

  std::vector<float> arr(totpix);

  time_t seed = time(0);
  srand(seed);

  for (size_t i = 0; i < arr.size(); i++)
  {
    arr[i] = offset + range * (rand() / (float)RAND_MAX);
  }

  return arr;
}

void writeDataBinary(const std::vector<size_t> &naxis, const std::vector<float> &arr, const std::string &filename)
{
  std::ofstream writer;
  writer.open(filename);

  size_t naxes = naxis.size();
  writer.write((char *)&naxes, sizeof(size_t));

  for (size_t i = 0; i < naxes; i++)
  {
    writer.write((char *)&naxis[i], sizeof(size_t));
  }

  for (size_t i = 0; i < arr.size(); i++)
  {
    writer.write((char *)&arr[i], sizeof(float));
  }

  writer.close();
}

void writeData(const std::vector<size_t> &naxis, const std::vector<float> &inputArr, const std::string &filename, askap::accessors::IImageAccess<casacore::Float> &accessor)
{
  size_t naxes = naxis.size();

  if (naxes != 4)
  {
    std::cerr << "This application requires a 4D array.";
    exit(1);
  }

  casacore::IPosition arrSize(naxes);
  for (size_t i = 0; i < naxes; i++)
  {
    arrSize(i) = naxis[i];
  }

  casacore::Array<casacore::Float> arr(arrSize);

  for (size_t i = 0; i < inputArr.size(); i++)
  {
    casacore::IPosition currentPos = casacore::toIPositionInArray(i, arr.shape());
    arr(currentPos) = inputArr[i];
  }

  // create casa file
  accessor.create(filename, arr.shape(), casacore::CoordinateUtil::defaultCoords4D());

  // write the array
  accessor.write(filename, arr);
}

void readDataBinary(const std::string &binaryDataFilePath, const std::string &jsonFilePath)
{
  Json::Reader jsonReader; // for reading the data
  Json::Value root;        // for modifying and storing new values

  std::ifstream dataFile;
  dataFile.open(binaryDataFilePath);
  std::ifstream jsonFile;
  jsonFile.open(jsonFilePath);

  // check if there is any error is getting data from the json jsonFile
  if (!jsonReader.parse(jsonFile, root, false))
  {
    std::cerr << jsonReader.getFormattedErrorMessages();
    exit(1);
  }

  size_t naxes;
  dataFile.read((char *)&naxes, sizeof(size_t));

  // for current application, naxes must be 4
  if (naxes != 4)
  {
    std::cerr << "This application requires a 4D array. Please recreate array using array_creator.";
    exit(1);
  }

  std::vector<size_t> naxis(naxes);

  for (size_t i = 0; i < naxes; i++)
  {
    dataFile.read((char *)&naxis[i], sizeof(size_t));
  }

  // reading whole data
  casacore::IPosition arrSize(naxes, naxis[0], naxis[1], naxis[2], naxis[3]);
  casacore::Array<casacore::Float> arr(arrSize);

  float value;
  for (size_t i = 0; i < arr.size(); i++)
  {
    casacore::IPosition currentPos = casacore::toIPositionInArray(i, arr.shape());
    dataFile.read((char *)&value, sizeof(float));
    arr(currentPos) = value;
  }

  for (Json::Value::ArrayIndex i = 0; i != root.size(); i++)
  {
    int sourceID = root[i]["sourceID"].asInt();

    casacore::Vector<casacore::Int64> slicerBegin(naxes);
    casacore::Vector<casacore::Int64> slicerEnd(naxes);
    casacore::Vector<casacore::Int64> stride(naxes);
    casacore::Vector<casacore::Int64> length(naxes);

    for (Json::Value::ArrayIndex j = 0; j < naxes; j++)
    {
      slicerBegin(j) = root[i]["slicerBegin"][j].asInt64();
      slicerEnd(j) = root[i]["slicerEnd"][j].asInt64();
      stride(j) = root[i]["stride"][j].asInt64();
      length(j) = root[i]["length"][j].asInt64();
    }

    casacore::IPosition blc(slicerBegin);
    casacore::IPosition trc(slicerEnd);

    casacore::Slicer slicer = casacore::Slicer(blc, trc, casacore::Slicer::endIsLast);
    casacore::Array<casacore::Float> output = arr(slicer);
  }

  dataFile.close();
  jsonFile.close();
}

void extractSourcesWithSingleRead(Parameters &parameters)
{
  std::string imageFilePath = parameters.imageFilePath;
  std::string jsonFilePath = parameters.jsonFilePath;
  std::string outputDirPath = parameters.outputDirPath;

  boost::shared_ptr<askap::accessors::IImageAccess<casacore::Float>> inputAccessor = generateAccessorFromImageType(parameters.inputImageType);
  boost::shared_ptr<askap::accessors::IImageAccess<casacore::Float>> outputAccessor = generateAccessorFromImageType(parameters.outputImageType);

  Json::Reader jsonReader; // for reading the data
  Json::Value root;        // for modifying and storing new values

  std::ifstream jsonFile;
  jsonFile.open(jsonFilePath);
  // check if there is any error is getting data from the json jsonFile
  if (!jsonReader.parse(jsonFile, root, false))
  {
    std::cerr << jsonReader.getFormattedErrorMessages();
    exit(1);
  }

  // reading whole data
  casacore::Array<casacore::Float> arr = inputAccessor->read(imageFilePath);
  const int naxes = arr.ndim();
  if (naxes != 4)
  {
    std::cerr << "This application requires a 4D array. Please recreate array using array_creator.";
    exit(1);
  }

  for (Json::Value::ArrayIndex i = 0; i != root.size(); i++)
  {
    int sourceID = root[i]["sourceID"].asInt();

    casacore::Vector<casacore::Int64> slicerBegin(naxes);
    casacore::Vector<casacore::Int64> slicerEnd(naxes);
    casacore::Vector<casacore::Int64> stride(naxes);
    casacore::Vector<casacore::Int64> length(naxes);

    for (Json::Value::ArrayIndex j = 0; j < naxes; j++)
    {
      slicerBegin(j) = root[i]["slicerBegin"][j].asInt64();
      slicerEnd(j) = root[i]["slicerEnd"][j].asInt64();
      stride(j) = root[i]["stride"][j].asInt64();
      length(j) = root[i]["length"][j].asInt64();
    }

    casacore::IPosition blc(slicerBegin);
    casacore::IPosition trc(slicerEnd);

    casacore::Slicer slicer = casacore::Slicer(blc, trc, casacore::Slicer::endIsLast);
    casacore::Array<casacore::Float> output = arr(slicer);

    std::string outFileName = outputDirPath + "Image_" + std::to_string(sourceID);
    // create casa file
    outputAccessor->create(outFileName, output.shape(), casacore::CoordinateUtil::defaultCoords4D());

    // write the array
    outputAccessor->write(outFileName, output);
  }
  jsonFile.close();
}

void extractSourcesWithSlicedReads(Parameters &parameters)
{
  std::string imageFilePath = parameters.imageFilePath;
  std::string jsonFilePath = parameters.jsonFilePath;
  std::string outputDirPath = parameters.outputDirPath;

  boost::shared_ptr<askap::accessors::IImageAccess<casacore::Float>> inputAccessor = generateAccessorFromImageType(parameters.inputImageType);
  boost::shared_ptr<askap::accessors::IImageAccess<casacore::Float>> outputAccessor = generateAccessorFromImageType(parameters.outputImageType);

  Json::Reader jsonReader; // for reading the data
  Json::Value root;        // for modifying and storing new values

  std::ifstream jsonFile;
  jsonFile.open(jsonFilePath);
  // check if there is any error is getting data from the json jsonFile
  if (!jsonReader.parse(jsonFile, root, false))
  {
    std::cerr << jsonReader.getFormattedErrorMessages();
    exit(1);
  }

  // fixed
  int naxes = 4;

  for (Json::Value::ArrayIndex i = 0; i != root.size(); i++)
  {
    int sourceID = root[i]["sourceID"].asInt();

    casacore::Vector<casacore::Int64> slicerBegin(naxes);
    casacore::Vector<casacore::Int64> slicerEnd(naxes);
    casacore::Vector<casacore::Int64> stride(naxes);
    casacore::Vector<casacore::Int64> length(naxes);

    for (Json::Value::ArrayIndex j = 0; j < naxes; j++)
    {
      slicerBegin(j) = root[i]["slicerBegin"][j].asInt64();
      slicerEnd(j) = root[i]["slicerEnd"][j].asInt64();
      stride(j) = root[i]["stride"][j].asInt64();
      length(j) = root[i]["length"][j].asInt64();
    }

    casacore::IPosition blc(slicerBegin);
    casacore::IPosition trc(slicerEnd);

    casacore::Array<casacore::Float> output = inputAccessor->read(imageFilePath, blc, trc);

    std::string outFileName = outputDirPath + "Image_" + std::to_string(sourceID);
    // create casa file
    outputAccessor->create(outFileName, output.shape(), casacore::CoordinateUtil::defaultCoords4D());

    // write the array
    outputAccessor->write(outFileName, output);
  }

  jsonFile.close();
}

namespace chrono = std::chrono;

class Timer
{
private:
  chrono::time_point<chrono::high_resolution_clock> start;
  chrono::time_point<chrono::high_resolution_clock> end;

public:
  void start_timer() { this->start = chrono::high_resolution_clock::now(); }
  void stop_timer() { this->end = chrono::high_resolution_clock::now(); }

  auto time_elapsed()
  {
    auto time_taken = chrono::duration_cast<chrono::milliseconds>(this->end - this->start).count();

    return std::to_string(time_taken) + " ms";
  }
};

#endif