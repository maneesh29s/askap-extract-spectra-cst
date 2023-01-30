#include <iostream>
#include <vector>
#include <fstream>
#include <json/json.h>

#include <casacore/casa/Arrays.h>
#include <casacore/coordinates/Coordinates/CoordinateUtil.h>
#include <casacore/coordinates/Coordinates/CoordinateSystem.h>

#include "CasaImageAccess.h"

#include "helper.hpp"

static void writeDataBinary(const std::vector<size_t> &naxis, const std::vector<float> &arr)
{
    std::ofstream writer;
    writer.open("test/test_array_data.dat");

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

static void writeDataCasa(const std::vector<size_t> &naxis, const std::vector<float> &inputArr)
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

    askap::accessors::CasaImageAccess<casacore::Float> accessor;

    // create casa fits file
    accessor.create("test/test_casa_array", arr.shape(), casacore::CoordinateUtil::defaultCoords4D());

    // write the array
    accessor.write("test/test_casa_array", arr);
}

static void readDataBinary()
{
    Json::Reader jsonReader; // for reading the data
    Json::Value root;        // for modifying and storing new values

    std::ifstream dataFile;
    dataFile.open("test/test_array_data.dat");
    std::ifstream jsonFile;
    jsonFile.open("test/test_log.json");

    // check if there is any error is getting data from the json jsonFile
    if (!jsonReader.parse(jsonFile, root, false))
    {
        std::cerr << jsonReader.getFormattedErrorMessages();
        exit(1);
    }

    std::cout << "readDataBinary()" << std::endl << std::endl;

    size_t naxes;
    dataFile.read((char *)&naxes, sizeof(size_t));

    // for current application, naxes must be 4
    if (naxes != 4)
    {
        std::cerr << "This application requires a 4D array";
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

    std::cout << "Original Array" << std::endl;
    float value;
    for (size_t i = 0; i < arr.size(); i++)
    {
        casacore::IPosition currentPos = casacore::toIPositionInArray(i, arr.shape());
        dataFile.read((char *)&value, sizeof(float));
        std::cout << value << " ";
        arr(currentPos) = value;
        if ((i + 1) % naxis[1] == 0)
        {
            std::cout << std::endl;
        }
    }
    std::cout << std::endl;

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

        std::cout << "Sliced array " << i << " dimensions :" << std::endl;
        std::cout << "numelements : " << output.size() << std::endl;
        std::cout << "numdimensions : " << output.ndim() << std::endl;
        std::cout << "sourceID : " << sourceID << std::endl;

        std::cout << "Sliced Array " << i << std::endl;
        for (size_t j = 0; j < output.size(); j++)
        {
            casacore::IPosition currentPos = casacore::toIPositionInArray(j, output.shape());
            std::cout << output(currentPos) << " ";
            if ((j + 1) % output.shape()[1] == 0)
            {
                std::cout << std::endl;
            }
        }
        std::cout << std::endl;
    }

    dataFile.close();
    jsonFile.close();
}

static void readDataCasa()
{
    Json::Reader jsonReader; // for reading the data
    Json::Value root;        // for modifying and storing new values

    std::ifstream jsonFile;
    jsonFile.open("test/test_log.json");
    // check if there is any error is getting data from the json jsonFile
    if (!jsonReader.parse(jsonFile, root, false))
    {
        std::cerr << jsonReader.getFormattedErrorMessages();
        exit(1);
    }

    // reading whole data
    casacore::Array<casacore::Float> arr;
    askap::accessors::CasaImageAccess<casacore::Float> accessor;
    arr = accessor.read("test/test_casa_array");

    const int naxes = arr.ndim();
    if (naxes != 4)
    {
        std::cerr << "This application requires a 4D array";
        exit(1);
    }
    const casacore::IPosition naxis = arr.shape();

    std::cout << "------------------------------------------" << std::endl;
    std::cout << "readDataCasa() " << std::endl
              << std::endl;

    std::cout << "Original CASA Array" << std::endl;
    for (size_t i = 0; i < arr.size(); i++)
    {
        casacore::IPosition currentPos = casacore::toIPositionInArray(i, arr.shape());
        std::cout << arr(currentPos) << " ";
        if ((i + 1) % naxis[1] == 0)
        {
            std::cout << std::endl;
        }
    }
    std::cout << std::endl;

    std::vector<float> vec;
    arr.tovector(vec);
    std::cout << "Vectorised Array" << std::endl;
    for (size_t i = 0; i < vec.size(); i++)
    {
        std::cout << vec[i] << " ";
        if ((i + 1) % naxis[1] == 0)
        {
            std::cout << std::endl;
        }
    }
    std::cout << std::endl;

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

        std::cout << "Sliced array " << i << " dimensions :" << std::endl;
        std::cout << "numelements : " << output.size() << std::endl;
        std::cout << "numdimensions : " << output.ndim() << std::endl;
        std::cout << "sourceID : " << sourceID << std::endl;

        std::cout << "Sliced Array " << i << std::endl;
        for (size_t j = 0; j < output.size(); j++)
        {
            casacore::IPosition currentPos = casacore::toIPositionInArray(j, output.shape());
            std::cout << output(currentPos) << " ";
            if ((j + 1) % output.shape()[1] == 0)
            {
                std::cout << std::endl;
            }
        }
        std::cout << std::endl;
    }
    jsonFile.close();
}

static void readDataSlicedCasa()
{
    Json::Reader jsonReader; // for reading the data
    Json::Value root;        // for modifying and storing new values

    std::ifstream jsonFile;
    jsonFile.open("test/test_log.json");

    // check if there is any error is getting data from the json jsonFile
    if (!jsonReader.parse(jsonFile, root, false))
    {
        std::cerr << jsonReader.getFormattedErrorMessages();
        exit(1);
    }

    askap::accessors::CasaImageAccess<casacore::Float> accessor;

    casacore::PagedImage<casacore::Float> img("test/test_casa_array");
    const int naxes = img.ndim();
    if (naxes != 4)
    {
        std::cerr << "This application requires a 4D array";
        exit(1);
    }
    const casacore::IPosition naxis = img.shape();

    std::cout << "------------------------------------------" << std::endl;
    std::cout << "ReadDataSlicedCasa() " << std::endl
              << std::endl;

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

        casacore::Array<casacore::Float> output = accessor.read("test/test_casa_array", blc, trc);

        std::cout << "Sliced array " << i << " dimensions :" << std::endl;
        std::cout << "numelements : " << output.size() << std::endl;
        std::cout << "numdimensions : " << output.ndim() << std::endl;
        std::cout << "sourceID : " << sourceID << std::endl;

        std::cout << "Sliced Array " << i << std::endl;
        for (size_t j = 0; j < output.size(); j++)
        {
            casacore::IPosition currentPos = casacore::toIPositionInArray(j, output.shape());
            std::cout << output(currentPos) << " ";
            if ((j + 1) % output.shape()[1] == 0)
            {
                std::cout << std::endl;
            }
        }
        std::cout << std::endl;
    }

    jsonFile.close();
}

int main(int argc, char const *argv[])
{
    // size_t naxes = 4;
    std::vector<size_t> naxis{9, 12, 1, 1};

    std::vector<float> arr = generateSequentialData(naxis, 100.0f);
    // std::vector<float> arr = generateRandomData(naxis, 10.0f , -5.0f);

    writeDataBinary(naxis, arr);
    readDataBinary();

    writeDataCasa(naxis, arr);
    readDataCasa();
    readDataSlicedCasa();

    return 0;
}
