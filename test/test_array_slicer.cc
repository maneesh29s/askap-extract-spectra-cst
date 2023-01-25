#include <iostream>
#include <vector>
#include <fstream>
#include <json/json.h>

#include <casacore/casa/Arrays.h>

int main(int argc, char const *argv[])
{
    // --------------------------- Creating array data file -----------------------
    std::ofstream writer;
    writer.open("test/test_array_data.dat");

    int naxes = 4;
    std::vector<size_t> naxis{10, 10, 1, 1};

    size_t totpix = 1;
    for (int i = 0; i < naxes; i++)
    {
        totpix *= naxis[i];
    }

    time_t seed = time(0);
    std::cerr << "Seed: " << seed << std::endl;
    srand(seed);

    std::cerr << "Array dim : " << totpix << std::endl;

    float offset = -5.0f;
    float range = 5.0f;
    float temp;
    for (size_t i = 0; i < totpix; i++)
    {
        temp = i + 100;

        writer.write((char *)&temp, sizeof(float));
    }

    std::cerr << "Array creation done" << std::endl;

    writer.close();

    // ---------------------- Reading the data file and a slice -----------------------

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

    // reading whole data
    casacore::IPosition arrSize(4, naxis[0], naxis[1], naxis[2], naxis[3]);
    casacore::Array<casacore::Float> arr(arrSize);

    std::cout << "Original Array" << std::endl;
    float value;
    for (size_t row = 0; row < naxis[0]; row++)
    {
        for (size_t col = 0; col < naxis[1]; col++)
        {
            casacore::IPosition currentPos(4, row, col, 0, 0);
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
        for (size_t row = 0; row < length(0); row++)
        {
            for (size_t col = 0; col < length(1); col++)
            {
                casacore::IPosition currentPos(4, row, col, 0, 0);
                std::cout << output(currentPos) << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }

    dataFile.close();
    jsonFile.close();

    return 0;
}
