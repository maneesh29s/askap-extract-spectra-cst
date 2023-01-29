#include <iostream>
#include <vector>
#include <fstream>
#include <json/json.h>

#include <casacore/casa/Arrays.h>
#include <casacore/coordinates/Coordinates/CoordinateUtil.h>
#include <casacore/coordinates/Coordinates/CoordinateSystem.h>

#include "CasaImageAccess.h"
#include "helper.h"

static void generateSequentialData(const std::vector<size_t> &naxis, std::vector<float> &arr, float start)
{
    for (size_t i = 0; i < arr.size(); i++)
    {
        // logic
        arr[i] = start + (float)i;
    }
}

static void generateRandomData(const std::vector<size_t> &naxis, std::vector<float> &arr, float range, float offset)
{
    time_t seed = time(0);
    srand(seed);

    for (size_t i = 0; i < arr.size(); i++)
    {
        arr[i] = offset + range * (rand() / (float)RAND_MAX);
    }
}

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

    // this writes data in column-major format. Not as expected
    // for (size_t i = 0; i < inputArr.size(); i++)
    // {
    //     casacore::IPosition currentPos = casacore::toIPositionInArray(i,arr.shape());
    //     arr(currentPos) = inputArr[i];
    // }

    for (size_t row = 0; row < naxis[0]; row++)
    {
        for (size_t col = 0; col < naxis[1]; col++)
        {
            casacore::IPosition currentPos(naxes, row, col, 0, 0);
            arr(currentPos) = inputArr[row * naxis[1] + col];
        }
    }

    askap::accessors::CasaImageAccess<casacore::Float> accessor;

    // create casa fits file
    accessor.create("test/casa_test_image.FITS", arr.shape(), casacore::CoordinateUtil::defaultCoords4D());

    // write the array
    accessor.write("test/casa_test_image.FITS", arr);
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

    std::cout << "Original Array" << std::endl;
    float value;
    for (size_t row = 0; row < naxis[0]; row++)
    {
        for (size_t col = 0; col < naxis[1]; col++)
        {
            casacore::IPosition currentPos(naxes, row, col, 0, 0);
            dataFile.read((char *)&value, sizeof(float));
            std::cout << value << " ";
            arr(currentPos) = value;
        }
        std::cout << std::endl;
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

        std::cout << "Sliced array dimensions :" << std::endl;
        std::cout << "numelements : " << output.size() << std::endl;
        std::cout << "numdimensions : " << output.ndim() << std::endl;
        std::cout << "sourceID : " << sourceID << std::endl;
        std::cout << std::endl;

        std::cout << "Sliced Array" << std::endl;
        for (size_t row = 0; row < output.shape()(0); row++)
        {
            for (size_t col = 0; col < output.shape()(1); col++)
            {
                casacore::IPosition currentPos(naxes, row, col, 0, 0);
                std::cout << output(currentPos) << " ";
            }
            std::cout << std::endl;
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

    // reading whole data
    casacore::Array<casacore::Float> arr;

    askap::accessors::CasaImageAccess<casacore::Float> accessor;

    arr = accessor.read("test/casa_test_image.FITS");
    const int naxes = arr.ndim();
    const casacore::IPosition naxis = arr.shape();

    // check if there is any error is getting data from the json jsonFile
    if (!jsonReader.parse(jsonFile, root, false))
    {
        std::cerr << jsonReader.getFormattedErrorMessages();
        exit(1);
    }

    std::cout << "Original Array" << std::endl;
    float value;
    for (size_t row = 0; row < naxis(0); row++)
    {
        for (size_t col = 0; col < naxis(1); col++)
        {
            casacore::IPosition currentPos(4, row, col, 0, 0);

            std::cout << arr(currentPos) << " ";
        }
        std::cout << std::endl;
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

        std::cout << "Sliced array dimensions :" << std::endl;
        std::cout << "numelements : " << output.size() << std::endl;
        std::cout << "numdimensions : " << output.ndim() << std::endl;
        std::cout << "sourceID : " << sourceID << std::endl;
        std::cout << std::endl;

        std::cout << "Sliced Array" << std::endl;
        for (size_t row = 0; row < output.shape()(0); row++)
        {
            for (size_t col = 0; col < output.shape()(1); col++)
            {
                casacore::IPosition currentPos(4, row, col, 0, 0);
                std::cout << output(currentPos) << " ";
            }
            std::cout << std::endl;
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

    casacore::PagedImage<casacore::Float> img("test/casa_test_image.FITS");
    const int naxes = img.ndim();
    const casacore::IPosition naxis = img.shape();

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

        casacore::Array<casacore::Float> output = accessor.read("test/casa_test_image.FITS", blc, trc);

        std::cout << "Sliced array dimensions :" << std::endl;
        std::cout << "numelements : " << output.size() << std::endl;
        std::cout << "numdimensions : " << output.ndim() << std::endl;
        std::cout << "sourceID : " << sourceID << std::endl;
        std::cout << std::endl;

        std::cout << "Sliced Array" << std::endl;
        for (size_t row = 0; row < output.shape()(0); row++)
        {
            for (size_t col = 0; col < output.shape()(1); col++)
            {
                casacore::IPosition currentPos(4, row, col, 0, 0);
                std::cout << output(currentPos) << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }

    // dataFile.close();
    jsonFile.close();
}

int main(int argc, char const *argv[])
{
    // size_t naxes = 4;
    std::vector<size_t> naxis{10, 10, 1, 1};
    Timer timer;

    size_t totpix = 1;
    for (size_t i = 0; i < naxis.size(); i++) totpix *= naxis[i];

    std::vector<float> arr(totpix);

    generateSequentialData(naxis, arr, 100.0f);
    // generateRandomData(naxis, arr, 10.0f , -5.0f);

    writeDataBinary(naxis , arr);
    readDataBinary();

    timer.start_timer();
    writeDataCasa(naxis , arr);
    timer.stop_timer();

    std::cerr << "Time taken for writing to CASA image: " << timer.time_elapsed() << " us" << std::endl;

    timer.start_timer();
    readDataCasa();
    timer.stop_timer();

    std::cerr << "Time taken for reading whole data from CASA image: " << timer.time_elapsed() << " us" << std::endl;

    timer.start_timer();
    readDataSlicedCasa();
    timer.stop_timer();

    std::cerr << "Time taken for reading slice by slice from CASA image: " << timer.time_elapsed() << " us" << std::endl;

    return 0;
}
