#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#include <json/json.h>

class ImageSource
{
public:
    int64_t sourceID;
    std::vector<int64_t> slicerBegin;
    std::vector<int64_t> slicerEnd;
    std::vector<int64_t> stride;
    std::vector<int64_t> length;

    ImageSource(){};

    ImageSource(int64_t sid, std::vector<int64_t> sb, std::vector<int64_t> se, std::vector<int64_t> st, std::vector<int64_t> len) : sourceID(sid), slicerBegin(sb), slicerEnd(se), stride(st), length(len){};

    bool operator<(const ImageSource &str) const
    {
        for (size_t i = 0; i < slicerBegin.size(); i++)
        {
            if (slicerBegin[i] < str.slicerBegin[i])
            {
                return true;
            }
            if (slicerBegin[i] == str.slicerBegin[i])
            {
                continue;
            }
            return false;
        }
        return false;
    }
};

// testing of jsoncpp library to read and write to json
int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: ./build/json_sourter.out <input_json_file>";
        exit(1);
    }

    std::string inputFilePath = argv[1];
    std::string outputFilePath = inputFilePath.substr(0, inputFilePath.size() - 5) + "-sorted.json";


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

    std::vector<ImageSource> sourceList(root.size());

    for (Json::Value::ArrayIndex i = 0; i != root.size(); i++)
    {
        int64_t sourceID = root[i]["sourceID"].asInt();

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

        sourceList[i] = ImageSource(sourceID, slicerBegin, slicerEnd, stride, length);
    }

    std::sort(sourceList.begin(), sourceList.end());

    // writing to a new array
    Json::Value writerRoot(Json::arrayValue);

    for (size_t i = 0; i < sourceList.size(); i++)
    {
        Json::Value source;

        Json::Value sourceID(sourceList[i].sourceID);
        source["sourceID"] = sourceID;

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