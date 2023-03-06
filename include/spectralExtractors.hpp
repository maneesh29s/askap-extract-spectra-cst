
#ifndef spectral_extractors_hpp
#define spectral_extractors_hpp

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

void spectrumExtractionWithSlicedReads(Parameters &parameters) {
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

    std::string sourceID = root[i]["sourceID"].asString();
    std::string stokes = root[i]["stokes"].asString();

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

    // HARDCODING WARNING
    // initialising output array
    // length(2) = stokes / polar axis, length(3) = spatial / freq axis
    casacore::IPosition shape(4, 1, 1, length(2), length(3));
    casacore::Array<casacore::Float> itsArray =
        casacore::Array<casacore::Float>(shape, 0.0f);

    // HARDCODING WARNING
    // Since we are using only 1 stoke out of 4, we will run the same operation
    // 4 times to replicate real scenario
    for (auto k = 0; k < 4; k++) {
      casacore::Array<casacore::Float> subarray(inputAccessor->read(
          imageFilePath + "_" + std::to_string(k + 1), blc, trc));

      casacore::IPosition outBLC(itsArray.ndim(), 0),
          outTRC(itsArray.shape() - 1);

      casacore::Array<casacore::Float> sumarray =
          partialSums(subarray, casacore::IPosition(2, 0, 1));
      itsArray(outBLC, outTRC) =
          sumarray.reform(itsArray(outBLC, outTRC).shape());

      // TODO: Implement logic for mask also
    }

    if (parameters.outputImageType == "bp") {
      writeStokesToBp(sourceID, itsArray, io, writer);
    } else {
      std::string outFileName = outputDirPath + "Image_" + sourceID;
      // create file
      outputAccessor->create(outFileName, itsArray.shape(),
                             casacore::CoordinateUtil::defaultCoords4D());
      // write the array
      outputAccessor->write(outFileName, itsArray);
    }
  }

  jsonFile.close();
  if (parameters.outputImageType == "bp") {
    writer.Close();
  }
}

void spectrumExtractionWithSortedGroupedReads(Parameters &parameters) {
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

  // HARDCODING WARNING
  // reading data, from x=600 to x=1100`
  casacore::Vector<casacore::Int64> groupBegin{600, 0, 0, 0};
  casacore::Vector<casacore::Int64> groupEnd{1100, 5999, 0, 143};

  casacore::IPosition blc(groupBegin);
  casacore::IPosition trc(groupEnd);

  // HARDCODING WARNING
  // Reading from 4 images at same time in different variables
  casacore::Array<casacore::Float> Iarr(
      inputAccessor->read(imageFilePath + "_1", blc, trc));
  casacore::Array<casacore::Float> Qarr(
      inputAccessor->read(imageFilePath + "_2", blc, trc));
  casacore::Array<casacore::Float> Uarr(
      inputAccessor->read(imageFilePath + "_3", blc, trc));
  casacore::Array<casacore::Float> Varr(
      inputAccessor->read(imageFilePath + "_4", blc, trc));

  std::vector<casacore::Array<casacore::Float>> StokeArrays{Iarr, Qarr, Uarr,
                                                            Varr};

  for (Json::Value::ArrayIndex i = 0; i != root.size(); i++) {

    std::string sourceID = root[i]["sourceID"].asString();
    std::string stokes = root[i]["stokes"].asString();

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

    // HARDCODING
    // Shifting first element of slicers to adjust for indexing
    slicerBegin(0) = slicerBegin(0) - groupBegin(0);
    slicerEnd(0) = slicerEnd(0) - groupBegin(0);

    casacore::IPosition blc(slicerBegin);
    casacore::IPosition trc(slicerEnd);

    casacore::Slicer slicer =
        casacore::Slicer(blc, trc, casacore::Slicer::endIsLast);

    // HARDCODING WARNING
    // initialising output array
    // length(2) = stokes / polar axis, length(3) = spatial / freq axis
    casacore::IPosition shape(4, 1, 1, length(2), length(3));
    casacore::Array<casacore::Float> itsArray =
        casacore::Array<casacore::Float>(shape, 0.0f);

    // HARDCODING WARNING
    // Since we are using only 1 stoke out of 4, we will run the same operation
    // 4 times to replicate real scenario
    for (auto k = 0; k < 4; k++) {
      casacore::Array<casacore::Float> subarray = StokeArrays[k](slicer);

      casacore::IPosition outBLC(itsArray.ndim(), 0),
          outTRC(itsArray.shape() - 1);

      casacore::Array<casacore::Float> sumarray =
          partialSums(subarray, casacore::IPosition(2, 0, 1));
      itsArray(outBLC, outTRC) =
          sumarray.reform(itsArray(outBLC, outTRC).shape());

      // TODO: Implement logic for mask also
    }

    if (parameters.outputImageType == "bp") {
      writeStokesToBp(sourceID, itsArray, io, writer);
    } else {
      std::string outFileName = outputDirPath + "Image_" + sourceID;
      // create file
      outputAccessor->create(outFileName, itsArray.shape(),
                             casacore::CoordinateUtil::defaultCoords4D());
      // write the array
      outputAccessor->write(outFileName, itsArray);
    }
  }

  jsonFile.close();
  if (parameters.outputImageType == "bp") {
    writer.Close();
  }
}

#endif