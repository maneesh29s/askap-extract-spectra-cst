#include <iostream>
#include <vector>
#include <fstream>
#include <json/json.h>
#include <string.h>

#include <casacore/casa/Arrays.h>

int main(int argc, char const *argv[])
{
    Json::Reader jsonReader; // for reading the data
    Json::Value root;        // for modifying and storing new values
    
    if (argc != 2)
    {
        std::cerr << "Usage: ./build/main.out <path to processed.json file>";
        exit(1);
    }

    std::ifstream dataFile;
    dataFile.open("data/test_array_data.dat");

    std::string filePath = argv[1];
    std::ifstream jsonFile;
    jsonFile.open(filePath);

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
    std::cout << "Dimesions are: ";
    for (size_t i = 0; i < naxes; i++)
    {
        dataFile.read((char *)&naxis[i], sizeof(size_t));
        std::cout << naxis[i] << " ";
    }
    std::cout << std::endl;

    // reading whole data
    casacore::IPosition arrSize(naxes, naxis[0], naxis[1], naxis[2], naxis[3]);
    casacore::Array<casacore::Float> arr(arrSize);

    float value;
    for (size_t row = 0; row < naxis[0]; row++)
    {
        for (size_t col = 0; col < naxis[1]; col++)
        {
            casacore::IPosition currentPos(naxes, row, col, 0, 0);
            dataFile.read((char *)&value, sizeof(float));
            arr(currentPos) = value;
        }
    }
    std::cout << "Input array elements: " << arr.size() << std::endl;

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

        // std::cout << output.size() << std::endl;
    }

    dataFile.close();
    jsonFile.close();

    return 0;
}
