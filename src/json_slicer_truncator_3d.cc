#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#include <json/json.h>
#include "helper.hpp"

// testing of jsoncpp library to read and write to json
int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: ./build/json_slicer_truncator_3d.out <input_json_file>";
        exit(1);
    }

    std::string inputFilePath = argv[1];
    std::string outputFilePath = inputFilePath.substr(0, inputFilePath.size() - 5) + "-slicer-truncatedBy2.json";

    Json::Value root;          // for modifying and storing new values
    Json::Reader reader;       // for reading the data
    Json::StyledWriter writer; // for writing in json files

    size_t naxes = 4;

    // opening inputFile using fstream
    std::ifstream inputFile;
    inputFile.open(inputFilePath);

    std::ofstream outputFile;
    outputFile.open(outputFilePath);

    // check if there is any error is getting data from the json inputFile
    if (!reader.parse(inputFile, root, false))
    {
        std::cout << reader.getFormattedErrorMessages();
        exit(1);
    }

    std::vector<SpectralImageSource> sourceList(root.size());

    for (Json::Value::ArrayIndex i = 0; i != root.size(); i++)
    {
        std::string sourceID = root[i]["sourceID"].asString();
        std::string stokes = root[i]["stokes"].asString(); 

        std::vector<int64_t> slicerBegin(naxes);
        std::vector<int64_t> slicerEnd(naxes);
        std::vector<int64_t> stride(naxes);
        std::vector<int64_t> length(naxes);


        for (Json::Value::ArrayIndex j = 0; j < naxes; j++)
        {
            slicerBegin[j] = root[i]["slicerBegin"][j].asInt64();
            slicerEnd[j] = root[i]["slicerEnd"][j].asInt64();
            stride[j] = root[i]["stride"][j].asInt64();
            length[j] = root[i]["length"][j].asInt64();
        }

        sourceList[i] = SpectralImageSource(sourceID, slicerBegin, slicerEnd, stride, length, stokes);
    }

    // Truncation of slicer
    for (size_t i = 0 ; i < sourceList.size(); i++) {
        for (int j = 0; j < sourceList[i].slicerBegin.size() ; j++) {
            // Dividing by 2, because test cube has almost half the dimensions of original
            sourceList[i].slicerBegin[j] /= 2; 
            sourceList[i].slicerEnd[j] = sourceList[i].slicerBegin[j] + sourceList[i].length[j] - 1;
        }
    }


    Json::Value writerRoot(Json::arrayValue);

    for (size_t i = 0; i < sourceList.size(); i++)
    {
        Json::Value source;

        Json::Value sourceID(sourceList[i].sourceID);
        source["sourceID"] = sourceID;

        Json::Value stokes(sourceList[i].stokes);
        source["stokes"] = stokes;

        Json::Value slicerBegin(Json::arrayValue);
        Json::Value slicerEnd(Json::arrayValue);
        Json::Value stride(Json::arrayValue);
        Json::Value length(Json::arrayValue);

        for (Json::Value::ArrayIndex j = 0; j < naxes; j++)
        {
            slicerBegin.append(Json::Value(sourceList[i].slicerBegin[j]));
            slicerEnd.append(Json::Value(sourceList[i].slicerEnd[j]));
            stride.append(Json::Value(sourceList[i].stride[j]));
            length.append(Json::Value(sourceList[i].length[j]));
        }

        source["slicerBegin"] = slicerBegin;
        source["slicerEnd"] = slicerEnd;
        source["stride"] = stride;
        source["length"] = length;

        writerRoot.append(source);
    }

    outputFile << writer.write(writerRoot);

    inputFile.close();
    outputFile.close();

    return 0;
}