
#ifndef cubelet_extractors_hpp
#define cubelet_extractors_hpp

#include <json/json.h>
#include <string>
#include <vector>

#include <adios2.h>
#include <casacore/casa/Arrays.h>
#include <casacore/coordinates/Coordinates/CoordinateSystem.h>
#include <casacore/coordinates/Coordinates/CoordinateUtil.h>

#include "CasaImageAccess.h"
#include "FitsImageAccess.h"
#include "helper.hpp"

void cubeletExtractionWithSingleRead(Parameters &parameters) {
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
      writeToBp(std::to_string(sourceID), output, io, writer);
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

void cubeletExtractionWithSlicedReads(Parameters &parameters) {
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
      writeToBp(std::to_string(sourceID), output, io, writer);
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

#endif