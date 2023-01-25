#include <iostream>
#include <vector>
#include <fstream>
#include <json/json.h>

#include <casacore/casa/Arrays.h>

int main(int argc, char const *argv[])
{
    Json::Reader jsonReader; // for reading the data
    Json::Value root;        // for modifying and storing new values
    int naxes = 4;
    std::vector<size_t> naxis{12427, 12424, 1, 1};

    std::ifstream dataFile;
    dataFile.open("data/test_array_data.dat");
    std::ifstream jsonFile;
    jsonFile.open("data/selavy-process-24-processed.json");

    // check if there is any error is getting data from the json jsonFile
    if (!jsonReader.parse(jsonFile, root, false))
    {
        std::cerr << jsonReader.getFormattedErrorMessages();
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

        casacore::IPosition arrSize(4, naxis[0], naxis[1], naxis[2], naxis[3]);
        casacore::Array<casacore::Float> arr(arrSize);

        float value;
        for (size_t row = 0; row < naxis[0]; row++)
        {
            for (size_t col = 0; col < naxis[1]; col++)
            {
                casacore::IPosition currentPos(4, row, col, 0, 0);
                dataFile.read((char *)&value, sizeof(float));
                arr(currentPos) = value;
            }
        }

        casacore::IPosition blc(slicerBegin);
        casacore::IPosition trc(slicerEnd);

        casacore::Slicer slicer = casacore::Slicer(blc, trc, casacore::Slicer::endIsLast);

        casacore::Array<casacore::Float> output = arr(slicer);

        std::cout << arr.size() << std::endl;
        std::cout << arr.ndim() << std::endl;
        std::cout << output.size() << std::endl;
        std::cout << output.ndim() << std::endl;
    }

    dataFile.close();
    jsonFile.close();

    return 0;
}
